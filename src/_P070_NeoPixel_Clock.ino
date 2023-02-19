#include "_Plugin_Helper.h"
#ifdef USES_P070

# include "src/PluginStructs/P070_data_struct.h"

// #######################################################################################################
// #################################### Plugin 070: NeoPixel ring clock #######################################
// #######################################################################################################


// A clock that uses a strip/ring of 60 WS2812 NeoPixel LEDs as display for a classic clock.
// The hours are RED, the minutes are GREEN, the seconds are BLUE and the hour marks are WHITE.
// The brightness of the clock hands and the hour marks can be set in the device page,
// or can be set by commands. The format is as follows:
//	Clock,<Enabled 1/0>,<Hand brightness 0-255>,<Mark brightness 0-255>


# define PLUGIN_070
# define PLUGIN_ID_070         70
# define PLUGIN_NAME_070       "Output - NeoPixel Ring Clock"
# define PLUGIN_VALUENAME1_070 "Enabled"
# define PLUGIN_VALUENAME2_070 "Brightness"
# define PLUGIN_VALUENAME3_070 "Marks"


boolean Plugin_070(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_070;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].GlobalSyncOption   = false;

      // FIXME TD-er: Not sure if access to any existing task data is needed when saving
      Device[deviceCount].ExitTaskBeforeSave = false;
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
      event->String1 = formatGpioName_output(F("LED"));
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
      P070_data_struct *P070_data = static_cast<P070_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P070_data) {
        P070_data->display_enabled       = PCONFIG(0);
        P070_data->brightness            = PCONFIG(1);
        P070_data->brightness_hour_marks = PCONFIG(2);
        P070_data->offset_12h_mark       = PCONFIG(3);
        P070_data->thick_12_mark         = PCONFIG(4);
        P070_data->calculateMarks();
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P070_data_struct());
      P070_data_struct *P070_data = static_cast<P070_data_struct *>(getPluginTaskData(event->TaskIndex));

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
      P070_data_struct *P070_data = static_cast<P070_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P070_data) {
        P070_data->Clock_update();
      }
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      String lowerString = string;
      lowerString.toLowerCase();
      String command = parseString(lowerString, 1);
      String param1  = parseString(lowerString, 2);
      String param2  = parseString(lowerString, 3);
      String param3  = parseString(lowerString, 4);

      P070_data_struct *P070_data = static_cast<P070_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P070_data) && (equals(command, F("clock")))) {
        int val_Mode;

        if (validIntFromString(param1, val_Mode)) {
          if ((val_Mode > -1) && (val_Mode < 2)) {
            P070_data->display_enabled = val_Mode;
            PCONFIG(0)                 = P070_data->display_enabled;
          }
        }
        int val_Bright;

        if (validIntFromString(param2, val_Bright)) {
          if ((val_Bright > -1) && (val_Bright < 256)) {
            P070_data->brightness = val_Bright;
            PCONFIG(1)            = P070_data->brightness;
          }
        }
        int val_Marks;

        if (validIntFromString(param3, val_Marks)) {
          if ((val_Marks > -1) && (val_Marks < 256)) {
            P070_data->brightness_hour_marks = val_Marks;
            PCONFIG(2)                       = P070_data->brightness_hour_marks;
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
      P070_data_struct *P070_data = static_cast<P070_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P070_data) {
        UserVar[event->BaseVarIndex]     = P070_data->display_enabled;
        UserVar[event->BaseVarIndex + 1] = P070_data->brightness;
        UserVar[event->BaseVarIndex + 2] = P070_data->brightness_hour_marks;

        success = true;
      }
    }
  }
  return success;
}

#endif // USES_P070
