#include "_Plugin_Helper.h"
#ifdef USES_P008

// #######################################################################################################
// ################################# Plugin 008: Wiegand RFID Tag Reader #################################
// #######################################################################################################

/*
   History:
   2023-01-22 tonhuisman: Disable some strings in BUILD_NO_DEBUG builds to reduce size, minor optimizations
   2022-12-04 tonhuisman: Fix initialization issue (hanginging ESP...) when GPIO pins are not configured correctly
   2022-12-03 tonhuisman: Add Get Config values for tag value and bits received
   2022-08-02 tonhuisman: Enable multi-instance use, handle interrupts multi-instance compatible
                          use named defines for settings, rename variables where possible, clean up sources
   2022-08-02 tonhuisman: Reduce usage of iRam by optimizing the ISR
   2021-11-20 tonhuisman: Add optional switching of GPIO pins, new default will be 'swapped' existing settings stay unaltered
                          to ensure that existing codes keep working.
                          Apply casting by 'ull' postfix instead of explicit 'static_cast<uint64_t>(0x1)' where possible
   2021-11-19 tonhuisman: Fix casting bug after adding > 34 bit support
                          Fix swapped GPIO's to show same/expected results as other Wiegand readers
   2021-08-02 tonhuisman: Add checkbos for 'Alternative decoding', swapping the receving of the bits, resulting
           in little-endian versus big-endian output. This is supposed to give the same output as the
           official Wiegand RFID scanner.
           Reformatted using Uncrustify
   2020-07-04 tonhuisman: Add checkbox for 'Present hex as decimal value' option (with note) so hexadecimal
           value of f.e. a numeric keypad using the Wiegand protocol (hexadecimal data) will be cast to decimal.
           When enabled entering 1234# will result in Tag = 1234 instead of 4660 (= 0x1234), any A-F
           entered will result in a 0 in the output value.
   -------------
   No initial history available.
 */


# define PLUGIN_008
# define PLUGIN_ID_008         8
# define PLUGIN_NAME_008       "RFID - Wiegand"
# define PLUGIN_VALUENAME1_008 "Tag"

# include "src/PluginStructs/P008_data_struct.h"


boolean Plugin_008(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_008;
      Device[deviceCount].Type               = DEVICE_TYPE_DUAL;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_LONG;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_008);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_008));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_input(F("D0 (Green, 5V)"));
      event->String2 = formatGpioName_input(F("D1 (White, 5V)"));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P008_DATA_BITS      = 26;  // Minimal nr. of bits
      P008_COMPATIBILITY  = 1;   // Use swapped by default, unswapped = backward compatible
      P008_REMOVE_TIMEOUT = 500; // Default time-out 500 mSec
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P008_data_struct(event));
      P008_data_struct *P008_data = static_cast<P008_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P008_data) {
        success = P008_data->plugin_init(event);
      }
      break;
    }

    case PLUGIN_TASKTIMER_IN:
    {
      P008_data_struct *P008_data = static_cast<P008_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P008_data) {
        success = P008_data->plugin_timer_in(event);
      }
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P008_data_struct *P008_data = static_cast<P008_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P008_data) {
        success = P008_data->plugin_once_a_second(event);
      }
      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P008_data_struct *P008_data = static_cast<P008_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P008_data) {
        success = P008_data->plugin_get_config(event, string);
      }
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormCheckBox(F("Enable backward compatibility mode"), F("comp"), P008_COMPATIBILITY == 0);
      # ifndef BUILD_NO_DEBUG
      addFormNote(F("Earlier versions of this plugin have used GPIO pins inverted, giving different Tag results."));
      # endif // ifndef BUILD_NO_DEBUG

      addFormNumericBox(F("Wiegand Type (bits)"), F("ptype"), P008_DATA_BITS, 26, 64);
      addUnit(F("26..64 bits"));
      # ifdef BUILD_NO_DEBUG
      addFormNote(F("Select the number of bits to be received, f.e. 26, 34, 37."));
      # endif // ifdef BUILD_NO_DEBUG

      addFormCheckBox(F("Present hex as decimal value"), F("hdec"), P008_HEX_AS_DEC == 1);
      # ifndef BUILD_NO_DEBUG
      addFormNote(F("Useful only for numeric keypad input!"));
      # endif // ifndef BUILD_NO_DEBUG

      addFormCheckBox(F("Automatic Tag removal"), F("autormv"), P008_AUTO_REMOVE == 0);                   // Inverted state!

      if (P008_REMOVE_TIMEOUT == 0) { P008_REMOVE_TIMEOUT = 500; } // Default 500 mSec (was hardcoded value)
      addFormNumericBox(F("Automatic Tag removal after"), F("rmvtime"), P008_REMOVE_TIMEOUT, 250, 60000); // 0.25 to 60 seconds
      addUnit(F("mSec."));

      // Max allowed is int = 0x7FFFFFFF ...
      addFormNumericBox(F("Value to set on Tag removal"), F("rmvval"), P008_REMOVE_VALUE, 0);

      addFormCheckBox(F("Event on Tag removal"), F("rstevt"), P008_REMOVE_EVENT == 1); // Normal state!

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P008_DATA_BITS      = getFormItemInt(F("ptype"));
      P008_HEX_AS_DEC     = isFormItemChecked(F("hdec")) ? 1 : 0;
      P008_AUTO_REMOVE    = isFormItemChecked(F("autormv")) ? 0 : 1; // Inverted logic!
      P008_REMOVE_EVENT   = isFormItemChecked(F("rstevt")) ? 1 : 0;
      P008_COMPATIBILITY  = isFormItemChecked(F("comp")) ? 0 : 1;    // Inverted logic!
      P008_REMOVE_VALUE   = getFormItemInt(F("rmvval"));
      P008_REMOVE_TIMEOUT = getFormItemInt(F("rmvtime"));

      // uint64_t keyMask = 0LL;
      // keyMask = (0x1ull << (P008_DATA_BITS - 2));
      // keyMask--;
      // String log = F("P008: testing keyMask = 0x");
      // log += ull2String(keyMask, HEX);
      // log += F(" bits: ");
      // log += P008_DATA_BITS;
      // addLog(LOG_LEVEL_INFO, log);

      success = true;
      break;
    }
  }
  return success;
}

#endif // USES_P008
