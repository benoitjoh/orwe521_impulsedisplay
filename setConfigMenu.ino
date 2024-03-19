// Menu for set the configregisters
// Values are int 
//   cfgBoundary:
//     in menu values that can be choosen are limited to min, max and the stepwidth
// 


#define SETCONFIG_KEY_ENTER (30 + KEY_ENTER) 
#define SETCONFIG_KEY_RIGHT (30 + KEY_RIGHT)  
#define SETCONFIG_KEY_LEFT  (30 + KEY_LEFT) 
#define SETCONFIG_KEY_UP    (30 + KEY_UP) 
#define SETCONFIG_KEY_DOWN  (30 + KEY_DOWN) 
#define SETCONFIG_KEY_UP_LONG    (30 + KEY_UP_LONG) 
#define SETCONFIG_KEY_DOWN_LONG  (30 + KEY_DOWN_LONG) 




byte codeAddr = 0; // index for setting the parameters
int newVal = 0;

// upper limit, lower limit, stepwidth and name for the configparameters
struct cfgBdr_t
{
    int minVal;
    int maxVal;
    byte step; 
    String descriptor; 
} cfgBoundary[CONF_ADDR_COUNT] = 

{
//  min   max   step    name
    {-2000,   2000,  1,  "tCor"},
    {1,   50,   1,  "v1"},
    {50,  90,   10, "v2"},
    {0,  100,   5,  "v3"},
    {0,   3,    1,  "v4"},
    {0,   255,  5,  "v5"},
    {0,   255,  5,  "v6"},
    {0,   255,  5,  "v7"},
};


// -----------------------------------------------------------------------------
void display_setConfigMenu()
{
    // print name and value of the selected configuration address //
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Set config");
    lcd.setCursor(0,1);
    lcd.print("[" + leftFill(cfgBoundary[codeAddr].descriptor, 4, " ") + "] : " 
                  + leftFill(String(storage.cfg[codeAddr]), 4, " ") + "  ");
    return;
}

void incValue(byte step) {
    newVal = storage.cfg[codeAddr] + step;
    if (newVal > cfgBoundary[codeAddr].maxVal) {
        storage.cfg[codeAddr] = cfgBoundary[codeAddr].minVal;
        }
    else {
        storage.cfg[codeAddr] = newVal;
        }
    }
    
void decValue(byte step) {
    newVal = storage.cfg[codeAddr] - step;
    if (newVal < cfgBoundary[codeAddr].minVal or newVal > cfgBoundary[codeAddr].maxVal) {
        storage.cfg[codeAddr] = cfgBoundary[codeAddr].maxVal;
        }
    else {
        storage.cfg[codeAddr] = newVal;
        }
    }

void handleKeystroke_setConfigMenu() {
    switch (kbdValue) {
    case SETCONFIG_KEY_ENTER:   // enter (key 0 in set date time mode
        // finish
        displayMode = 0;
        kbdValue = 255; 
        storeEEprom();
        break;
        
    case SETCONFIG_KEY_RIGHT:
        codeAddr++;
        if (codeAddr >= CONF_ADDR_COUNT) { codeAddr = 0; }
        break;
        
    case SETCONFIG_KEY_LEFT: 
        if (codeAddr == 0) { codeAddr = CONF_ADDR_COUNT; }
        codeAddr--;
        break;
        
    case SETCONFIG_KEY_UP: //increment value
        incValue(cfgBoundary[codeAddr].step);
        break;
        
    case SETCONFIG_KEY_UP_LONG: //increment value more
        incValue(cfgBoundary[codeAddr].step * 100);
        break;
        
    case SETCONFIG_KEY_DOWN: //decrement value
        decValue(cfgBoundary[codeAddr].step);
        break;

    case SETCONFIG_KEY_DOWN_LONG: //decrement value
        decValue(cfgBoundary[codeAddr].step * 100);
        break;
    }
}