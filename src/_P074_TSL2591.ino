#include "_Plugin_Helper.h"
#ifdef USES_P074

# include "src/PluginStructs/P074_data_struct.h"

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

# define PLUGIN_074
# define PLUGIN_ID_074 74
# define PLUGIN_NAME_074 "Light/Lux - TSL2591"
# define PLUGIN_VALUENAME1_074 "Lux"
# define PLUGIN_VALUENAME2_074 "Full"
# define PLUGIN_VALUENAME3_074 "Visible"
# define PLUGIN_VALUENAME4_074 "IR"

boolean Plugin_074(uint8_t function, struct EventStruct *event, String& string) {
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
      Device[deviceCount].PluginStats        = true;
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

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { TSL2591_ADDR };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 1, i2cAddressValues,
                           TSL2591_ADDR); // Only for display I2C address
      } else {
        success = (event->Par1 == TSL2591_ADDR);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = TSL2591_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

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
        const __FlashStringHelper *optionsMode[6] = { F("100"), F("200"), F("300"),
                                                      F("400"), F("500"), F("600") };
        addFormSelector(F("Integration Time"), F("itime"), 6, optionsMode,
                        nullptr, PCONFIG(1));
        addUnit(F("ms"));
      }

      //        TSL2591_GAIN_LOW                  = 0x00,    // low gain (1x)
      //        TSL2591_GAIN_MED                  = 0x10,    // medium gain (25x)
      //        TSL2591_GAIN_HIGH                 = 0x20,    // medium gain (428x)
      //        TSL2591_GAIN_MAX                  = 0x30,    // max gain (9876x)
      {
        const __FlashStringHelper *optionsGain[4] = { F("low gain (1x)"),      F("medium gain (25x)"),
                                                      F("medium gain (428x)"), F("max gain (9876x)") };
        addFormSelector(F("Value Mapping"), F("gain"), 4, optionsGain, nullptr,
                        PCONFIG(2));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      // PCONFIG(0) = getFormItemInt(F("i2c_addr"));
      PCONFIG(1) = getFormItemInt(F("itime"));
      PCONFIG(2) = getFormItemInt(F("gain"));

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
          addLogMove(LOG_LEVEL_INFO, log);
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
          // TSL2591_FULLSPECTRUM: Reads two uint8_t value from channel 0 (visible + infrared)
          const uint16_t full = (fullLuminosity & 0xFFFF);

          // TSL2591_INFRARED: Reads two uint8_t value from channel 1 (infrared)
          const uint16_t ir =  (fullLuminosity >> 16);

          // TSL2591_VISIBLE: Reads all and subtracts out just the visible!
          const uint16_t visible =  ((fullLuminosity & 0xFFFF) - (fullLuminosity >> 16));

          const float lux = P074_data->tsl.calculateLuxf(full, ir); // get LUX

          UserVar[event->BaseVarIndex + 0] = lux;
          UserVar[event->BaseVarIndex + 1] = full;
          UserVar[event->BaseVarIndex + 2] = visible;
          UserVar[event->BaseVarIndex + 3] = ir;

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log;
            log += concat(F("TSL2591: Lux: "), toString(lux));
            log += concat(F(" Full: "), static_cast<int>(full));
            log += concat(F(" Visible: "), static_cast<int>(visible));
            log += concat(F(" IR: "), static_cast<int>(ir));
            log += concat(F(" duration: "), static_cast<int>(P074_data->duration));
            addLogMove(LOG_LEVEL_INFO, log);
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
          success                    = true;
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
