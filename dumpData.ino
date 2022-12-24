
// dump persistent data from eeprom to the serial console


void dumpEpromToSerial() {
  lcd.clear();
  lcd.write("dump data to");
  lcd.setCursor(0,1);
  lcd.write("  serial port..");
  Serial.println("totalWh = " + String(storage.totalWh) + ";");
  Serial.println(F("days_cWh[7] = {"));
  delay(10);
  for (int i=0; i<7; i++) {
    Serial.println(String(storage.days_cWh[i]) + ",");
    delay(1);
  }
  Serial.println(F("};\nmonths_cWh[13] = {"));
  for (int i=0; i<13; i++) {
    Serial.println(String(storage.months_cWh[i]) + ",");
    delay(1);
  }
  Serial.println(F("};\ndayOfYear_Totals = {"));
  for (int i=0; i<367; i++) {
    Serial.println(String(i) + ": " + String(getDayOfYearTotal(i)) + ",");
    delay(1);
  }
  Serial.println("};");

}
   
