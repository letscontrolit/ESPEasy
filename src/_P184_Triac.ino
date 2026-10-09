// This plugin detect zerocross and trigger a triac. Used for power electronics.
// Comon uses are AC light dimming or AC Fan speed control. 

#include "_Plugin_Helper.h"

#ifdef USES_P184
#define PLUGIN_184
#define PLUGIN_ID_184     184           // plugin id
#define PLUGIN_NAME_184   "Output - Triac" // "Plugin Name" is what will be dislpayed in the selection list
#define PLUGIN_VALUENAME1_184 "Trigger" // variable output of the plugin. The label is in quotation marks
#define PLUGIN_VALUENAME2_184 "Power" // multiple outputs are supported
#define P184_OUTPUT_TYPE_INDEX  2


#define P184_60HZ_HALF_WAVE_TIME_US_ONE_PERCENT ((uint32_t)83) // 1/60/2*1%
#define P184_50HZ_HALF_WAVE_TIME_US_ONE_PERCENT ((uint32_t)100) // 1/50/2*1%

#define P184_TRIGGER_CONFIG()      PCONFIG(0)
#define P184_TRIGGER_EDGE_CONFIG() PCONFIG(1)
#define P184_DEAD_ZONE_CONFIG()    PCONFIG(2)
#define P184_FREQ_CONFIG()         PCONFIG(3)

#define P184_ZERO_CROSS_PIN()      PIN(0)
#define P184_TRIGGER_PIN()         PIN(1)

struct P184_data_struct : public PluginTaskData_base {
  gpio_num_t   zero_crossing_pin = GPIO_NUM_NC;
  gpio_num_t   trigger_pin   = GPIO_NUM_NC;
  uint8_t  trigger_value    = 0;
  uint8_t  power_value      = 0;
  uint8_t  dead_zone        = 0;
  uint32_t freq_timing_val  = P184_60HZ_HALF_WAVE_TIME_US_ONE_PERCENT;
  hw_timer_t *timer         = NULL;

  // Funções de interrupção como membros estáticos da struct
  static void IRAM_ATTR zero_crossing_handler(void *arg) {
    P184_data_struct* p184_data = static_cast<P184_data_struct*>(arg);
    if (p184_data->trigger_value == 0) {
      REG_WRITE(GPIO_OUT_W1TS_REG, (1 << p184_data->trigger_pin)); // Fast gpio_set_level(HIGH)
    } else {
      REG_WRITE(GPIO_OUT_W1TC_REG, (1 << p184_data->trigger_pin)); // Fast gpio_set_level(LOW)
      timerRestart(p184_data->timer);
    }
  }

  static void IRAM_ATTR timer_handler(void *arg) {
    P184_data_struct* p184_data = static_cast<P184_data_struct*>(arg);
    if (p184_data->trigger_value != 100) {
      REG_WRITE(GPIO_OUT_W1TS_REG, (1 << p184_data->trigger_pin)); // Fast gpio_set_level(HIGH)
    }
  }
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
constexpr uint8_t power_to_trigger_lut_size = NR_ELEMENTS(power_to_trigger_lut);

// A plugin has to implement the following function
boolean Plugin_184(uint8_t function, struct EventStruct *event, String& string)
{

  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      // This case defines the device characteristics, edit appropriately
      // Attention: dev Values set to 0 or false should be removed to save a few bytes (unneeded assignments)

      auto& dev = Device[++deviceCount];
      dev.Number             = PLUGIN_ID_184;                    // Plugin ID number.   (PLUGIN_ID_184)
      dev.VType              = Sensor_VType::SENSOR_TYPE_DIMMER; // Type of value the plugin will return. e.g. SENSOR_TYPE_STRING
      dev.ValueCount         = 2;                                // The number of output values of a plugin. The value should match the number of keys PLUGIN_VALUENAME1_184
      dev.OutputDataType     = Output_Data_type_t::Simple;      // Subset of selectable output data types  (Default = no selection)
      dev.SendDataOption     = true;                            // Allow to send data to a controller.
      dev.GlobalSyncOption   = true;                             // No longer used. Was used for ESPeasy values sync between nodes
      dev.TimerOption        = true;                             // Allow to set the "Interval" timer for the plugin.
      dev.DecimalsOnly       = true;                             // Allow to set the number of decimals (otherwise treated a 0 decimals)
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_184);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_184));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_184));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      // Set a default config here, which will be called when a plugin is assigned to a task.
      P184_TRIGGER_CONFIG() = 0;
      P184_DEAD_ZONE_CONFIG() = 5;
      P184_ZERO_CROSS_PIN() = GPIO_NUM_NC;
      P184_TRIGGER_PIN() = GPIO_NUM_NC;
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
      P184_TRIGGER_CONFIG() = getFormItemInt(F("trigger"));
      P184_DEAD_ZONE_CONFIG() = getFormItemInt(F("trigger_deadzone"));
      P184_ZERO_CROSS_PIN() = getFormItemInt(F("zero_crossing_pin"));
      P184_TRIGGER_PIN() = getFormItemInt(F("trigger_pin"));
      P184_TRIGGER_EDGE_CONFIG() = getFormItemInt(F("trigger_edge"));
      P184_FREQ_CONFIG() = getFormItemInt(F("freq"));

      success = true;
      break;
    }
    case PLUGIN_INIT:
    {
      P184_data_struct* p184_data = (P184_data_struct*)getPluginTaskData(event->TaskIndex);
      if (p184_data == nullptr) {
          // Aloca a memória se ainda não existir
          p184_data = new P184_data_struct;
          if (p184_data == nullptr) {
              // Falha na alocação de memória
              return false;
          }
          initPluginTaskData(event->TaskIndex, p184_data);
      }
      // this case defines code to be executed when the plugin is initialised
      p184_data->trigger_pin       = (gpio_num_t)P184_TRIGGER_PIN();
      p184_data->trigger_value     = P184_TRIGGER_CONFIG();
      p184_data->dead_zone         = P184_DEAD_ZONE_CONFIG();
      p184_data->freq_timing_val   = P184_FREQ_CONFIG();
      p184_data->zero_crossing_pin = (gpio_num_t)P184_ZERO_CROSS_PIN();

      // Calculate initial power value based on the loaded trigger value
      for (int i = 0; i <= power_to_trigger_lut_size; ++i) {
        if (pgm_read_byte(&power_to_trigger_lut[i]) <= p184_data->trigger_value) {
          p184_data->power_value = i;
          break; // Found the highest power for this trigger level or lower
        }
      }

      if (p184_data->zero_crossing_pin == GPIO_NUM_NC || p184_data->trigger_pin == GPIO_NUM_NC) {
        detachInterrupt(digitalPinToInterrupt(p184_data->zero_crossing_pin));
        gpio_set_level(p184_data->trigger_pin, LOW);
        p184_data->timer = NULL;
        return false;
      } 
      else {
        pinMode(p184_data->zero_crossing_pin, INPUT_PULLUP);
        pinMode(p184_data->trigger_pin, OUTPUT);
        gpio_set_level(p184_data->trigger_pin, LOW); // REG_WRITE(GPIO_OUT_W1TC_REG, (1 << p184_data->trigger_pin)); // Fast gpio_set_level(LOW)
        attachInterruptArg(digitalPinToInterrupt(p184_data->zero_crossing_pin), &P184_data_struct::zero_crossing_handler, p184_data, P184_TRIGGER_EDGE_CONFIG());
        
        
        p184_data->timer = timerBegin(1000000); // 1MHz - timer can be set to microseconds
        uint32_t time_us = ( (p184_data->trigger_value > p184_data->dead_zone ? p184_data->trigger_value : p184_data->dead_zone) ) * p184_data->freq_timing_val;
        timerAttachInterruptArg(p184_data->timer, &P184_data_struct::timer_handler, p184_data);
        timerAlarm(p184_data->timer, (uint64_t)time_us, true, 0);
      }

      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      P184_data_struct* p184_data = (P184_data_struct*)getPluginTaskData(event->TaskIndex);
      if (p184_data == nullptr) return false;
      // code to be executed to read data
      UserVar.setFloat(event->TaskIndex, 0, p184_data->trigger_value);
      UserVar.setFloat(event->TaskIndex, 1, p184_data->power_value);

      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      P184_data_struct* p184_data = (P184_data_struct*)getPluginTaskData(event->TaskIndex);
      if (p184_data == nullptr) return false;
      // parse string to extract the command
      String tmpString = parseString(string, 1); // already converted to lowercase

      if (equals(tmpString, F("triac"))) {
        String subcmd = parseString(string, 2);
        String valueStr = parseString(string, 3);
        long value = event->Par2;

        if (value >= 0 && value <= 100) {
          if (equals(subcmd, F("power"))) {
            // User wants to set POWER: triac,power,<value>
            p184_data->power_value = value;
            // Find the corresponding trigger value from LUT
            uint8_t new_trigger = pgm_read_byte(&power_to_trigger_lut[p184_data->power_value]);
            p184_data->trigger_value = new_trigger;
            success = true;
          } else if (equals(subcmd, F("trigger"))) {
            // User wants to set TRIGGER directly: triac,trigger,<value>
            uint8_t new_trigger = value;
            p184_data->trigger_value = new_trigger;

            // Let's find the closest power value for the new trigger.
            // This is a slow lookup, but only happens on command.
            // The LUT maps power (index) to trigger (value). We need to find the index (power)
            // for a given trigger value.
            for (int i = 0; i <= power_to_trigger_lut_size; ++i) {
              // Find the first power level (i) where the corresponding trigger
              // is less than or equal to the one we just set.
              if (pgm_read_byte(&power_to_trigger_lut[i]) <= p184_data->trigger_value) {
                p184_data->power_value = i;
                break; // Found the highest power for this trigger level or lower
              }
            }
            success = true;
          }

          if (success && p184_data->timer != NULL) {
            P184_TRIGGER_CONFIG() = p184_data->trigger_value; // Save state
            uint32_t time_us = ( (p184_data->trigger_value > p184_data->dead_zone ? p184_data->trigger_value : p184_data->dead_zone) ) * p184_data->freq_timing_val;
            timerAlarm(p184_data->timer, (uint64_t)time_us, true, 0);
            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              addLog(LOG_LEVEL_INFO, strformat(F("P184 CMD : Trigger %d%% . Power %d%%"), p184_data->trigger_value, p184_data->power_value));
            }
          }
        }
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      P184_data_struct* p184_data = (P184_data_struct*)getPluginTaskData(event->TaskIndex);
      if (p184_data == nullptr) return true; // Nothing to do
      // perform cleanup tasks here. For example, free memory, shut down/clear a display
      if (p184_data->zero_crossing_pin != GPIO_NUM_NC)
      {
        detachInterrupt(digitalPinToInterrupt(p184_data->zero_crossing_pin));
        p184_data->zero_crossing_pin = GPIO_NUM_NC;
      }
      if (p184_data->trigger_pin != GPIO_NUM_NC)
      {
        gpio_set_level(p184_data->trigger_pin, LOW);
        if (p184_data->timer != NULL) {
          timerEnd(p184_data->timer);
          p184_data->timer = NULL;
        }
        p184_data->trigger_pin = GPIO_NUM_NC;
      }
      
      clearPluginTaskData(event->TaskIndex);

      success = true;
      break;
    }
  } // switch
  return success;
}   // function


#endif