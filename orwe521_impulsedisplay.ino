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


#define ANALOG_PIN 1
#define SIGNAL_PIN 2

// a signal must remain for at least some millisecs
#define MIN_SIGNAL_PERIOD 70

unsigned long lastSignalMillis;
unsigned long signalMillis;
unsigned long actMillis;
unsigned long sinceSignalMillis;
unsigned long diffMillis;

unsigned long power;
int adValue = 1;

boolean waitForValidation;
boolean waitForReset;
boolean interruptHandled;

// --- Status variables that will be stored in eeprom --------------------------------- 
#include "eeAny.h"
#define EE_OFFSET 128
struct {
    long secondsTicker = 0;
    int dailyWattHours = 0;
    
} storage;



#include <Wire.h>
#include <LiquidCrystal_SR2W.h>

LiquidCrystal_SR2W lcd(8, 7, POSITIVE);


// --- uses keyboard driver from AnalogKbd.cpp //
#include<AnalogKbd.h>
#define PIN_ANALOG_KBD   0  // ad0 for input of analog Keyboard...
#define KBD_NR_OF_KEYS   5  // how many keys are built up in the circuit
#define KBD_RELIABLE_TIME_DELTA     30   // ms a key must be pressed
#define KBD_LONGPRESS_TIME_DELTA    600  // ms a key must be pressed for long value

AnalogKbd kbd(PIN_ANALOG_KBD, KBD_NR_OF_KEYS, KBD_RELIABLE_TIME_DELTA, KBD_LONGPRESS_TIME_DELTA);
byte kbdValue = 255; //the value that is read from keyboard


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

// methods for clock and Timer
unsigned long lastDeciSecsIncMillis;
byte deciSecondsCounter;

boolean incrementDeciSeconds()
{
    if (millis() - lastDeciSecsIncMillis >= 100 )
    {
        deciSecondsCounter++;
        lastDeciSecsIncMillis += 100;
        if (deciSecondsCounter >= 10) {
          deciSecondsCounter = 0;
          storage.secondsTicker++;
          // midnight: 
          if (storage.secondsTicker > 86399) {
            storage.secondsTicker = 0;

            storage.dailyWattHours = 0; // reset daily work
          }
        }
        
        return true;
    }
    return false;
}


String seconds2hrsMinSec(unsigned long secsTicker)
{
    String divis = "";
    unsigned long hrs = secsTicker / 3600;
    byte mins = ( secsTicker - hrs * 3600 ) / 60;
    byte secs = secsTicker % 60; 
    return leftFill(String(abs(hrs)), 2, "0") + ":" + leftFill(String(abs(mins)), 2, "0") + ":" +  leftFill(String(abs(secs)), 2, "0");
}


void setup()
{
  
  lastSignalMillis = 0;
  signalMillis = 0;
  diffMillis = 1000000;
  waitForValidation = false;
  waitForReset = false;
  
  lcd.begin(16,2);               // initialize the lcd
  lcd.home();                   // go home
  lcd.setBacklight(1);
  lcd.print("   OR-WE-521");
  lcd.setCursor(0,1);
  lcd.print(" impuls counter");
  delay(1000);
  
  if (analogRead(PIN_ANALOG_KBD) < 100)  {
    // if no key is pressed on start, read values from eeprom
    EEPROM_readAnything(EE_OFFSET, storage);
    }
  
  lcd.noCursor();
  lcd.clear();

  // pin for signal from OR-WE-521 
  pinMode(SIGNAL_PIN, INPUT);
  
//  // serial device for debugging purpose!
//  Serial.begin(9600);
//  while (!Serial)
//  {
//      ; // wait for serial Pin to connect. otherwise reset if serial console is started :-/
//  }

}

void loop(){
    delay(1);
    // actual Time
    actMillis = millis();

    // read the pin
    byte pinLevel = digitalRead(SIGNAL_PIN);
    
    if (pinLevel == 1) {
      // if positive, store the time once and then wait vor it to validate 
      if (!waitForValidation and !waitForReset) {
        signalMillis = actMillis;    
        waitForValidation = true;
//        Serial.println("signal candidate at: " + String(signalMillis) );
      }
       
    }
    else {
      if (true) {
        waitForReset = false;
        waitForValidation = false;
      }
      
    }
    sinceSignalMillis = actMillis - signalMillis;

    if (!waitForReset) {
      if (waitForValidation) {
        if (sinceSignalMillis > MIN_SIGNAL_PERIOD  ) {
//          Serial.println("act:" + String(signalMillis) + " since:" + String(sinceSignalMillis));
          storage.dailyWattHours++;
    
          diffMillis = signalMillis - lastSignalMillis;
          lastSignalMillis = signalMillis;
          
          if (storage.dailyWattHours > 1) {
            // wait to second event as the first is incorrect !
            power = 3600000 / diffMillis; 
          }
          waitForValidation = false;
          waitForReset = true;
        }
      }
    }

    // update display each 100ms
    if (incrementDeciSeconds()) {
      float kwh = storage.dailyWattHours / 1000.0;
      lcd.setCursor(0,0);
      lcd.print(leftFill(String(power), 4, " ") + "W  " + leftFill(String(kwh, 3), 6, " ") + "kWh");
      lcd.setCursor(0,1);
      lcd.print(String("30.4") + "\xdf" + "C     " + seconds2hrsMinSec(storage.secondsTicker));
      
      //float temp = 0.1105 * adValue + 0.583;
      //lcd.print("" + String(adValue) + "  t=" + String(temp) + "\xdf"+"C       ");

    }
    
    // input from keyboard 
    if (kbdValue != 255) //key is pressed
        {
            switch (kbdValue)
            {
            case 0:
                EEPROM_writeAnything(EE_OFFSET, storage);
                lcd.setCursor(0,1);
                lcd.print("stored. ");
                delay(1000);
                break;
             
            case 3:
                storage.secondsTicker += 60;
                break;
            case 2:
                storage.secondsTicker += 3600;
                break;
            case 131: // key 2 long
                storage.secondsTicker -= 60;
                break;
            case 130:
                storage.secondsTicker -= 3600;
                break;
            default:
                break;
            }
            if (storage.secondsTicker < 0) {
              storage.secondsTicker += 86400;
            }
         }
        kbdValue = kbd.read();

  }
