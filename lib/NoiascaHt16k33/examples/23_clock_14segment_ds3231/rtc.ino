/* *******************************************************************
   Real Time Clock
   Datasheet: https://datasheets.maximintegrated.com/en/ds/DS3231.pdf
   ***************************************************************** */

byte decToBcd(byte val)
{
  // Convert normal decimal numbers to binary coded decimal
  return ( (val / 10 * 16) + (val % 10) );
}


byte bcdToDec(byte val)
{
  // Convert binary coded decimal to normal decimal numbers
  return ( (val / 16 * 10) + (val % 16) );
}


void setDS3231time(byte second, byte minute, byte hour)
{
  // sets time to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);                // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour));   // set hours
  Wire.endTransmission();
}


void setDS3231datetime(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);                    // set next input to start at the seconds register
  Wire.write(decToBcd(second));     // set seconds
  Wire.write(decToBcd(minute));     // set minutes
  Wire.write(decToBcd(hour));       // set hours
  Wire.write(decToBcd(dayOfWeek));  // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month));      // set month
  Wire.write(decToBcd(year));       // set year (0 to 99)
  Wire.endTransmission();
}


void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  byte datalen = 7;
  Wire.requestFrom(DS3231_I2C_ADDRESS, datalen);             // request seven bytes of data from DS3231 starting from register 00h
  if (Wire.available()) {
    *second = bcdToDec(Wire.read() & 0b01111111);            // 0x00 seconds
    *minute = bcdToDec(Wire.read() & 0b01111111);            // 0x01 minutes
    *hour = bcdToDec(Wire.read() & 0b00111111);              // 0x02 hours
    *dayOfWeek = bcdToDec(Wire.read() & 0b00000111);         // 0x03 "day" in the datasheet
    *dayOfMonth = bcdToDec(Wire.read() & 0b00111111);        // 0x04 "date" in the datasheet
    *month = bcdToDec(Wire.read() & 0b00011111);             // 0x05 month
    *year = bcdToDec(Wire.read());                           // 0x06 year
  }
  else
  {
    Serial.println(F("\nE071 no data from DS3231"));
  }
}


// set the initial time here:
// DS3231       seconds, minutes, hours, day, date, month, year
//                   ss mm hh w dd mm yy
// setDS3231datetime(00,32,20,3,24,11,15);  // set once time and date
// setDS3231time(0,51,19);                  // if necessaray set the time

float getDS3231temp()
{
  float temp3231 = 0;
  int8_t tMSB; // according datasheet MSB has sign, therefore we are using signed variable
  byte tLSB;
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0x11);   //temp registers (11h-12h) get updated automatically every 64s
  Wire.endTransmission();
  byte datalen = 2;
  Wire.requestFrom(DS3231_I2C_ADDRESS, datalen);
  if (Wire.available())
  {
    tMSB = Wire.read();                // 2's complement int portion
    tLSB = Wire.read();                // fraction portion
    temp3231 = tMSB;                   // includes sign, therefore NO bitmask
    temp3231 += ((tLSB >> 6) * 0.25 ); // only care about bits 7 & 8
  }
  else
  {
    Serial.println(F("\nE102 no data from DS3231"));
  }
  return temp3231;
}
