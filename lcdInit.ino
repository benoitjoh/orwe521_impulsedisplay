


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
uint8_t divise[8] = {
  B00000,
  B00000,
  B00000,
  B00111,
  B00000,
  B00000,
  B00000,
  B00000
};
uint8_t frown[8] = {
  B00000,
  B00000,
  B01010,
  B00000,
  B01110,
  B10001,
  B00000 
};

uint8_t degCentigree[8] = {
  B01000,
  B10100,
  B01000,
  B00011,
  B00100,
  B00100,
  B00100,
  B00011
};
void init_lcd() {
  lcd.begin(16,2);               // initialize the lcd
  lcd.createChar(1, hook);
  lcd.createChar(2, frown);
  lcd.createChar(3, degCentigree);
  lcd.createChar(4, divise);
  lcd.clear();
  lcd.home();                   // go home  

}
