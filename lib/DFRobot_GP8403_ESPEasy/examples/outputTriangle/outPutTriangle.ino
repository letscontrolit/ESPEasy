/*!
 * @file outPutTriangle.ino
 * @brief Use DAC to output triangular wave.
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
}

void loop(){
  /**
   * @brief Call the function to output triangle wave
   * @param amp Set triangle wave amplitude Vp, 10V range 0-5000, 5V range 0-2500
   * @param freq Set triangle wave frequency f, range 0-100
   * @param offset Set triangle wave DC offset Voffset, 10V range 0-5000, 5V range 0-2500
   * @param dutyCycle Set triangle(sawtooth) wave duty cycle, range 0-100
   * @param channel Output channel. 0: channel 0; 1: channel 1; 2: all the channels
   */
 dac.outputTriangle(5000, 10, 0, 50, 0);
}
