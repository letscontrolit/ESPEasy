#include "_Plugin_Helper.h"
#ifdef USES_P074

// #######################################################################################################
// ######################## Plugin 074 TSL2591 I2C Lux/IR Sensor
// #########################################
// #######################################################################################################
//
// by: https://github.com/krikk
// this plugin is based on the adafruit library
// written based on version 1.0.2 from
// https://github.com/adafruit/Adafruit_TSL2591_Library
// does need Adafruit Sensors Library
// added lux calculation improvement
// https://github.com/adafruit/Adafruit_TSL2591_Library/issues/14
// added fix for issue
// https://github.com/adafruit/Adafruit_TSL2591_Library/issues/17

#define PLUGIN_074
#define PLUGIN_ID_074 74
#define PLUGIN_NAME_074 "Light/Lux - TSL2591 [TESTING]"
#define PLUGIN_VALUENAME1_074 "Lux"
#define PLUGIN_VALUENAME2_074 "Full"
#define PLUGIN_VALUENAME3_074 "Visible"
#define PLUGIN_VALUENAME4_074 "IR"

#include "Adafruit_TSL2591.h"
#include <Adafruit_Sensor.h>


struct P074_data_struct : public PluginTaskData_base {
  P074_data_struct() {
    tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier
                                  // (for your use later)
  }

  // Changing the integration time gives you a longer time over which to sense
  // light
  // longer timelines are slower, but are good in very low light situtations!
  void setIntegrationTime(int time) {
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
  void setGain(int gain) {
    switch (gain) {
      default:
      case 0: tsl.setGain(TSL2591_GAIN_LOW);  break; // 1x gain (bright light)
      case 1: tsl.setGain(TSL2591_GAIN_MED);  break; // 25x (Medium)
      case 2: tsl.setGain(TSL2591_GAIN_HIGH); break; // 428x (High)
      case 3: tsl.setGain(TSL2591_GAIN_MAX);  break; // 9876x (Max)
    }
  }

  // Return true when value is present.
  bool getFullLuminosity(uint32_t & value) {
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
          integrationStart = millis();
          duration = 0;
          integrationActive = true;
          startIntegrationNeeded = false;
        }
      }
      return false; // Started integration, so no value possible yet.
    }
    bool finished = false;
    value = tsl.getFullLuminosity(finished);
    duration = timePassedSince(integrationStart);
    if (finished) {
      integrationActive = false;
      integrationStart = 0;
      newValuePresent = true;
    } else {
      if (duration > 1000) {
        // Max integration time is 600 msec, so if we still have no value, reset the current state
        integrationStart = 0;
        integrationActive = false;
        newValuePresent = false;
        startIntegrationNeeded = true; // Apparently a value was needed
      }
    }
    if (!integrationActive) {
      tsl.disable();
    }
    return finished;
  }

  Adafruit_TSL2591 tsl;
  unsigned long integrationStart = 0;
  unsigned long duration = 0;
  bool integrationActive = false;
  bool newValuePresent = false;
  bool startIntegrationNeeded = false;
};

boolean Plugin_074(byte function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_074;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = false;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_074);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_074));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_074));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_074));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_074));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      int optionValues[1] = { TSL2591_ADDR };
      addFormSelectorI2C(F("p074_i2c_addr"), 1, optionValues,
                         TSL2591_ADDR); // Only for display I2C address
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      //        P074_data_struct* P074_data =
      //        static_cast<P074_data_struct*>(getPluginTaskData(event->TaskIndex));
      //        if (nullptr != P074_data) {

      //          P074_data->tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  //
      //          shortest integration time (bright light)
      // P074_data->tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
      // P074_data->tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
      // P074_data->tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
      // P074_data->tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
      // P074_data->tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest
      // integration time (dim light)
      //        }
      {
        String optionsMode[6] = { F("100ms"), F("200ms"), F("300ms"),
                                  F("400ms"), F("500ms"), F("600ms") };
        addFormSelector(F("Integration Time"), F("p074_itime"), 6, optionsMode,
                        NULL, PCONFIG(1));
      }

      //        TSL2591_GAIN_LOW                  = 0x00,    // low gain (1x)
      //        TSL2591_GAIN_MED                  = 0x10,    // medium gain (25x)
      //        TSL2591_GAIN_HIGH                 = 0x20,    // medium gain (428x)
      //        TSL2591_GAIN_MAX                  = 0x30,    // max gain (9876x)
      {
        String optionsGain[4] = { F("low gain (1x)"),      F("medium gain (25x)"),
                                  F("medium gain (428x)"), F("max gain (9876x)") };
        addFormSelector(F("Value Mapping"), F("p074_gain"), 4, optionsGain, NULL,
                        PCONFIG(2));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      // PCONFIG(0) = getFormItemInt(F("p074_i2c_addr"));
      PCONFIG(1) = getFormItemInt(F("p074_itime"));
      PCONFIG(2) = getFormItemInt(F("p074_gain"));

      success = true;
      break;
    }

    case PLUGIN_INIT: {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P074_data_struct());
      P074_data_struct *P074_data =
        static_cast<P074_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P074_data) {
        return success;
      }

      if (P074_data->tsl.begin()) {
        P074_data->setIntegrationTime(PCONFIG(1));
        P074_data->setGain(PCONFIG(2));

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("TSL2591: Address: 0x");
          log += String(TSL2591_ADDR, HEX);
          log += F(": Integration Time: ");
          log += String((P074_data->tsl.getTiming() + 1) * 100, DEC);
          log += F(" ms");

          /* Display the gain and integration time for reference sake */
          log += (F(" Gain: "));

          switch (P074_data->tsl.getGain()) {
            default:
            case TSL2591_GAIN_LOW:  log += F("1x (Low)");     break; // 1x gain (bright light)
            case TSL2591_GAIN_MED:  log += F("25x (Medium)"); break;
            case TSL2591_GAIN_HIGH: log += F("428x (High)");  break;
            case TSL2591_GAIN_MAX:  log += F("9876x (Max)");  break;
          }
          addLog(LOG_LEVEL_INFO, log);
        }
      } else {
        clearPluginTaskData(event->TaskIndex);
        addLog(LOG_LEVEL_ERROR,
               F("TSL2591: No sensor found ... check your wiring?"));
      }

      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND: {
      P074_data_struct *P074_data =
        static_cast<P074_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P074_data) {
        uint32_t fullLuminosity;
        if (P074_data->getFullLuminosity(fullLuminosity)) {
          // TSL2591_FULLSPECTRUM: Reads two byte value from channel 0 (visible + infrared)
          const uint16_t full = (fullLuminosity & 0xFFFF);

          // TSL2591_INFRARED: Reads two byte value from channel 1 (infrared)
          const uint16_t ir =  (fullLuminosity >> 16);

          // TSL2591_VISIBLE: Reads all and subtracts out just the visible!
          const uint16_t visible =  ( (fullLuminosity & 0xFFFF) - (fullLuminosity >> 16));

          const uint16_t lux     = P074_data->tsl.calculateLuxf(full, ir); // get LUX

          UserVar[event->BaseVarIndex + 0] = lux;
          UserVar[event->BaseVarIndex + 1] = full;
          UserVar[event->BaseVarIndex + 2] = visible;
          UserVar[event->BaseVarIndex + 3] = ir;

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("TSL2591: Lux: ");
            log += String(lux);
            log += F(" Full: ");
            log += String(full);
            log += F(" Visible: ");
            log += String(visible);
            log += F(" IR: ");
            log += String(ir);
            log += F(" duration: ");
            log += P074_data->duration;
            addLog(LOG_LEVEL_INFO, log);
          }

          // Update was succesfull, schedule a read.
          Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
        }
      }
      break;
    }

    case PLUGIN_READ: {
      P074_data_struct *P074_data =
        static_cast<P074_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P074_data) {
        // PLUGIN_READ is either triggered by the task interval timer, or re-scheduled 
        // from PLUGIN_TEN_PER_SECOND when there is new data.
        if (P074_data->newValuePresent) {
          // Value was read in the PLUGIN_TEN_PER_SECOND call, just notify controllers.
          P074_data->newValuePresent = false;
          success = true;
        } else {
          if (!P074_data->integrationActive) {
            // No reading in progress and no new value present, so must trigger a new reading.
            P074_data->startIntegrationNeeded = true;
          }
        }
      } else {
        addLog(LOG_LEVEL_ERROR, F("TSL2591: Sensor not initialized!?"));
      }
      break;
    }
  }
  return success;
}

#endif // USES_P074
