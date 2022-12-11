/* reacts on the positive impulse from a OR-WE-521 
 * an impulse is sent for each watt-hour that was measured, 
 * meaning 1000 imp sums up to 1 kW/h
 * 
 * impulse is expected on SIGNAL_PIN
 * signal has to been validated (stable over MIN_SIGNAL_PERIOD milisecs)
 * to exclude noise
 * 
 * next impulse the mean power can be calculated as 3600 / secs
 * 
 * note: all data in storage are stored in deci-Wh (means a 10 = 1Wh)
 * 
 */

#include "config.h"
#define VERSION "v1.1/" __DATE__


#define ORWE_PULSE_PER_KWH 1000
#define ORWE_DECIWH_PER_PULSE ( 10000 / ORWE_PULSE_PER_KWH )


unsigned long power;
int temp = 0;
byte displayMode = 0; 
long resetDisplayModeSeconds = 0;
byte actualMonth = 0;

// --- liquid crystal display driver from LiquidCrystal.h or LiquidCrystal_SR2W.h ---------------------------------------------

#ifdef USE_STANDARD_LCD_LIBRARY
  // using the standard LCD Library
  #include <LiquidCrystal.h>
  //            lcd(rs, e, d4, d5, d6, d7); backlight: pin D6
  LiquidCrystal lcd(LCD_RS,  LCD_E,   LCD_D4, LCD_D5, LCD_D6, LCD_D7);
  
  void setBacklight(byte state) {
    digitalWrite(6, state);
  }
#else
  // using the latchregister
  #include <LiquidCrystal_SR2W.h>
  LiquidCrystal_SR2W lcd(LCD_DATA, LCD_CLOCK, POSITIVE);

  void setBacklight(byte state) {
    lcd.setBacklight(state);
  }
#endif // USE_STANDARD_LCD_LIBRARY


// --- keyboard driver from AnalogKbd.cpp ---------------------------------------------
#include<AnalogKbd.h>

AnalogKbd kbd(PIN_ANALOG_KBD, KBD_NR_OF_KEYS);
byte kbdValue = 255; //the value that is read from keyboard 255 is neutral value


// --- timer helpers from TimeLoop.cpp ---------------------------------------------
#include<TimeLoop.h>
TimeLoop tmh(1);

// ---- string helper   ---------------------------------------------------------------  
String leftFill(String a, byte wantedLen, String fillLetter)
{
    // fills a string with letters from left side that the resulting length is reached
    while (a.length() < wantedLen)
    {
        a = fillLetter + a;
    }
    return a;
}



// EEPROM ------------------------------------------------------------------
#include <EEPROM.h>

#define EE_OFFSET 0  // start adress for the EEPROM data area
#define EE_OFFSET_DOY 128 //start adress for the 367 bytes where the daily revenue for all Days of Year is stored

// --- Variables that will be stored in eeprom persistent 
struct {
    long secondsCounter = 70000;
    long dayCounter = 1100;
    unsigned long totalWh = 591300;
    unsigned long daysWh[7] = {7400, 16100, 15900, 18700, 1700, 100, 7500}; // monday = 0 
    unsigned long monthsWh[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 223000, 278100 ,71500}; // jan = 1, dec = 12
    int version = 1;
} storage;

void loadEEprom(bool reset, int offset) {
  lcd.setCursor(0,1);
  if (!reset) {
    EEPROM.get(offset, storage);
    lcd.print(F("data loaded.     "));
  }
  else {
    // load values from default above.
    lcd.print(F("EEPROM reset.     "));
    delay(500);
    // comment in to reset the day of year values 
    /*for (int i=0; i<367; i++) {
      updateDayOfYearTotal(i, 0);
    }*/
    }
  
  
  tmh.setSecondsCounter(storage.secondsCounter);
  tmh.setDayCounter(storage.dayCounter);
  
  delay(800);
  
}

void storeEEprom(int offset) {
  storage.secondsCounter = tmh.getSecondsCounter();
  storage.dayCounter = tmh.getDayCounter();
  EEPROM.put(offset, storage);
  lcd.setCursor(0,1);
  lcd.print(F("data saved. "));
  delay(500);
}

// --------  array of 367 values in eeprom for each day of year that holds 
//           the day sumary (in hecto Watt hours) -----
//           i=1 is first of january

void updateDayOfYearTotal(int dayOfYear, byte value) {
  EEPROM.update(EE_OFFSET_DOY + dayOfYear, value);
}


byte getDayOfYearTotal(int dayOfYear) {
  byte value;
  EEPROM.get(EE_OFFSET_DOY + dayOfYear, value);
  return value;
}


// ------------------------------------------------------------------------------------------



// Initialization ---------------------------------------------------------------------------

void setup()
{
  
  displayMode = 0; // 0 means actual day.
  resetDisplayModeSeconds = 0;

  lcd.begin(16,2);               // initialize the lcd
  lcd.home();                   // go home  
  lcd.print(F(" # OR-WE-521 #")); 
  delay(500);
   
  // if no key is pressed on start, read values from eeprom, else reset
  bool doReset = false;
  if (analogRead(PIN_ANALOG_KBD) > 100) {
    lcd.setCursor(0,0);
    lcd.print(F("ovewrite EEPROM?"));
    lcd.setCursor(2,1);
    lcd.print(F("yes --> Enter"));
    byte kbdResult = kbd.wait_till_read();
    doReset = bool(kbdResult == 0);
  }

  loadEEprom( doReset, EE_OFFSET );
  
  setBacklight(1);

  lcd.setCursor(0,1);
  lcd.print(VERSION);
  delay(2000);
  lcd.home();                   // go home  
  lcd.print(F("deciWh/pulse "));
  lcd.print(String(ORWE_DECIWH_PER_PULSE));
  delay(2000);
   
  lcd.noCursor();
  lcd.clear();

  // pin for signal from OR-WE-521 
  pinMode(SIGNAL_PIN, INPUT);
  pinMode(13, OUTPUT);
  
  // serial device for debugging purpose!
  Serial.begin(9600);
  while (!Serial)
  {
      ; // wait for serial Pin to connect. otherwise reset if serial console is started :-/
  }

}


// Mainloop ---------------------------------------------------------------------------
unsigned long lastMeasurement = 0;

void loop(){
    actualMonth = tmh.getMonth(0);

    // get the milliseconds between last and actual signal.
    // if a result is valid, a value > 0 is passed back
    
    unsigned long timeDeltaMillis = getStableSignalDelta(SIGNAL_PIN);
    
    if ( timeDeltaMillis > 0 ) {
      // one impulse received means 1 Wh was comsumed. time passed since last impulse --> power
      
      PORTB |=  B00100000; //set pin13 to HIGH (for a long blink)
      
      storage.daysWh[tmh.getDayOfWeek(0)] += ORWE_DECIWH_PER_PULSE;
      storage.monthsWh[tmh.getMonth(0)] += ORWE_DECIWH_PER_PULSE;
      storage.totalWh += ORWE_DECIWH_PER_PULSE;
      
      power = 360000 * ORWE_DECIWH_PER_PULSE / timeDeltaMillis; 
      lastMeasurement = millis();
      delay(10);

    }
    
   
    byte timeState = tmh.actualize(); // update the timeloop handler. state: 0: nothing; 1: 100ms passed; 2: 1s passed; 3: midnight

    if ( timeState >= 1 ) {
      // --- 100ms has passed ---- this part needs 4ms for calculation... 
 
      if ( timeState >= 2 ) {
        // --- 1 sec has passed ---- 

        // set power=0 if for 12minutes no impulse was received.
        PORTB |=  B00100000; //set pin13 to HIGH, give led a quick blink each second     
        unsigned long actMillis = millis();
        unsigned long passedMillis = actMillis - lastMeasurement;
        if ( passedMillis > 720000 and power > 0){
          // set power to zero after 12 minutes without a signal
          timeDeltaMillis = passedMillis;
          lastMeasurement = actMillis; 
          power = 0; 
        }
        
        delayMicroseconds(50);
        PORTB &= ~B00100000; //set pin13 to LOW 
        
        temp = read_pt1000(analogRead(ANALOG_TEMPSENSOR_PIN));
        }     

      if ( timeState >= 3 ) {
        // ----  action at midnight: fill the history --------------------------

        if ( actualMonth != tmh.getMonth(0) ) {
          // new month has started... so reset the actual counter 
          storage.monthsWh[tmh.getMonth(0)] = 0;
        }
        //reset the new wh counter for the new day
        storage.daysWh[tmh.getDayOfWeek(0)] = 0; 

        storeEEprom(EE_OFFSET);       

        // finally fill the value from yesterday in the dayOfYear array in EEPROM
        byte yesterdayTotal_hWh = byte(storage.daysWh[tmh.getDayOfWeek(-1)] / 1000);
        updateDayOfYearTotal(tmh.getDayOfYear(-1), yesterdayTotal_hWh);
        
      }       
      
      PORTB &= ~B00100000; //set pin13 to LOW 
      
      float kwh = storage.daysWh[tmh.getDayOfWeek(0)] / 10000.0;


      // ---- refresh display ----------------------------------------------------------------------------

      
      
      if ( displayMode == 0 ) {
        // -- display normal information each second
          lcd.setCursor(0,0);
          lcd.print(leftFill(String(power), 4, " ") + "W   " + leftFill(String(kwh, 2), 5, " ") + "kWh");
          lcd.setCursor(0,1);
          lcd.print(leftFill(String(temp), 4, " ") + "\xdf" + "C  " +  tmh.getDayOfWeekName(0) + " " + tmh.getHrsMinSec());
      }
      
      else {
        if ( displayMode == 99 ) {
          // set Time and date
          display_setDateTime();
        }
        else {
          if ( displayMode == 2 ) {
            // -- display overall kWh
            lcd.clear();
            kwh = storage.totalWh / 10000.0;
            lcd.print("Ges.: " + leftFill(String(kwh, 2), 7, " ") + "kWh");
          }
          else {
            // -- flipp through the history ... 
            int index = 0;
            unsigned long wh1 = 0;
            unsigned long wh2 = 0;
            String lbl1; 
            String lbl2;
            switch ( displayMode ) {
              case 4 ... 8:
                // days hist
                index = - (displayMode - 3);
                wh1 = storage.daysWh[tmh.getDayOfWeek(index)];
                wh2 = storage.daysWh[tmh.getDayOfWeek(index -1)];
                lbl1 = tmh.getDayOfWeekName(index);
                lbl2 = tmh.getDayOfWeekName(index - 1);
                break;
              case 10 ... 22:
                // months hist
                index = - (displayMode - 10);
                wh1 = storage.monthsWh[tmh.getMonth(index)];
                wh2 = storage.monthsWh[tmh.getMonth(index - 1)];
                lbl1 = tmh.getMonthName(index);
                lbl2 = tmh.getMonthName(index - 1);
                break;
            
            }
            //Serial.println("index:" + String(index) + " lbl1: " + lbl1 + "  " + lbl2 + "  " + String(wh1));
   
            lcd.setCursor(0,0);
            kwh = wh1 / 10000.0;
            lcd.print(leftFill(lbl1, 3, " ") + ".:  " + leftFill(String(kwh, 2), 6, " ") + "kWh");
            lcd.setCursor(0,1);
            kwh = wh2  / 10000.0;;
            lcd.print(leftFill(lbl2, 3, " ") + ".:  " + leftFill(String(kwh, 2), 6, " ") + "kWh");
         }
           
          if (tmh.getSecondsCounter() > resetDisplayModeSeconds ) {
            // fall back to standard display after 5 seconds
            displayMode = 0;
          }
        }
      }
            
     } // 100ms
 
    else {
      delay(1); //minimum loop time if nothing else happened...       
    }
    
    // ---- deal with an eventually pressed key -----------------------
    kbdValue = kbd.read();
      
     if (kbdValue != 255) { //key is pressed
        if (displayMode == 99) {
          // if its 99 (set date time) then keys for set date time is aktive
          // keys 0, 1, 2, 3, 4 as 20, 21, 22, 23 ,24 
          kbdValue += 20;
        }
        switch (kbdValue)
          {
          case KEY_ENTER:
              storeEEprom(EE_OFFSET);
              break;
          case KEY_RIGHT:
              displayMode += 2;
              if ( displayMode > 20 ) {
                displayMode = 0;
              }
              resetDisplayModeSeconds = tmh.getSecondsCounter() + 5;
              break;
          case 128 + KEY_RIGHT:
              dumpEpromToSerial();
              break;
          case KEY_UP:
              tmh.incrementSecondsCounter(60);
              break;
          case KEY_DOWN:
              tmh.incrementSecondsCounter(-60);
              break;
              
          case 128 + KEY_ENTER:  // key 0 long
              start_setDateTime();
              break;

          case 20 ... 24: //enter
              handleKeystroke_setDateTime();
              break;
              
          default:
              break;
          }
       }
  }
