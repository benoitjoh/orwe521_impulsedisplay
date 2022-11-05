



#define SETTIME_KEY_ENTER (20 + KEY_ENTER) 
#define SETTIME_KEY_RIGHT (20 + KEY_RIGHT)  
#define SETTIME_KEY_LEFT  (20 + KEY_LEFT) 
#define SETTIME_KEY_UP    (20 + KEY_UP) 
#define SETTIME_KEY_DOWN  (20 + KEY_DOWN) 


byte settingIdx = 1;
const byte cursPos[6] = {0, 1, 4, 9, 12, 15};
bool setInitialized = false;
bool refresh = true;

void start_setDateTime() {
  displayMode = 99; 
}


void display_setDateTime() {
  if (!setInitialized) {
    lcd.cursor();
    lcd.blink();
  }
  if (refresh) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Set time      " +  tmh.getDowName(0)) ;
    lcd.setCursor(0,1);
    /// pos        1  4    9  c  f
    /// show     "dd.mm.yyyy hh:mm" the :ss is cut of
    lcd.print(tmh.getDayMonYear()+ " " + tmh.getHrsMinSec());
    lcd.setCursor(cursPos[settingIdx], 1);
    refresh = false;

  }
}

void handleKeystroke_setDateTime() {
  refresh = true;
  switch (kbdValue) 
    {
    case SETTIME_KEY_ENTER:   // enter (key 0 in set date time mode
      // finish
      displayMode = 0;
      kbdValue = 255; 
      lcd.noCursor();
      lcd.noBlink();

      storeEEprom();
       break;
    case SETTIME_KEY_RIGHT: // right (key 1 in set date time mode
        settingIdx++;
        if (settingIdx > 5) { settingIdx = 1; }
        break;
    case SETTIME_KEY_LEFT: // left (key 2 in set date time mode
        settingIdx--;
        if (settingIdx < 1) { settingIdx = 5; }
        break;
    case SETTIME_KEY_UP: // up 
        switch (settingIdx) {
            case 1: //day
                tmh.incrementDayCounter(1);
                break;
            case 2: //day
                tmh.incrementMonth();
                break;
            case 3: //day
                tmh.incrementYear();
                break;
            case 4: //day
                tmh.incrementSecondsCounter(3600);
                break;
            case 5: //day
                tmh.incrementSecondsCounter(60);
                break;
        }
        break;
    case SETTIME_KEY_DOWN: // down 
        switch (settingIdx) {
            case 1: //day
                tmh.incrementDayCounter(-1);
                break;
            case 2: //day
                tmh.decrementMonth();
                break;
            case 3: //day
                tmh.decrementYear();
                break;
            case 4: //day
                tmh.incrementSecondsCounter(-3600);
                break;
            case 5: //day
                tmh.incrementSecondsCounter(-60);
                break;
        }
        break;
                    
                    
    default:
      break;
    
    }
}
  
