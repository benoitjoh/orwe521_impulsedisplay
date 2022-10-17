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
 * 
 *  
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
int signalCounter; 
int adValue = 1;

boolean waitForValidation;
boolean waitForReset;
boolean interruptHandled;

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
unsigned long secondsTicker;
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
          secondsTicker++;
          // midnight: 
          if (secondsTicker > 86399) {
            secondsTicker = 0;
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
    if (secs < 0) 
    {
         divis = "-";
    }
    return divis + leftFill(String(abs(hrs)), 2, "0") + ":" + leftFill(String(abs(mins)), 2, "0") + ":" +  leftFill(String(abs(secs)), 2, "0");
}






void setup()
{
  lastSignalMillis = 0;
  signalMillis = 0;
  diffMillis = 1000000;
  signalCounter = 0; 
  waitForValidation = false;
  waitForReset = false;
  kbdValue = 255;
  
  lcd.begin(16,2);               // initialize the lcd
  lcd.home();                   // go home
  lcd.setBacklight(1);
  lcd.print("   OR-WE-521");
  lcd.setCursor(0,1);
  lcd.print(" impuls counter");
  delay(1000);
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
          signalCounter++;
    
          diffMillis = signalMillis - lastSignalMillis;
          lastSignalMillis = signalMillis;
          
          if (signalCounter > 1) {
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
      lcd.setCursor(0,0);
      lcd.print(leftFill(String(signalCounter), 4, " ") + "Wh     " + leftFill(String(power), 4, " ") + "W ");
      lcd.setCursor(0,1);
      lcd.print("        " + seconds2hrsMinSec(secondsTicker));
      
      //float temp = 0.1105 * adValue + 0.583;
      //lcd.print("" + String(adValue) + "  t=" + String(temp) + "\xdf"+"C       ");

    }
    
    // input from keyboard
    if (kbdValue != 255) //pressed
        {
            switch (kbdValue)
            {
            case 0:
                // Vortag?
                break;
             
            case 3:
                secondsTicker += 60;
                break;
            case 2:
                secondsTicker += 3600;
                break;
            case 131: // key 2 long
                secondsTicker -= 60;
                break;
            case 130:
                secondsTicker -= 3600;
                break;
            default:
                break;
            }
         }
        kbdValue = kbd.read();

  }
