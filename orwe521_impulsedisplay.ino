/*   reacts on the positive impulse from a OR-WE-521 
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
#define VERSION "v1.2/" __DATE__

#define ORWE_PULSE_PER_KWH 1000
#define ORWE_DECIWH_PER_PULSE ( 10000 / ORWE_PULSE_PER_KWH )

// alarm for second temp sensor
// alarm limit defined in storage.cfg[CONF_FRIDGE_ALARM]
boolean fridge_alarm = false;
boolean fridge_quit = false;

unsigned long power;
unsigned long nowMillis;
unsigned long lastIntervalStart;
float lastTimeDeltaMillis;

int temp = 0;
int temp_2 = 0;
byte displayMode = 0; 
long resetDisplayModeSeconds = 0;
byte actualMonth = 0;

char statusSign; 
byte backlight = 1;

// --- liquid crystal display driver from LiquidCrystal.h or LiquidCrystal_SR2W.h ---------------------------------------------
#ifdef USE_STANDARD_LCD_LIBRARY
    // using the standard LCD Library
    #include <LiquidCrystal.h>
    //            lcd(rs, e, d4, d5, d6, d7);   backlight: pin D6
    LiquidCrystal lcd(LCD_RS,  LCD_E,   LCD_D4, LCD_D5, LCD_D6, LCD_D7);

    void setBacklight(byte state) {
        digitalWrite(LCD_BACKLIGHT, state);
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
TimeLoop tmh(0);

// ---- string helper   ---------------------------------------------------------------  
String leftFill(String a, byte wantedLen, String fillLetter) {
    // fills a string with letters from left side that the resulting length is reached
    while (a.length() < wantedLen) {
        a = fillLetter + a;
        }
        
    for (byte i=0; i<wantedLen; i++) {
        if (a[i] == '-') {
            a[i] = 4;
            }
        }
    return a;
    }

byte divideAndRoundBy(unsigned long value, int dividend) {
    // divide by dividend [10, 100, 1000] and round 
    byte result = byte(value / dividend);
    if ( value % dividend  > ((dividend / 2) - 1) ) {
        result +=1;
        }
}

// EEPROM ------------------------------------------------------------------
#include <EEPROM.h>

#define EE_OFFSET 0  // start adress for the EEPROM data area 
#define EE_OFFSET_DOY 128 //start adress for the 367 bytes where the daily revenue for all Days of Year is stored

#define CONF_ADDR_COUNT   8  // how many values do we have in the conf array? 

// --- Variables that will be stored in eeprom persistent actually 100byte long --- 
struct {
long secondsCounter = 76500;
long dayCounter = 1540;
unsigned long totalWh = 9571800;
unsigned long days_cWh[7] = {7950,37570,7080,35850,20330,9760,11550,};
unsigned long months_cWh[13] = {0,280550,394850,288930,720580,919470,1123920,927750,809320,966570,603040,223820,226410,};
unsigned int hour_cWh[24] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int cfg[CONF_ADDR_COUNT] = {423,-15,-1,-1,-1,-1,-1,-1,};
} storage;

// *** config paramters adjustable in configMenu ************************************** //
#define CONF_TIMECORR_MS_PER_HOUR 0
#define CONF_FRIDGE_ALARM 1
#define CONF_V2 2
#define CONF_V3 3
#define CONF_V4 4
#define CONF_V5 5
#define CONF_V6 6
#define CONF_V7 7

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
    tmh.setMsPerHourCorrection(storage.cfg[CONF_TIMECORR_MS_PER_HOUR]);

    delay(800);

    }

void storeEEprom() {
    // apply configuration changes, and store all to eprom 
    storage.secondsCounter = tmh.getSecondsCounter();
    storage.dayCounter = tmh.getDayCounter();
    tmh.setMsPerHourCorrection(storage.cfg[CONF_TIMECORR_MS_PER_HOUR]);
    EEPROM.put(EE_OFFSET, storage);
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


// Initialization ---------------------------------------------------------------------------

void setup() {
    displayMode = 0; // 0 means actual day.
    resetDisplayModeSeconds = 0;

    init_lcd();
    lcd.noCursor();

    lcd.print(F(" # OR-WE-521 #")); 
    lcd.setCursor(0,1);
    lcd.print(VERSION);
    delay(2000);

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
    actualMonth = tmh.getMonth(0);
    setBacklight(1);


    // pin for signal from OR-WE-521 
    pinMode(SIGNAL_PIN, INPUT);
    pinMode(13, OUTPUT);

    // serial device for debugging purpose!
    Serial.begin(9600);
    while (!Serial)
    {
        ; // wait for serial Pin to connect. otherwise reset if serial console is started :-/
        }
        
    statusSign = 32;

}


// Mainloop ---------------------------------------------------------------------------
unsigned long lastMeasurement = 0;

void loop() {
    nowMillis = millis();
    // get the milliseconds between last and actual signal.
    // if a result is valid, a value > 0 is passed back

    unsigned long timeDeltaMillis = getStableSignalDelta(SIGNAL_PIN);

    if ( timeDeltaMillis > 0 ) {
        // one impulse received means 1 Wh was comsumed. time passed since last impulse --> power

        PORTB |=  B00100000; //set pin13 to HIGH (for a long blink)
        storage.days_cWh[tmh.getDayOfWeek(0)] += ORWE_DECIWH_PER_PULSE;
        storage.months_cWh[tmh.getMonth(0)] += ORWE_DECIWH_PER_PULSE;
        storage.hour_cWh[tmh.getHour()] += ORWE_DECIWH_PER_PULSE;
        storage.totalWh += ORWE_DECIWH_PER_PULSE;
        power = 360000 * ORWE_DECIWH_PER_PULSE / timeDeltaMillis; 
        Serial.println("loop: now:" + String(nowMillis) + " delta:" + String(timeDeltaMillis)+ " pwr:" + String(power));
        lastIntervalStart = millis() - timeDeltaMillis;
        lastTimeDeltaMillis = float(timeDeltaMillis);
        delay(10);
        statusSign = 1;
        }

    byte timeState = tmh.actualize(); // update the timeloop handler. state: 0: nothing; 1: 100ms passed; 2: 1s passed; 3: midnight

    if ( timeState >= 1 ) {
        // --- 100ms has passed ---- 

        if ( fridge_alarm ) {
            backlight = !backlight;
            }
        else {
            backlight = 1;
            }

        setBacklight(backlight);

        if ( timeState >= 2 ) {
            // --- 1 sec has passed ---- 

            PORTB |=  B00100000; //set pin13 to HIGH, give led a quick blink each second     

            unsigned long passedMillis = nowMillis - lastIntervalStart - lastTimeDeltaMillis;

            if ( passedMillis > (lastTimeDeltaMillis) and power > 0){

                // reduce time if for longer time no impulse was fetched
                float decrement = ( passedMillis - (lastTimeDeltaMillis / 2) ) / passedMillis;

                power = ( 360000 * ORWE_DECIWH_PER_PULSE / passedMillis ) * decrement;
                //Serial.println("red : now:" + String(nowMillis) + " lastTimeDeltaMillis:" + String(lastTimeDeltaMillis)
                //             + " passedMillis:" + String(passedMillis)+ " decrFactor:" + String(decrement));
                statusSign = 32;
                }

            if ( passedMillis > 720000 and power > 0){
                // set power to zero after 12 minutes without a signal
                power = 0; 
                }

            delayMicroseconds(50);
            PORTB &= ~B00100000; //set pin13 to LOW 
            
            temp = read_pt1000(analogRead(ANALOG_TEMP_PT1000_PIN));
            temp_2 = read_dellakku_tempsensor(analogRead(ANALOG_TEMP_DELL_PIN));

            // fridge alarm
            if (temp_2 > storage.cfg[CONF_FRIDGE_ALARM]) {
                if ( !fridge_quit ) {
                    fridge_alarm = true;
                    }
                }
            else {
                fridge_alarm = false;
                fridge_quit = false;
                }
            
            if ( tmh.getSecondsCounter() % 3600 == 1) {
                // new hour started... so reset the actual counter 
                storage.hour_cWh[tmh.getHour()] = 0;
                Serial.println("new hour."); 
                }

            } // --- 1 sec    

        if ( timeState >= 3 ) {
            // ----  action at midnight: fill the history --------------------------

            if ( actualMonth != tmh.getMonth(0) ) {
                // new month has started... so reset the actual counter 
                storage.months_cWh[tmh.getMonth(0)] = 0;
            }
            //reset the new wh counter for the new day
            storage.days_cWh[tmh.getDayOfWeek(0)] = 0; 
            storeEEprom();       

            // fill the value from yesterday in the dayOfYear array in EEPROM as hektoWh (=1000 centiWh)
            updateDayOfYearTotal(tmh.getDayOfYear(-1), 
                                divideAndRoundBy(storage.days_cWh[tmh.getDayOfWeek(-1)], 1000) );
            }  
                 
        
        actualMonth = tmh.getMonth(0);

        PORTB &= ~B00100000; //set pin13 to LOW 

        float kwh = storage.days_cWh[tmh.getDayOfWeek(0)] / 10000.0;


        // ---- refresh display ----------------------------------------------------------------------------

        switch ( displayMode )
            {
            case 0:
                // -- display normal information each second
                lcd.setCursor(0,0);
                lcd.print(leftFill(String(power), 4, " ") + "W ");
                lcd.write(statusSign);
                lcd.print(" " + leftFill(String(kwh, 2), 5, " ") + "kWh");
                lcd.setCursor(0,1);
                lcd.print(leftFill(String(temp), 2, " ") + "\xdf" + "" +
                          leftFill(String(temp_2), 3, " ") + "\xdf" + " " +
                          tmh.getHrsMinSec());
                break;

            case 2:
                // -- display overall kWh
                lcd.clear();
                kwh = storage.totalWh / 10000.0;
                lcd.print("Ges.: " + leftFill(String(kwh, 2), 7, " ") + "kWh");
                break; 

            case 30:
                // -- debug information values of ad inputs
                lcd.setCursor(0,0);
                lcd.print("  A0  A1  A2  A3");
                lcd.setCursor(0,1);
                lcd.print(leftFill(String(analogRead(0)), 4, " ") +
                    leftFill(String(analogRead(1)), 4, " ") + 
                    leftFill(String(analogRead(2)), 4, " ") + 
                    leftFill(String(analogRead(3)), 4, " ") );
                break;
            
            case 98:
                // -- set Time and date
                display_setConfigMenu();
                break;

            case 99:
                // -- set Time and date
                display_setDateTime();
                break;
                
            case 4 ... 22 :
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
                        wh1 = storage.days_cWh[tmh.getDayOfWeek(index)];
                        wh2 = storage.days_cWh[tmh.getDayOfWeek(index -1)];
                        lbl1 = tmh.getDayOfWeekName(index);
                        lbl2 = tmh.getDayOfWeekName(index - 1);
                        break;
                    case 10 ... 22:
                        // months hist
                        index = - (displayMode - 10);
                        wh1 = storage.months_cWh[tmh.getMonth(index)];
                        wh2 = storage.months_cWh[tmh.getMonth(index - 1)];
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

                break;

            }
        
        if (displayMode < 30 and displayMode > 1 and
            tmh.getSecondsCounter() > resetDisplayModeSeconds ) {
            // fall back to standard display after 5 seconds exept display mode over 30 
            // these displays should be manually reset
            displayMode = 0;
            }


        } // --- 100ms

    else {
        delay(1); //minimum loop time if nothing else happened...       
        }

    // ---- deal with an eventually pressed key -----------------------
    kbdValue = kbd.read();

        if (kbdValue != 255) { //key is pressed
            // reset fridge alarm
            fridge_alarm = false; 
            fridge_quit = true; 

            if (displayMode == 99) {
                // if its 99 (set date time) then keys for set date time is active
                // keys 0, 1, 2, 3, 4 as 20, 21, 22, 23 ,24 
                kbdValue += 20;
                }

            if (displayMode == 98) { // set config menu
                kbdValue += 30;
                }
            Serial.println("kbd:" + String(kbdValue) + " ad:" + String(kbd.getLastAdValue()));
            
            switch (kbdValue)
                {
                case KEY_ENTER:
                    storeEEprom();
                    break;

                case KEY_RIGHT:
                    displayMode += 2; // flip through the history pages
                    if ( displayMode > 20 ) {
                        displayMode = 0;
                    }
                    resetDisplayModeSeconds = tmh.getSecondsCounter() + 5;
                    break;

                case KEY_RIGHT_LONG:
                    dumpEpromToSerial();
                    break;

                case KEY_LEFT:
                    displayMode = 30; //debug
                    resetDisplayModeSeconds = tmh.getSecondsCounter() + 5;
                    break;

                case KEY_UP:
                    tmh.incrementSecondsCounter(60);
                    break;

                case KEY_DOWN:
                    tmh.incrementSecondsCounter(-60);
                    break;

                case KEY_ENTER_LONG:
                    displayMode = 99; // start setDateTime 
                    break;

                case KEY_DOWN_LONG:
                    displayMode = 98; // start setConfig 
                    break;

                case 20 ... 24: 
                    handleKeystroke_setDateTime();
                    break;

                default:
                    break;
                }

            handleKeystroke_setConfigMenu();
        }
}
