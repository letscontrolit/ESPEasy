#include "../Commands/Servo.h"

#include "../Commands/Common.h"
#include "../Commands/GPIO.h"
#include "../DataStructs/EventStructCommandWrapper.h"
#include "../DataStructs/PinMode.h"
#include "../DataStructs/PortStatusStruct.h"
#include "../ESPEasyCore/Controller.h"
#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/GlobalMapPortStatus.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/PortStatus.h"

// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter
#ifdef USE_SERVO
# include <Servo.h>
ServoPinMap_t ServoPinMap;
#endif // ifdef USE_SERVO

const __FlashStringHelper * Command_Servo(struct EventStruct *event, const char *Line)
{
#ifdef USE_SERVO

  // GPIO number is stored inside event->Par2 instead of event->Par1 as in all the other commands
  // So needs to reload the tempPortStruct.

  // FIXME TD-er: For now only fixed to "P001" even when it is for internal GPIO pins
  pluginID_t pluginID = PLUGIN_GPIO;

  // Par1: Servo ID (obsolete/unused since 2020/11/22)
  // Par2: GPIO pin
  // Par3: angle 0...180 degree
  if (checkValidPortRange(pluginID, event->Par2)) {
    portStatusStruct tempStatus;
    const uint32_t   key = createKey(pluginID, event->Par2); // WARNING: 'servo' uses Par2 instead of Par1
    // WARNING: operator [] creates an entry in the map if key does not exist
    // So the next command should be part of each command:
    tempStatus = globalMapPortStatus[key];

    String log = F("Servo : GPIO ");
    log += event->Par2;

    // SPECIAL CASE TO ALLOW SERVO TO BE DETATTCHED AND SAVE POWER.
    if (event->Par3 >= 9000) {
      auto it = ServoPinMap.find(event->Par2);

      if (it != ServoPinMap.end()) {
        it->second.detach();
        # ifdef ESP32
          detachLedChannel(event->Par2);
        # endif // ifdef ESP32
        ServoPinMap.erase(it);
      }

      // Set parameters to make sure the port status will be removed.
      tempStatus.task    = 0;
      tempStatus.monitor = 0;
      tempStatus.command = 0;
      savePortStatus(key, tempStatus);
      log += F(" Servo detached");
      addLog(LOG_LEVEL_INFO, log);
      return return_command_success();

    }
    # ifdef ESP32
      // Must keep track of used channels or else cause conflicts with PWM
      int8_t ledChannel = attachLedChannel(event->Par2);
      ServoPinMap[event->Par2].attach(event->Par2, ledChannel);
    # else // ifdef ESP32
      ServoPinMap[event->Par2].attach(event->Par2);
    # endif // ifdef ESP32
    ServoPinMap[event->Par2].write(event->Par3);

    tempStatus.command   = 1; // set to 1 in order to display the status in the PinStatus page
    tempStatus.state     = 1;
    tempStatus.output    = 1;
    tempStatus.dutyCycle = event->Par3;

    // setPinState(PLUGIN_ID_001, event->Par2, PIN_MODE_SERVO, event->Par3);
    tempStatus.mode = PIN_MODE_SERVO;
    savePortStatus(key, tempStatus);
    log += F(" Servo set to ");
    log += event->Par3;
    addLog(LOG_LEVEL_INFO, log);
    SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);

    // SendStatus(event, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par2, log, 0));
    return return_command_success();
  }
    #else // ifdef USE_SERVO
  addLog(LOG_LEVEL_ERROR, F("USE_SERVO not included in build"));
    #endif // USE_SERVO
  return return_command_failed();
}
