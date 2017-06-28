//#######################################################################################################
//#################################### Plugin 064: APDS9960 Gesture ##############################
//#######################################################################################################

// ESPEasy Plugin to scan a ??? chip APDS9960
// written by Jochen Krapf (jk@nerd2nerd.org)


// ScanCode;

#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_064
#define PLUGIN_ID_064         64
#define PLUGIN_NAME_064       "??? Gesture - APDS9960 [TESTING]"
#define PLUGIN_VALUENAME1_064 "Gesture"
#define PLUGIN_VALUENAME2_064 "Proximity"
#define PLUGIN_VALUENAME3_064 "Light"
#define PLUGIN_VALUENAME4_064 "R"
#define PLUGIN_VALUENAME5_064 "G"
#define PLUGIN_VALUENAME6_064 "B"

#include <SparkFun_APDS9960.h>

#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif

SparkFun_APDS9960 apds = SparkFun_APDS9960();


boolean Plugin_064(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_064;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 6;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_064);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[5], PSTR(PLUGIN_VALUENAME6_064));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        //byte addr = CONFIG(0);

        int optionValues[1] = { 0x39 };
        addFormSelectorI2C(string, F("i2c_addr"), 1, optionValues, 0x39);

        //addFormNote(string, F("???"));

        //String options[3] = { F("MCP23017 (Matrix 9x8)"), F("PCF8574 (Matrix 5x4)"), F("PCF8574 (Direct 8)") };
        //addFormSelector(string, F("Chip (Mode)"), F("chip"), 3, options, NULL, CONFIG(1));

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        CONFIG(0) = getFormItemInt(F("i2c_addr"));

        //CONFIG(1) = getFormItemInt(F("chip"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        String log = F("APDS : ");
        if ( apds.init() )
        {
          log += F("Init");
          if (! apds.enableGestureSensor(false))
            log += F(" - Error during gesture sensor init!");
          if (! apds.enableLightSensor(false))
            log += F(" - Error during light sensor init!");
          if (! apds.enableProximitySensor(false))
            log += F(" - Error during proximity sensor init!");
        }
        else
        {
          log += F("Error during APDS-9960 init!");
        }

        // Start running the APDS-9960 gesture sensor engine

        //switch (CONFIG(1))
        {
        }

        addLog(LOG_LEVEL_INFO, log);

        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        Serial.print(".");
        if ( 1 && apds.isGestureAvailable() )
        {
          Serial.print("<");
          byte gesture = apds.readGesture();
          Serial.print(">");

          String log = F("APDS : Gesture=");

          switch ( gesture )
          {
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
          log += F(")");

          UserVar[event->BaseVarIndex] = (float)gesture;
          event->sensorType = SENSOR_TYPE_SWITCH;

          sendData(event);

          addLog(LOG_LEVEL_INFO, log);
        }

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        // Gesture - work is done in PLUGIN_FIFTY_PER_SECOND

        if (1)
        {
          uint8_t proximity_data = 0;
          apds.readProximity(proximity_data);
          UserVar[event->BaseVarIndex + 1] = (float)proximity_data;

          uint16_t ambient_light = 0;
          uint16_t red_light = 0;
          uint16_t green_light = 0;
          uint16_t blue_light = 0;
          apds.readAmbientLight(ambient_light);
          apds.readRedLight(red_light);
          apds.readGreenLight(green_light);
          apds.readBlueLight(blue_light);
          UserVar[event->BaseVarIndex + 2] = (float)ambient_light;
          UserVar[event->BaseVarIndex + 3] = (float)red_light;
          UserVar[event->BaseVarIndex + 4] = (float)green_light;
          UserVar[event->BaseVarIndex + 5] = (float)blue_light;
        }

        success = true;
        break;
      }

  }
  return success;
}

#endif
