/**************************************************************************************
  Title : Sessami CAP1114 Button class
  Created by : Kaz Wong
  Date : 7 Dec 2016
  Description : class for handle button state
 ***************************************************************************************/

#include "CAP1114_LED.h"

void Sessami_LED::SetDutyCycle(uint8_t mx, uint8_t mn) {
  SetLEDDC((0x300 | B0), mn);
}

