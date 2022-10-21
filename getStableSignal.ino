

// called in loop()
// measures the time between the actual and the last rising signal
// one signal is valid if it lasts for MIN_SIGNAL_PERIOD


// a signal must remain for at least some millisecs
#define MIN_SIGNAL_PERIOD 70

unsigned long actMillis;
unsigned long signalMillis = 0;
unsigned long lastSignalMillis = 0;
unsigned long signalStableMillis = 0;

boolean waitForValidation;
boolean waitForReset;

   
unsigned long getStableSignalDelta(byte pin) {
     
   unsigned long timeDeltaMillis = 0;
   
   actMillis = millis();

    // read the pin
    byte pinLevel = digitalRead(pin);
    
    if (pinLevel == 1) {
      // if positive, store the time once and then wait vor it to validate 
      if (!waitForValidation and !waitForReset) {
        signalMillis = actMillis;    
        waitForValidation = true;
        Serial.println("signal candidate at: " + String(signalMillis) );
      }
       
    }
    else {
      if (true) {
        waitForReset = false;
        waitForValidation = false;
      }
      
    }
    signalStableMillis = actMillis - signalMillis;

    if (!waitForReset) {
      if (waitForValidation) {
        if ( signalStableMillis > MIN_SIGNAL_PERIOD  ) {
          timeDeltaMillis = signalMillis - lastSignalMillis;
          lastSignalMillis = signalMillis;
          waitForValidation = false;
          waitForReset = true;
          Serial.println("act:" + String(signalMillis) + " delta:" + String(timeDeltaMillis));
        }
      }
    }
  return timeDeltaMillis;
    
}
