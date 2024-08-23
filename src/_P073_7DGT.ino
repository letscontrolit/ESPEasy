#include "_Plugin_Helper.h"
#ifdef USES_P073

// #######################################################################################################
// ###################   Plugin 073 - 7-segment display plugin TM1637/MAX7219       ######################
// #######################################################################################################
//
// Chips/displays supported:
//  0 - TM1637     -- 2 pins - 4 digits and colon in the middle (XX:XX)
//  1 - TM1637     -- 2 pins - 4 digits and dot on each digit (X.X.X.X.)
//  2 - TM1637     -- 2 pins - 6 digits and dot on each digit (X.X.X.X.X.X.)
//  3 - MAX7219/21 -- 3 pins - 8 digits and dot on each digit (X.X.X.X.X.X.X.X.)
//  4 - 74HC595 xDgt  3 pins - 2,2+2,3,3+2,2+3,4,4+4,6,3+3,3+4,4+3,8 digits and dot on each digit (X.X. .. X.X.X.X.X.X.X.X.)
//
// Plugin can be setup as:
//  - Manual        -- display is manually updated sending commands
//  "7dn,<number>"        (number can be negative or positive, even with decimal)
//  "7dt,<temperature>"   (temperature can be negative or positive and containing decimals)
//  "7ddt,<temperature>,<temperature>"   (Dual temperatures on Max7219/74HC595 (8 digits) only, temperature can be negative or
//                                        positive and containing decimals)
//  "7dst,<hh>,<mm>,<ss>" (show manual time -not current-, no checks done on numbers validity!)
//  "7dsd,<dd>,<mm>,<yy>" (show manual date -not current-, no checks done on numbers validity!)
//  "7dtext,<text>"       (show free text - supported chars 0-9,a-z,A-Z," ","-","=","_","/","^") Depending on Font used
//  "7dfont,<font>"       (select the used font: 0/7DGT/Default = default, 1/Siekoo = Siekoo, 2/Siekoo_Upper = Siekoo
//                         with uppercase CHNORUX, 3/dSEG7 = dSEG7)
//                        Siekoo: https://www.fakoo.de/siekoo (uppercase CHNORUX is a local extension)
//                        dSEG7 : https://www.keshikan.net/fonts-e.html
//  "7dbin,[uint8_t],..."    (show data binary formatted, bits clock-wise from left to right, dot, top, right 2x, bottom,
//                            left 2x, center), scroll-enabled
//
// Generic commands:
//  - "7don"         -- turn ON the display
//  - "7doff"        -- turn OFF the display
//  - "7db,<0-15>    -- set brightness to specific value between 0 and 15
//  - "7output,<0-5> -- select display output mode, 0:"Manual",1:"Clock 24h - Blink",2:"Clock 24h - No Blink",3:"Clock 12h - Blink",4:"Clock
//                      12h - No Blink",5:"Date"
//  - Clock-Blink     -- display is automatically updated with current time and blinking dot/lines
//  - Clock-NoBlink   -- display is automatically updated with current time and steady dot/lines
//  - Clock12-Blink   -- display is automatically updated with current time (12h clock) and blinking dot/lines
//  - Clock12-NoBlink -- display is automatically updated with current time (12h clock) and steady dot/lines
//  - Date            -- display is automatically updated with current date
//

/** History
 * 2024-08-23 tonhuisman: The 74HC595 Matrix displays mostly work, with the higest possible refresh rate. Still a bit of flickering,
 *                        but that might require adding extra hardware to solve.
 * 2024-08-08 tonhuisman: Add double-buffering for 74HC595 displays to improve update-speed
 * 2024-08-06 tonhuisman: Make 74HC595 multiplexed displays a separate compile-time option, as these require special treatment
 *                        Separate 7-segment font-related functions for re-use by other plugins
 *                        Size reduction by removing now unneeded function pin arguments
 * 2024-07-30 tonhuisman: Add support for larger combinations of sequential displays, 2..8 digits
 * 2024-07-27 tonhuisman: Move most code to P073 PluginStruct and remove now unneeded code, use explicit compile-time defines (0/1)
 * 2024-07-24 tonhuisman: Fixed the issue that most extended features where not included in the MAX or ESP32 builds
 * 2024-07-20 tonhuisman: Implement 74HC595 7-segment displays (2, 2+2, 3, 2+3, 3+2, 4, 6, 3+3 and 8 digits)
 *                        2 and 3 digit display use sequential data, 4, 6 and 8 digit displays use multiplexing, requiring continuous
 *                        refreshing of the display content.
 *                        The 2 and 3 digit displays can be coupled (output to input) in sets of max. 2 (2+2, 2+3, 3+2, 3+3), and will be
 *                        seen as a single display of 4, 5 or 6 digits.
 *                        Multiplexed displays can _not_ be coupled, nor can the TM1637 or MAX7219 displays.
 * 2023-03-30 tonhuisman: Correct 7dtext on 6-digit TM1637 to also swap the dots when swapping the digits
 * 2023-03-29 tonhuisman: Add option to suppress the leading zero on day and hour when < 10
 *                        Disable scrolling for content/commands that don't support that
 * 2023-03-28 tonhuisman: Guard scrolling feature to only be used for 7dtext and 7dbin commands, and fix scrolling on 6-digit display
 *                        Fix 7dbin command to also work correctly for TM1637 displays
 * 2022-02-03 tonhuisman: Move P073_data_struct to PluginStruct directory, code optimizations, de-duplication
 * 2021-10-06 tonhuisman: Store via commands changed output, font and brightness setting in settings variables, but not save them yet.
 * 2021-10-05 tonhuisman: Add 7output command for changing the Display Output setting. Not saved, unless the save command is also sent.
 * 2021-02-13 tonhuisman: Fixed self-introduced bug of conversion from MAX7219 to TM1637 bit mapping, removed now unused TM1637 character
 *                        maps, moved some logging to DEBUG level
 * 2021-01-30 tonhuisman: Added font support for 7Dgt (default), Siekoo, Siekoo with uppercase CHNORUX, dSEG7 fonts. Default/7Dgt comes
 *                        with these special characters: " -^=/_
 *                        Siekoo comes _without_ AOU with umlauts and Eszett characters, has many extra special characters
 *                        "%@.,;:+*#!?'\"<>\\()|", and optional uppercase "CHNORUX",
 *                        "^" displays as degree symbol and "|" displays overscsore (top-line only).
 *                        'Merged' fontdata for TM1637 by converting the data for MAX7219 (bits 0-6 are swapped around), to save a little
 *                        space and maintanance burden.
 *                        Added 7dfont,<font> command for changing the font dynamically runtime. NB: The numbers digits are equal for all
 *                        fonts!
 *                        Added Scroll Text option for scrolling texts longer then the display is wide
 *                        Added 7dbin,<uint8_t>[,...] for displaying binary formatted data bits clock-wise from left to right, dot, top,
 *                        right 2x, bottom, left 2x, center), scroll-enabled
 * 2021-01-10 tonhuisman: Added optional . as dot display (7dtext)
 *                        Added 7ddt,<temp1>,<temp2> dual temperature display
 *                        Added optional removal of degree symbol on temperature display
 *                        Added optional right-shift of 7dt,<temp> on MAX7219 display (is normally shifted to left by 1 digit, so last
 *                        digit is blank)
 */

# define PLUGIN_073
# define PLUGIN_ID_073           73
# define PLUGIN_NAME_073         "Display - 7-segment display"

# include "src/PluginStructs/P073_data_struct.h"


boolean Plugin_073(uint8_t function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number         = PLUGIN_ID_073;
      Device[deviceCount].Type             = DEVICE_TYPE_TRIPLE;
      Device[deviceCount].VType            = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].Ports            = 0;
      Device[deviceCount].ValueCount       = 0;
      Device[deviceCount].GlobalSyncOption = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_073);
      break;
    }

    # if P073_SCROLL_TEXT || P073_USE_74HC595
    case PLUGIN_SET_DEFAULTS: {
      #  if P073_SCROLL_TEXT
      P073_CFG_SCROLLSPEED = 10; // Default 10 * 0.1 sec scroll speed
      #  endif // if P073_SCROLL_TEXT
      #  if P073_USE_74HC595
      #   if P073_USE_74HCMULTIPLEX
      P073_CFG_DIGITS = 4; // Default number of digits
      #   else // if P073_USE_74HCMULTIPLEX
      P073_CFG_DIGITS = 9; // Default number of digits code, 9 = 4 sequential
      #   endif // if P073_USE_74HCMULTIPLEX
      #  endif // if P073_USE_74HC595
      break;
    }
    # endif // if P073_SCROLL_TEXT || P073_USE_74HC595

    case PLUGIN_WEBFORM_LOAD: {
      addFormNote(F("TM1637:  1st=CLK-Pin, 2nd=DIO-Pin"));
      addFormNote(F("MAX7219: 1st=DIN-Pin, 2nd=CLK-Pin, 3rd=CS-Pin"));
      # if P073_USE_74HC595
      addFormNote(F("74HC595: 1st=SDI/DIO-Pin, 2nd=CLK/SCLK-Pin, 3rd=LOAD/RCLK-Pin"));
      # endif // if P073_USE_74HC595
      {
        const __FlashStringHelper *displtype[] = { F("TM1637 - 4 digit (colon)"),
                                                   F("TM1637 - 4 digit (dots)"),
                                                   F("TM1637 - 6 digit"),
                                                   F("MAX7219 - 8 digit"),
                                                   # if P073_USE_74HC595
                                                   F("74HC595 - 2..8 digit"),
                                                   # endif // if P073_USE_74HC595
        };
        addFormSelector(F("Display Type"), F("displtype"), NR_ELEMENTS(displtype), displtype, nullptr, P073_CFG_DISPLAYTYPE);
      }
      # if P073_USE_74HC595

      if (P073_74HC595_2_8DGT == P073_CFG_DISPLAYTYPE) {
        if (0 == P073_CFG_DIGITS) {
          #  if P073_USE_74HCMULTIPLEX
          P073_CFG_DIGITS = 4;
          #  else // if P073_USE_74HCMULTIPLEX
          P073_CFG_DIGITS = 9;
          #  endif // if P073_USE_74HCMULTIPLEX
        }
        const __FlashStringHelper *digits[] = {
          F("2"),
          F("2+2"),
          F("3"),
          #  if P073_USE_74HCMULTIPLEX
          F("4 multiplexed"),
          #  endif // if P073_USE_74HCMULTIPLEX
          F("4"),
          F("3+2 / 2+3"),
          #  if P073_USE_74HCMULTIPLEX
          F("6 multiplexed"),
          #  endif // if P073_USE_74HCMULTIPLEX
          F("3+3"),
          F("3+4 / 4+3"),
          F("4+4"),
          #  if P073_USE_74HCMULTIPLEX
          F("8 multiplexed"),
          #  endif // if P073_USE_74HCMULTIPLEX
        };
        const int digitsOptions[] = {
          2,
          1,
          3,
          #  if P073_USE_74HCMULTIPLEX
          4,
          #  endif // if P073_USE_74HCMULTIPLEX
          9,
          5,
          #  if P073_USE_74HCMULTIPLEX
          6,
          #  endif // if P073_USE_74HCMULTIPLEX
          7,
          11,
          10,
          #  if P073_USE_74HCMULTIPLEX
          8,
          #  endif // if P073_USE_74HCMULTIPLEX
        };
        addFormSelector(F("Nr. of digits"), F("dgts"), NR_ELEMENTS(digitsOptions), digits, digitsOptions, P073_CFG_DIGITS);
      } else
      # endif // if P073_USE_74HC595
      {
        P073_CFG_DIGITS = P073_getDefaultDigits(P073_CFG_DISPLAYTYPE);
      }

      P073_display_output_selector(F("displout"), P073_CFG_OUTPUTTYPE);

      addFormNumericBox(F("Brightness"), F("brightness"), P073_CFG_BRIGHTNESS, 0, 15);
      addUnit(F("0..15"));

      # if P073_EXTRA_FONTS
      P073_font_selector(F("fontset"), P073_CFG_FONTSET);
      # endif // if P073_EXTRA_FONTS

      addFormSubHeader(F("Options"));

      addFormCheckBox(F("Text show periods as dot"),    F("periods"),     bitRead(P073_CFG_FLAGS, P073_OPTION_PERIOD));

      addFormCheckBox(F("Hide &deg; for Temperatures"), F("hide_degree"), bitRead(P073_CFG_FLAGS, P073_OPTION_HIDEDEGREE));
      # if P073_7DDT_COMMAND
      addFormNote(F("Commands 7dt,&lt;temp&gt; and 7ddt,&lt;temp1&gt;,&lt;temp2&gt;"));
      # else // if P073_7DDT_COMMAND
      addFormNote(F("Command 7dt,&lt;temp&gt;"));
      # endif // if P073_7DDT_COMMAND
      # if P073_SUPPRESS_ZERO
      addFormCheckBox(F("Suppress leading 0 on day/hour"), F("supp0"), bitRead(P073_CFG_FLAGS, P073_OPTION_SUPPRESS0));
      # endif // if P073_SUPPRESS_ZERO

      # if P073_SCROLL_TEXT
      addFormCheckBox(F("Scroll text &gt; display width"), F("scroll_text"), bitRead(P073_CFG_FLAGS, P073_OPTION_SCROLLTEXT));
      addFormCheckBox(F("Scroll text in from right"),      F("scroll_full"), bitRead(P073_CFG_FLAGS, P073_OPTION_SCROLLFULL));

      if (P073_CFG_SCROLLSPEED == 0) { P073_CFG_SCROLLSPEED = 10; }
      addFormNumericBox(F("Scroll speed (0.1 sec/step)"), F("scrollspeed"), P073_CFG_SCROLLSPEED, 1, 600);
      addUnit(F("1..600 = 0.1..60 sec/step"));
      # endif // if P073_SCROLL_TEXT

      # if P073_USE_74HC595
      addFormSubHeader(F("Options for 8 digit displays (MAX7219/74HC595)"));
      # else // if P073_USE_74HC595
      addFormSubHeader(F("Options for MAX7219 - 8 digit"));
      # endif // if P073_USE_74HC595

      bool bRightAlign = bitRead(P073_CFG_FLAGS, P073_OPTION_RIGHTALIGN);
      addFormCheckBox(F("Right-align Temperature (7dt)"), F("temp_rightalign"), bRightAlign);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      P073_CFG_DISPLAYTYPE = getFormItemInt(F("displtype"));
      P073_CFG_OUTPUTTYPE  = getFormItemInt(F("displout"));
      P073_CFG_BRIGHTNESS  = getFormItemInt(F("brightness"));
      uint32_t lSettings = 0;
      bitWrite(lSettings, P073_OPTION_PERIOD,     isFormItemChecked(F("periods")));
      bitWrite(lSettings, P073_OPTION_HIDEDEGREE, isFormItemChecked(F("hide_degree")));
      bitWrite(lSettings, P073_OPTION_RIGHTALIGN, isFormItemChecked(F("temp_rightalign")));
      # if P073_SCROLL_TEXT
      bitWrite(lSettings, P073_OPTION_SCROLLTEXT, isFormItemChecked(F("scroll_text")));
      bitWrite(lSettings, P073_OPTION_SCROLLFULL, isFormItemChecked(F("scroll_full")));
      P073_CFG_SCROLLSPEED = getFormItemInt(F("scrollspeed"));
      # endif // if P073_SCROLL_TEXT
      # if P073_SUPPRESS_ZERO
      bitWrite(lSettings, P073_OPTION_SUPPRESS0, isFormItemChecked(F("supp0")));
      # endif // if P073_SUPPRESS_ZERO
      # if P073_EXTRA_FONTS
      P073_CFG_FONTSET = getFormItemInt(F("fontset"));
      # endif // if P073_EXTRA_FONTS
      # if P073_USE_74HC595

      if (P073_74HC595_2_8DGT == P073_CFG_DISPLAYTYPE) {
        P073_CFG_DIGITS = getFormItemInt(F("dgts"));
      }
      # endif // if P073_USE_74HC595
      P073_CFG_FLAGS = lSettings;

      success = true;
      break;
    }

    case PLUGIN_EXIT: {
      success = true;
      break;
    }

    case PLUGIN_INIT: {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P073_data_struct());
      P073_data_struct *P073_data =
        static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P073_data) {
        P073_data->init(event);

        # if P073_USE_74HC595

        if (P073_data->is74HC595Matrix()) {
          Scheduler.setPluginTaskTimer(0, event->TaskIndex, 0);
        }
        # endif // if P073_USE_74HC595

        success = true;
      }
      break;
    }

    case PLUGIN_WRITE: {
      P073_data_struct *P073_data =
        static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P073_data) {
        success = P073_data->plugin_write(event, string);
      }
      break;
    }

    case PLUGIN_ONCE_A_SECOND: {
      P073_data_struct *P073_data =
        static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P073_data) {
        success = P073_data->plugin_once_a_second(event);
      }

      break;
    }

    # if P073_SCROLL_TEXT
    case PLUGIN_TEN_PER_SECOND: {
      P073_data_struct *P073_data =
        static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P073_data) {
        success = P073_data->plugin_ten_per_second(event);
      }

      break;
    }
    # endif // if P073_SCROLL_TEXT

    # if P073_USE_74HC595

    case PLUGIN_TASKTIMER_IN: {
      P073_data_struct *P073_data =
        static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P073_data) {
        success = P073_data->plugin_fifty_per_second(event);

        if (success) {
          Scheduler.setPluginTaskTimer(1, event->TaskIndex, 0);
        }

        // success = false; // Don't send out to (not configurable) Controllers or Rules
      }

      break;
    }
    # endif // if P073_USE_74HC595
  }
  return success;
}

#endif // ifdef USES_P073
