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

#define ANALOG_TEMPSENSOR_PIN 1
#define SIGNAL_PIN 2

unsigned long power;
int adValue = 1;
byte showHistEntry = 0; 
long resetshowHistEntrySeconds = 0;


// --- liquid crystal display driver from  ---------------------------------------------
//#define USE_STANDARD_LCD 1

#ifdef USE_STANDARD_LCD
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
#endif // USE_STANDARD_LCD


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

// string helper.. 
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
    long secondsTicker = 0;
    long dayTicker = 0;
    unsigned long totalWh = 0;
    unsigned long daysWh[7] = {0, 0, 0, 0, 0, 0, 0}; // monday = 0 
    unsigned long monthsWh[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0}; // jan = 1, dec = 12
} storage;


void loadEEprom() {
  EEPROM_readAnything(EE_OFFSET, storage);
  tmh.setSecondsTicker(storage.secondsTicker);
  tmh.setDayTicker(storage.dayTicker);
  lcd.setCursor(0,1);
  lcd.print("data loaded. ");
  delay(1000);
  
}

void storeEEprom() {
  storage.secondsTicker = tmh.getSecondsTicker();
  storage.dayTicker = tmh.getDayTicker();
  EEPROM_writeAnything(EE_OFFSET, storage);
  lcd.setCursor(0,1);
  lcd.print("data stored. ");
  delay(1000);
}
// ------------------------------------------------------------------------------------------



// Initialization ---------------------------------------------------------------------------

void setup()
{
  
  showHistEntry = 0; // 0 means actual day.
  resetshowHistEntrySeconds = 0;
  
  lcd.begin(16,2);               // initialize the lcd
  lcd.home();                   // go home  
  setBacklight(1);
  lcd.print("   OR-WE-521");
  lcd.setCursor(0,1);
  lcd.print(" impuls counter");
  delay(1000);
  
  if (analogRead(PIN_ANALOG_KBD) < 100)  {
    // if no key is pressed on start, read values from eeprom
    loadEEprom();
    }

  lcd.noCursor();
  lcd.clear();

  // pin for signal from OR-WE-521 
  pinMode(SIGNAL_PIN, INPUT);
  
  // serial device for debugging purpose!
  Serial.begin(9600);
  while (!Serial)
  {
      ; // wait for serial Pin to connect. otherwise reset if serial console is started :-/
  }

}


// Mainloop ---------------------------------------------------------------------------

void loop(){

    // get the milliseconds between last and actual signal.
    //  if a result is valid, a value > 0 is passed back
    unsigned long timeDeltaMillis = getStableSignalDelta(SIGNAL_PIN);
    if ( timeDeltaMillis > 0 ) {
      // one impulse received means 1 Wh was comsumed. time passed since last impulse --> power
      
      storage.daysWh[tmh.getDow(0)]++;
      storage.totalWh++;
      power = 3600000 / timeDeltaMillis; 
    }
   
    byte timeState = tmh.actualize(); // update the timeloop handler. state: 0: nothing; 1: 100ms passed; 2: 1s passed; 3: midnight

    if ( timeState >= 1 ) {
      // --- 100ms has passed ---- 
 
      if ( timeState >= 3 ) {
        // ----  action at midnight: fill the history --------------------------

        Serial.println("endofday: " + String(storage.daysWh[0]) + " total " + String(storage.totalWh) );
        storage.weeksWh[0] += storage.daysWh[0];
        storage.totalWh += storage.daysWh[0];
        // shift the array with the last weekdays Wh to right and add the actual on index 0
        for(int i=6; i>=0; i--){
          storage.daysWh[i] = storage.daysWh[i-1];
        }

        byte newDay = tmh.getDow(0);
        
        if (newDay = 0) {
          // End of Week
          storage.day = 0;
          for(int i=12; i>=0; i--){
            storage.weeksWh[i] = storage.weeksWh[i-1];
            }
          
        }
       
        storage.daysWh[0] = 0; // reset daily work
      } // midnight

        
       
      
      float kwh = storage.daysWh[0] / 1000.0;
      //float temp = 0.1105 * adValue + 0.583;
      //lcd.print("" + String(adValue) + "  t=" + String(temp) + "\xdf"+"C       ");


      // ---- refresh display ----------------------------------------------------------------------------
      if ( showHistEntry == 0 ) {
        // -- display normal information 
        lcd.setCursor(0,0);
        lcd.print(leftFill(String(power), 4, " ") + "W  " + leftFill(String(kwh, 3), 6, " ") + "kWh");
        lcd.setCursor(0,1);
        lcd.print(String("39.2") + "\xdf" + "C  " + tmh.getDowName() + " " + tmh.getHrsMinSec());
       
      }
      else {
        if ( showHistEntry == 2 ) {
          // -- display overall kWh
          lcd.clear();
          kwh = storage.totalWh / 1000.0;
          lcd.print("Ges. : " + leftFill(String(kwh, 1), 6, " ") + "kWh");
        }
        else {
          // -- flipp through the history ... 
          int index = 0;
          String label = "";
          unsigned long wh1 = 0;
          unsigned long wh2 = 0;
          
          switch ( showHistEntry ) {
            case 4 ... 8:
              // days hist
              index = showHistEntry - 3;
              label = "Tag";
              wh1 = storage.daysWh[index];
              wh2 = storage.daysWh[index + 1];
              break;
            case 10 ... 22:
              // months hist
              index = showHistEntry - 9;
              label = "Wo.";
              wh1 = storage.monthsWh[index];
              wh2 = storage.monthsWh[index + 1];
              break;
            
          }

          lcd.setCursor(0,0);
          kwh = wh1 / 1000.0;
          lcd.print(label +  leftFill("-" + String(index), 3, " ") + ": " + leftFill(String(kwh, 1), 5, " ") + "kWh");
          lcd.setCursor(0,1);
          kwh = wh2;
          lcd.print(label + leftFill("-" + String(index + 1), 3, " ") + ": " + leftFill(String(kwh, 1), 5, " ") + "kWh");
        }
         
        if (tmh.getSecondsTicker() > resetshowHistEntrySeconds ) {
          // fall back to standard display
          showHistEntry = 0;
        }
      }
    } // 100ms
 
    else {
      delay(1); //minimum loop time if nothing else happened...       
    }
    
    // input from keyboard -----------------------
    if (kbdValue != 255) //key is pressed
        {
            switch (kbdValue)
            {
            case 0:
                storeEEprom();
                break;
            case 1:
                showHistEntry += 2;
                if ( showHistEntry > 20 ) {
                  showHistEntry = 0;
                }
                resetshowHistEntrySeconds = tmh.getSecondsTicker() + 5;
                break;
            case 2:
                tmh.incrementDayTicker(1);
                break;
            case 3:
                tmh.incrementSecondsTicker(3600);
                break;
            case 4:
                tmh.incrementSecondsTicker(60);
                break;
            case 130:  // key 2 long
                tmh.incrementDayTicker(-1);
                break;
            case 131: // key 3 long
                tmh.incrementSecondsTicker(-3600);
                break;
            case 132: // key 4 long
                tmh.incrementSecondsTicker(-60);
                break;
            default:
                break;
            }

         }
        kbdValue = kbd.read();

  }
