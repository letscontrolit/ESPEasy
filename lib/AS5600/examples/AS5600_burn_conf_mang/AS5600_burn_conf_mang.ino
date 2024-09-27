//
//    FILE: AS5600_burn_conf_mang.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo (not tested yet - see issue #38)
//     URL: https://github.com/RobTillaart/AS5600
//
//  WARNING
//  As burning the settings can only be done once this sketch has to be used with care.
//
//  You need to
//  - read the datasheet so you understand what you do
//  - read issue #38 to understand the discussion that lead to this sketch
//  - uncomment burnSettings() in AS5600.h and AS5600.cpp.
//  - adjust settings and MaxAngle in burn_mang() function below ==> line 77++
//  - uncomment line 167 of this sketch
//
//  Examples may use AS5600 or AS5600L devices.
//  Check if your sensor matches the one used in the example.
//  Optionally adjust the code.


#include "AS5600.h"


AS5600 as5600;   //  use default Wire
//  AS5600L as5600;


void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);

  Wire.begin();

  as5600.begin(4);  //  set direction pin.
  as5600.setDirection(AS5600_CLOCK_WISE);  //  default, just be explicit.

  if (as5600.isConnected())
  {
    Serial.println("Connected");
  }
  else
  {
    Serial.println("Failed to connect. Check wires and reboot.");
    while (1);
  }


  Serial.println("\nWARNING  WARNING  WARNING  WARNING  WARNING  WARNING\n");
  Serial.println("This sketch will burn settings to your AS5600.");
  Serial.println("Adjust the settings in the sketch to your needs.");
  Serial.println("Press any key to continue.");
  Serial.println("\nWARNING  WARNING  WARNING  WARNING  WARNING  WARNING\n\n");
  while (Serial.available()) Serial.read();
  while (!Serial.available());
  Serial.read();


  while (Serial.available()) Serial.read();
  Serial.print("Are you sure to burn settings + maxangle? [Y for Yes]");
  while (!Serial.available());
  char c = Serial.read();
  if (c == 'Y')
  {
    burn_mang();
  }

  Serial.println("\nDone..");
}


void loop()
{
}


void burn_mang()
{
  //  ADJUST settings
  const uint16_t POWERMODE = 0;
  const uint16_t HYSTERESIS = 0;
  const uint16_t OUTPUTMODE = 0;
  const uint16_t PWMFREQUENCY = 0;
  const uint16_t SLOWFILTER = 0;
  const uint16_t FASTFILTER = 0;
  const uint16_t WATCHDOG = 0;
  const uint16_t MAXANGLE = 0;

  bool OK = true;
  OK = OK && as5600.setPowerMode(POWERMODE);
  OK = OK && (POWERMODE == as5600.getPowerMode());
  if (OK == false)
  {
    Serial.println("ERROR: POWERMODE.");
    return;
  }

  OK = OK && as5600.setHysteresis(HYSTERESIS);
  OK = OK && (HYSTERESIS == as5600.getHysteresis());
  if (OK == false)
  {
    Serial.println("ERROR: HYSTERESIS");
    return;
  }

  OK = OK && as5600.setOutputMode(OUTPUTMODE);
  OK = OK && (OUTPUTMODE == as5600.getOutputMode());
  if (OK == false)
  {
    Serial.println("ERROR: OUTPUTMODE");
    return;
  }

  OK = OK && as5600.setPWMFrequency(PWMFREQUENCY);
  OK = OK && (PWMFREQUENCY == as5600.getPWMFrequency());
  if (OK == false)
  {
    Serial.println("ERROR: PWMFREQUENCY");
    return;
  }

  OK = OK && as5600.setSlowFilter(SLOWFILTER);
  OK = OK && (SLOWFILTER == as5600.getSlowFilter());
  if (OK == false)
  {
    Serial.println("ERROR: SLOWFILTER");
    return;
  }

  OK = OK && as5600.setFastFilter(FASTFILTER);
  OK = OK && (FASTFILTER == as5600.getFastFilter());
  if (OK == false)
  {
    Serial.println("ERROR: FASTFILTER");
    return;
  }

  OK = OK && as5600.setWatchDog(WATCHDOG);
  OK = OK && (WATCHDOG == as5600.getWatchDog());
  if (OK == false)
  {
    Serial.println("ERROR: WATCHDOG");
    return;
  }

  OK = OK && as5600.setMaxAngle(MAXANGLE);
  OK = OK && (MAXANGLE == as5600.getMaxAngle());
  if (OK == false)
  {
    Serial.println("ERROR: MAXANGLE");
    return;
  }

  Serial.println();
  Serial.println("burning in 5 seconds");
  delay(1000);
  Serial.println("burning in 4 seconds");
  delay(1000);
  Serial.println("burning in 3 seconds");
  delay(1000);
  Serial.println("burning in 2 seconds");
  delay(1000);
  Serial.println("burning in 1 seconds");
  delay(1000);
  Serial.print("burning ...");
  //  uncomment next line
  //  as5600.burnSettings();
  Serial.println(" done.");
  Serial.println("Reboot AS5600 to use the new settings.");
}


//  -- END OF FILE --
