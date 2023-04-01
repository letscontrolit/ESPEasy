/*
 * This example shows how to read and write EEPROM data. Try writing
 * then removing power from both devices, commenting out the write, and
 * uploading again.
 */

#include "Adafruit_seesaw.h"

Adafruit_seesaw ss;

void setup() {
  uint8_t eepromval;
  
  Serial.begin(115200);
  
  while (!Serial) delay(10);   // wait until serial port is opened
  
  if(!ss.begin()){
    Serial.println(F("seesaw not found!"));
    while(1) delay(10);
  }
  
  Serial.println(F("seesaw started OK!"));

  Serial.print(F("Initial read from address 0x02...0x"));
  eepromval = ss.EEPROMRead8(0x02);
  Serial.println(eepromval, HEX);

  Serial.println(F("Incrementing value to address 0x02"));
  ss.EEPROMWrite8(0x02, eepromval+1);

  Serial.print(F("Second read from address 0x02...0x"));
  Serial.println(ss.EEPROMRead8(0x02), HEX);
}

void loop() {
  //DONT WRITE EEPROM IN A LOOP!!!! YOU WILL DESTROY YOUR FLASH!!!
}
