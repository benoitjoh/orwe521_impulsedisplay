
// dump persistent data from eeprom to the serial console

//    unsigned long totalWh = 0;
//    unsigned long daysWh[7] = {0, 0, 0, 0, 0, 0, 0}; // monday = 0 
//    unsigned long monthsWh[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0}; // jan = 1, dec = 12


void dumpEpromToSerial() {
  lcd.clear();
  lcd.write("dump data to");
  lcd.setCursor(0,1);
  lcd.write("  serial port..");
  Serial.println("totalWh = " + String(storage.totalWh) + ";");
  Serial.println("daysWh[7] = {");
  delay(10);
  for (int i=0; i<7; i++) {
    Serial.println(String(storage.daysWh[i]) + ",");
    delay(1);
  }
  Serial.println("};\nmonthsWh[13] = {");
  for (int i=0; i<13; i++) {
    Serial.println(String(storage.monthsWh[i]) + ",");
    delay(1);
  }
  Serial.println("};\ndayOfYear_Totals = {");
  for (int i=0; i<367; i++) {
    Serial.println(String(getDayOfYearTotal(i)) + ",");
    delay(1);
  }
  Serial.println("};");

}
   
