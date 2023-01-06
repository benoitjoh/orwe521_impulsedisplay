


// ------------------------------------------------------------------------------------------

uint8_t hook[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00010,
  B10100,
  B01000
};

void init_lcd() {
  lcd.begin(16,2);               // initialize the lcd
  lcd.createChar(1, hook);
  lcd.clear();
  lcd.home();                   // go home  

}
