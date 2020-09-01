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


#include <Adafruit_NeoPixel.h>
#include "_Plugin_Helper.h"

#define NUMBER_LEDS      60			//number of LED in the strip

struct P070_data_struct : public PluginTaskData_base {

  P070_data_struct() {}

  ~P070_data_struct() { reset(); }

  void reset() {
    if (Plugin_070_pixels != nullptr) {
      delete Plugin_070_pixels;
      Plugin_070_pixels = nullptr;
    }
  }

  void init(struct EventStruct *event) {
    if (!Plugin_070_pixels)
    {
      Plugin_070_pixels = new (std::nothrow) Adafruit_NeoPixel(NUMBER_LEDS, CONFIG_PIN1, NEO_GRB + NEO_KHZ800);
      if (Plugin_070_pixels == nullptr) {
        return;
      }
      Plugin_070_pixels->begin(); // This initializes the NeoPixel library.
    }
    set(event);
  }

  void set(struct EventStruct *event) {
    display_enabled = PCONFIG(0);
    brightness = PCONFIG(1);
    brightness_hour_marks = PCONFIG(2);
    offset_12h_mark = PCONFIG(3);
    thick_12_mark = PCONFIG(4);
  }



  void Clock_update()
  {
    clearClock();			//turn off the LEDs
    if (display_enabled > 0) {		//if the display is enabled, calculate the LEDs to turn on
      int Hours = node_time.hour();
      int Minutes = node_time.minute();
      int Seconds = node_time.second();
      timeToStrip(Hours, Minutes, Seconds);
    }
    Plugin_070_pixels->show(); // This sends the updated pixel color to the hardware.
  }

  void calculateMarks()
  { //generate a list of the LEDs that have hour marks
    for (int i = 0; i < 12; i++) {
      marks[i] = 5 * i + (offset_12h_mark % 5);
    }
    if (thick_12_mark) {
      if (offset_12h_mark == 0) {
        marks[12] = 1;
        marks[13] = 59;
      }
      else if (offset_12h_mark == 59) {
        marks[12] = 0;
        marks[13] = 58;
      }
      else {
        marks[12] = offset_12h_mark + 1;
        marks[13] = offset_12h_mark - 1;
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
    hours = (hours * 5) + (minutes / 12) + offset_12h_mark; //make the hour hand move each 12 minutes and apply the offset
    if (hours > 59) hours = hours - 60;
    minutes = minutes + offset_12h_mark;	//apply offset to minutes
    if (minutes > 59) minutes = minutes - 60;
    seconds = seconds + offset_12h_mark;	//apply offset to seconds
    if (seconds > 59) seconds = seconds - 60;
    for (int i = 0 ; i < 14; i ++) {	//set the hour marks as white;
      if ((marks[i] != hours) && (marks[i] != minutes) && (marks[i] != seconds) && (marks[i] != 255)) {	//do not draw a mark there is a clock hand in that position
        Plugin_070_pixels->setPixelColor(marks[i], Plugin_070_pixels->Color(brightness_hour_marks, brightness_hour_marks, brightness_hour_marks));
      }
    }
    uint32_t currentColor;
    uint8_t r_val, g_val, b_val;
    for (int i = 0; i < NUMBER_LEDS; i++) {	//draw the clock hands, adding the colors together
      if (i == hours) {	//hours hand is RED
        Plugin_070_pixels->setPixelColor(i, Plugin_070_pixels->Color(brightness, 0, 0));
      }
      if (i == minutes) { //minutes hand is GREEN
        currentColor = Plugin_070_pixels->getPixelColor(i);
        r_val = (uint8_t)(currentColor >> 16);
        Plugin_070_pixels->setPixelColor(i, Plugin_070_pixels->Color(r_val, brightness, 0));
      }
      if (i == seconds) {	//seconds hand is BLUE
        currentColor = Plugin_070_pixels->getPixelColor(i);
        r_val = (uint8_t)(currentColor >> 16);
        g_val = (uint8_t)(currentColor >>  8);
        Plugin_070_pixels->setPixelColor(i, Plugin_070_pixels->Color(r_val, g_val, brightness));
      }
    }
  }

  boolean display_enabled;		// used to enable/disable the display.
  byte brightness;	          // brightness of the clock "hands"
  byte brightness_hour_marks;	// brightness of the hour marks
  byte offset_12h_mark;		    // position of the 12 o'clock LED on the strip
  boolean thick_12_mark;      // thicker marking of the 12h position
  byte marks[14];             // Positions of the hour marks and dials

  Adafruit_NeoPixel * Plugin_070_pixels = nullptr;

};


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
		    event->String1 = formatGpioName_output("LED");
        break;
	  }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormSubHeader(F("Clock configuration"));
        addFormNumericBox(F("12 o'clock LED position"), F("offset"), PCONFIG(3), 0, 59);
        addFormNote(F("Position of the 12 o'clock LED in the strip"));
        addFormCheckBox(F("Thick 12 o'clock mark"), F("thick_12_mark"), PCONFIG(4));
        addFormNote(F("Check to have 3 LEDs marking the 12 o'clock position"));
        addFormCheckBox(F("Clock display enabled"), F("enabled"), PCONFIG(0));
        addFormNote(F("LED activation"));
        addFormNumericBox(F("LED brightness"), F("brightness"), PCONFIG(1), 0, 255);
        addFormNote(F("Brightness level of the H/M/S hands (0-255)"));
        addFormNumericBox(F("Hour mark brightness"), F("marks"), PCONFIG(2), 0, 255);
        addFormNote(F("Brightness level of the hour marks (0-255)"));

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(0) = isFormItemChecked(F("enabled"));
        PCONFIG(1) = getFormItemInt(F("brightness"));
        PCONFIG(2) = getFormItemInt(F("marks"));
        PCONFIG(3) = getFormItemInt(F("offset"));
        PCONFIG(4) = isFormItemChecked(F("thick_12_mark"));
        P070_data_struct* P070_data = static_cast<P070_data_struct*>(getPluginTaskData(event->TaskIndex));
        if (nullptr != P070_data) {
          P070_data->display_enabled = PCONFIG(0);
          P070_data->brightness = PCONFIG(1);
          P070_data->brightness_hour_marks = PCONFIG(2);
          P070_data->offset_12h_mark = PCONFIG(3);
          P070_data->thick_12_mark = PCONFIG(4);
          P070_data->calculateMarks();
        }

        success = true;
        break;
      }

    case PLUGIN_EXIT:
      {
        clearPluginTaskData(event->TaskIndex);
        success = true;
        break;
      }


    case PLUGIN_INIT:
      {
        initPluginTaskData(event->TaskIndex, new (std::nothrow) P070_data_struct());
        P070_data_struct* P070_data = static_cast<P070_data_struct*>(getPluginTaskData(event->TaskIndex));
        if (nullptr == P070_data) {
          return success;
        }
        P070_data->init(event);
        P070_data->calculateMarks();

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

        P070_data_struct* P070_data = static_cast<P070_data_struct*>(getPluginTaskData(event->TaskIndex));
        if (nullptr != P070_data && command == F("clock")) {
          int val_Mode;
          if (validIntFromString(param1, val_Mode)) {
            if (val_Mode > -1 && val_Mode < 2) {
              P070_data->display_enabled = val_Mode;
              PCONFIG(0) = P070_data->display_enabled;
            }
          }
          int val_Bright;
          if (validIntFromString(param2, val_Bright)) {
            if (val_Bright > -1 && val_Bright < 256) {
              P070_data->brightness = val_Bright;
              PCONFIG(1) = P070_data->brightness;
            }
          }
          int val_Marks;
          if (validIntFromString(param3, val_Marks)) {
            if (val_Marks > -1 && val_Marks < 256) {
              P070_data->brightness_hour_marks = val_Marks;
              PCONFIG(2) = P070_data->brightness_hour_marks;
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
        P070_data_struct* P070_data = static_cast<P070_data_struct*>(getPluginTaskData(event->TaskIndex));
        if (nullptr != P070_data) {
          UserVar[event->BaseVarIndex] = display_enabled;
          UserVar[event->BaseVarIndex + 1] = brightness;
          UserVar[event->BaseVarIndex + 2] = brightness_hour_marks;

          success = true;
        }
      }

  }
  return success;
}


#endif // PLUGIN_BUILD_DISABLED
#endif // USES_P070
