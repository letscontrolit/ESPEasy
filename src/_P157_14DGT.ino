#include "_Plugin_Helper.h"
#ifdef USES_P157

// #######################################################################################################
// ######################     Plugin 157 - 14-/7-segment display plugin HT16K33     ######################
// #######################################################################################################
//
// Chips/displays supported:
//  0 - HT16K33     -- I2C - 4 digits, 1..8 displays (4..32 digits) with sequential I2C addresses
//  1 - HT16K33     -- I2C - 8 digits, 1..8 displays (4..64 digits) with sequential I2C addresses
//
// Plugin can be setup as:
//  - Manual        -- display is manually updated sending commands
//  "7dn,<number>"        (number can be negative or positive, even with decimal)
//  "7dt,<temperature>"   (temperature can be negative or positive and containing decimals)
//  "7ddt,<temperature>,<temperature>"   (Dual temperatures on Max7219/74HC595 (8 digits) only, temperature can be negative or
//                                        positive and containing decimals)
//  "7dtext,<text>"       (show free text - supported low ascii, period, comma, colon, semicolon are displayed as dot)
//  "7dfont,<font>"       7-segment only: (select the used font: 0/7DGT/Default = default, 1/Siekoo = Siekoo, 2/Siekoo_Upper = Siekoo
//                         with uppercase CHNORUX, 3/dSEG7 = dSEG7)
//                        Siekoo: https://www.fakoo.de/siekoo (uppercase CHNORUX is a local extension)
//                        dSEG7 : https://www.keshikan.net/fonts-e.html
//  "7dbin,[uint16_t],..."    (show data binary formatted, bits clock-wise from left to right, dot, top, right 2x, bottom,
//                            left 2x, center), scroll-enabled
//
// Generic commands:
//  - "7don"         -- turn ON the display
//  - "7doff"        -- turn OFF the display
//  - "7db,<0-15>    -- set brightness to specific value between 0 and 15
//

/** History
 * 2026-05-25 tonhuisman: Add 7digit,<dgt>,<char/text> command for writing content from a specific digit 1..len
 *                        Remove right-align option, as this isn't implemented
 *                        Clean up source
 * 2026-05-23 tonhuisman: Add font-selector for 7-segment displays, re-using the fonts available in P073
 * 2026-05-15 tonhuisman: Start plugin, based on P073
 */

# define PLUGIN_157
# define PLUGIN_ID_157           157
# define PLUGIN_NAME_157         "Display - HT16K33 14-/7-segment"

# include "src/PluginStructs/P157_data_struct.h"

boolean Plugin_157(uint8_t function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number = PLUGIN_ID_157;
      dev.Type   = DEVICE_TYPE_I2C;
      dev.VType  = Sensor_VType::SENSOR_TYPE_NONE;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_157);
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      # if P157_SCROLL_TEXT
      P157_CFG_SCROLLSPEED = 10; // Default 10 * 0.1 sec scroll speed
      # endif // if P157_SCROLL_TEXT
      P157_CFG_DISPLAYS = 1;     // Default number of displays
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 8, i2cAddressValues, P157_CFG_I2C_ADDRESS);
      } else {
        success = intArrayContains(8, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P157_CFG_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *displtype[] = {
          P157_DisplayModel(P157_MODEL_4DGT),
          P157_DisplayModel(P157_MODEL_8DGT),
          P157_DisplayModel(P157_MODEL_4DGT_7SEG),
          P157_DisplayModel(P157_MODEL_8DGT_7SEG),
        };
        const int displOption[] = {
          P157_MODEL_4DGT,
          P157_MODEL_8DGT,
          P157_MODEL_4DGT_7SEG,
          P157_MODEL_8DGT_7SEG,
        };
        constexpr size_t optionCount = NR_ELEMENTS(displtype);
        FormSelectorOptions selector(optionCount, displtype, displOption);
        selector.reloadonchange = true;
        selector.addFormSelector(F("Display Type"), F("displtype"), P157_CFG_DISPLAYTYPE);
      }

      if (0 == P157_CFG_DISPLAYS) {
        P157_CFG_DISPLAYS = 1;
      }
      addFormNumericBox(F("Nr. of displays"), F("dspls"), P157_CFG_DISPLAYS, 1, 8);
      addUnit(F("1..8"));
      addFormNote(F("Displays must have consecutive I2C addresses"));

      P157_display_output_selector(F("displout"), P157_CFG_OUTPUTTYPE);

      addFormNumericBox(F("Brightness"), F("brightness"), P157_CFG_BRIGHTNESS, 0, 15);
      addUnit(F("0..15"));

      # if P157_EXTRA_FONTS

      if (P157_is7SegmentDisplay(P157_CFG_DISPLAYTYPE)) {
        P073_font_selector(F("fontset"), P157_CFG_FONTSET);
        addFormNote(F("Only available for 7-segment displays"));
      }
      # endif // if P157_EXTRA_FONTS

      addFormSubHeader(F("Options"));

      addFormCheckBox(F("Hide &deg; for Temperatures"), F("hide_degree"), bitRead(P157_CFG_FLAGS, P157_OPTION_HIDEDEGREE));
      # if P157_7DDT_COMMAND
      addFormNote(F("Commands 7dt,&lt;temp&gt; and 7ddt,&lt;temp1&gt;,&lt;temp2&gt;"));
      # else // if P157_7DDT_COMMAND
      addFormNote(F("Command 7dt,&lt;temp&gt;"));
      # endif // if P157_7DDT_COMMAND
      # if P157_SUPPRESS_ZERO
      addFormCheckBox(F("Suppress leading 0 on day/hour"), F("supp0"), bitRead(P157_CFG_FLAGS, P157_OPTION_SUPPRESS0));
      # endif // if P157_SUPPRESS_ZERO

      # if P157_SCROLL_TEXT
      addFormCheckBox(F("Scroll text &gt; display width"), F("scroll_text"), bitRead(P157_CFG_FLAGS, P157_OPTION_SCROLLTEXT));
      addFormCheckBox(F("Scroll text in from right"),      F("scroll_full"), bitRead(P157_CFG_FLAGS, P157_OPTION_SCROLLFULL));

      if (P157_CFG_SCROLLSPEED == 0) { P157_CFG_SCROLLSPEED = 10; }
      addFormNumericBox(F("Scroll speed (0.1 sec/step)"), F("scrollspeed"), P157_CFG_SCROLLSPEED, 1, 600);
      addUnit(F("1..600 = 0.1..60 sec/step"));
      # endif // if P157_SCROLL_TEXT

      // addFormCheckBox(F("Right-align Temperature (7dt)"), F("temp_rightalign"), bitRead(P157_CFG_FLAGS, P157_OPTION_RIGHTALIGN));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P157_CFG_I2C_ADDRESS = getFormItemInt(F("i2c_addr"));
      P157_CFG_DISPLAYTYPE = getFormItemInt(F("displtype"));
      P157_CFG_DISPLAYS    = getFormItemInt(F("dspls"));
      P157_CFG_OUTPUTTYPE  = getFormItemInt(F("displout"));
      P157_CFG_BRIGHTNESS  = getFormItemInt(F("brightness"));

      # if P157_EXTRA_FONTS

      if (P157_is7SegmentDisplay(P157_CFG_DISPLAYTYPE)) {
        P157_CFG_FONTSET = getFormItemInt(F("fontset"));
      } else
      # endif // if P157_EXTRA_FONTS
      {
        P157_CFG_FONTSET = 0;
      }
      uint32_t lSettings = 0;

      // bitWrite(lSettings, P157_OPTION_PERIOD,     isFormItemChecked(F("periods")));
      bitWrite(lSettings, P157_OPTION_HIDEDEGREE, isFormItemChecked(F("hide_degree")));

      // bitWrite(lSettings, P157_OPTION_RIGHTALIGN, isFormItemChecked(F("temp_rightalign")));
      # if P157_SCROLL_TEXT
      bitWrite(lSettings, P157_OPTION_SCROLLTEXT, isFormItemChecked(F("scroll_text")));
      bitWrite(lSettings, P157_OPTION_SCROLLFULL, isFormItemChecked(F("scroll_full")));
      P157_CFG_SCROLLSPEED = getFormItemInt(F("scrollspeed"));
      # endif // if P157_SCROLL_TEXT
      # if P157_SUPPRESS_ZERO
      bitWrite(lSettings, P157_OPTION_SUPPRESS0, isFormItemChecked(F("supp0")));
      # endif // if P157_SUPPRESS_ZERO
      P157_CFG_FLAGS = lSettings;

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (0 == P157_CFG_DISPLAYS) {
        P157_CFG_DISPLAYS = 1;
      }
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P157_data_struct());
      P157_data_struct *P157_data =
        static_cast<P157_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P157_data) {
        addLog(LOG_LEVEL_INFO, F("P157 : Calling init()"));
        success = P157_data->init(event);
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      P157_data_struct *P157_data =
        static_cast<P157_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P157_data) {
        success = P157_data->plugin_write(event, string);
      }
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P157_data_struct *P157_data =
        static_cast<P157_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P157_data) {
        success = P157_data->plugin_once_a_second(event);
      }

      break;
    }

    # if P157_SCROLL_TEXT
    case PLUGIN_TEN_PER_SECOND:
    {
      P157_data_struct *P157_data =
        static_cast<P157_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P157_data) {
        success = P157_data->plugin_ten_per_second(event);
      }

      break;
    }
    # endif // if P157_SCROLL_TEXT

  }
  return success;
}

#endif // ifdef USES_P157
