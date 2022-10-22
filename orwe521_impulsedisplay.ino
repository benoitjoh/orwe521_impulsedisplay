/* reacts on the positive impulse from a OR-WE-521 
 * an impulse is sent for each watt-hour that was measured, 
 * meaning 1000 imp sums up to 1 kW/h
 * 
 * impulse is expected on SIGNAL_PIN
 * signal has to been validated (stable over MIN_SIGNAL_PERIOD milisecs)
 * to exclude noise
 * 
 * next impulse the mean power can be calculated as 3600 / secs
 */
#define USE_STANDARD_LCD_LIBRARY 1
//#define ENABLE_DEBUG_SERIAL 1


#define ANALOG_TEMPSENSOR_PIN 1
#define SIGNAL_PIN 4

#define KEY_ENTER 0
#define KEY_RIGHT 1 
#define KEY_LEFT  4
#define KEY_UP    3
#define KEY_DOWN  2



unsigned long power;
int adValue = 1;
byte displayMode = 0; 
long resetDisplayModeSeconds = 0;
byte actualMonth = 0;

// --- liquid crystal display driver from  ---------------------------------------------

#ifdef USE_STANDARD_LCD_LIBRARY
  // using the standard LCD Library
  #include <LiquidCrystal.h>
  //            lcd(rs, en, d4, d5, d6, d7); backlight: pin D6
  LiquidCrystal lcd(8,  7,   9, 10, 11, 12);
  
  void setBacklight(byte state) {
    digitalWrite(6, state);
  }
#else
  // using the latchregister
  #include <LiquidCrystal_SR2W.h>
  LiquidCrystal_SR2W lcd(8, 7, POSITIVE);

  void setBacklight(byte state) {
    lcd.setBacklight(state);
  }
#endif // USE_STANDARD_LCD_LIBRARY


// --- keyboard driver from AnalogKbd.cpp ---------------------------------------------
#include<AnalogKbd.h>
#define PIN_ANALOG_KBD   0  // ad0 for input of analog Keyboard...
#define KBD_NR_OF_KEYS   5  // how many keys are built up in the circuit
#define KBD_RELIABLE_TIME_DELTA     30   // ms a key must be pressed
#define KBD_LONGPRESS_TIME_DELTA    600  // ms a key must be pressed for long value

AnalogKbd kbd(PIN_ANALOG_KBD, KBD_NR_OF_KEYS, KBD_RELIABLE_TIME_DELTA, KBD_LONGPRESS_TIME_DELTA);
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


// --- Variables that will be stored in eeprom if system goes power off --------------------- 
#include "eeAny.h"
#define EE_OFFSET 128
struct {
    long secondsCounter = 70000;
    long dayCounter = 100;
    unsigned long totalWh = 0;
    unsigned long daysWh[7] = {0, 1, 2, 3, 4, 5, 6}; // monday = 0 
    unsigned long monthsWh[13] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 ,12}; // jan = 1, dec = 12
} storage;


void loadEEprom(bool reset) {
  lcd.setCursor(0,1);
  if (!reset) {
    EEPROM_readAnything(EE_OFFSET, storage);
    lcd.print("data loaded.     ");
  }
  else {
    lcd.print("EEPROM reset.     ");
  }
  tmh.setSecondsCounter(storage.secondsCounter);
  tmh.setDayCounter(storage.dayCounter);
  
  delay(800);
  
}

void storeEEprom() {
  storage.secondsCounter = tmh.getSecondsCounter();
  storage.dayCounter = tmh.getDayCounter();
  EEPROM_writeAnything(EE_OFFSET, storage);
  lcd.setCursor(0,1);
  lcd.print("data stored. ");
  delay(1000);
}
// ------------------------------------------------------------------------------------------



// Initialization ---------------------------------------------------------------------------

void setup()
{
  
  displayMode = 0; // 0 means actual day.
  resetDisplayModeSeconds = 0;
  
  lcd.begin(16,2);               // initialize the lcd
  lcd.home();                   // go home  
  setBacklight(1);
  lcd.print("   OR-WE-521");
  lcd.setCursor(0,1);
  lcd.print(" impuls counter");
  delay(1000);
  
  // if no key is pressed on start, read values from eeprom, else reset
  loadEEprom( bool(analogRead(PIN_ANALOG_KBD) > 100) );
  
  lcd.noCursor();
  lcd.clear();

  // pin for signal from OR-WE-521 
  pinMode(SIGNAL_PIN, INPUT);
  pinMode(13, OUTPUT);
  
#ifdef ENABLE_DEBUG_SERIAL
  // serial device for debugging purpose!
  Serial.begin(9600);
  while (!Serial)
  {
      ; // wait for serial Pin to connect. otherwise reset if serial console is started :-/
  }
#endif

}


// Mainloop ---------------------------------------------------------------------------

void loop(){
    actualMonth = tmh.getMonth(0);

    // get the milliseconds between last and actual signal.
    //  if a result is valid, a value > 0 is passed back
    unsigned long timeDeltaMillis = getStableSignalDelta(SIGNAL_PIN);
    if ( timeDeltaMillis > 0 ) {
      // one impulse received means 1 Wh was comsumed. time passed since last impulse --> power
      digitalWrite(13, 1);
      
      storage.daysWh[tmh.getDow(0)]++;
      storage.monthsWh[tmh.getMonth(0)]++;
      storage.totalWh++;
      power = 3600000 / timeDeltaMillis; 
      delay(10);
      digitalWrite(13, 0);
    }
   
    byte timeState = tmh.actualize(); // update the timeloop handler. state: 0: nothing; 1: 100ms passed; 2: 1s passed; 3: midnight

    if ( timeState >= 1 ) {
      // --- 100ms has passed ---- this part needs 4ms for calculation... 
      if ( timeState >= 3 ) {
        // ----  action at midnight: fill the history --------------------------

        if ( actualMonth != tmh.getMonth(0) ) {
          // new month has started... so reset the actual counter 
          storage.monthsWh[tmh.getMonth(0)] = 0;
        }
        //reset the new wh counter for the new day
        storage.daysWh[tmh.getDow(0)] = 0; 
        storeEEprom();       
      }       
      
      float kwh = storage.daysWh[tmh.getDow(0)] / 1000.0;
      //float temp = 0.1105 * adValue + 0.583;
      //lcd.print("" + String(adValue) + "  t=" + String(temp) + "\xdf"+"C       ");


      // ---- refresh display ----------------------------------------------------------------------------

      
      
      if ( displayMode == 0 ) {
        // -- display normal information 
        lcd.setCursor(0,0);
        lcd.print(leftFill(String(power), 4, " ") + "W   " + leftFill(String(kwh, 2), 5, " ") + "kWh");
        lcd.setCursor(0,1);
        lcd.print(String("39.2") + "\xdf" + "C  " +  tmh.getDowName(0) + " " + tmh.getHrsMinSec());
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
            kwh = storage.totalWh / 1000.0;
            lcd.print("Ges.: " + leftFill(String(kwh, 1), 7, " ") + "kWh");
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
                wh1 = storage.daysWh[tmh.getDow(index)];
                wh2 = storage.daysWh[tmh.getDow(index -1)];
                lbl1 = tmh.getDowName(index);
                lbl2 = tmh.getDowName(index - 1);
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
            kwh = wh1 / 1000.0;
            lcd.print(leftFill(lbl1, 3, " ") + ".:  " + leftFill(String(kwh, 1), 6, " ") + "kWh");
            lcd.setCursor(0,1);
            kwh = wh2  / 1000.0;;
            lcd.print(leftFill(lbl2, 3, " ") + ".:  " + leftFill(String(kwh, 1), 6, " ") + "kWh");
         }
           
          if (tmh.getSecondsCounter() > resetDisplayModeSeconds ) {
            // fall back to standard display
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
              storeEEprom();
              break;
          case KEY_RIGHT:
              displayMode += 2;
              if ( displayMode > 20 ) {
                displayMode = 0;
              }
              resetDisplayModeSeconds = tmh.getSecondsCounter() + 5;
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
