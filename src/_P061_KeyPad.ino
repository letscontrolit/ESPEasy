#include "_Plugin_Helper.h"
#ifdef USES_P061

// #######################################################################################################
// ################################ Plugin 061: PCF8574/MCP23017/PCF8575 KeyPad ##########################
// #######################################################################################################

/** Changelog:
 * 2022-01-23 tonhuisman: Add support for MCP23017 Direct mode, see https://github.com/letscontrolit/ESPEasy/issues/557
 *                        Add support for PCF8575 Matrix and Direct mode (requires pull-ups, 10-100k, on all 8 or 16 inputs!)
 *                        Support for PCF8575 can be disabled by disabling #define P061_ENABLE_PCF8575
 *                        Add logging in low-level read functions
 * 2022-01-22 tonhuisman: Make plugin multi-instance compatible, by moving most code to src/PluginStructs/P061_data_struct.*
 *                        Replaced PCONFIG(x) macros by more meaningful named macros
 *                        Formatted source using Uncrustify
 *
 * pre-2022-01 No changelog recorded
 * Originally written by Jochen Krapf (jk@nerd2nerd.org)
 */

// ESPEasy Plugin to scan a (up to) 9x8 key pad matrix chip MCP23017
// or a (up to) 5x4 key pad matrix chip PCF8574
// written by Jochen Krapf (jk@nerd2nerd.org)

// Connecting KeyPad matrix to MCP23017 / PCF8575 chip:
// row 0 = GND   (optional if 9 rows needed)
// row 1 = GPA0 / P00
// row 2 = GPA1 / P01
// ...
// row 8 = GPA7 / P07
//
// column 1 = GPB0 / P10
// column 2 = GPB1 / P11
// ...
// column 8 = GPB7 / P17

// Typical Key Pad:
//      C1  C2  C3
// R1   [1] [2] [3]
// R2   [4] [5] [6]
// R3   [7] [8] [9]
// R4   [*] [0] [#]

// Connecting KeyPad matrix to PCF8574 chip:
// row 0 = GND   (optional if 5 rows needed)
// row 1 = P0
// row 2 = P1
// row 3 = P2
// row 4 = P3
//
// column 1 = P4
// column 2 = P5
// column 3 = P6
// column 4 = P7

// Connecting KeyPad direct to PCF8574 / MCP23017 / PCF8575 chip:
// common = GND
// key 1 = P0 / GPA0 / P00
// key 2 = P1 / GPA1 / P01
// ...
// key 8 = P7 / GPA7 / P07
// For 16 bit I/O expanders
// key 9 = -- / GPB0 / P10
// key 10 = -- / GPB1 / P11
// ...
// key 16 = -- / GPB7 / P17
// NB: PCF8575 needs pull-up resistors on all 16 pins to work as intended, as the chip doesn't have internal pull-ups

// ScanCode;
// 16*col + row
// Pressing the top left key (typically "1") the code is 17 (0x11)
// Pressing the key in rowumn 2 and col 3 (typically "8") the code is 35 (0x23)
// No key - the code 0
// If more than one key is pressed, the scan code is the code with the lowest value

# include "src/PluginStructs/P061_data_struct.h"

# define PLUGIN_061
# define PLUGIN_ID_061         61
# define PLUGIN_NAME_061       "Keypad - PCF8574 / MCP23017 / PCF8575 [TESTING]"
# define PLUGIN_VALUENAME1_061 "ScanCode"


boolean Plugin_061(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_061;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SWITCH;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_061);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_061));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"),
                           (P061_CONFIG_KEYPAD_TYPE == 0 || P061_CONFIG_KEYPAD_TYPE > 2) ? 8 : 16,
                           i2cAddressValues,
                           P061_CONFIG_I2C_ADDRESS);

        if ((P061_CONFIG_KEYPAD_TYPE == 1) || (P061_CONFIG_KEYPAD_TYPE == 2)) { // PCF8574(A)
          addFormNote(F("PCF8574 uses address 0x20+; PCF8574<b>A</b> uses address 0x38+"));
        }
      } else {
        success = intArrayContains(16, i2cAddressValues, event->Par1);
      }
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      const __FlashStringHelper *options[] = {
        F("MCP23017 (Matrix 9x8)"),
        F("PCF8574 (Matrix 5x4)"),
        F("PCF8574 (Direct 8)"),
        F("MCP23017 (Direct 16)"),
        # ifdef P061_ENABLE_PCF8575
        F("PCF8575 (Matrix 9x8)"),
        F("PCF8575 (Direct 16)")
        # endif // ifdef P061_ENABLE_PCF8575
      };
      int optionsCount =
      # ifdef P061_ENABLE_PCF8575
        6;
      # else // ifdef P061_ENABLE_PCF8575
        4;
      # endif // ifdef P061_ENABLE_PCF8575
      addFormSelector(F("Chip (Mode)"), F("chip"), optionsCount, options, NULL, P061_CONFIG_KEYPAD_TYPE);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P061_CONFIG_I2C_ADDRESS = getFormItemInt(F("i2c_addr"));

      P061_CONFIG_KEYPAD_TYPE = getFormItemInt(F("chip"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P061_data_struct(P061_CONFIG_I2C_ADDRESS, P061_CONFIG_KEYPAD_TYPE));
      P061_data_struct *P061_data = static_cast<P061_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P061_data) {
        return success;
      }

      success = P061_data->plugin_init(event);

      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P061_data_struct *P061_data = static_cast<P061_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P061_data) {
        return success;
      }

      success = P061_data->plugin_fifty_per_second(event);
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

#endif // USES_P061
