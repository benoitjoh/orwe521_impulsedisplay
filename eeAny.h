// Templates for storage of complex structures in EEPROM
// see http://playground.arduino.cc/Code/EEPROMWriteAnything
//

#include <EEPROM.h>
#include <Arduino.h>  // for type definitions

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}


// Example: 
/*
struct test_t
{
    long alarm 1234567;
    int mode 89;
} conf;

#define EE_OFFSET 128
EEPROM_writeAnything(EE_OFFSET, conf);
EEPROM_readAnything(EE_OFFSET, conf);
*/

