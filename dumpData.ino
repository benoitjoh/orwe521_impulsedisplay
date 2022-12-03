
// dump and set eprom data

//    unsigned long totalWh = 0;
//    unsigned long daysWh[7] = {0, 0, 0, 0, 0, 0, 0}; // monday = 0 
//    unsigned long monthsWh[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0}; // jan = 1, dec = 12


void dumpEpromToSerial() {
  lcd.clear();
  lcd.write("dump data to");
  lcd.setCursor(0,1);
  lcd.write("  serial port..");
  Serial.println("total\t" + String(storage.totalWh) + "\nDays:");
  delay(10);
  for (int i=0; i<7; i++) {
    Serial.println(String(i) +"\t" + String(storage.daysWh[i]));
    delay(10);
  }
  Serial.println("Months:");
  for (int i=1; i<13; i++) {
    Serial.println(String(i) +"\t" + String(storage.monthsWh[i]));
    delay(10);
  }
}
   
