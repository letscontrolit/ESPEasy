#include "../Helpers/Network.h"

#include <ArduinoOTA.h>

#include "../../ESPEasyWifi.h"
#include "../../ESPEasyNetwork.h"
#include "../../ESPEasy_common.h"

#include "../Globals/Settings.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Misc.h"

/********************************************************************************************\
   Status LED
 \*********************************************************************************************/
void statusLED(bool traffic)
{
  static int gnStatusValueCurrent = -1;
  static long int gnLastUpdate    = millis();

  if (Settings.Pin_status_led == -1) {
    return;
  }

  if (gnStatusValueCurrent < 0) {
    pinMode(Settings.Pin_status_led, OUTPUT);
  }

  int nStatusValue = gnStatusValueCurrent;

  if (traffic)
  {
    nStatusValue += STATUS_PWM_TRAFFICRISE; // ramp up fast
  }
  else
  {
    if (NetworkConnected())
    {
      long int delta = timePassedSince(gnLastUpdate);

      if ((delta > 0) || (delta < 0))
      {
        nStatusValue -= STATUS_PWM_NORMALFADE; // ramp down slowly
        nStatusValue  = std::max(nStatusValue, STATUS_PWM_NORMALVALUE);
        gnLastUpdate  = millis();
      }
    }

    // AP mode is active
    else if (WifiIsAP(WiFi.getMode()))
    {
      nStatusValue = ((millis() >> 1) & PWMRANGE_FULL) - (PWMRANGE_FULL >> 2); // ramp up for 2 sec, 3/4 luminosity
    }

    // Disconnected
    else
    {
      nStatusValue = (millis() >> 1) & (PWMRANGE_FULL >> 2); // ramp up for 1/2 sec, 1/4 luminosity
    }
  }

  nStatusValue = constrain(nStatusValue, 0, PWMRANGE_FULL);

  if (gnStatusValueCurrent != nStatusValue)
  {
    gnStatusValueCurrent = nStatusValue;

    long pwm = nStatusValue * nStatusValue; // simple gamma correction
    pwm >>= 10;

    if (Settings.Pin_status_led_Inversed) {
      pwm = PWMRANGE_FULL - pwm;
    }

    #if defined(ESP8266)
    analogWrite(Settings.Pin_status_led, pwm);
    #endif // if defined(ESP8266)
    #if defined(ESP32)
    analogWriteESP32(Settings.Pin_status_led, pwm);
    #endif // if defined(ESP32)
  }
}