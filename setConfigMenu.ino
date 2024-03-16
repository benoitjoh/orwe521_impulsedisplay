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


// *** config paramters adjustable in configMenu ************************************** //

// defines access values...
#define CONF_TIMECORR_SEC_PER_DAY 0
#define CONF_V1 1
#define CONF_V2 2
#define CONF_V3 3
#define CONF_V4 4
#define CONF_V5 5
#define CONF_V6 6
#define CONF_V7 7

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
    {-255,   255,  1,  "tCor"},
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


void handleKeystroke_setConfigMenu() {
    switch (kbdValue) {
    case SETCONFIG_KEY_ENTER:   // enter (key 0 in set date time mode
        // finish
        displayMode = 0;
        kbdValue = 255; 
        storeEEprom(EE_OFFSET);
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
        newVal = storage.cfg[codeAddr] + cfgBoundary[codeAddr].step;
        if (newVal > cfgBoundary[codeAddr].maxVal) {
            storage.cfg[codeAddr] = cfgBoundary[codeAddr].minVal;
            }
        else {
            storage.cfg[codeAddr] = newVal;
            }
        break;
        
    case SETCONFIG_KEY_DOWN: //decrement value
        newVal = storage.cfg[codeAddr] - cfgBoundary[codeAddr].step;
        if (newVal < cfgBoundary[codeAddr].minVal or newVal > cfgBoundary[codeAddr].maxVal) {
            storage.cfg[codeAddr] = cfgBoundary[codeAddr].maxVal;
            }
        else {
            storage.cfg[codeAddr] = newVal;
            }
        break;
    }
}