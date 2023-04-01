#include "_Plugin_Helper.h"
#ifdef USES_P112

// #######################################################################################################
// #################### Plugin 112 I2C AS7265X Triad Spectroscopy Sensor and White, IR and UV LED ########
// #######################################################################################################
//
// Triad Spectroscopy Sensor and White, IR and UV LED
// like this one: https://www.sparkfun.com/products/15050
// based on this library: https://github.com/sparkfun/SparkFun_AS7265x_Arduino_Library
// this code is based on 29 Mar 2019-03-29 version of the above library
//
// 2021-03-29 heinemannj: Initial commit
//

# include "src/PluginStructs/P112_data_struct.h"

# define PLUGIN_112
# define PLUGIN_ID_112         112
# define PLUGIN_NAME_112       "Color - AS7265X [DEVELOPMENT]"
# define PLUGIN_VALUENAME1_112 "TempMaster"
# define PLUGIN_VALUENAME2_112 "TempAverage"
# define PLUGIN_VALUENAME3_112 "State"
# define AS7265X_ADDR 0x49

boolean Plugin_112(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_112;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].DecimalsOnly       = true;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].OutputDataType     = Output_Data_type_t::All;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_112);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_112));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_112));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_112));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[1] = { AS7265X_ADDR };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 1, i2cAddressValues, AS7265X_ADDR);
      } else {
        success = (event->Par1 == AS7265X_ADDR);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = AS7265X_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      PCONFIG_LONG(0) = AS7265X_GAIN_37X;               // Set Gain (AS7265X_GAIN_37X) => This is 3.7x
      PCONFIG_LONG(1) = 254;                            // Set Integration Cycles => 254*2.8ms = 711ms per Reading
      PCONFIG(0) = 0;                                   // Blue Status LED On
      PCONFIG(1) = AS7265X_INDICATOR_CURRENT_LIMIT_8MA; // Blue Status LED Current Limit
      PCONFIG(2) = AS7265X_LED_CURRENT_LIMIT_12_5MA;    // White LED Current Limit
      PCONFIG(3) = AS7265X_LED_CURRENT_LIMIT_12_5MA;    // IR LED Current Limit
      PCONFIG(4) = AS7265X_LED_CURRENT_LIMIT_12_5MA;    // UV LED Current Limit
      PCONFIG(5) = 0;                                   // During Measurement turn White, IR and UV LEDs On
      PCONFIG(6) = 1;                                   // Use Calibrated Readings

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        // sensor.setGain(AS7265X_GAIN_1X);  //Default
        // sensor.setGain(AS7265X_GAIN_37X); //This is 3.7x
        // sensor.setGain(AS7265X_GAIN_16X);
        // sensor.setGain(AS7265X_GAIN_64X);
        const __FlashStringHelper *optionsMode[] = {
          F("1x"),
          F("3.7x (default)"),
          F("16x"),
          F("64x"),
        };
        const int optionValuesMode[] = {
          AS7265X_GAIN_1X,
          AS7265X_GAIN_37X,
          AS7265X_GAIN_16X,
          AS7265X_GAIN_64X,
        };
        addFormSelector(F("Gain"), F("Gain"), 4, optionsMode, optionValuesMode, PCONFIG_LONG(0));
      }
      {
        // Integration cycles from 0 (2.78ms) to 255 (711ms)
        // sensor.setIntegrationCycles(49); //Default: 50*2.8ms = 140ms per reading
        // sensor.setIntegrationCycles(1);  //2*2.8ms = 5.6ms per reading
        const __FlashStringHelper *optionsMode2[] = {
          F("2.8 ms"),
          F("28 ms"),
          F("56 ms"),
          F("140 ms"),
          F("280 ms"),
          F("711 ms (default)"),
        };
        const int optionValuesMode2[] = {
          0,
          9,
          19,
          49,
          99,
          254,
        };
        addFormSelector(F("Integration Time"), F("IntegrationTime"), 6, optionsMode2, optionValuesMode2, PCONFIG_LONG(1));
      }
      addFormNote(F("Raw Readings shall not reach the upper limit of 65535 (Sensor Saturation)."));

      addFormSubHeader(F("LED settings"));
      addFormCheckBox(F("Blue"), PCONFIG_LABEL(0), PCONFIG(0));
      addHtml(F(" Status LED On"));
      {
        // sensor.setIndicatorCurrent(AS7265X_INDICATOR_CURRENT_LIMIT_1MA);
        // sensor.setIndicatorCurrent(AS7265X_INDICATOR_CURRENT_LIMIT_2MA);
        // sensor.setIndicatorCurrent(AS7265X_INDICATOR_CURRENT_LIMIT_4MA);
        // sensor.setIndicatorCurrent(AS7265X_INDICATOR_CURRENT_LIMIT_8MA); //Default
        const __FlashStringHelper *optionsMode3[] = {
          F("1 mA"),
          F("2 mA"),
          F("4 mA"),
          F("8 mA (default)"),
        };
        const int optionValuesMode3[] = {
          AS7265X_INDICATOR_CURRENT_LIMIT_1MA,
          AS7265X_INDICATOR_CURRENT_LIMIT_2MA,
          AS7265X_INDICATOR_CURRENT_LIMIT_4MA,
          AS7265X_INDICATOR_CURRENT_LIMIT_8MA,
        };
        addFormSelector(EMPTY_STRING, PCONFIG_LABEL(1), 4, optionsMode3, optionValuesMode3, PCONFIG(1));
      }
      addHtml(F(" Current Limit"));
      addFormNote(F("Activate Status LEDs only for debugging purpose."));

      {
        // White LED has max forward current of 120mA
        // sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_WHITE); //Default
        // sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_25MA, AS7265x_LED_WHITE);   //Allowed
        // sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_50MA, AS7265x_LED_WHITE);   //Allowed
        // sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_100MA, AS7265x_LED_WHITE);  //Allowed
        const __FlashStringHelper *optionsMode4[] = {
          F("12.5 mA (default)"),
          F("25 mA"),
          F("50 mA"),
          F("100 mA"),
        };
        const int optionValuesMode4[] = {
          AS7265X_LED_CURRENT_LIMIT_12_5MA,
          AS7265X_LED_CURRENT_LIMIT_25MA,
          AS7265X_LED_CURRENT_LIMIT_50MA,
          AS7265X_LED_CURRENT_LIMIT_100MA,
        };
        addFormSelector(F("White"), PCONFIG_LABEL(2), 4, optionsMode4, optionValuesMode4, PCONFIG(2));
      }
      addHtml(F(" Current Limit"));

      {
        // IR LED has max forward current of 65mA
        // sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_IR);    //Default
        // sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_25MA, AS7265x_LED_IR);      //Allowed
        // sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_50MA, AS7265x_LED_IR);      //Allowed
        // sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_100MA, AS7265x_LED_IR-bad); //Not allowed
        const __FlashStringHelper *optionsMode5[] = {
          F("12.5 mA (default)"),
          F("25 mA"),
          F("50 mA"),
        };
        const int optionValuesMode5[] = {
          AS7265X_LED_CURRENT_LIMIT_12_5MA,
          AS7265X_LED_CURRENT_LIMIT_25MA,
          AS7265X_LED_CURRENT_LIMIT_50MA,
        };
        addFormSelector(F("IR"), PCONFIG_LABEL(3), 3, optionsMode5, optionValuesMode5, PCONFIG(3));
      }

      {
        // UV LED has max forward current of 30mA so do not set the drive current higher
        // sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_UV);    //Default
        // sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_25MA, AS7265x_LED_UV-bad);  //Not allowed
        // sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_50MA, AS7265x_LED_UV-bad);  //Not allowed
        // sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_100MA, AS7265x_LED_UV-bad); //Not allowed
        const __FlashStringHelper *optionsMode6[] = { F("12.5 mA (default)") };
        const int optionValuesMode6[]             = { AS7265X_LED_CURRENT_LIMIT_12_5MA };
        addFormSelector(F("UV"), PCONFIG_LABEL(4), 1, optionsMode6, optionValuesMode6, PCONFIG(4));
      }
      addFormNote(F("Control Gain and Integration Time after any change to avoid Sensor Saturation!"));

      addFormSubHeader(F("Measurement settings"));
      addFormCheckBox(F("LEDs"), PCONFIG_LABEL(5), PCONFIG(5));
      addHtml(F(" White, IR and UV On"));
      addFormCheckBox(F("Calibrated Readings"), PCONFIG_LABEL(6), PCONFIG(6));
      addFormNote(F("Unchecked (Raw Readings): Use only for the adjustment of Device and LED settings"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG_LONG(0) = getFormItemInt(F("Gain"));
      PCONFIG_LONG(1) = getFormItemInt(F("IntegrationTime"));
      PCONFIG(0)      = isFormItemChecked(PCONFIG_LABEL(0));
      PCONFIG(1)      = getFormItemInt(PCONFIG_LABEL(1));
      PCONFIG(2)      = getFormItemInt(PCONFIG_LABEL(2));
      PCONFIG(3)      = getFormItemInt(PCONFIG_LABEL(3));
      PCONFIG(4)      = getFormItemInt(PCONFIG_LABEL(4));
      PCONFIG(5)      = isFormItemChecked(PCONFIG_LABEL(5));
      PCONFIG(6)      = isFormItemChecked(PCONFIG_LABEL(6));
      success         = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P112_data_struct());
      P112_data_struct *P112_data =
        static_cast<P112_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P112_data) {
        P112_data->initialized = false; // Force re-init just in case the address changed.

        if (P112_data->begin()) {
          addLog(LOG_LEVEL_INFO, F("AS7265X: Found sensor"));

          success = P112_data->initialized;
          P112_data->sensor.setGain(PCONFIG_LONG(0));
          P112_data->sensor.setIntegrationCycles(PCONFIG_LONG(1));

          if (PCONFIG(0)) // Blue Status LED
          {
            P112_data->sensor.enableIndicator();
          } else {
            P112_data->sensor.disableIndicator();
          }
          P112_data->sensor.setIndicatorCurrent(PCONFIG(1));

          P112_data->sensor.disableBulb(AS7265x_LED_WHITE);
          P112_data->sensor.disableBulb(AS7265x_LED_IR);
          P112_data->sensor.disableBulb(AS7265x_LED_UV);
          P112_data->sensor.setBulbCurrent(PCONFIG(2), AS7265x_LED_WHITE);
          P112_data->sensor.setBulbCurrent(PCONFIG(3), AS7265x_LED_IR);
          P112_data->sensor.setBulbCurrent(PCONFIG(4), AS7265x_LED_UV);

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("AS7265X: AMS Device Type: 0x");
            log += P112_data->sensor.getDeviceType();
            addLogMove(LOG_LEVEL_INFO, log);

            log  = F("AS7265X: AMS Hardware Version: 0x");
            log += P112_data->sensor.getHardwareVersion();
            addLogMove(LOG_LEVEL_INFO, log);

            log  = F("AS7265X: AMS Major Firmware Version: 0x");
            log += P112_data->sensor.getMajorFirmwareVersion();
            addLogMove(LOG_LEVEL_INFO, log);

            log  = F("AS7265X: AMS Patch Firmware Version: 0x");
            log += P112_data->sensor.getPatchFirmwareVersion();
            addLogMove(LOG_LEVEL_INFO, log);

            log  = F("AS7265X: AMS Build Firmware Version: 0x");
            log += P112_data->sensor.getBuildFirmwareVersion();
            addLogMove(LOG_LEVEL_INFO, log);
          }

          success = true;
        } else {
          addLog(LOG_LEVEL_INFO, F("AS7265X: No sensor found"));
          success = false;
        }
      }
      break;
    }
    case PLUGIN_TEN_PER_SECOND:
    {
      P112_data_struct *P112_data =
        static_cast<P112_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P112_data) {
        if ((P112_data->sensor.dataAvailable()) or (P112_data->MeasurementStatus >= 1)) {
          P112_data->MeasurementStatus = P112_data->MeasurementStatus + 1;

          // String RuleEvent;
          // RuleEvent  = getTaskDeviceName(event->TaskIndex);
          // RuleEvent += '#';

          switch (P112_data->MeasurementStatus) {
            case 1:
              P112_data->sensor.disableBulb(AS7265x_LED_WHITE);
              P112_data->sensor.disableBulb(AS7265x_LED_IR);
              P112_data->sensor.disableBulb(AS7265x_LED_UV);

              queueEvent(event->TaskIndex, 410, PCONFIG(6) ? P112_data->sensor.getCalibratedA() : P112_data->sensor.getA());
              break;
            case 2:
              queueEvent(event->TaskIndex, 435, PCONFIG(6) ? P112_data->sensor.getCalibratedB() : P112_data->sensor.getB());
              break;
            case 3:
              queueEvent(event->TaskIndex, 460, PCONFIG(6) ? P112_data->sensor.getCalibratedC() : P112_data->sensor.getC());
              break;
            case 4:
              queueEvent(event->TaskIndex, 485, PCONFIG(6) ? P112_data->sensor.getCalibratedD() : P112_data->sensor.getD());
              break;
            case 5:
              queueEvent(event->TaskIndex, 510, PCONFIG(6) ? P112_data->sensor.getCalibratedE() : P112_data->sensor.getE());
              break;
            case 6:
              queueEvent(event->TaskIndex, 535, PCONFIG(6) ? P112_data->sensor.getCalibratedF() : P112_data->sensor.getF());
              break;
            case 7:
              queueEvent(event->TaskIndex, 560, PCONFIG(6) ? P112_data->sensor.getCalibratedG() : P112_data->sensor.getG());
              break;
            case 8:
              queueEvent(event->TaskIndex, 585, PCONFIG(6) ? P112_data->sensor.getCalibratedH() : P112_data->sensor.getH());
              break;
            case 9:
              queueEvent(event->TaskIndex, 610, PCONFIG(6) ? P112_data->sensor.getCalibratedR() : P112_data->sensor.getR());
              break;
            case 10:
              queueEvent(event->TaskIndex, 645, PCONFIG(6) ? P112_data->sensor.getCalibratedI() : P112_data->sensor.getI());
              break;
            case 11:
              queueEvent(event->TaskIndex, 680, PCONFIG(6) ? P112_data->sensor.getCalibratedS() : P112_data->sensor.getS());
              break;
            case 12:
              queueEvent(event->TaskIndex, 705, PCONFIG(6) ? P112_data->sensor.getCalibratedJ() : P112_data->sensor.getJ());
              break;
            case 13:
              queueEvent(event->TaskIndex, 730, PCONFIG(6) ? P112_data->sensor.getCalibratedT() : P112_data->sensor.getT());
              break;
            case 14:
              queueEvent(event->TaskIndex, 760, PCONFIG(6) ? P112_data->sensor.getCalibratedU() : P112_data->sensor.getU());
              break;
            case 15:
              queueEvent(event->TaskIndex, 810, PCONFIG(6) ? P112_data->sensor.getCalibratedV() : P112_data->sensor.getV());
              break;
            case 16:
              queueEvent(event->TaskIndex, 860, PCONFIG(6) ? P112_data->sensor.getCalibratedW() : P112_data->sensor.getW());
              break;
            case 17:
              queueEvent(event->TaskIndex, 900, PCONFIG(6) ? P112_data->sensor.getCalibratedK() : P112_data->sensor.getK());
              break;
            case 18:
              queueEvent(event->TaskIndex, 940, PCONFIG(6) ? P112_data->sensor.getCalibratedL() : P112_data->sensor.getL());

              P112_data->MeasurementStatus     = 0; // FIXME Why is this only executed for case 18?
              UserVar[event->BaseVarIndex + 2] = 0;

              if (PCONFIG(0))                       // Blue Status LED
              {
                P112_data->sensor.enableIndicator();
              }
              break;
          }
        }
      }
      break;
    }
    case PLUGIN_READ:
    {
      P112_data_struct *P112_data =
        static_cast<P112_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P112_data->MeasurementStatus == 0) {
        if (P112_data->begin()) {
          UserVar[event->BaseVarIndex + 2] = 1;

          P112_data->sensor.disableIndicator(); // Blue Status LEDs Off

          if (PCONFIG(5))                       // Measurement With LEDs On
          {
            P112_data->sensor.enableBulb(AS7265x_LED_WHITE);
            P112_data->sensor.enableBulb(AS7265x_LED_IR);
            P112_data->sensor.enableBulb(AS7265x_LED_UV);
          }

          // There are four measurement modes - the datasheet describes it best
          // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
          // sensor.setMeasurementMode(AS7265X_MEASUREMENT_MODE_4CHAN); //Channels STUV on x51
          // sensor.setMeasurementMode(AS7265X_MEASUREMENT_MODE_4CHAN_2); //Channels RTUW on x51
          // sensor.setMeasurementMode(AS7265X_MEASUREMENT_MODE_6CHAN_CONTINUOUS); //All 6 channels on all devices
          // sensor.setMeasurementMode(AS7265X_MEASUREMENT_MODE_6CHAN_ONE_SHOT); //Default: All 6 channels, all devices, just once
          //
          P112_data->sensor.setMeasurementMode(AS7265X_MEASUREMENT_MODE_6CHAN_ONE_SHOT);

          UserVar[event->BaseVarIndex + 0] = P112_data->sensor.getTemperature();
          UserVar[event->BaseVarIndex + 1] = P112_data->sensor.getTemperatureAverage();
        }
      }
      success = true;
      break;
    }
  }
  return success;
}

void queueEvent(taskIndex_t TaskIndex, int wavelength, float value) {
  eventQueue.add(TaskIndex, String(wavelength), toString(value, 2));
}

#endif // USES_P112
