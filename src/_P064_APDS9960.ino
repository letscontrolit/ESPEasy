#ifdef USES_P064
//#######################################################################################################
//#################################### Plugin 064: APDS9960 Gesture ##############################
//#######################################################################################################

// ESPEasy Plugin to scan a gesture, proximity and light chip APDS9960
// written by Jochen Krapf (jk@nerd2nerd.org)

// A new gesture is send immediately to controllers.
// Proximity and Light are scanned frequently by given 'Delay' setting.
// RGB is not scanned because there are only 4 vars per task.

// Known BUG: While performing a gesture the reader function blocks rest of ESPEasy processing!!! (Feel free to fix...)

// Note: The chip has a wide view-of-angle. If housing is in this angle the chip blocks!



#define PLUGIN_064
#define PLUGIN_ID_064         64
#define PLUGIN_NAME_064       "Gesture - APDS9960 [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_064 "Gesture"
#define PLUGIN_VALUENAME2_064 "Proximity"
#define PLUGIN_VALUENAME3_064 "Light"
/*
#define PLUGIN_VALUENAME4_064 "R"
#define PLUGIN_VALUENAME5_064 "G"
#define PLUGIN_VALUENAME6_064 "B"
*/

#include <SparkFun_APDS9960.h>   //Lib is modified to work with ESP

#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif

SparkFun_APDS9960* PLUGIN_064_pds = NULL;


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
        Device[deviceCount].ValueCount = 3;
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
        /*
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_064));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[5], PSTR(PLUGIN_VALUENAME6_064));
        */
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte addr = 0x39;   // CONFIG(0); chip has only 1 address

        int optionValues[1] = { 0x39 };
        addFormSelectorI2C(F("i2c_addr"), 1, optionValues, addr);  //Only for display I2C address

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        //CONFIG(0) = getFormItemInt(F("i2c_addr"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (PLUGIN_064_pds)
          delete PLUGIN_064_pds;
        PLUGIN_064_pds = new SparkFun_APDS9960();

        String log = F("APDS : ");
        if ( PLUGIN_064_pds->init() )
        {
          log += F("Init");

          PLUGIN_064_pds->enablePower();

          if (! PLUGIN_064_pds->enableLightSensor(false))
            log += F(" - Error during light sensor init!");
          if (! PLUGIN_064_pds->enableProximitySensor(false))
            log += F(" - Error during proximity sensor init!");

          if (! PLUGIN_064_pds->enableGestureSensor(false))
            log += F(" - Error during gesture sensor init!");
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
        if (!PLUGIN_064_pds)
          break;

        if ( !PLUGIN_064_pds->isGestureAvailable() )
          break;

        int gesture = PLUGIN_064_pds->readGesture();

        //int gesture = PLUGIN_064_pds->readGestureNonBlocking();

        //if (gesture == -1) serialPrint(".");
        //if (gesture == -2) serialPrint(":");
        //if (gesture == -3) serialPrint("|");

        //if ( 0 && PLUGIN_064_pds->isGestureAvailable() )
        if (gesture >= 0)
        {
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
          log += " (";
          log += gesture;
          log += ')';

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
        if (!PLUGIN_064_pds)
          break;

        // Gesture - work is done in PLUGIN_FIFTY_PER_SECOND

        if (1)
        {
          uint8_t proximity_data = 0;
          PLUGIN_064_pds->readProximity(proximity_data);
          UserVar[event->BaseVarIndex + 1] = (float)proximity_data;

          uint16_t ambient_light = 0;
          PLUGIN_064_pds->readAmbientLight(ambient_light);
          UserVar[event->BaseVarIndex + 2] = (float)ambient_light;

          /*
          uint16_t red_light = 0;
          uint16_t green_light = 0;
          uint16_t blue_light = 0;
          PLUGIN_064_pds->readRedLight(red_light);
          PLUGIN_064_pds->readGreenLight(green_light);
          PLUGIN_064_pds->readBlueLight(blue_light);
          UserVar[event->BaseVarIndex + 3] = (float)red_light;
          UserVar[event->BaseVarIndex + 4] = (float)green_light;
          UserVar[event->BaseVarIndex + 5] = (float)blue_light;
          */
        }

        success = true;
        break;
      }

  }
  return success;
}

#endif // USES_P064
