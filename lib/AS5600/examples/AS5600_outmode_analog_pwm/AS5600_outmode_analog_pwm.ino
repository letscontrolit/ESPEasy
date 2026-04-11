//
//    FILE: AS5600_outmode_analog_pwm.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: experimental
//     URL: https://github.com/RobTillaart/AS5600
//
//  connect the OUT pin to the analog port of the processor
//  use a resistor and a capacitor to create a low pass filter
//  so the PWM behave a bit like a analog signal
//
//  alternative one can read the PWM with interrupt pin and
//  determine the duty cycle
//
//  Examples may use AS5600 or AS5600L devices.
//  Check if your sensor matches the one used in the example.
//  Optionally adjust the code.


#include "AS5600.h"


AS5600 as5600;   //  use default Wire


void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);

  Wire.begin();

  as5600.begin(4);  //  set direction pin.
  as5600.setDirection(AS5600_CLOCK_WISE);  //  default, just be explicit.
  as5600.setOutputMode(AS5600_OUTMODE_PWM);
  as5600.setPWMFrequency(AS5600_PWM_920);
}


void loop()
{
  Serial.print(millis());
  Serial.print("\t");
  Serial.print(as5600.readAngle());
  Serial.print("\t");
  Serial.println(analogRead(A0));

  delay(1000);
}


//  -- END OF FILE --
