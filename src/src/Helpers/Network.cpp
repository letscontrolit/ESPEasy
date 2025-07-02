#include "../Helpers/Network.h"

#include "../../ESPEasy_common.h"

#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"

#include "../Globals/Settings.h"
#include "../Globals/Services.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Hardware_GPIO.h"
#include "../Helpers/Hardware_PWM.h"
#include "../Helpers/Misc.h"


#ifdef ESP32
# include <esp32-hal-periman.h>
#endif // ifdef ESP32


/********************************************************************************************\
   Status LED
 \*********************************************************************************************/
void statusLED(bool traffic)
{
  static int gnStatusValueCurrent = -1;
  static long int gnLastUpdate    = millis();

  // Reset GPIO when status LED pin has changed to free up LEDC channel
  #ifdef ESP32
  static int8_t last_status_led_pin = -1;
  if (last_status_led_pin != Settings.Pin_status_led) {
    if (last_status_led_pin != -1) {
      set_Gpio_PWM(last_status_led_pin, 0);
    }
    last_status_led_pin = Settings.Pin_status_led;
  }
  #endif

  if (!validGpio(Settings.Pin_status_led)) {
    return;
  }

  #ifdef ESP32
  if (gnStatusValueCurrent < 0) {
    set_Gpio_PWM(Settings.Pin_status_led, 0);
  }
  #endif

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

    set_Gpio_PWM(Settings.Pin_status_led, pwm, 1000);
#ifdef ESP32
    // Need to have this string as C-string, not F-string
    perimanSetPinBusExtraType(Settings.Pin_status_led, "WiFi Status LED");
#endif
  }
}