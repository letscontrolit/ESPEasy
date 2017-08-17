//#######################################################################################################
//#################################### Plugin 070: NeoPixel ring clock #######################################
//#######################################################################################################

//A clock that uses a strip/ring of 60 WS2812 LEDs (NeoPixel)

#ifdef PLUGIN_BUILD_TESTING

#include <Adafruit_NeoPixel.h>

#define NUMBER_LEDS      60			//number of LED in the strip

byte Plugin_070_enabled = 1;		//used to enable/disable the display.
byte Plugin_070_brightness = 64;	//brightness of the clock "hands"
byte Plugin_070_marks = 4;			//brightness of the hour marks
byte Plugin_070_offset = 30;		//position of the 12 o'clock LED on the strip

Adafruit_NeoPixel * Plugin_070_pixels;

#define PLUGIN_070
#define PLUGIN_ID_070         70
#define PLUGIN_NAME_070       "NeoPixel - Ring Clock [TESTING]"
#define PLUGIN_VALUENAME1_070 "Enabled"
#define PLUGIN_VALUENAME2_070 "Brightness"
#define PLUGIN_VALUENAME3_070 "Marks"
boolean Plugin_070(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_070;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].Custom = true;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].TimerOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_070);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_070));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_070));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_070));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        char tmpString[128];
        string += F("<TR><TD>LED pin:<TD>");
        addPinSelect(false, string, "taskdevicepin1", Settings.TaskDevicePin1[event->TaskIndex]);

        sprintf_P(tmpString, PSTR("<TR><TD>LED Offset:<TD><input title=\"Position of the 12 o'clock LED in the strip\" type='text' name='plugin_070_offset' size='3' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][3]);
        string += tmpString;
        sprintf_P(tmpString, PSTR("<TR><TD>Clock display enabled:<TD><input title=\"LED activation\" type='text' name='plugin_070_enabled' size='3' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
        string += tmpString;
        sprintf_P(tmpString, PSTR("<TR><TD>LED brightness:<TD><input title=\"Brightness level of the H/M/S hands (0-255)\" type='text' name='plugin_070_brightness' size='3' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
        string += tmpString;
        sprintf_P(tmpString, PSTR("<TR><TD>Hour mark brightness:<TD><input title=\"Brightness level of the hour marks (0-255)\" type='text' name='plugin_070_marks' size='3' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][2]);
        string += tmpString;
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg(F("plugin_070_enabled"));
        if (plugin1.toInt() > 1) Settings.TaskDevicePluginConfig[event->TaskIndex][0] = 1;	//ignore values greater than 1
        else Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        String plugin2 = WebServer.arg(F("plugin_070_brightness"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = plugin2.toInt();
        String plugin3 = WebServer.arg(F("plugin_070_marks"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = plugin3.toInt();
        String plugin4 = WebServer.arg(F("plugin_070_offset"));
        if (plugin4.toInt() > 59) Settings.TaskDevicePluginConfig[event->TaskIndex][3] = 0;
        else Settings.TaskDevicePluginConfig[event->TaskIndex][3] = plugin4.toInt();

        Plugin_070_enabled = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        Plugin_070_brightness = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        Plugin_070_marks = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        Plugin_070_offset = Settings.TaskDevicePluginConfig[event->TaskIndex][3];

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (!Plugin_070_pixels)
        {
          Plugin_070_pixels = new Adafruit_NeoPixel(NUMBER_LEDS, Settings.TaskDevicePin1[event->TaskIndex], NEO_GRB + NEO_KHZ800);
          Plugin_070_pixels->begin(); // This initializes the NeoPixel library.
        }
        Plugin_070_enabled = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        Plugin_070_brightness = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        Plugin_070_marks = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        Plugin_070_offset = Settings.TaskDevicePluginConfig[event->TaskIndex][3];

        success = true;
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        Clock_update();
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String tmpString  = string;

        if (tmpString.startsWith("Clock,")) {
          int idx1 = tmpString.indexOf(',');
          int idx2 = tmpString.indexOf(',', idx1 + 1);
          int idx3 = tmpString.indexOf(',', idx2 + 1);
          int idx4 = tmpString.indexOf(',', idx3 + 1);
          String val_Mode = tmpString.substring(idx1 + 1, idx2);
          String val_Bright = tmpString.substring(idx2 + 1, idx3);
          String val_Marks = tmpString.substring(idx3 + 1, idx4);

          if (val_Mode != "") {
            if (val_Mode.toInt() > -1 && val_Mode.toInt() < 2) {
              Settings.TaskDevicePluginConfig[event->TaskIndex][0] = val_Mode.toInt();
              Plugin_070_enabled = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
            }
          }
          if (val_Bright != "") {
            if (val_Bright.toInt() > -1 && val_Bright.toInt() < 256) {
              Settings.TaskDevicePluginConfig[event->TaskIndex][1] = val_Bright.toInt();
              Plugin_070_brightness = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
            }
          }
          if (val_Marks != "") {
            if (val_Marks.toInt() > -1 && val_Marks.toInt() < 256) {
              Settings.TaskDevicePluginConfig[event->TaskIndex][2] = val_Marks.toInt();
              Plugin_070_marks = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
            }
          }
          success = true;
        }
        break;
      }

    case PLUGIN_READ:
      {
        UserVar[event->BaseVarIndex] = Plugin_070_enabled;
        UserVar[event->BaseVarIndex + 1] = Plugin_070_brightness;
        UserVar[event->BaseVarIndex + 2] = Plugin_070_marks;

        success = true;
      }

  }
  return success;
}

void Clock_update()
{
  clearClock();			//turn off the LEDs
  if (Plugin_070_enabled > 0) {		//if the display is enabled, calculate the LEDs to turn on
    int Hours = hour();
    int Minutes = minute();
    int Seconds = second();
    timeToStrip(Hours, Minutes, Seconds);
  }
  Plugin_070_pixels->show(); // This sends the updated pixel color to the hardware.
}


void clearClock() {
  for (int i = 0; i < NUMBER_LEDS; i++) {
    Plugin_070_pixels->setPixelColor(i, Plugin_070_pixels->Color(0, 0, 0));
  }
}

void timeToStrip(int hours, int minutes, int seconds) {
  if (hours > 11) hours = hours - 12;
  hours = (hours * 5) + (minutes / 12) + Plugin_070_offset; //make the hour hand move each 12 minutes and apply the offset
  if (hours > 59) hours = hours - 60;
  minutes = minutes + Plugin_070_offset;	//apply offset to minutes
  if (minutes > 59) minutes = minutes - 60;
  seconds = seconds + Plugin_070_offset;	//apply offset to seconds
  if (seconds > 59) seconds = seconds - 60;
  for (int i = 0; i < NUMBER_LEDS; i = i + 5) {	//set the hour marks as white;
    if ((i != hours) && (i != minutes) && (i != seconds)) {	//do not draw a mark there is a clock hand in that position
      Plugin_070_pixels->setPixelColor(i, Plugin_070_pixels->Color(Plugin_070_marks, Plugin_070_marks, Plugin_070_marks));
    }
  }
  uint32_t currentColor;
  uint8_t r_val, g_val, b_val;
  for (int i = 0; i < NUMBER_LEDS; i++) {	//draw the clock hands, adding the colors together
    if (i == hours) {	//hours hand is RED
      Plugin_070_pixels->setPixelColor(i, Plugin_070_pixels->Color(Plugin_070_brightness, 0, 0));
    }
    if (i == minutes) { //minutes hand is GREEN
      currentColor = Plugin_070_pixels->getPixelColor(i);
      r_val = (uint8_t)(currentColor >> 16);
      Plugin_070_pixels->setPixelColor(i, Plugin_070_pixels->Color(r_val, Plugin_070_brightness, 0));
    }
    if (i == seconds) {	//seconds hand is BLUE
      currentColor = Plugin_070_pixels->getPixelColor(i);
      r_val = (uint8_t)(currentColor >> 16);
      g_val = (uint8_t)(currentColor >>  8);
      Plugin_070_pixels->setPixelColor(i, Plugin_070_pixels->Color(r_val, g_val, Plugin_070_brightness));
    }
  }
}

#endif
