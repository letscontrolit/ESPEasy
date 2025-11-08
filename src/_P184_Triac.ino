// This plugin detect zerocross and trigger a triac. Used for power electronics.
// Comon uses are AC light dimming or AC Fan speed control. 

#include "_Plugin_Helper.h"

#ifdef USES_P184
#define PLUGIN_184
#define PLUGIN_ID_184     184           // plugin id
#define PLUGIN_NAME_184   "Triac" // "Plugin Name" is what will be dislpayed in the selection list
#define PLUGIN_VALUENAME1_184 "Trigger" // variable output of the plugin. The label is in quotation marks
#define PLUGIN_VALUENAME2_184 "Power" // multiple outputs are supported
// #define PLUGIN_184_DEBUG  false         // set to true for extra log info in the debug
#define P184_OUTPUT_TYPE_INDEX  2


#define P184_60HZ_HALF_WAVE_TIME_US_ONE_PERCENT ((uint32_t)83) // 1/60/2*1%
#define P184_50HZ_HALF_WAVE_TIME_US_ONE_PERCENT ((uint32_t)100) // 1/50/2*1%

#define P184_TRIGGER_CONFIG()      PCONFIG(0)
#define P184_TRIGGER_EDGE_CONFIG() PCONFIG(1)
#define P184_DEAD_ZONE_CONFIG()    PCONFIG(2)
#define P184_FREQ_CONFIG()         PCONFIG(3)

#define P184_ZERO_CROSS_PIN()      PIN(0)
#define P184_TRIGGER_PIN()         PIN(1)

// int8_t P184_TRIGGER_PIN();
// uint8_t p184_trigger;

struct P184_data_struct {
  int8_t   zero_crossing_pin = -1;
  int8_t   trigger_pin      = -1;
  uint8_t  trigger_value    = 0;
  uint8_t  power_value      = 0;
  uint8_t  dead_zone        = 0;
  uint32_t freq_timing_val  = P184_60HZ_HALF_WAVE_TIME_US_ONE_PERCENT;
  uint64_t time_us          = 0;
  hw_timer_t *p184_timer = NULL;
  // hw_timer_t *debounce = NULL;
};

// Lookup table to map Power % (index) to Trigger % (value)
// Generated from the formula: power_ratio = 1 - (t/pi) + sin(2t)/(2pi)
// This avoids floating point math in real-time and allows setting power directly.
const uint8_t power_to_trigger_lut[101] PROGMEM = {
  // Power % (index) -> Trigger % (value)
  100, 87, 82, 78, 75, 73, 71, 69, 67, 66, 64, 63, 61, 60, 59, 58, 57, 56, 55, 54,
   53, 52, 51, 50, 49, 48, 48, 47, 46, 45, 44, 44, 43, 42, 42, 41, 40, 40, 39, 38,
   38, 37, 36, 36, 35, 34, 34, 33, 32, 32, 31, 30, 30, 29, 28, 28, 27, 26, 26, 25,
   24, 24, 23, 22, 22, 21, 20, 20, 19, 18, 18, 17, 16, 16, 15, 14, 13, 13, 12, 11,
   11, 10,  9,  8,  8,  7,  6,  5,  5,  4,  3,  2,  2,  1,  0,  0,  0,  0,  0,  0,
   0 // Power 100% -> Trigger 0%
};

P184_data_struct P184_data;

void IRAM_ATTR P184_zero_crossing() {
  // This is only necessary for debounce
  // detachInterrupt(P184_data.zero_crossing_pin);
  if (P184_data.trigger_pin != -1 && P184_data.p184_timer != NULL) {
    if (P184_data.trigger_value == 0) {
      digitalWrite(P184_data.trigger_pin, HIGH);
    } else {
      digitalWrite(P184_data.trigger_pin, LOW);
      timerRestart(P184_data.p184_timer);
    }
  }
}

void IRAM_ATTR P184_timer_handler() {
  if (P184_data.trigger_pin != -1) {
    if (P184_data.trigger_value != 100) {
      digitalWrite(P184_data.trigger_pin, HIGH);
    }
  }
}

// A plugin has to implement the following function
boolean Plugin_184(uint8_t function, struct EventStruct *event, String& string)
{
  // function: reason the plugin was called
  // event: ??add description here??
  // string: ??add description here??

  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      // This case defines the device characteristics, edit appropriately
      // Attention: dev Values set to 0 or false should be removed to save a few bytes (unneeded assignments)

      auto& dev = Device[++deviceCount];
      dev.Number             = PLUGIN_ID_184;                    // Plugin ID number.   (PLUGIN_ID_184)
      // dev.Type               = DEVICE_TYPE_DUMMY;               // How the device is connected. e.g. DEVICE_TYPE_SINGLE => connected through 1 datapin
      dev.VType              = Sensor_VType::SENSOR_TYPE_DIMMER; // Type of value the plugin will return. e.g. SENSOR_TYPE_STRING
      // dev.Ports              = 0;                                // Port to use when device has multiple I/O pins  (N.B. not used much)
      dev.ValueCount         = 2;                                // The number of output values of a plugin. The value should match the number of keys PLUGIN_VALUENAME1_184
      dev.OutputDataType     = Output_Data_type_t::Simple;      // Subset of selectable output data types  (Default = no selection)
      dev.PullUpOption       = false;                            // Allow to set internal pull-up resistors.
      dev.InverseLogicOption = false;                            // Allow to invert the boolean state (e.g. a switch)
      dev.FormulaOption      = false;                            // Allow to enter a formula to convert values during read. (not possible with Custom enabled)
      dev.Custom             = false;
      dev.SendDataOption     = true;                            // Allow to send data to a controller.
      dev.GlobalSyncOption   = true;                             // No longer used. Was used for ESPeasy values sync between nodes
      dev.TimerOption        = true;                             // Allow to set the "Interval" timer for the plugin.
      dev.TimerOptional      = false;                            // When taskdevice timer is not set and not optional, use default "Interval" delay (Settings.Delay)
      dev.DecimalsOnly       = true;                             // Allow to set the number of decimals (otherwise treated a 0 decimals)
      dev.CustomVTypeVar     = false;                            // Enable to allow the user to configure the Sensor_VType per Value that's available for the plugin
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      // return the device name
      string = F(PLUGIN_NAME_184);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_184));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_184));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      // Called to show non default pin assignment or addresses like for plugins using serial or 1-Wire
      // string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      // This is only called when dev.OutputDataType is not Output_Data_type_t::Default
      // The position in the config parameters used in this example is PCONFIG(P184_OUTPUT_TYPE_INDEX)
      // Must match the one used in case PLUGIN_GET_DEVICEVTYPE  (best to use a define for it)
      // see P026_Sysinfo.ino for more examples.
      event->Par1 = 2; //getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P184_OUTPUT_TYPE_INDEX)));
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      // This is only called when dev.OutputDataType is not Output_Data_type_t::Default
      // The position in the config parameters used in this example is PCONFIG(P184_OUTPUT_TYPE_INDEX)
      // Must match the one used in case PLUGIN_GET_DEVICEVALUECOUNT  (best to use a define for it)
      // IDX is used here to mark the PCONFIG position used to store the Device VType.
      // see _P026_Sysinfo.ino for more examples.
      // event->idx        = P184_OUTPUT_TYPE_INDEX;
      event->sensorType = Sensor_VType::SENSOR_TYPE_DIMMER; //static_cast<Sensor_VType>(PCONFIG(event->idx));
      success           = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      // Set a default config here, which will be called when a plugin is assigned to a task.
      P184_TRIGGER_CONFIG() = 0;
      P184_DEAD_ZONE_CONFIG() = 0;
      P184_ZERO_CROSS_PIN() = -1;
      P184_TRIGGER_PIN() = -1;
      PCONFIG(P184_OUTPUT_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_DIMMER);
      success = true;
      break;
    }


    case PLUGIN_WEBFORM_LOAD:
    {
      addRowLabel(F("Trigger Pin"));
      addPinSelect(PinSelectPurpose::Generic_output, "trigger_pin", P184_TRIGGER_PIN());

      addRowLabel(F("Zero Cross Pin"));
      addPinSelect(PinSelectPurpose::Generic_input, "zero_crossing_pin", P184_ZERO_CROSS_PIN());
      {
        const __FlashStringHelper* optionsEdge[] = { F("RISING"), F("FALLING")};
        const int optionsValsEdge[] = { RISING, FALLING };
        constexpr int optionsCountEdge = NR_ELEMENTS(optionsValsEdge);
        const FormSelectorOptions trigEdgeSelector(optionsCountEdge, optionsEdge, optionsValsEdge);
        trigEdgeSelector.addFormSelector(F("Interrupt mode"), F("trigger_edge"),  P184_TRIGGER_EDGE_CONFIG());
      }
      
      {
        const __FlashStringHelper* optionsFreq[] = { F("60Hz"), F("50Hz")};
        const int optionsValsFreq[] = { P184_60HZ_HALF_WAVE_TIME_US_ONE_PERCENT, P184_50HZ_HALF_WAVE_TIME_US_ONE_PERCENT };
        constexpr int optionsCountFreq = NR_ELEMENTS(optionsValsFreq);
        const FormSelectorOptions freqSelector(optionsCountFreq, optionsFreq, optionsValsFreq);
        freqSelector.addFormSelector(F("Grid frequency"), F("freq"),  P184_FREQ_CONFIG());
      }
      

      addFormNumericBox(F("Trigger"), F("trigger"), P184_TRIGGER_CONFIG(), 0, 100);
      addUnit(F("%"));

      addFormNumericBox(F("Trigger Deadzone"), F("trigger_deadzone"), P184_DEAD_ZONE_CONFIG(), 0, 20);
      addUnit(F("%"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {

      if (P184_data.zero_crossing_pin != -1)
      {
        detachInterrupt(P184_data.zero_crossing_pin);
      }

      P184_TRIGGER_CONFIG() = getFormItemInt(F("trigger"));
      P184_DEAD_ZONE_CONFIG() = getFormItemInt(F("trigger_deadzone"));
      P184_ZERO_CROSS_PIN() = getFormItemInt(F("zero_crossing_pin"));
      P184_TRIGGER_PIN() = getFormItemInt(F("trigger_pin"));
      P184_TRIGGER_EDGE_CONFIG() = getFormItemInt(F("trigger_edge"));
      P184_FREQ_CONFIG() = getFormItemInt(F("freq"));

      P184_data.trigger_value = P184_TRIGGER_CONFIG();
      // Recalculate power value based on the new trigger value from the form
      for (int i = 0; i <= 100; ++i) {
        if (pgm_read_byte(&power_to_trigger_lut[i]) <= P184_data.trigger_value) {
          P184_data.power_value = i;
          break; // Found the highest power for this trigger level or lower
        }
      }
      // after the form has been saved successfuly, set success and break
      success = true;
      break;
    }
    case PLUGIN_INIT:
    {
      // this case defines code to be executed when the plugin is initialised
      P184_data.trigger_pin     = P184_TRIGGER_PIN();
      P184_data.trigger_value   = P184_TRIGGER_CONFIG();
      P184_data.dead_zone       = P184_DEAD_ZONE_CONFIG();
      P184_data.freq_timing_val = P184_FREQ_CONFIG();
      P184_data.zero_crossing_pin = P184_ZERO_CROSS_PIN();

      P184_data.trigger_value = P184_TRIGGER_CONFIG();
      // Calculate initial power value based on the loaded trigger value
      for (int i = 0; i <= 100; ++i) {
        if (pgm_read_byte(&power_to_trigger_lut[i]) <= P184_data.trigger_value) {
          P184_data.power_value = i;
          break; // Found the highest power for this trigger level or lower
        }
      }


      if (P184_data.zero_crossing_pin != -1)
      {
        pinMode(P184_data.zero_crossing_pin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(P184_data.zero_crossing_pin), P184_zero_crossing, P184_TRIGGER_EDGE_CONFIG());
      }
      if (P184_data.trigger_pin != -1)
      {
        pinMode(P184_data.trigger_pin, OUTPUT);
        digitalWrite(P184_data.trigger_pin, LOW);
        P184_data.p184_timer = timerBegin(1000000);
        P184_data.time_us = uint64_t(P184_data.trigger_value) * P184_data.freq_timing_val;
        timerAttachInterrupt(P184_data.p184_timer, &P184_timer_handler);
        timerAlarm(P184_data.p184_timer, P184_data.time_us, true, 0);
      }

      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      // code to be executed to read data
      UserVar.setFloat(event->TaskIndex, 0, P184_data.trigger_value);
      UserVar.setFloat(event->TaskIndex, 1, P184_data.power_value);

      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {

      // parse string to extract the command
      String tmpString = parseString(string, 1); // already converted to lowercase

      if (equals(tmpString, F("triac"))) {
        String subcmd = parseString(string, 2);
        String valueStr = parseString(string, 3);
        long value = valueStr.toInt();

        if (value >= 0 && value <= 100) {
          if (equals(subcmd, F("power"))) {
            // User wants to set POWER: triac,power,<value>
            P184_data.power_value = value;
            // Find the corresponding trigger value from LUT
            uint8_t new_trigger = pgm_read_byte(&power_to_trigger_lut[P184_data.power_value]);
            P184_data.trigger_value = new_trigger;
            success = true;
          } else if (equals(subcmd, F("trigger"))) {
            // User wants to set TRIGGER directly: triac,trigger,<value>
            uint8_t new_trigger = value;

            // Apply dead zone logic
            if (new_trigger > (100 - P184_data.dead_zone)) {
              new_trigger = 100;
            } else if (new_trigger < P184_data.dead_zone) {
              new_trigger = 0;
            }
            P184_data.trigger_value = new_trigger;

            // Let's find the closest power value for the new trigger.
            // This is a slow lookup, but only happens on command.
            // The LUT maps power (index) to trigger (value). We need to find the index (power)
            // for a given trigger value.
            for (int i = 0; i <= 100; ++i) {
              // Find the first power level (i) where the corresponding trigger
              // is less than or equal to the one we just set.
              if (pgm_read_byte(&power_to_trigger_lut[i]) <= P184_data.trigger_value) {
                P184_data.power_value = i;
                break; // Found the highest power for this trigger level or lower
              }
            }
            success = true;
          }

          if (success) {
            P184_TRIGGER_CONFIG() = P184_data.trigger_value; // Save state
            P184_data.time_us = uint64_t(P184_data.trigger_value) * P184_data.freq_timing_val;
            timerAlarm(P184_data.p184_timer, P184_data.time_us, true, 0);
            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              String log = strformat(F("P184 CMD : Trigger %d%% . Power %d%%"), P184_data.trigger_value, P184_data.power_value);
              addLogMove(LOG_LEVEL_INFO, log);
            }
          }
        }
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      // perform cleanup tasks here. For example, free memory, shut down/clear a display
      if (P184_data.zero_crossing_pin != -1)
      {
        detachInterrupt(digitalPinToInterrupt(P184_data.zero_crossing_pin));
      }
      if (P184_data.trigger_pin != -1)
      {
        digitalWrite(P184_data.trigger_pin, LOW);
        if (P184_data.p184_timer != NULL) {
          timerEnd(P184_data.p184_timer);
          P184_data.p184_timer = NULL;
        }
      }
      success = true;
      break;
    }

    // case PLUGIN_ONCE_A_SECOND:
    // {
    //   // code to be executed once a second. Tasks which do not require fast response can be added here

    //   success = true;
    // }

    // case PLUGIN_TEN_PER_SECOND:
    // {
    //   // code to be executed 10 times per second. Tasks which require fast response can be added here
    //   // be careful on what is added here. Heavy processing will result in slowing the module down!

    //   success = true;
    // }
  } // switch
  return success;
}   // function


#endif