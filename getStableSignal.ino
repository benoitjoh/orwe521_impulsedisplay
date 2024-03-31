

// called in loop()
// measures the time between the actual and the last rising signal
// one signal is valid if it lasts for MIN_SIGNAL_PERIOD


// a signal must remain for at least some millisecs
#define MIN_SIGNAL_PERIOD 50


unsigned long signalStartMillis = 0;
unsigned long lastSignalStartMillis = 0;
boolean waitForValidation;
boolean waitForReset;

   
unsigned long getStableSignalDelta(byte pin) {
     
    unsigned long timeDeltaMillis = 0;
    unsigned long actMillis = millis();

    // read the pin
    
    if (digitalRead(pin) == 1) {
      // if positive, store the time once and then wait for it to validate 
      if (!waitForValidation and !waitForReset) {
        signalStartMillis = actMillis;    
        waitForValidation = true;
        //Serial.println("signal candidate at: " + String(signalStartMillis) );
      }
       
    }
    else {
        waitForReset = false;
        waitForValidation = false;
    }
    
    unsigned long signalStableMillis = actMillis - signalStartMillis;

    if (!waitForReset) {
        if (waitForValidation) {
            if ( signalStableMillis > MIN_SIGNAL_PERIOD  ) {
                // now it is a good signal. return the timespan between start and last start
                timeDeltaMillis = signalStartMillis - lastSignalStartMillis;
                lastSignalStartMillis = signalStartMillis;
                waitForValidation = false;
                waitForReset = true;
                //Serial.println("act:" + String(signalStartMillis) + " delta:" + String(timeDeltaMillis));
            }
        }
    }
    return timeDeltaMillis;
    
}
