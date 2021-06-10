#include "../Helpers/DeepSleep.h"

#include "../../ESPEasy_common.h"
#include "../../ESPEasy-Globals.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyEth.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/ESPEasyRules.h"

#include "../Globals/EventQueue.h"
#include "../Globals/RTC.h"
#include "../Globals/Settings.h"
#include "../Globals/Statistics.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Misc.h"
#include "../Helpers/PeriodicalActions.h"

#include <limits.h>


/**********************************************************
*                                                         *
* Deep Sleep related functions                            *
*                                                         *
**********************************************************/
int getDeepSleepMax()
{
  int dsmax = 4294; // About 71 minutes, limited by hardware

#if defined(CORE_POST_2_5_0)
  dsmax = INT_MAX;

  if ((ESP.deepSleepMax() / 1000000ULL) <= (uint64_t)INT_MAX) {
    dsmax = (int)(ESP.deepSleepMax() / 1000000ULL);
  }
#endif // if defined(CORE_POST_2_5_0)
  return dsmax;
}

bool isDeepSleepEnabled()
{
  if (!Settings.deepSleep_wakeTime) {
    return false;
  }

  // cancel deep sleep loop by pulling the pin GPIO16(D0) to GND
  // recommended wiring: 3-pin-header with 1=RST, 2=D0, 3=GND
  //                    short 1-2 for normal deep sleep / wakeup loop
  //                    short 2-3 to cancel sleep loop for modifying settings
  pinMode(16, INPUT_PULLUP);

  if (!digitalRead(16))
  {
    return false;
  }
  return true;
}

bool readyForSleep()
{
  if (!isDeepSleepEnabled()) {
    return false;
  }

  if (!NetworkConnected()) {
    // Allow 12 seconds to establish connections
    return timeOutReached(timerAwakeFromDeepSleep + 12000);
  }
  return timeOutReached(timerAwakeFromDeepSleep + 1000 * Settings.deepSleep_wakeTime);
}

void prepare_deepSleep(int dsdelay)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("prepare_deepSleep"));
  #endif

  if (!isDeepSleepEnabled())
  {
    // Deep sleep canceled by GPIO16(D0)=LOW
    return;
  }

  // first time deep sleep? offer a way to escape
  if (lastBootCause != BOOT_CAUSE_DEEP_SLEEP)
  {
    addLog(LOG_LEVEL_INFO, F("SLEEP: Entering deep sleep in 30 seconds."));

    if (Settings.UseRules && isDeepSleepEnabled())
    {
      eventQueue.add(F("System#NoSleep=30"));
      while (processNextEvent()) {
        delay(1);
      }
    }
    delayBackground(30000);

    // disabled?
    if (!isDeepSleepEnabled())
    {
      addLog(LOG_LEVEL_INFO, F("SLEEP: Deep sleep cancelled (GPIO16 connected to GND)"));
      return;
    }
  }
  deepSleepStart(dsdelay); // Call deepSleepStart function after these checks
}

void deepSleepStart(int dsdelay)
{
  // separate function that is called from above function or directly from rules, usign deepSleep_wakeTime as a one-shot
  if (Settings.UseRules)
  {
    eventQueue.add(F("System#Sleep"));
    while (processNextEvent()) {
      delay(1);
    }
  }

  addLog(LOG_LEVEL_INFO, F("SLEEP: Powering down to deepsleep..."));
  RTC.deepSleepState = 1;
  prepareShutdown(ESPEasy_Scheduler::IntendedRebootReason_e::DeepSleep);

  #if defined(ESP8266)
    # if defined(CORE_POST_2_5_0)
  uint64_t deepSleep_usec = dsdelay * 1000000ULL;

  if ((deepSleep_usec > ESP.deepSleepMax()) || (dsdelay < 0)) {
    deepSleep_usec = ESP.deepSleepMax();
  }

  if (Settings.UseAlternativeDeepSleep()) {
    // See: https://github.com/esp8266/Arduino/issues/6318#issuecomment-711389479
    #include "c_types.h"
    // system_phy_set_powerup_option:
    // 1 = RF initialization only calibrate VDD33 and Tx power which will take about 18 ms
    // 2 = RF initialization only calibrate VDD33 which will take about 2 ms
    system_phy_set_powerup_option(2); // calibrate only 2ms;
    system_deep_sleep_set_option(static_cast<int>(WAKE_RF_DEFAULT));
    uint32_t*RT= (uint32_t *)0x60000700;
    uint32 t_us = 1.31 * deepSleep_usec;
    {
      RT[4] = 0;
      *RT = 0;
      RT[1]=100;
      RT[3] = 0x10010;
      RT[6] = 8;
      RT[17] = 4;
      RT[2] = 1<<20;
      ets_delay_us(10);
      RT[1]=t_us>>3;
      RT[3] = 0x640C8;
      RT[4]= 0;
      RT[6] = 0x18;
      RT[16] = 0x7F;
      RT[17] = 0x20;
      RT[39] = 0x11;
      RT[40] = 0x03;
      RT[2] |= 1<<20;
      __asm volatile ("waiti 0");
    }
    yield();
  } else {
    ESP.deepSleepInstant(deepSleep_usec, WAKE_RF_DEFAULT);
  }
    # else // if defined(CORE_POST_2_5_0)

  if ((dsdelay > 4294) || (dsdelay < 0)) {
    dsdelay = 4294; // max sleep time ~71 minutes
  }
  ESP.deepSleep((uint32_t)dsdelay * 1000000, WAKE_RF_DEFAULT);
    # endif // if defined(CORE_POST_2_5_0)
  #endif // if defined(ESP8266)
  #if defined(ESP32)
  esp_sleep_enable_timer_wakeup((uint32_t)dsdelay * 1000000);
  esp_deep_sleep_start();
  #endif // if defined(ESP32)
}

