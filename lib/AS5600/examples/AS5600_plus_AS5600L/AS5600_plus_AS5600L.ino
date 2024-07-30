//    FILE: AS5600_plus_AS5600L.ino
//  AUTHOR: laptophead
// PURPOSE: showing working of a AS5600 and AS5600L side by side
//     URL: https://github.com/RobTillaart/AS5600
//     URL: https://github.com/RobTillaart/AS5600/issues/48


#include "AS5600.h"


AS5600 as5600_R;    //  uses default Wire
AS5600L as5600_L;   //  uses default Wire


int Raw_R;
int Raw_Prev;

float Deg_R,  Deg_L;
float Deg_Prev_R, Deg_Prev_L;

static uint32_t lastTime = 0;


void setup()
{
  Serial.begin(230400);
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);
  
  Wire.begin();
  
  // as5600_L.setAddress(0x40);  //  AS5600L only
  as5600_L.begin(15);  //  set direction pin.
  as5600_L.setDirection(AS5600_CLOCK_WISE);  //
  delay(1000);
  
  as5600_R.begin(14);  //  set direction pin.
  as5600_R.setDirection(AS5600_CLOCK_WISE);  // default, just be explicit.

  delay(1000);
  Serial.print ("Address For AS5600 R ");
  Serial.println(as5600_R.getAddress());
  Serial.print ("Address For AS5600 L ");
  Serial.println(as5600_L.getAddress());

  Serial.print("Connect_R: ");
  Serial.println(as5600_R.isConnected() ? "true" : "false");

  Serial.print("Connect device LEFT: ");
  Serial.println(as5600_L.isConnected() ? "true" : "false");

  delay(1000);
  as5600_R.resetPosition();
  as5600_L.resetPosition();
}


void loop()
{
  Deg_R = convertRawAngleToDegrees(as5600_R.getCumulativePosition());
  Deg_L = convertRawAngleToDegrees(as5600_L.getCumulativePosition());
  //  update every 100 ms
  //  should be enough up to ~200 RPM
  if (millis() - lastTime >= 100 and Deg_R != Deg_Prev_R)
  {
    lastTime = millis();

    Serial.println("REV R: " + String(as5600_R.getRevolutions()));
    Serial.println("REV L: " + String(as5600_L.getRevolutions()));
    Serial.println("DEG R= " + String(Deg_R, 0) + "°" );
    Serial.println("DEG L= " + String(Deg_L, 0) + "°" );
    Deg_Prev_R = Deg_R;
  }

  //  just to show how reset can be used
  if (as5600_R.getRevolutions() >= 1 )
  {
    as5600_R.resetPosition();

    if ( as5600_R.getRevolutions() <= -1 )
    {
      as5600_R.resetPosition();
    }
  }
}


float convertRawAngleToDegrees(uint32_t newAngle)
{
  //  Raw data reports 0 - 4095 segments, which is 0.087890625 of a degree
  float retVal = newAngle * 0.087890625;
  retVal = round(retVal);
  //  retVal=retVal/10;
  return retVal;
}


//  -- END OF FILE --
