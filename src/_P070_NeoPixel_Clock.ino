#ifdef USES_P070
//#######################################################################################################
//#################################### Plugin 070: NeoPixel ring clock #######################################
//#######################################################################################################

//This plugin is disabled, change it to PLUGIN_BUILD_NORMAL to re-enable it.
#ifdef PLUGIN_BUILD_DISABLED

//A clock that uses a strip/ring of 60 WS2812 NeoPixel LEDs as display for a classic clock.
//The hours are RED, the minutes are GREEN, the seconds are BLUE and the hour marks are WHITE.
//The brightness of the clock hands and the hour marks can be set in the device page,
//or can be set by commands. The format is as follows:
//	Clock,<Enabled 1/0>,<Hand brightness 0-255>,<Mark brightness 0-255>


#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif

#include <Adafruit_NeoPixel.h>

#define NUMBER_LEDS      60			//number of LED in the strip

boolean Plugin_070_enabled;		//used to enable/disable the display.
byte Plugin_070_brightness;	//brightness of the clock "hands"
byte Plugin_070_marks;			//brightness of the hour marks
byte Plugin_070_offset;		//position of the 12 o'clock LED on the strip
boolean thick_12_mark;
byte marks[14];

Adafruit_NeoPixel * Plugin_070_pixels;

#define PLUGIN_070
#define PLUGIN_ID_070         70
#define PLUGIN_NAME_070       "Output - NeoPixel Ring Clock [TESTING]"
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
        Device[deviceCount].VType = SENSOR_TYPE_TRIPLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
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

	case PLUGIN_GET_DEVICEGPIONAMES:
	  {
		    event->String1 = F("GPIO &rarr; LED");
        break;
	  }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormSubHeader(F("Clock configuration"));
        addFormNumericBox(F("12 o'clock LED position"), F("offset"), CONFIG(3), 0, 59);
        addFormNote(F("Position of the 12 o'clock LED in the strip"));
        addFormCheckBox(F("Thick 12 o'clock mark"), F("thick_12_mark"), CONFIG(4));
        addFormNote(F("Check to have 3 LEDs marking the 12 o'clock position"));
        addFormCheckBox(F("Clock display enabled"), F("enabled"), CONFIG(0));
        addFormNote(F("LED activation"));
        addFormNumericBox(F("LED brightness"), F("brightness"), CONFIG(1), 0, 255);
        addFormNote(F("Brightness level of the H/M/S hands (0-255)"));
        addFormNumericBox(F("Hour mark brightness"), F("marks"), CONFIG(2), 0, 255);
        addFormNote(F("Brightness level of the hour marks (0-255)"));

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        CONFIG(0) = isFormItemChecked(F("enabled"));
        CONFIG(1) = getFormItemInt(F("brightness"));
        CONFIG(2) = getFormItemInt(F("marks"));
        CONFIG(3) = getFormItemInt(F("offset"));
        CONFIG(4) = isFormItemChecked(F("thick_12_mark"));

        Plugin_070_enabled = CONFIG(0);
        Plugin_070_brightness = CONFIG(1);
        Plugin_070_marks = CONFIG(2);
        Plugin_070_offset = CONFIG(3);
        thick_12_mark = CONFIG(4);

        calculateMarks();

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
        Plugin_070_enabled = CONFIG(0);
        Plugin_070_brightness = CONFIG(1);
        Plugin_070_marks = CONFIG(2);
        Plugin_070_offset = CONFIG(3);
        thick_12_mark = CONFIG(4);

        calculateMarks();

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
        String lowerString=string;
        lowerString.toLowerCase();
        String command = parseString(lowerString, 1);
        String param1 = parseString(lowerString, 2);
        String param2 = parseString(lowerString, 3);
        String param3 = parseString(lowerString, 4);

        if (command == F("clock")) {
          if (param1 != "") {
            int val_Mode = param1.toInt();
            if (val_Mode > -1 && val_Mode < 2) {
              Plugin_070_enabled = val_Mode;
              CONFIG(0) = Plugin_070_enabled;
            }
          }
          if (param2 != "") {
            int val_Bright = param2.toInt();
            if (val_Bright > -1 && val_Bright < 256) {
              Plugin_070_brightness = val_Bright;
              CONFIG(1) = Plugin_070_brightness;
            }
          }
          if (param3 != "") {
            int val_Marks = param3.toInt();
            if (val_Marks > -1 && val_Marks < 256) {
              Plugin_070_marks = val_Marks;
              CONFIG(2) = Plugin_070_marks;
            }
          }
/*        //Command debuging routine
          String log = F("Clock: ");
          addLog(LOG_LEVEL_INFO,log);
          log = F("   Enabled = ");
          log += param1;
          addLog(LOG_LEVEL_INFO,log);
          log = F("   Brightness = ");
          log += param2;
          addLog(LOG_LEVEL_INFO,log);
          log = F("   Marks = ");
          log += param3;
          addLog(LOG_LEVEL_INFO,log);
*/
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

void calculateMarks()
{ //generate a list of the LEDs that have hour marks
  for (int i = 0; i < 12; i++) {
    marks[i] = 5 * i + (Plugin_070_offset % 5);
  }
  if (thick_12_mark) {
    if (Plugin_070_offset == 0) {
      marks[12] = 1;
      marks[13] = 59;
    }
    else if (Plugin_070_offset == 59) {
      marks[12] = 0;
      marks[13] = 58;
    }
    else {
      marks[12] = Plugin_070_offset + 1;
      marks[13] = Plugin_070_offset - 1;
    }
  }
  else {
    marks[12] = 255;
    marks[13] = 255;
  }
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
  for (int i = 0 ; i < 14; i ++) {	//set the hour marks as white;
    if ((marks[i] != hours) && (marks[i] != minutes) && (marks[i] != seconds) && (marks[i] != 255)) {	//do not draw a mark there is a clock hand in that position
      Plugin_070_pixels->setPixelColor(marks[i], Plugin_070_pixels->Color(Plugin_070_marks, Plugin_070_marks, Plugin_070_marks));
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

#endif // PLUGIN_BUILD_DISABLED 
#endif // USES_P070
