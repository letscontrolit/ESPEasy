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

// Note: The chip has a wide view-of-angle. If housing is in this angle the chip blocks!

// 2020-04-25 tonhuisman: Added Plugin Mode setting to switch between Proximity/Ambient Light Sensor or R/G/B Colors.
//   Added settings for Gain (Gesture, Proximity, Ambient Light Sensor), Led Power (Gesture and Proximity/ALS) and Led Boost (Gesture)
//   to allow better tuning for use of the sensor. Also adapted the SparkFun_APDS9960 driver for enabling this.
//   R/G/B Colors mode has it's settings shared with the Gesture/Proximity/ALS as they are the exact same parameters, but with different
// labels only.


#define PLUGIN_064
#define PLUGIN_ID_064             64
#define PLUGIN_NAME_064           "Gesture - APDS9960 [DEVELOPMENT]"
#define PLUGIN_GPL_VALUENAME1_064 "Gesture"
#define PLUGIN_GPL_VALUENAME2_064 "Proximity"
#define PLUGIN_GPL_VALUENAME3_064 "Light"

#define PLUGIN_RGB_VALUENAME1_064 "R"
#define PLUGIN_RGB_VALUENAME2_064 "G"
#define PLUGIN_RGB_VALUENAME3_064 "B"

#define PLUGIN_MODE_GPL_064       0 // GPL = Gesture/Proximity/(Ambient) Light Sensor mode
#define PLUGIN_MODE_RGB_064       1 // RGB = R/G/B Colors mode

#define P064_ADDR                 PCONFIG(0)
#define P064_MODE                 PCONFIG(1)
#define P064_GGAIN                PCONFIG(2)
#define P064_GLDRIVE              PCONFIG(3)
#define P064_LED_BOOST            PCONFIG(4)
#define P064_PGAIN                PCONFIG(5)
#define P064_AGAIN                PCONFIG(6)
#define P064_LDRIVE               PCONFIG(7)

#define P064_IS_GPL_SENSOR        (P064_MODE == PLUGIN_MODE_GPL_064)
#define P064_IS_RGB_SENSOR        (P064_MODE == PLUGIN_MODE_RGB_064)

#include <SparkFun_APDS9960.h> // Lib is modified to work with ESP
#include "_Plugin_Helper.h"

SparkFun_APDS9960 *PLUGIN_064_pds = NULL;

boolean Plugin_064(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_064;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].VType              = SENSOR_TYPE_SWITCH;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = true;
      Device[deviceCount].GlobalSyncOption   = true;
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

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      byte addr = 0x39;                                         // P064_ADDR; chip has only 1 address

      int optionValues[1] = { 0x39 };
      addFormSelectorI2C(F("i2c_addr"), 1, optionValues, addr); // Only for display I2C address
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      String optionsPluginMode[2];
      optionsPluginMode[0] = F("Gesture/Proximity/Ambient Light Sensor");
      optionsPluginMode[1] = F("R/G/B Colors");
      int optionsPluginModeValues[2] = { PLUGIN_MODE_GPL_064, PLUGIN_MODE_RGB_064 };
      addFormSelector(F("Plugin Mode"), F("p064_mode"), 2, optionsPluginMode, optionsPluginModeValues, P064_MODE, true);
      addFormNote(F("After changing Plugin Mode you may want to change the Values names, below."));

      if (P064_IS_RGB_SENSOR // R/G/B Colors mode and default Gesture/Proximity/ALS values: Set new default names
          && (strcmp_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_GPL_VALUENAME1_064)) == 0)
          && (strcmp_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_GPL_VALUENAME2_064)) == 0)
          && (strcmp_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_GPL_VALUENAME3_064)) == 0)) {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_RGB_VALUENAME1_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_RGB_VALUENAME2_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_RGB_VALUENAME3_064));

        // Reset values
        UserVar[event->BaseVarIndex + 0] = 0.0;
        UserVar[event->BaseVarIndex + 1] = 0.0;
        UserVar[event->BaseVarIndex + 2] = 0.0;
      }

      if (P064_IS_GPL_SENSOR // Gesture/Proximity/ALS mode and default R/G/B values: Set new default names
          && (strcmp_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_RGB_VALUENAME1_064)) == 0)
          && (strcmp_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_RGB_VALUENAME2_064)) == 0)
          && (strcmp_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_RGB_VALUENAME3_064)) == 0)) {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_GPL_VALUENAME1_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_GPL_VALUENAME2_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_GPL_VALUENAME3_064));

        // Reset values
        UserVar[event->BaseVarIndex + 0] = 0.0;
        UserVar[event->BaseVarIndex + 1] = 0.0;
        UserVar[event->BaseVarIndex + 2] = 0.0;
      }

      // Gain options, multiple gain optionsets in SparkFun_APDS9960.h have the same valueset, so only defined once here
      String optionsGain[4];
      optionsGain[0] = F("1x");
      optionsGain[1] = F("2x");
      optionsGain[2] = F("4x (default)");
      optionsGain[3] = F("8x");
      int optionsGainValues[4] = { PGAIN_1X, PGAIN_2X, PGAIN_4X, PGAIN_8X }; // Also used for optionsALSGain
      // Ambient Light Sensor Gain options, values are equal to PGAIN values, so again avoid duplication
      String optionsALSGain[4];
      optionsALSGain[0] = F("1x");
      optionsALSGain[1] = F("4x (default)");
      optionsALSGain[2] = F("16x");
      optionsALSGain[3] = F("64x");

      // Led_Drive options, all Led_Drive optionsets in SparkFun_APDS9960.h have the same valueset, so only defined once here
      String optionsLedDrive[4];
      optionsLedDrive[0] = F("100 mA (default)");
      optionsLedDrive[1] = F("50 mA");
      optionsLedDrive[2] = F("25 mA");
      optionsLedDrive[3] = F("12.5 mA");
      int optionsLedDriveValues[4] = { LED_DRIVE_100MA, LED_DRIVE_50MA, LED_DRIVE_25MA, LED_DRIVE_12_5MA };

      // Gesture Led-boost values
      String optionsLedBoost[4];
      optionsLedBoost[0] = F("100 %");
      optionsLedBoost[1] = F("150 %");
      optionsLedBoost[2] = F("200 %");
      optionsLedBoost[3] = F("300 % (default)");
      int optionsLedBoostValues[4] = { LED_BOOST_100, LED_BOOST_150, LED_BOOST_200, LED_BOOST_300 };

      String lightSensorGainLabel;
      String lightSensorDriveLabel;

      if (P064_IS_GPL_SENSOR) { // Gesture/Proximity/ALS mode
        addFormSubHeader(F("Gesture parameters"));

        addFormSelector(F("Gesture Gain"),      F("p064_ggain"),   4, optionsGain,     optionsGainValues,     P064_GGAIN);

        addFormSelector(F("Gesture LED Drive"), F("p064_gldrive"), 4, optionsLedDrive, optionsLedDriveValues, P064_GLDRIVE);

        addFormSelector(F("Gesture LED Boost"), F("p064_lboost"),  4, optionsLedBoost, optionsLedBoostValues, P064_LED_BOOST);

        addFormSubHeader(F("Proximity & Ambient Light Sensor parameters"));

        addFormSelector(F("Proximity Gain"), F("p064_pgain"), 4, optionsGain, optionsGainValues, P064_PGAIN);

        lightSensorGainLabel  = F("Ambient Light Sensor Gain");
        lightSensorDriveLabel = F("Proximity & ALS LED Drive");
      } else {
        addFormSubHeader(F("R/G/B Colors parameters"));

        lightSensorGainLabel  = F("Light Sensor Gain");
        lightSensorDriveLabel = F("Light Sensor LED Drive");
      }
      addFormSelector(lightSensorGainLabel,  F("p064_again"),  4, optionsALSGain,  optionsGainValues,     P064_AGAIN);

      addFormSelector(lightSensorDriveLabel, F("p064_ldrive"), 4, optionsLedDrive, optionsLedDriveValues, P064_LDRIVE);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      // P064_ADDR = getFormItemInt(F("i2c_addr"));

      P064_MODE = getFormItemInt(F("p064_mode"));

      if (P064_IS_GPL_SENSOR) {
        P064_GGAIN     = getFormItemInt(F("p064_ggain"));
        P064_GLDRIVE   = getFormItemInt(F("p064_gldrive"));
        P064_LED_BOOST = getFormItemInt(F("p064_lboost"));
        P064_PGAIN     = getFormItemInt(F("p064_pgain"));
      }
      P064_AGAIN  = getFormItemInt(F("p064_again"));
      P064_LDRIVE = getFormItemInt(F("p064_ldrive"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (PLUGIN_064_pds) {
        delete PLUGIN_064_pds;
      }
      PLUGIN_064_pds = new (std::nothrow) SparkFun_APDS9960();
      if (PLUGIN_064_pds == nullptr) {
        break;
      }

      String log = F("APDS : ");

      if (PLUGIN_064_pds->init(P064_GGAIN, P064_GLDRIVE, P064_PGAIN, P064_AGAIN, P064_LDRIVE))
      {
        log += F("Init");

        PLUGIN_064_pds->enablePower();

        if (!PLUGIN_064_pds->enableLightSensor(false)) {
          log += F(" - Error during light sensor init!");
        }

        if (P064_IS_GPL_SENSOR) { // Gesture/Proximity/ALS mode
          if (!PLUGIN_064_pds->enableProximitySensor(false)) {
            log += F(" - Error during proximity sensor init!");
          }

          if (!PLUGIN_064_pds->enableGestureSensor(false, P064_LED_BOOST)) {
            log += F(" - Error during gesture sensor init!");
          }
        }
      }
      else
      {
        log += F("Error during APDS-9960 init!");
      }

      addLog(LOG_LEVEL_INFO, log);
      success = true;
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      if (!PLUGIN_064_pds) {
        break;
      }

      if ((P064_MODE != PLUGIN_MODE_GPL_064) || !PLUGIN_064_pds->isGestureAvailable()) {
        break;
      }

      int gesture = PLUGIN_064_pds->readGesture();

      // int gesture = PLUGIN_064_pds->readGestureNonBlocking();

      // if (gesture == -1) serialPrint(".");
      // if (gesture == -2) serialPrint(":");
      // if (gesture == -3) serialPrint("|");

      // if ( 0 && PLUGIN_064_pds->isGestureAvailable() )
      if (gesture >= 0)
      {
        String log = F("APDS : Gesture=");

        switch (gesture)
        {
          case DIR_UP:      log += F("UP");      break;
          case DIR_DOWN:    log += F("DOWN");    break;
          case DIR_LEFT:    log += F("LEFT");    break;
          case DIR_RIGHT:   log += F("RIGHT");   break;
          case DIR_NEAR:    log += F("NEAR");    break;
          case DIR_FAR:     log += F("FAR");     break;
          default:          log += F("NONE");    break;
        }
        log += " (";
        log += gesture;
        log += ')';

        UserVar[event->BaseVarIndex] = static_cast<float>(gesture);
        event->sensorType            = SENSOR_TYPE_SWITCH;

        sendData(event);

        addLog(LOG_LEVEL_INFO, log);
      }

      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      if (!PLUGIN_064_pds) {
        break;
      }

      // Gesture - work is done in PLUGIN_FIFTY_PER_SECOND

      if (1)
      {
        if (P064_IS_GPL_SENSOR) { // Gesture/Proximity/ALS mode
          uint8_t proximity_data = 0;
          PLUGIN_064_pds->readProximity(proximity_data);
          UserVar[event->BaseVarIndex + 1] = static_cast<float>(proximity_data);

          uint16_t ambient_light = 0;
          PLUGIN_064_pds->readAmbientLight(ambient_light);
          UserVar[event->BaseVarIndex + 2] = static_cast<float>(ambient_light);
        } else {
          uint16_t red_light   = 0;
          uint16_t green_light = 0;
          uint16_t blue_light  = 0;
          PLUGIN_064_pds->readRedLight(red_light);
          PLUGIN_064_pds->readGreenLight(green_light);
          PLUGIN_064_pds->readBlueLight(blue_light);
          UserVar[event->BaseVarIndex + 0] = static_cast<float>(red_light);
          UserVar[event->BaseVarIndex + 1] = static_cast<float>(green_light);
          UserVar[event->BaseVarIndex + 2] = static_cast<float>(blue_light);
        }
      }

      success = true;
      break;
    }
  }
  return success;
}

#endif // USES_P064
