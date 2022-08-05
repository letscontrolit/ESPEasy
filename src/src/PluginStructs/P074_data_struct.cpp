#include "../PluginStructs/P074_data_struct.h"

#ifdef USES_P074


P074_data_struct::P074_data_struct() {
  tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier
                                // (for your use later)
}

// Changing the integration time gives you a longer time over which to sense
// light
// longer timelines are slower, but are good in very low light situtations!
void P074_data_struct::setIntegrationTime(int time) {
  switch (time) {
    default:
    case 0: tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS); break;
    case 1: tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS); break;
    case 2: tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS); break;
    case 3: tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS); break;
    case 4: tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS); break;
    case 5: tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS); break;
  }
}

// You can change the gain on the fly, to adapt to brighter/dimmer light
// situations
void P074_data_struct::setGain(int gain) {
  switch (gain) {
    default:
    case 0: tsl.setGain(TSL2591_GAIN_LOW);  break; // 1x gain (bright light)
    case 1: tsl.setGain(TSL2591_GAIN_MED);  break; // 25x (Medium)
    case 2: tsl.setGain(TSL2591_GAIN_HIGH); break; // 428x (High)
    case 3: tsl.setGain(TSL2591_GAIN_MAX);  break; // 9876x (Max)
  }
}

// Return true when value is present.
bool P074_data_struct::getFullLuminosity(uint32_t& value) {
  value = 0;

  if (newValuePresent) {
    // don't try to read a new value until the last one was processed.
    return false;
  }

  if (!integrationActive) {
    if (startIntegrationNeeded) {
      // Fix to re-set the gain/timing before every read.
      // See https://github.com/letscontrolit/ESPEasy/issues/3347
      if (tsl.begin()) {
        tsl.enable();
        integrationStart       = millis();
        duration               = 0;
        integrationActive      = true;
        startIntegrationNeeded = false;
      }
    }
    return false; // Started integration, so no value possible yet.
  }
  bool finished = false;

  value    = tsl.getFullLuminosity(finished);
  duration = timePassedSince(integrationStart);

  if (finished) {
    integrationActive = false;
    integrationStart  = 0;
    newValuePresent   = true;
  } else {
    if (duration > 1000) {
      // Max integration time is 600 msec, so if we still have no value, reset the current state
      integrationStart       = 0;
      integrationActive      = false;
      newValuePresent        = false;
      startIntegrationNeeded = true; // Apparently a value was needed
    }
  }

  if (!integrationActive) {
    tsl.disable();
  }
  return finished;
}

#endif // ifdef USES_P074
