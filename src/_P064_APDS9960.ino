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
        Device[deviceCount].ValueCount = 1;
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
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte addr = CONFIG(0);

        int optionValues[1] = { 0x39 };
        addFormSelectorI2C(string, F("i2c_addr"), 1, optionValues, addr);

        addFormNote(string, F("???"));

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
        if ( apds.init() ) {
          Serial.println(F("APDS-9960 initialization complete"));
        } else {
          Serial.println(F("Something went wrong during APDS-9960 init!"));
        }

        // Start running the APDS-9960 gesture sensor engine
        if ( apds.enableGestureSensor(true) ) {
          Serial.println(F("Gesture sensor is now running"));
        } else {
          Serial.println(F("Something went wrong during gesture sensor init!"));
        }

        //switch (CONFIG(1))
        {
        }

        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        static byte lastScanCode = 0xFF;
      	static byte sentScanCode = 0xFF;
        byte actScanCode = 0;

        if ( apds.isGestureAvailable() )
        {
          switch ( apds.readGesture() )
          {
            case DIR_UP:
              Serial.println("UP");
              break;
            case DIR_DOWN:
              Serial.println("DOWN");
              break;
            case DIR_LEFT:
              Serial.println("LEFT");
              break;
            case DIR_RIGHT:
              Serial.println("RIGHT");
              break;
            case DIR_NEAR:
              Serial.println("NEAR");
              break;
            case DIR_FAR:
              Serial.println("FAR");
              break;
            default:
              Serial.println("NONE");
          }
        }

        //switch (CONFIG(1))

      	if (lastScanCode == actScanCode)   // debounced? - two times the same value?
      	{
      		if (sentScanCode != actScanCode)   // any change to last sent data?
      		{
            UserVar[event->BaseVarIndex] = (float)actScanCode;
            event->sensorType = SENSOR_TYPE_SWITCH;

            String log = F("APDS : Gesture=");
            log += String(actScanCode, 10);
            addLog(LOG_LEVEL_INFO, log);

            sendData(event);

      			sentScanCode = actScanCode;
      		}
      	}
      	else
      		lastScanCode = actScanCode;

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        // work is done in PLUGIN_FIFTY_PER_SECOND
        success = true;
        break;
      }

  }
  return success;
}

#endif
