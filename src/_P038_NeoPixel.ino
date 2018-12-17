#ifdef USES_P038
//#######################################################################################################
//#################################### Plugin 038: NeoPixel Basic #######################################
//#######################################################################################################

// 15-June-2017: Fixed broken plugin; tested with several neopixels (single LED, 8 LED bars and 300 leds strips.

// List of commands:
// (1) NeoPixel,<led nr>,<red 0-255>,<green 0-255>,<blue 0-255>
// (2) NeoPixelAll,<red 0-255>,<green 0-255>,<blue 0-255>
// (3) NeoPixelLine,<start led nr>,<stop led nr>,<red 0-255>,<green 0-255>,<blue 0-255>

// Usage:
// (1): Set RGB Color to specified LED number (eg. NeoPixel,5,255,255,255)
// (2): Set all LED to specified color (eg. NeoPixelAll,255,255,255)
//		If you use 'NeoPixelAll' this will off all LED (like NeoPixelAll,0,0,0)
// (3): Set color LED between <start led nr> and <stop led nr> to specified color (eg. NeoPixelLine,1,6,255,255,255)

//RGBW note:
// for RGBW strips append the additional <brightness> to the commands
// eg: NeoPixel,<led nr>,<red 0-255>,<green 0-255>,<blue 0-255>,<brightness 0-255>
// The NeoPixelLine command does not work for RGBW, cause espeasy currently only allows max. 5 parameters

#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel *Plugin_038_pixels;

#define PLUGIN_038
#define PLUGIN_ID_038         38
#define PLUGIN_NAME_038       "Output - NeoPixel (Basic)"
#define PLUGIN_VALUENAME1_038 ""

int MaxPixels = 0;

boolean Plugin_038(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_038;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].Custom = true;
        Device[deviceCount].TimerOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_038);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_038));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        const String options[] = { F("GRB"), F("GRBW") };
        int indices[] = { 1, 2 };

      	addFormNumericBox(F("Led Count"), F("p038_leds"), Settings.TaskDevicePluginConfig[event->TaskIndex][0],1,999);

        // FIXME TD-er: Why isn't this using the normal pin selection functions?
      	addFormPinSelect(F("GPIO"), F("taskdevicepin1"), Settings.TaskDevicePin1[event->TaskIndex]);
        addFormSelector(F("Strip Type"), F("p038_strip"), 2, options, indices, Settings.TaskDevicePluginConfig[event->TaskIndex][1] );

      	success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("p038_leds"));
        MaxPixels = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("p038_strip"));
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (!Plugin_038_pixels)
        {
          byte striptype = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
          if (striptype == 1)
            Plugin_038_pixels = new Adafruit_NeoPixel(Settings.TaskDevicePluginConfig[event->TaskIndex][0], Settings.TaskDevicePin1[event->TaskIndex], NEO_GRB + NEO_KHZ800);
          else if (striptype == 2)
            Plugin_038_pixels = new Adafruit_NeoPixel(Settings.TaskDevicePluginConfig[event->TaskIndex][0], Settings.TaskDevicePin1[event->TaskIndex], NEO_GRBW + NEO_KHZ800);
          else
            Plugin_038_pixels = new Adafruit_NeoPixel(Settings.TaskDevicePluginConfig[event->TaskIndex][0], Settings.TaskDevicePin1[event->TaskIndex], NEO_GRB + NEO_KHZ800);

          Plugin_038_pixels->begin(); // This initializes the NeoPixel library.
        }
        MaxPixels = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        if (Plugin_038_pixels)
        {
          String tmpString  = string;
          int argIndex = tmpString.indexOf(',');
          if (argIndex)
            tmpString = tmpString.substring(0, argIndex);

          if (tmpString.equalsIgnoreCase(F("NeoPixel")))
          {
            // char Line[80];
            // char TmpStr1[80];
            // TmpStr1[0] = 0;
            // string.toCharArray(Line, 80);
            // int Par4 = 0;
            // if (GetArgv(Line, TmpStr1, 5)) Par4 = str2int(TmpStr1);
            Plugin_038_pixels->setPixelColor(event->Par1 - 1, Plugin_038_pixels->Color(event->Par2, event->Par3, event->Par4, event->Par5));
            Plugin_038_pixels->show(); // This sends the updated pixel color to the hardware.
            success = true;
          }

          if (tmpString.equalsIgnoreCase(F("NeoPixelAll")))
				  {
					  // char Line[80];
					  // char TmpStr1[80];
					  // TmpStr1[0] = 0;
					  // string.toCharArray(Line, 80);
					  for (int i = 0; i < MaxPixels; i++)
					  {
                Plugin_038_pixels->setPixelColor(i, Plugin_038_pixels->Color(event->Par1, event->Par2, event->Par3, event->Par4));
					  }
					  Plugin_038_pixels->show();
					  success = true;
          }

          if (tmpString.equalsIgnoreCase(F("NeoPixelLine")))
				  {
					  // char Line[80];
					  // char TmpStr1[80];
					  // TmpStr1[0] = 0;
					  // string.toCharArray(Line, 80);
  					// int Par4 = 0;
	  				// int Par5 = 0;
		  			// if (GetArgv(Line, TmpStr1, 5)) Par4 = str2int(TmpStr1);
			  		// if (GetArgv(Line, TmpStr1, 6)) Par5 = str2int(TmpStr1);
  					for (int i = event->Par1 - 1; i < event->Par2; i++)
	  				{
		  				Plugin_038_pixels->setPixelColor(i, Plugin_038_pixels->Color(event->Par3, event->Par4, event->Par5));
			  		}
				  	Plugin_038_pixels->show();
					  success = true;
          }
        }
        break;
      }

  }
  return success;
}
#endif // USES_P038
