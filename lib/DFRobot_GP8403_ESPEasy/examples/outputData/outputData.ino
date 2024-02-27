/*!
 * @file outputData.ino
 * @brief A use example for DAC, execute it to output different values from different channels.
 * @copyright  Copyright (c) 2021 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author [TangJie](jie.tang@dfrobot.com)
 * @version V1.0
 * @date 2022-03-07
 * @url https://github.com/DFRobot/DFRobot_GP8403
 */

#include "DFRobot_GP8403.h"
DFRobot_GP8403 dac(&Wire,0x58);

void setup() {
  Serial.begin(115200);
  while(dac.begin()!=0){
    Serial.println("init error");
    delay(1000);
   }
  Serial.println("init succeed");
  //Set DAC output range
  dac.setDACOutRange(dac.eOutputRange5V);
  //Set the output value for DAC channel 0, range 0-5000
  dac.setDACOutVoltage(2500, 0);
  delay(1000);
  //Store data in the chip
  dac.store();
}

void loop(){
}
