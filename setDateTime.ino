
byte setIdx = 0;
const byte cursPos[5] = {1, 4, 9, 12, 15};
bool setInitialized = false;
bool refresh = true;
bool isActiveSetTime = false; 

void start_setDateTime() {
  isActiveSetTime = true; 
}

void end_setDateTime() {
  isActiveSetTime = false; 
}

void display_setDateTime() {
  if (!setInitialized) {
    lcd.cursor();
    lcd.blink();
  }
  if (refresh) {
    lcd.setCursor(0,0);
    lcd.print("Set date ...  " +  tmh.getDowName(0)) ;
    lcd.setCursor(0,1);
    /// pos        1  4    9  c  f
    /// show     "dd.mm.yyyy hh:mm" the :ss is cut of
    lcd.print(tmh.getDayMonYear()+ " " + tmh.getHrsMinSec());
    lcd.setCursor(cursPos[setIdx], 1);
  }
}

void handleKeystroke_setDateTime() {
  
  switch (kbdValue) 
    {
    case 128:   // 0 long
      // finish
      lcd.setCursor(0,1);
      lcd.print("saved  ...  ");
      delay(200);
      break;
      
    default:
      break;
    
    }
}
  
