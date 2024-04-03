
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
  Serial.println("unsigned long totaldWh = " + String(storage.totaldWh) + ";");
  Serial.print(F("unsigned long days_dWh[7] = {"));
  delay(10);
  for (int i=0; i<7; i++) {
    Serial.print(String(storage.days_dWh[i]) + ", ");
    delay(1);
  }
  Serial.print(F("};\nunsigned long months_dWh[13] = {"));
  for (int i=0; i<13; i++) {
    Serial.print(String(storage.months_dWh[i]) + ", ");
    delay(1);
  }
  Serial.print(F("};\nint cfg[CONF_ADDR_COUNT] = {"));
  for (int i=0; i<8; i++) {
    Serial.print(String(storage.cfg[i]) + ", ");
    delay(1);
  }  
  Serial.println(F("};\n} storage;\n"));
  
  Serial.println(F("\nhourly values (Wh)\nh;mo;tu;we;th;fr;sa;su"));
  for (int hour=0; hour<24; hour++) {
    Serial.print(String(hour) + ";");
    for (int dow=0; dow<7; dow++) {
      Serial.print(String(storage.hour_dWh[dow][hour]/10) + ";");
    delay(0);
    }
    Serial.println("");
  }
}

