// definition of parameters of hardwaresetup
// create a copy config.h and adapt it

#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H


// PINS for sensor input ------------
#define ANALOG_TEMPSENSOR_PIN 1
#define SIGNAL_PIN 4

// Keyboard -------------------------
#define PIN_ANALOG_KBD   0  // ad0 for input of analog Keyboard...
#define KBD_NR_OF_KEYS   5  // how many keys are built up in the circuit

// variables for the keys
#define KEY_ENTER 0
#define KEY_RIGHT 1 
#define KEY_LEFT  4
#define KEY_UP    2
#define KEY_DOWN  3

// Display ---------------------------

#define USE_STANDARD_LCD_LIBRARY  // which library used?


#ifdef USE_STANDARD_LCD_LIBRARY
  // use LiquidChrystal.h ...
  #define LDC_RS 8
  #define LDC_E  7
  #define LDC_D4 9
  #define LDC_D5 10
  #define LDC_D6 11
  #define LDC_D7 12
#else
  // ... or use LiquidCrystal_SR2W.h
  #define LDC_DATA 8
  #define LDC_CLOCK  7
#endif  


// EEPROM ----------------------------
#define EE_OFFSET 128  // start adress for the EEPROM data area


#endif //PROJECT_CONFIG_H
