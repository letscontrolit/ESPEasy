#include "_Plugin_Helper.h"
#ifdef USES_P064

// #######################################################################################################
// #################################### Plugin 064: APDS9960 Gesture ##############################
// #######################################################################################################

// ESPEasy Plugin to scan a gesture, proximity and light chip APDS9960
// written by Jochen Krapf (jk@nerd2nerd.org)

// A new gesture is send immediately to controllers.
// Proximity and Light are scanned frequently by given 'Delay' setting.
// RGB is not scanned because there are only 4 vars per task.

// Known BUG: While performing a gesture the reader function blocks rest of ESPEasy processing!!! (Feel free to fix...)
// See below, fixed by dropping out after 32 consecutive loops in reading gesture data.

// Note: The chip has a wide view-of-angle. If housing is in this angle the chip blocks!

// 2022-06-17 tonhuisman: Remove I2C address selector, as there is nothing to choose...
//   Clean up source, avoid (memory) inefficient code
// 2022-03-21 tonhuisman: Attempt to stop the sensor from blocking ESPEasy, by dropping out after 32 loops in reading gesture data
//   This should fix the Known BUG above.
//   Lowered reading gesture data from 50/sec to 10/sec, as it still won't be processed quick enough
//   Change sensor to TESTING from DEVELOPMENT
// 2020-04-25 tonhuisman: Added Plugin Mode setting to switch between Proximity/Ambient Light Sensor or R/G/B Colors.
//   Added settings for Gain (Gesture, Proximity, Ambient Light Sensor), Led Power (Gesture and Proximity/ALS) and Led Boost (Gesture)
//   to allow better tuning for use of the sensor. Also adapted the SparkFun_APDS9960 driver for enabling this.
//   R/G/B Colors mode has it's settings shared with the Gesture/Proximity/ALS as they are the exact same parameters, but with different
// labels only.


# define PLUGIN_064
# define PLUGIN_ID_064             64
# define PLUGIN_NAME_064           "Gesture - APDS9960 [TESTING]"
# define PLUGIN_GPL_VALUENAME1_064 "Gesture"
# define PLUGIN_GPL_VALUENAME2_064 "Proximity"
# define PLUGIN_GPL_VALUENAME3_064 "Light"

# define PLUGIN_RGB_VALUENAME1_064 "R"
# define PLUGIN_RGB_VALUENAME2_064 "G"
# define PLUGIN_RGB_VALUENAME3_064 "B"

# define PLUGIN_MODE_GPL_064       0 // GPL = Gesture/Proximity/(Ambient) Light Sensor mode
# define PLUGIN_MODE_RGB_064       1 // RGB = R/G/B Colors mode

# define P064_MODE                 PCONFIG(1)
# define P064_GGAIN                PCONFIG(2)
# define P064_GLDRIVE              PCONFIG(3)
# define P064_LED_BOOST            PCONFIG(4)
# define P064_PGAIN                PCONFIG(5)
# define P064_AGAIN                PCONFIG(6)
# define P064_LDRIVE               PCONFIG(7)

# define P064_IS_GPL_SENSOR        (P064_MODE == PLUGIN_MODE_GPL_064)
# define P064_IS_RGB_SENSOR        (P064_MODE == PLUGIN_MODE_RGB_064)


# include "src/PluginStructs/P064_data_struct.h"


boolean Plugin_064(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_064;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SWITCH;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_064);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      if (P064_IS_GPL_SENSOR) { // Gesture/Proximity/ALS mode
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_GPL_VALUENAME1_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_GPL_VALUENAME2_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_GPL_VALUENAME3_064));
      } else {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_RGB_VALUENAME1_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_RGB_VALUENAME2_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_RGB_VALUENAME3_064));
      }
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = (event->Par1 == 0x39);

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *optionsPluginMode[2] = {
          F("Gesture/Proximity/Ambient Light Sensor"),
          F("R/G/B Colors") };
        const int optionsPluginModeValues[2] = { PLUGIN_MODE_GPL_064, PLUGIN_MODE_RGB_064 };
        addFormSelector(F("Plugin Mode"), F("mode"), 2, optionsPluginMode, optionsPluginModeValues, P064_MODE, true);
        # ifndef BUILD_NO_DEBUG
        addFormNote(F("After changing Plugin Mode you may want to change the Values names, below."));
        # endif // ifndef BUILD_NO_DEBUG
      }

      if (P064_IS_RGB_SENSOR // R/G/B Colors mode and default Gesture/Proximity/ALS values: Set new default names
          && (strcmp_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_GPL_VALUENAME1_064)) == 0)
          && (strcmp_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_GPL_VALUENAME2_064)) == 0)
          && (strcmp_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_GPL_VALUENAME3_064)) == 0)) {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_RGB_VALUENAME1_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_RGB_VALUENAME2_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_RGB_VALUENAME3_064));

        // Reset values
        UserVar[event->BaseVarIndex + 0] = 0.0f;
        UserVar[event->BaseVarIndex + 1] = 0.0f;
        UserVar[event->BaseVarIndex + 2] = 0.0f;
      }

      if (P064_IS_GPL_SENSOR // Gesture/Proximity/ALS mode and default R/G/B values: Set new default names
          && (strcmp_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_RGB_VALUENAME1_064)) == 0)
          && (strcmp_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_RGB_VALUENAME2_064)) == 0)
          && (strcmp_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_RGB_VALUENAME3_064)) == 0)) {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_GPL_VALUENAME1_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_GPL_VALUENAME2_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_GPL_VALUENAME3_064));

        // Reset values
        UserVar[event->BaseVarIndex + 0] = 0.0f;
        UserVar[event->BaseVarIndex + 1] = 0.0f;
        UserVar[event->BaseVarIndex + 2] = 0.0f;
      }

      {
        // Gain options, multiple gain optionsets in SparkFun_APDS9960.h have the same valueset, so only defined once here
        const __FlashStringHelper *optionsGain[4] = {
          F("1x"),
          F("2x"),
          F("4x (default)"),
          F("8x") };
        const int optionsGainValues[4] = { PGAIN_1X, PGAIN_2X, PGAIN_4X, PGAIN_8X }; // Also used for optionsALSGain

        // Led_Drive options, all Led_Drive optionsets in SparkFun_APDS9960.h have the same valueset, so only defined once here
        const __FlashStringHelper *optionsLedDrive[4] = {
          F("100 mA (default)"),
          F("50 mA"),
          F("25 mA"),
          F("12.5 mA") };
        const int optionsLedDriveValues[4] = { LED_DRIVE_100MA, LED_DRIVE_50MA, LED_DRIVE_25MA, LED_DRIVE_12_5MA };


        String lightSensorGainLabel;
        String lightSensorDriveLabel;

        if (P064_IS_GPL_SENSOR) { // Gesture/Proximity/ALS mode
          addFormSubHeader(F("Gesture parameters"));

          addFormSelector(F("Gesture Gain"),      F("ggain"),   4, optionsGain,     optionsGainValues,     P064_GGAIN);

          addFormSelector(F("Gesture LED Drive"), F("gldrive"), 4, optionsLedDrive, optionsLedDriveValues, P064_GLDRIVE);
          {
            // Gesture Led-boost values
            const __FlashStringHelper *optionsLedBoost[4] = {
              F("100 %"),
              F("150 %"),
              F("200 %"),
              F("300 % (default)") };
            const int optionsLedBoostValues[4] = { LED_BOOST_100, LED_BOOST_150, LED_BOOST_200, LED_BOOST_300 };
            addFormSelector(F("Gesture LED Boost"), F("lboost"), 4, optionsLedBoost, optionsLedBoostValues, P064_LED_BOOST);
          }

          addFormSubHeader(F("Proximity & Ambient Light Sensor parameters"));

          addFormSelector(F("Proximity Gain"), F("pgain"), 4, optionsGain, optionsGainValues, P064_PGAIN);

          lightSensorGainLabel  = F("Ambient Light Sensor Gain");
          lightSensorDriveLabel = F("Proximity & ALS LED Drive");
        } else {
          addFormSubHeader(F("R/G/B Colors parameters"));

          lightSensorGainLabel  = F("Light Sensor Gain");
          lightSensorDriveLabel = F("Light Sensor LED Drive");
        }
        {
          // Ambient Light Sensor Gain options, values are equal to PGAIN values, so again avoid duplication
          const __FlashStringHelper *optionsALSGain[4] = {
            F("1x"),
            F("4x (default)"),
            F("16x"),
            F("64x") };
          addFormSelector(lightSensorGainLabel, F("again"), 4, optionsALSGain, optionsGainValues, P064_AGAIN);
        }
        addFormSelector(lightSensorDriveLabel, F("ldrive"), 4, optionsLedDrive, optionsLedDriveValues, P064_LDRIVE);
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P064_MODE = getFormItemInt(F("mode"));

      if (P064_IS_GPL_SENSOR) {
        P064_GGAIN     = getFormItemInt(F("ggain"));
        P064_GLDRIVE   = getFormItemInt(F("gldrive"));
        P064_LED_BOOST = getFormItemInt(F("lboost"));
        P064_PGAIN     = getFormItemInt(F("pgain"));
      }
      P064_AGAIN  = getFormItemInt(F("again"));
      P064_LDRIVE = getFormItemInt(F("ldrive"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P064_data_struct());
      P064_data_struct *P064_data = static_cast<P064_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P064_data) {
        String log = F("APDS : ");

        if (P064_data->sensor.init(P064_GGAIN, P064_GLDRIVE, P064_PGAIN, P064_AGAIN, P064_LDRIVE)) {
          log += F("Init");

          P064_data->sensor.enablePower();

          if (!P064_data->sensor.enableLightSensor(false)) {
            log += F("Error during light sensor init!");
          }

          if (P064_IS_GPL_SENSOR) { // Gesture/Proximity/ALS mode
            if (!P064_data->sensor.enableProximitySensor(false)) {
              log += F("Error during proximity sensor init!");
            }

            if (!P064_data->sensor.enableGestureSensor(false, P064_LED_BOOST)) {
              log += F("Error during gesture sensor init!");
            }
          }
        } else {
          log += F("Error during APDS-9960 init!");
        }

        addLogMove(LOG_LEVEL_INFO, log);
        success = true;
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P064_data_struct *P064_data = static_cast<P064_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr == P064_data) || (P064_MODE != PLUGIN_MODE_GPL_064) || !P064_data->sensor.isGestureAvailable()) {
        break;
      }

      const int gesture = P064_data->sensor.readGesture();

      if (gesture >= 0) {
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("APDS : Gesture=");

          switch (gesture) {
            case DIR_UP:      log += F("UP");      break;
            case DIR_DOWN:    log += F("DOWN");    break;
            case DIR_LEFT:    log += F("LEFT");    break;
            case DIR_RIGHT:   log += F("RIGHT");   break;
            case DIR_NEAR:    log += F("NEAR");    break;
            case DIR_FAR:     log += F("FAR");     break;
            default:          log += F("NONE");    break;
          }
          log += F(" (");
          log += gesture;
          log += ')';
          addLogMove(LOG_LEVEL_DEBUG, log);
        }
        # endif // ifndef BUILD_NO_DEBUG

        UserVar[event->BaseVarIndex] = static_cast<float>(gesture);
        event->sensorType            = Sensor_VType::SENSOR_TYPE_SWITCH;

        sendData(event); // Process immediately
      }

      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      P064_data_struct *P064_data = static_cast<P064_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P064_data) {
        // Gesture - work is done in PLUGIN_TEN_PER_SECOND

        if (P064_IS_GPL_SENSOR) { // Gesture/Proximity/ALS mode
          uint8_t proximity_data = 0;
          P064_data->sensor.readProximity(proximity_data);
          UserVar[event->BaseVarIndex + 1] = static_cast<float>(proximity_data);

          uint16_t ambient_light = 0;
          P064_data->sensor.readAmbientLight(ambient_light);
          UserVar[event->BaseVarIndex + 2] = static_cast<float>(ambient_light);
        } else {
          uint16_t red_light   = 0;
          uint16_t green_light = 0;
          uint16_t blue_light  = 0;
          P064_data->sensor.readRedLight(red_light);
          P064_data->sensor.readGreenLight(green_light);
          P064_data->sensor.readBlueLight(blue_light);
          UserVar[event->BaseVarIndex + 0] = static_cast<float>(red_light);
          UserVar[event->BaseVarIndex + 1] = static_cast<float>(green_light);
          UserVar[event->BaseVarIndex + 2] = static_cast<float>(blue_light);
        }

        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P064
