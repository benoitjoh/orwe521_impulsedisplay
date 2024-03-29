
// dump persistent data from eeprom to the serial console


void dumpEpromToSerial() {
  lcd.clear();
  lcd.write(F("dump data to"));
  lcd.setCursor(0,1);
  lcd.write(F("  serial port.."));
  Serial.println(F("// dump of configuration and data structure //"));
  Serial.println(F("struct {"));
  Serial.println("long secondsCounter = " + String(storage.secondsCounter) + ";");
  Serial.println("long dayCounter = " + String(storage.dayCounter) + ";");
  Serial.println("unsigned long totalWh = " + String(storage.totalWh) + ";");
  Serial.print(F("unsigned long days_cWh[7] = {"));
  delay(10);
  for (int i=0; i<7; i++) {
    Serial.print(String(storage.days_cWh[i]) + ", ");
    delay(1);
  }
  Serial.print(F("};\nunsigned long months_cWh[13] = {"));
  for (int i=0; i<13; i++) {
    Serial.print(String(storage.months_cWh[i]) + ", ");
    delay(1);
  }
  Serial.print(F("};\nint cfg[CONF_ADDR_COUNT] = {"));
  for (int i=0; i<8; i++) {
    Serial.print(String(storage.cfg[i]) + ", ");
    delay(1);
  }  
  Serial.println(F("};\n} storage;\n"));
  
  Serial.println(F("\n\n ---- dayOfYear_Totals -----"));
  Serial.println(F("day;Wh"));
  for (int i=0; i<367; i++) {
    Serial.println(String(i) + ";" + String(getDayOfYearTotal(i) * 100));
    delay(1);
  }

}
   
