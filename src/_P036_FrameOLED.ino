#include "_Plugin_Helper.h"
#ifdef USES_P036

// #######################################################################################################
// #################################### Plugin 036: OLED SSD1306 display #################################
//
// This is a modification to Plugin_023 with graphics library provided from squix78 github
// https://github.com/squix78/esp8266-oled-ssd1306
//
// The OLED can display up to 12 strings in four frames - i.e. 12 frames with 1 line, 6 with 2 lines, 4 with 3 lines or 3 with 4 lines.
// The font size is adjusted according to the number of lines required per frame.
//
// Major work on this plugin has been done by 'Namirda'
// Added to the main repository with some optimizations and some limitations.
// Al long as the device is not selected, no RAM is waisted.
//
// @uwekaditz: 2022-05-08
// FIX: left alignment again with leading spaces
// CHG: UI suggestions from tonhuisman
// @uwekaditz: 2022-05-07
// CHG: font Dialog_plain_12 is optional (not used with LIMIT_BUILD_SIZE) 
// @uwekaditz: 2022-04-24
// CHG: font Dialog_plain_18 is optional (not used with LIMIT_BUILD_SIZE)
// @uwekaditz: 2022-04-23
// FIX: empty page was shown if just one frame has text
// @uwekaditz: 2022-04-21
// FIX: last frame was not shown (frameCounter check was wrong)
// @uwekaditz: 2022-04-19
// NEW: use the split token <|> to split lines into left and right part
//      the split token is replaced by a number of space chars to fit the display width
//      if the modified line is longer than the display width (even with only one space as replacement), the modified line will be scrolled, if line scrolling is enabled
//      no need to use a special alignment for displaying the line left and right aligned
// CHG: Setting for user defined contrast simplified (parse int from event->Par3..5)
// @uwekaditz: 2022-04-18
// ADD: Setting and support for user defined contrast: oledframedcmd,display,user,contrast,precharge,comdetect (contrast, precharge and comdetect are integers)
// CHG: setting for low contrast modified, low was invisible!
// @uwekaditz: 2022-04-16
// MSG: code reduced by 530 bytes to fit the build size for 'esp8266, normal_ESP8266_1M' (equal code put into functions)
// @uwekaditz: 2022-04-15
// NEW: individual font settings for each line
// NEW: simple structur tLineSettings to hold all font and page settings
// FIX: while setting new 'Lines per Frame' by 'oledframedcmd' the max page count was not updated
// FIX: 'Tweaked to match the 13 pix font to fit for 4 lines display' did only work for 128x64 displays
// CHG: 'Turn on/off the Indicator if the number of frames changes' did not work if the indicator is not shown, removed because it is not needed
// CHG: CalculateFontSettings() starts with the biggest font
// CHG: 'Update max page count'()' checks only the first byte of the content (0 == empty)
// MSG: Macros for P036_DisplayIsOn and P036_SetDisplayOn for easier code reading
// MSG: new debug logs with P036_CHECK_HEAP and P036_CHECK_INDIVIDUAL_FONT
// MSG: Preperation for css style xwide (define P036_USE_XWIDE)
// @uwekaditz: 2022-04-09
// FIX: wrong pixel calculation for alignment for non-scrolling lines on 64x48 display (the first shown left pixel on 64x48 displays is 32!)
// CHG: new class P036_LineContent(), just for loading the DisplayLinesV1
// @uwekaditz: 2022-04-05
// FIX: wrong alignment (only centered) for long, non-scrolling lines
// FIX: definition of the debug function FontName() was wrong
// @uwekaditz: 2022-04-04
// ADD: global alignment setting includes now also TEXT_ALIGN_RIGHT, paramter moved to PCONFIG_LONG(1)=P036_FLAGS_1 because it needs 2 bits
// CHG: item DisplayLinesV1[].FontHeight renamed into DisplayLinesV1[].ModifyLayout
// CHG: Alignment can be set for each line (saved in DisplayLinesV1[].ModifyLayout Bits 5-3, eAlignment)
// CHG: Preparation to modify font for each line (saved in DisplayLinesV1[].ModifyLayout Bits 2-0, eModifyFont)
// CHG: Device layout uses a sub table with line number, content, modify font and alignment
// CHG: Setting 'Hide indicator' is only visible for 128x64 display, its already true for all other displays
// ADD: Add font Dialog_plain_18 to usable fonts
// ADD: Setting and support for oledframedcmd,align,<0|1|2> subcommand, par2: (0=centre|1=left|2=right)
// @tonhuisman: 2022-02-13
// CHG: Tweak font metrics for 4-lines display to not need to use an 8px font (height 10 pix).
// DEL: Removed the extra fonts added earlier in this change, including the mono-spaced font
// @tonhuisman: 2022-02-06
// ADD: Setting and support for showing the content left-aligned, and oledframedcmd,leftalign,<0|1> subcommand
// CHG: Move #define for settings to P036_data_struct.h file,
// ADD: Setting for always hiding indicator (footer), recalculate fontsize when number of frames changes
// @tonhuisman: 2022-02-05
// CHG: Small optimizations and code improvements, add optional logging for CalculateFontSettings
// @tonhuisman: 2022-02-01
// ADD: oledframedcmd Subcommand 'linecount' for changing the number of lines per frame. Also generates an event on init and when changed,
// when enabled.
// @tonhuisman: 2022-01-31
// BUG: Enable 4 line frames again by using a smaller font and tweaking the footer
// @tonhuisman: 2021-06-11
// ADD: Send events on Frame and Line
// CHG: Setting contrast always turns on display, reduce code size by some refactoring
// CHG: Introduce #define constants for 'magic numbers', replace PCONFIG_LONG(0) by P036_FLAGS_0 to anticipate on a second set of 32 flags
// @tonhuisman: 2021-06-10
// NEW: Add option to send events on Display On/Off (1,0) and Contrast Low/Med/High (0,1,2)
// CHG: Eliminate a couple of superfluous variables saving a little .bin size
// @uwekaditz: 2020-10-19
// CHG: ressouce-saving string calculation
// @uwekaditz: 2020-10-211
// NEW: Support for 128x32 displays (see https://www.letscontrolit.com/forum/viewtopic.php?p=39840#p39840)
// NEW: Option to hide the header
// CHG: Calculate font setting, if necessary reduce lines per page (fonts are not longer a fixed setting)
// CHG: Reduce espeasy_logo to 32x32 to fit all displays
// CHG: Calculate font setting for splash screen
// @uwekaditz: 2020-06-22
// BUG: MaxFramesToDisplay was not updated if all display lines were empty -> display_indicator() crashed due to memory overflow
// CHG: MaxFramesToDisplay will be updated after receiving command with new line content
// CHG: Added some checks if P036_data are valid and if P036_data->isInitialized()
// @uwekaditz: 2020-05-11
// CHG: clearing window for scrolling lines was 1pix too large in y direction
// CHG: font settings for 64x48 updated
// @uwekaditz: 2020-05-05
// CHG: re-schedule Settings.TaskDeviceTimer after JumpToPage to avoid any scheduled page change while jumping to pages
// CHG: correct calculation of the page indicator counts
// @uwekaditz: 2020-05-04
// BUG: temporary pointer to P036_data_struct was not deleted in PLUGIN_WEBFORM_LOAD
// CHG: PLUGIN_WEBFORM_SAVE does not use a temporary pointer to P036_data_struct
// CHG: Jump to any page (Display button or command) is always without page scrolling
// CHG: Jump to next page (Display button) is not allowed while page scrolling is running
// @uwekaditz: 2020-05-03
// CHG: Memory optimization (issue #2799)
// CHG: Added setting for 'Disable scrolling while WiFi is not connected', scrolling is not smooth as long as the ESP tries to connect Wifi
// (Load 100%!)
// BUG: Wifi-Bars not fully right for 128 pixel wide displays
// BUG: clearing window for scrolling lines was too high and to narrow
// BUG: content of lines with pixel width > 255 had to be shortened
// CHG: scrolling lines improved
// CHG: variables renamed for better understanding
// @tonhuisman: 2020-04-13
// CHG: Added setting for 'Step through frames with Display button', when enabled, pressing the button while the display is on
// cycles through the available frames.
// CHG: Added command oledframedcmd,frame,[0..n] to select the next frame (0) or explicitly frame 1..n, where n is the last frame available
// IMP: Improvement to immediately show the frame the text is placed on when text is received.
// @tonhuisman: 2020-03-05
// CHG: Added setting for 'Wake display on receiving text', when unticked doesn't enable the display if it is off by time-out
// @uwekaditz: 2019-11-22
// CHG: Each line can now have 64 characters (version is saved as Bit23-20 in PCONFIG_LONG(0)))
// FIX: Overlapping while page scrolling (size of line content for scrolling pages limited to 128 pixel)
// CHG: Using a calculation to reduce line content for scrolling pages instead of a while loop
// CHG: Using SetBit and GetBit functions to change the content of PCONFIG_LONG(0)
// CHG: Memory usage reduced (only P036_DisplayLinesV1 is now used)
// CHG: using uint8_t and uint16_t instead of uint8_t and word
// @uwekaditz: 2019-11-17
// CHG: commands for P036
// 1. Display commands: oledframedcmd display [on, off, low, med, high]
//    turns display on or off or changes contrast (low, med or high)
// 2. Content commands: oledframedcmd <line> <text> <mode>
//    displays <text> in line (1-12), next frame is showing the changed line, mode not implemented yet
// @uwekaditz: 2019-11-16
// CHG: Header content only changed once within display interval. before every second
// CHG: more predefined constants to understand the meaning of the values
// @uwekaditz: 2019-11-11
// CHG: PageScrolling based on Timer (PLUGIN_TIMER_IN) to reduce time for PLUGIN_READ (blocking) from 700ms to 80ms
// @uwekaditz: 2019-11-05
// NEW: Optional scrolling for long lines (wider than display width)
// @uwekaditz: 2019-11-04
// FIX: Wifi bars for 128x64 and 128x32 displays must be on right side
// FIX: Rotated image did not work for displays with height less then 64 pix
// NEW: Added pagination to customized header
// CHG: Time can be always selected in customized header, even if the time is diplayed by default for 128x68
// @uwekaditz: 2019-11-02
// NEW: more OLED sizes (128x32, 64x48) added to the original 128x64 size
// NEW: Display button can be inverted (saved as Bit16 in PCONFIG_LONG(0))
// NEW: Content of header is adjustable, also the alternating function (saved as Bit 15-0 in PCONFIG_LONG(0))
// CHG: Parameters sorted


# include "src/PluginStructs/P036_data_struct.h"
# ifdef P036_CHECK_HEAP
#  include "src/Helpers/Memory.h"
# endif // ifdef P036_CHECK_HEAP

# define PLUGIN_036
# define PLUGIN_ID_036         36
# define PLUGIN_NAME_036       "Display - OLED SSD1306/SH1106 Framed"
# define PLUGIN_VALUENAME1_036 "OLED"

# ifdef P036_SEND_EVENTS
#  define P036_EVENT_DISPLAY  0 // event: <taskname>#display=0/1
#  define P036_EVENT_CONTRAST 1 // event: <taskname>#contrast=0/1/2
#  define P036_EVENT_FRAME    2 // event: <taskname>#frame=1..n
#  define P036_EVENT_LINE     3 // event: <taskname>#line=1..n
#  define P036_EVENT_LINECNT  4 // event: <taskname>#linecount=1..4
void P036_SendEvent(struct EventStruct *event,
                    uint8_t             eventId,
                    int16_t             eventValue);
# endif // ifdef P036_SEND_EVENTS

# ifdef P036_CHECK_HEAP
void    P036_CheckHeap(String dbgtxt);
# endif // ifdef P036_CHECK_HEAP

boolean Plugin_036(uint8_t function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_036;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 0;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_036);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_036)); // OnOff
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x3c, 0x3d };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 2, i2cAddressValues, P036_ADR);
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      string  = F("Btn: ");
      string += formatGpioLabel(CONFIG_PIN3, false);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
# ifdef PLUGIN_036_DEBUG
      addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_WEBFORM_LOAD ..."));
# endif // PLUGIN_036_DEBUG
# ifdef P036_CHECK_HEAP
      P036_CheckHeap(F("_LOAD: Entering"));
# endif // P036_CHECK_HEAP

      // Use number 5 to remain compatible with existing configurations,
      // but the item should be one of the first choices.
      {
        const __FlashStringHelper *options[2] = {
          F("SSD1306 (128x64 dot controller)"),
          F("SH1106 (132x64 dot controller)")
        };
        const int optionValues[2] = { 1, 2 };
        addFormSelector(F("Controller"), F("p036_controller"), 2, options, optionValues, P036_CONTROLLER);
      }

      {
        const __FlashStringHelper *options[P36_MaxSizesCount] = { F("128x64"), F("128x32"), F("64x48") };
        const int optionValues[P36_MaxSizesCount]                   =
        { static_cast<int>(p036_resolution::pix128x64),
          static_cast<int>(p036_resolution::pix128x32),
          static_cast<int>(p036_resolution::pix64x48) };
        addFormSelector(F("Size"), F("p036_size"), P36_MaxSizesCount, options, optionValues, nullptr, P036_RESOLUTION, true);
      }

      {
        const __FlashStringHelper *options[2] = { F("Normal"), F("Rotated") };
        const int optionValues[2]             = { 1, 2 };
        addFormSelector(F("Rotation"), F("p036_rotate"), 2, options, optionValues, P036_ROTATE);
      }

      {
        p036_resolution tOLEDIndex = static_cast<p036_resolution>(P036_RESOLUTION);
        addFormNumericBox(F("Lines per Frame"),
                          F("p036_nlines"),
                          P036_NLINES,
                          1,
                          P036_data_struct::getDisplaySizeSettings(tOLEDIndex).MaxLines);
      }
# ifdef P036_ENABLE_LEFT_ALIGN
      {
        addFormCheckBox(F("Reduce line number to fit font"), F("p036_ReduceLineNo"), bitRead(P036_FLAGS_1, P036_FLAG_REDUCE_LINE_NO));
        addFormNote(F("When checked, 'Lines per Frame' will be automatically reduced to fit the individual line settings."));
      }
# endif // ifdef P036_ENABLE_LEFT_ALIGN
      {
        const __FlashStringHelper *options[5] = {
          F("Very Slow"),
          F("Slow"),
          F("Fast"),
          F("Very Fast"),
          F("Instant")
        };
        const int optionValues[5] =
        { static_cast<int>(ePageScrollSpeed::ePSS_VerySlow),
          static_cast<int>(ePageScrollSpeed::ePSS_Slow),
          static_cast<int>(ePageScrollSpeed::ePSS_Fast),
          static_cast<int>(ePageScrollSpeed::ePSS_VeryFast),
          static_cast<int>(ePageScrollSpeed::ePSS_Instant) };
        addFormSelector(F("Scroll"), F("p036_scroll"), 5, options, optionValues, P036_SCROLL);
      }

      // FIXME TD-er: Why is this using pin3 and not pin1? And why isn't this using the normal pin selection functions?
      addFormPinSelect(PinSelectPurpose::Generic_input, formatGpioName_input_optional(F("Display button")), F("taskdevicepin3"), CONFIG_PIN3);

      {
        uint8_t choice  = uint8_t(bitRead(P036_FLAGS_0, P036_FLAG_INPUT_PULLUP)); // Bit 26 Input PullUp
        int     Opcount = 2;
# ifdef INPUT_PULLDOWN
        choice += uint8_t(bitRead(P036_FLAGS_0, P036_FLAG_INPUT_PULLDOWN)) * 2;   // Bit 27 Input PullDown

        if (choice > 2) {
          choice = 2;
        }
        Opcount = 3;
# endif // ifdef INPUT_PULLDOWN
        const __FlashStringHelper *options[3] = { F("Input"), F("Input pullup"), F("Input pulldown") };
        const int optionValues[3]             =
        { static_cast<int>(eP036pinmode::ePPM_Input),
          static_cast<int>(eP036pinmode::ePPM_InputPullUp),
          static_cast<int>(eP036pinmode::ePPM_InputPullDown) };
        addFormSelector(F("Pin mode"), F("p036_pinmode"), Opcount, options, optionValues, choice);
      }

      addFormCheckBox(F("Inversed Logic"),                          F("p036_pin3invers"), bitRead(P036_FLAGS_0, P036_FLAG_PIN3_INVERSE));

      addFormCheckBox(F("Step through frames with Display button"), F("p036_StepPages"),  bitRead(P036_FLAGS_0, P036_FLAG_STEP_PAGES_BUTTON));

      addFormNumericBox(F("Display Timeout"), F("p036_timer"), P036_TIMER);

      {
        uint8_t choice = P036_CONTRAST;

        if (choice == 0) { choice = P36_CONTRAST_HIGH; }
        const __FlashStringHelper *options[3] = { F("Low"), F("Medium"), F("High") };
        const int optionValues[3]             = { P36_CONTRAST_LOW, P36_CONTRAST_MED, P36_CONTRAST_HIGH };
        addFormSelector(F("Contrast"), F("p036_contrast"), 3, options, optionValues, choice);
      }

      addFormCheckBox(F("Disable all scrolling while WiFi is disconnected"), F("p036_ScrollWithoutWifi"),
                      !bitRead(P036_FLAGS_0, P036_FLAG_SCROLL_WITHOUTWIFI)); // Bit 24
      addFormNote(F("When checked, all scrollings (pages and lines) are disabled as long as WiFi is not connected."));

# ifdef P036_SEND_EVENTS
      {
        uint8_t choice = 0;
        bitWrite(choice, 0, bitRead(P036_FLAGS_0, P036_FLAG_SEND_EVENTS));
        bitWrite(choice, 1, bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE));
        const __FlashStringHelper *options[3] = {
          F("None"),
          F("Display &amp; Contrast"),
          F("Display, Contrast, Frame, Line &amp; Linecount")
        };
        const int optionValues[3] = { 0, 1, 3 }; // Bitmap
        addFormSelector(F("Generate events"), F("p036_generateEvents"), 3, options, optionValues, choice);

        addFormNote(F("Events: &lt;taskname&gt; #display=1/0 (on/off), #contrast=0/1/2 (low/med/high),"));
        addFormNote(F("and #frame=&lt;framenr&gt;, #line=&lt;linenr&gt; and #linecount=&lt;lines&gt;"));
      }
# endif // ifdef P036_SEND_EVENTS

      addFormSubHeader(F("Content"));

      addFormCheckBox(F("Hide header"), F("p036_HideHeader"), bitRead(P036_FLAGS_0, P036_FLAG_HIDE_HEADER)); // Bit 25
# ifdef P036_ENABLE_HIDE_FOOTER

      if (static_cast<p036_resolution>(P036_RESOLUTION) == p036_resolution::pix128x64) {
        // show CheckBox only if footer can be displayed
        addFormCheckBox(F("Hide indicator"), F("p036_HideFooter"), bitRead(P036_FLAGS_0, P036_FLAG_HIDE_FOOTER)); // Bit 30
      }
# endif // ifdef P036_ENABLE_HIDE_FOOTER

      {
        const __FlashStringHelper *options9[14] =
        { F("SSID"),         F("SysName"),         F("IP"),                 F("MAC"),                 F("RSSI"),                 F("BSSID"),
          F("WiFi channel"), F("Unit"),            F("SysLoad"),            F("SysHeap"),             F("SysStack"),             F("Date"),
          F("Time"),         F("PageNumbers") };
        const int optionValues9[14] =
        { static_cast<int>(eHeaderContent::eSSID),
          static_cast<int>(eHeaderContent::eSysName),
          static_cast<int>(eHeaderContent::eIP),
          static_cast<int>(eHeaderContent::eMAC),
          static_cast<int>(eHeaderContent::eRSSI),
          static_cast<int>(eHeaderContent::eBSSID),
          static_cast<int>(eHeaderContent::eWiFiCh),
          static_cast<int>(eHeaderContent::eUnit),
          static_cast<int>(eHeaderContent::eSysLoad),
          static_cast<int>(eHeaderContent::eSysHeap),
          static_cast<int>(eHeaderContent::eSysStack),
          static_cast<int>(eHeaderContent::eDate),
          static_cast<int>(eHeaderContent::eTime),
          static_cast<int>(eHeaderContent::ePageNo) };
        addFormSelector(F("Header"), F("p036_header"), 14, options9, optionValues9,
                        get8BitFromUL(P036_FLAGS_0, P036_FLAG_HEADER));             // HeaderContent
        addFormSelector(F("Header (alternate)"), F("p036_headerAlternate"), 14, options9, optionValues9,
                        get8BitFromUL(P036_FLAGS_0, P036_FLAG_HEADER_ALTERNATIVE)); // HeaderContentAlternative
      }

      addFormCheckBox(F("Scroll long lines"),              F("p036_ScrollLines"), bitRead(P036_FLAGS_0, P036_FLAG_SCROLL_LINES));

      addFormCheckBox(F("Wake display on receiving text"), F("p036_NoDisplay"),   !bitRead(P036_FLAGS_0, P036_FLAG_NODISPLAY_ONRECEIVE));

      addFormNote(F("When checked, the display wakes up at receiving remote updates."));

      addFormSubHeader(F("Lines"));
# ifdef P036_ENABLE_LEFT_ALIGN
      {
        const __FlashStringHelper *optionsAlignment[3] =
        { F("left"), F("center"), F("right") };
        const int optionValuesAlignment[3] =
        { static_cast<int>(eAlignment::eLeft),
          static_cast<int>(eAlignment::eCenter),
          static_cast<int>(eAlignment::eRight)
        };
        addFormSelector(F("Align content (global)"), F("p036_LeftAlign"), 3, optionsAlignment, optionValuesAlignment,
                        get2BitFromUL(P036_FLAGS_1, P036_FLAG_LEFT_ALIGNED));
      }
# endif // ifdef P036_ENABLE_LEFT_ALIGN

      {
        // For load and save of the display lines, we must not rely on the data in memory.
        // This data in memory can be altered through write commands.
        // Therefore we must read the lines from flash in a temporary object.
# ifdef P036_CHECK_HEAP
        P036_CheckHeap(F("_LOAD: Before (*P036_lines = new)"));
# endif // P036_CHECK_HEAP

        {
          P036_LineContent P036_lines;
# ifdef P036_CHECK_HEAP
          P036_CheckHeap(F("_LOAD: Before loadDisplayLines()"));
# endif // P036_CHECK_HEAP
          P036_lines.loadDisplayLines(event->TaskIndex, get4BitFromUL(P036_FLAGS_0, P036_FLAG_SETTINGS_VERSION)); // Bit23-20 Version
                                                                                                                   // CustomTaskSettings
# ifdef P036_CHECK_HEAP
          P036_CheckHeap(F("_LOAD: After loadDisplayLines()"));
# endif // P036_CHECK_HEAP

          String strID;
          const __FlashStringHelper *optionsFont[5] =
          { F("Use smallest"), F("Reduce to smaller"), F("None"), F("Enlarge to bigger"), F("Use biggest") };
          const int optionValuesFont[5] =
          { static_cast<int>(eModifyFont::eMinimize),
            static_cast<int>(eModifyFont::eReduce),
            static_cast<int>(eModifyFont::eNone),
            static_cast<int>(eModifyFont::eEnlarge),
            static_cast<int>(eModifyFont::eMaximize)
          };
          uint8_t FontChoice[P36_Nlines];

          const __FlashStringHelper *optionsAlignment[4] =
          { F("Use global"), F("left"), F("center"), F("right") };
          const int optionValuesAlignment[4] =
          { static_cast<int>(eAlignment::eGlobal),
            static_cast<int>(eAlignment::eLeft),
            static_cast<int>(eAlignment::eCenter),
            static_cast<int>(eAlignment::eRight)
          };
          uint8_t AlignmentChoice[P36_Nlines];

          addRowLabel(F("Line"));
          html_table(EMPTY_STRING); // Sub-table
          html_table_header(F("&nbsp;#&nbsp;"));
          html_table_header(F("Content"), 500);
          html_table_header(F("Modify font"));
          html_table_header(F("Alignment"));

          for (uint8_t varNr = 0; varNr < P36_Nlines; varNr++)
          {
            html_TR_TD(); // All columns use max. width available
            addHtml(F("&nbsp;"));
            addHtmlInt(varNr + 1);
# ifdef P036_USE_XWIDE
            html_TD(F("padding-right: 8px")); // text box is (100% - 8 pixel) on right side wide
            addTextBox(getPluginCustomArgName(varNr),
                       String(P036_lines.DisplayLinesV1[varNr].Content),
                       P36_NcharsV1 - 1,
                       false,                         // readonly,
                       false,                         // required,
                       EMPTY_STRING,                  // pattern,
                       F("xwide")
                       );                             // class name
# else // ifdef P036_CSS_Changed
            html_TD();                                // content
            addTextBox(getPluginCustomArgName(varNr), // column oly 80% width!
                       String(P036_lines.DisplayLinesV1[varNr].Content),
                       P36_NcharsV1 - 1
                       );
# endif // ifdef P036_CSS_Changed
            html_TD(); // font
            strID             = F("FontID");
            strID            += (varNr + 1);
            FontChoice[varNr] = get3BitFromUL(P036_lines.DisplayLinesV1[varNr].ModifyLayout, P036_FLAG_ModifyLayout_Font);
            addSelector(strID,
                        5,
                        optionsFont,
                        optionValuesFont,
                        nullptr,           // attr[],
                        FontChoice[varNr], // selectedIndex,
                        false,             // reloadonchange,
                        true,              // enabled,
                        EMPTY_STRING       // class name
                        );
            html_TD();                     // alignment
            strID                  = F("AlignmentID");
            strID                 += (varNr + 1);
            AlignmentChoice[varNr] = get3BitFromUL(P036_lines.DisplayLinesV1[varNr].ModifyLayout, P036_FLAG_ModifyLayout_Alignment);
            addSelector(strID,
                        4,
                        optionsAlignment,
                        optionValuesAlignment,
                        nullptr,                // attr[],
                        AlignmentChoice[varNr], // selectedIndex,
                        false,                  // reloadonchange,
                        true,                   // enabled,
                        EMPTY_STRING            // class name
                        );
          }
          html_end_table();
        }
      }

# ifdef P036_CHECK_HEAP
      P036_CheckHeap(F("_LOAD: Before exit"));
# endif // P036_CHECK_HEAP
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
# ifdef PLUGIN_036_DEBUG
      addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_WEBFORM_SAVE ..."));
# endif // PLUGIN_036_DEBUG
# ifdef P036_CHECK_HEAP
      P036_CheckHeap(F("_SAVE: Entering"));
# endif // P036_CHECK_HEAP

      // update now
      Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);

      P036_ADR        = getFormItemInt(F("i2c_addr"));
      P036_ROTATE     = getFormItemInt(F("p036_rotate"));
      P036_NLINES     = getFormItemInt(F("p036_nlines"));
      P036_SCROLL     = getFormItemInt(F("p036_scroll"));
      P036_TIMER      = getFormItemInt(F("p036_timer"));
      P036_CONTROLLER = getFormItemInt(F("p036_controller"));
      P036_CONTRAST   = getFormItemInt(F("p036_contrast"));
      P036_RESOLUTION = getFormItemInt(F("p036_size"));

      uint32_t lSettings = 0;
      set8BitToUL(lSettings, P036_FLAG_HEADER,             uint8_t(getFormItemInt(F("p036_header")) & 0xff));          // HeaderContent
      set8BitToUL(lSettings, P036_FLAG_HEADER_ALTERNATIVE, uint8_t(getFormItemInt(F("p036_headerAlternate")) & 0xff)); // HeaderContentAlternative
      bitWrite(lSettings, P036_FLAG_PIN3_INVERSE,        isFormItemChecked(F("p036_pin3invers")));                     // Pin3Invers
      bitWrite(lSettings, P036_FLAG_SCROLL_LINES,        isFormItemChecked(F("p036_ScrollLines")));                    // ScrollLines
      bitWrite(lSettings, P036_FLAG_NODISPLAY_ONRECEIVE, !isFormItemChecked(F("p036_NoDisplay")));                     // NoDisplayOnReceivingText
      bitWrite(lSettings, P036_FLAG_STEP_PAGES_BUTTON,   isFormItemChecked(F("p036_StepPages")));                      // StepThroughPagesWithButton
      // save CustomTaskSettings always in version V1
      set4BitToUL(lSettings, P036_FLAG_SETTINGS_VERSION, 0x01);                                                        // Bit23-20 Version
                                                                                                                       // CustomTaskSettings
                                                                                                                       // ->
      // version V1
      bitWrite(lSettings, P036_FLAG_SCROLL_WITHOUTWIFI, !isFormItemChecked(F("p036_ScrollWithoutWifi")));              // ScrollWithoutWifi
      bitWrite(lSettings, P036_FLAG_HIDE_HEADER,        isFormItemChecked(F("p036_HideHeader")));                      // Hide header
# ifdef P036_ENABLE_HIDE_FOOTER
      bitWrite(lSettings, P036_FLAG_HIDE_FOOTER,        isFormItemChecked(F("p036_HideFooter")));                      // Hide footer
# endif // ifdef P036_ENABLE_HIDE_FOOTER

      int P036pinmode = getFormItemInt(F("p036_pinmode"));

      switch (P036pinmode) {
        case 1:
        {
          bitWrite(lSettings, P036_FLAG_INPUT_PULLUP, true); // Input PullUp
          break;
        }
        case 2:
        {
          bitWrite(lSettings, P036_FLAG_INPUT_PULLDOWN, true); // Input PullDown
          break;
        }
      }
# ifdef P036_SEND_EVENTS
      uint8_t generateEvents = getFormItemInt(F("p036_generateEvents")) & 0xFF;
      bitWrite(lSettings, P036_FLAG_SEND_EVENTS,       bitRead(generateEvents, 0)); // SendEvents
      bitWrite(lSettings, P036_FLAG_EVENTS_FRAME_LINE, bitRead(generateEvents, 1)); // SendEventsFrameLine
# endif // ifdef P036_SEND_EVENTS

      P036_FLAGS_0 = lSettings;

# ifdef P036_ENABLE_LEFT_ALIGN
      lSettings = 0;
      set2BitToUL(lSettings, P036_FLAG_LEFT_ALIGNED, uint8_t(getFormItemInt(F("p036_LeftAlign")) & 0xff)); // Alignment
      bitWrite(lSettings, P036_FLAG_REDUCE_LINE_NO, isFormItemChecked(F("p036_ReduceLineNo")));            // Reduce line numbers
      P036_FLAGS_1 = lSettings;
# endif // ifdef P036_ENABLE_LEFT_ALIGN

      {
        // For load and save of the display lines, we must not rely on the data in memory.
        // This data in memory can be altered through write commands.
        // Therefore we must use a temporary version to store the settings.
# ifdef P036_CHECK_HEAP
        P036_CheckHeap(F("_SAVE: Before (*P036_lines = new)"));
# endif // P036_CHECK_HEAP

        {
          P036_LineContent P036_lines;
          String   error, strID;
          uint32_t lModifyLayout;

# ifdef P036_CHECK_HEAP
          P036_CheckHeap(F("_SAVE: After (*P036_lines = new)"));
# endif // P036_CHECK_HEAP

          for (uint8_t varNr = 0; varNr < P36_Nlines; varNr++)
          {
            P036_lines.DisplayLinesV1[varNr].Content  = webArg(getPluginCustomArgName(varNr));
            P036_lines.DisplayLinesV1[varNr].FontType = 0xff;
            lModifyLayout                             = 0xC0;                                                   // keep 2 upper bits
                                                                                                                 // untouched
            strID  = F("FontID");
            strID += (varNr + 1);
            set3BitToUL(lModifyLayout, P036_FLAG_ModifyLayout_Font,      uint8_t(getFormItemInt(strID) & 0xff)); // ModifyFont
            strID  = F("AlignmentID");
            strID += (varNr + 1);
            set3BitToUL(lModifyLayout, P036_FLAG_ModifyLayout_Alignment, uint8_t(getFormItemInt(strID) & 0xff)); // Alignment
            P036_lines.DisplayLinesV1[varNr].ModifyLayout = uint8_t(lModifyLayout & 0xff);
            P036_lines.DisplayLinesV1[varNr].FontSpace    = 0xff;
            P036_lines.DisplayLinesV1[varNr].reserved     = 0xff;
          }

          error = P036_lines.saveDisplayLines(event->TaskIndex);
          if (error.length() > 0) {
            addHtmlError(error);
          }
        }
      }

# ifdef PLUGIN_036_DEBUG
      addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_WEBFORM_SAVE Done"));
# endif // PLUGIN_036_DEBUG
# ifdef P036_CHECK_HEAP
      P036_CheckHeap(F("_SAVE: Before exit"));
# endif // P036_CHECK_HEAP
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
# ifdef P036_CHECK_HEAP
      P036_CheckHeap(F("_INIT: Entering"));
# endif // P036_CHECK_HEAP
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P036_data_struct());
# ifdef P036_CHECK_HEAP
      P036_CheckHeap(F("_INIT: Before (*P036_data = static_cast<P036_data_struct *>)"));
# endif // P036_CHECK_HEAP
      P036_data_struct *P036_data =
        static_cast<P036_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P036_data) {
        addLog(LOG_LEVEL_ERROR, F("P036_PLUGIN_INIT: P036_data are zero!"));
        return success;
      }

      // Load the custom settings from flash

      P036_data->bHideHeader = bitRead(P036_FLAGS_0, P036_FLAG_HIDE_HEADER);  // Bit 25 Hide header
      # ifdef P036_ENABLE_HIDE_FOOTER
      P036_data->bHideFooter |= bitRead(P036_FLAGS_0, P036_FLAG_HIDE_FOOTER); // Bit 30 Hide footer
      # endif // ifdef P036_ENABLE_HIDE_FOOTER
      # ifdef P036_ENABLE_LEFT_ALIGN
      P036_data->setTextAlignment(static_cast<eAlignment>(get2BitFromUL(P036_FLAGS_1, P036_FLAG_LEFT_ALIGNED)));
      P036_data->bReduceLinesPerFrame = bitRead(P036_FLAGS_1, P036_FLAG_REDUCE_LINE_NO); // Bit 2 Reduce line number
      # endif // ifdef P036_ENABLE_LEFT_ALIGN

      // Init the display and turn it on
# ifdef P036_CHECK_HEAP
      P036_CheckHeap(F("_INIT: Before P036_data->init()"));
# endif // P036_CHECK_HEAP

      if (!(P036_data->init(event->TaskIndex,
                            get4BitFromUL(P036_FLAGS_0, P036_FLAG_SETTINGS_VERSION), // Bit23-20 Version CustomTaskSettings
                            P036_CONTROLLER,                                         // Type
                            P036_ADR,                                                // I2C address
                            Settings.Pin_i2c_sda,
                            Settings.Pin_i2c_scl,
                            static_cast<p036_resolution>(P036_RESOLUTION),           // OLED index
                            (P036_ROTATE == 2),                                      // 1 = Normal, 2 = Rotated
                            P036_CONTRAST,
                            P036_TIMER,
                            P036_NLINES
                            ))) {
        clearPluginTaskData(event->TaskIndex);
        P036_data = nullptr;

        // success   = true; // INIT was NOT successful!
        break;
      }
# ifdef P036_CHECK_HEAP
      P036_CheckHeap(F("_INIT: After P036_data->init()"));
# endif // P036_CHECK_HEAP

      //      Set the initial value of OnOff to On
      P036_SetDisplayOn(1);
      # ifdef P036_SEND_EVENTS

      if (bitRead(P036_FLAGS_0, P036_FLAG_SEND_EVENTS)) {
        P036_SendEvent(event, P036_EVENT_DISPLAY, 1);

        if (bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE)) {
          #  ifdef P036_ENABLE_LINECOUNT
          P036_SendEvent(event, P036_EVENT_LINECNT, P036_NLINES); // Send the current nr. of lines per frame
          #  endif // P036_ENABLE_LINECOUNT
          P036_SendEvent(event, P036_EVENT_FRAME,   P036_data->currentFrameToDisplay + 1);
        }
      }
      # endif // ifdef P036_SEND_EVENTS

      if (validGpio(CONFIG_PIN3)) {                            // Button related setup
        # ifdef INPUT_PULLDOWN

        if (bitRead(P036_FLAGS_0, P036_FLAG_INPUT_PULLDOWN)) { // Bit 27 Input PullDown
          pinMode(CONFIG_PIN3, INPUT_PULLDOWN);                // Reset pinstate to PIN_MODE_INPUT_PULLDOWN
        }
        else
        # endif // ifdef INPUT_PULLDOWN
        {
          if (bitRead(P036_FLAGS_0, P036_FLAG_INPUT_PULLUP)) { // Bit 26 Input PullUp
            pinMode(CONFIG_PIN3, INPUT_PULLUP);                // Reset pinstate to PIN_MODE_INPUT_PULLUP
          }
          else {
            pinMode(CONFIG_PIN3, INPUT);                       // Reset pinstate to PIN_MODE_INPUT
          }
        }

        P036_data->DebounceCounter = 0;
        P036_data->RepeatCounter   = 0;
        P036_data->ButtonState     = false;
      }

# ifdef PLUGIN_036_DEBUG

      if (P036_data->isInitialized()) {
        addLog(LOG_LEVEL_INFO, F("P036_init Done"));
      } else {
        addLog(LOG_LEVEL_INFO, F("P036_init Not initialized"));
      }
# endif // PLUGIN_036_DEBUG

# ifdef P036_CHECK_HEAP
      P036_CheckHeap(F("_INIT: Before exit"));
# endif // P036_CHECK_HEAP
      success = true;
      break;
    }

    case PLUGIN_EXIT:
    {
# ifdef PLUGIN_036_DEBUG
      addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_EXIT ..."));
# endif // PLUGIN_036_DEBUG
# ifdef P036_CHECK_HEAP
      P036_CheckHeap(F("_EXIT: Before exit"));
# endif // P036_CHECK_HEAP
      success = true;
      break;
    }

    // Check more often for debouncing the button, when enabled
    case PLUGIN_FIFTY_PER_SECOND:
    {
      P036_data_struct *P036_data =
        static_cast<P036_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P036_data) {
        return success;
      }

      if (validGpio(CONFIG_PIN3)) {
        P036_data->registerButtonState(digitalRead(CONFIG_PIN3), bitRead(P036_FLAGS_0, P036_FLAG_PIN3_INVERSE)); // Bit 16
      }
      success = true;
      break;
    }

    // Check frequently to see if we have a pin signal to switch on display
    case PLUGIN_TEN_PER_SECOND:
    {
      P036_data_struct *P036_data =
        static_cast<P036_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P036_data) {
        return success;
      }

      if (P036_DisplayIsOn && (P036_data->disableFrameChangeCnt)) {
        // display is on
        //  disableFrameChangeCnt==0 enables next page change after JumpToPage
        P036_data->disableFrameChangeCnt--;
      }

      P036_data->bAlternativHeader = (++P036_data->HeaderCount > (Settings.TaskDeviceTimer[event->TaskIndex] * 5)); // change header after
                                                                                                                    // half of display time

      if ((validGpio(CONFIG_PIN3)) && P036_data->ButtonState) {
        if (bitRead(P036_FLAGS_0, P036_FLAG_STEP_PAGES_BUTTON) &&                                                   // Bit 19 When display
                                                                                                                    // already on, switch to
                                                                                                                    // next page when
                                                                                                                    // enabled
            P036_DisplayIsOn) {
          if (P036_data->ScrollingPages.Scrolling == 0) {                                                           // page scrolling not
                                                                                                                    // running -> switch to
                                                                                                                    // next page is allowed
            P036_data->P036_JumpToPage(event, 0xFF);                                                                //  Start to display the
                                                                                                                    // next page, function
                                                                                                                    // needs 65ms!
          }
        } else {
          P036_data->display->displayOn();

          P036_SetDisplayOn(1);                 //  Save the fact that the display is now ON
          P036_data->P036_JumpToPage(event, 0); //  Start to display the first page, function needs 65ms!
          # ifdef P036_SEND_EVENTS

          if (bitRead(P036_FLAGS_0, P036_FLAG_SEND_EVENTS)) {
            P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
          }
          # endif // ifdef P036_SEND_EVENTS
        }
        P036_data->markButtonStateProcessed();

# ifdef INPUT_PULLDOWN

        if (bitRead(P036_FLAGS_0, P036_FLAG_INPUT_PULLDOWN)) { // Bit 27 Input PullDown
          pinMode(CONFIG_PIN3, INPUT_PULLDOWN);                // Reset pinstate to PIN_MODE_INPUT_PULLDOWN
        }
        else
        # endif // ifdef INPUT_PULLDOWN
        {
          if (bitRead(P036_FLAGS_0, P036_FLAG_INPUT_PULLUP)) { // Bit 26 Input PullUp
            pinMode(CONFIG_PIN3, INPUT_PULLUP);                // Reset pinstate to PIN_MODE_INPUT_PULLUP
          } else {
            pinMode(CONFIG_PIN3, INPUT);                       // Reset pinstate to PIN_MODE_INPUT
          }
        }
      }

      if (P036_data->bLineScrollEnabled) {
        # ifdef P036_SEND_EVENTS
        uint8_t currentFrame = P036_data->currentFrameToDisplay;
        # endif // ifdef P036_SEND_EVENTS

        if (P036_DisplayIsOn && (P036_data->ScrollingPages.Scrolling == 0)) {
          // Display is on.
          P036_data->display_scrolling_lines(); // line scrolling
        }
        # ifdef P036_SEND_EVENTS

        if (bitRead(P036_FLAGS_0, P036_FLAG_SEND_EVENTS) &&
            bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE) &&
            (currentFrame != P036_data->currentFrameToDisplay)) { // Bit 28 Send Events && Bit 29 Send Events Frame & Line
          P036_SendEvent(event, P036_EVENT_FRAME, P036_data->currentFrameToDisplay + 1);
        }
        # endif // ifdef P036_SEND_EVENTS
      }
      success = true;
      break;
    }

    // Switch off display after displayTimer seconds, update header content
    case PLUGIN_ONCE_A_SECOND:
    {
      if (Settings.TaskDeviceEnabled[event->TaskIndex] == false) {
# ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_ONCE_A_SECOND Not enabled"));
# endif // PLUGIN_036_DEBUG
        return success;
      }
      P036_data_struct *P036_data =
        static_cast<P036_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P036_data) {
# ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_ONCE_A_SECOND NoData"));
# endif // PLUGIN_036_DEBUG
        return success;
      }

      if (!P036_data->isInitialized()) {
# ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_ONCE_A_SECOND Not initialized"));
# endif // PLUGIN_036_DEBUG
        return success;
      }

      if (P036_data->displayTimer > 0) {
        P036_data->displayTimer--;

        if (P036_data->displayTimer == 0) {
          P036_data->display->displayOff();

          P036_SetDisplayOn(0); //  Save the fact that the display is now OFF
          # ifdef P036_SEND_EVENTS

          if (bitRead(P036_FLAGS_0, P036_FLAG_SEND_EVENTS)) {
            P036_SendEvent(event, P036_EVENT_DISPLAY, 0);
          }
          # endif // ifdef P036_SEND_EVENTS
        }
      }

      if (P036_DisplayIsOn) {
        // Display is on.

        P036_data->HeaderContent            = static_cast<eHeaderContent>(get8BitFromUL(P036_FLAGS_0, P036_FLAG_HEADER)); // HeaderContent
        P036_data->HeaderContentAlternative = static_cast<eHeaderContent>(get8BitFromUL(P036_FLAGS_0, P036_FLAG_HEADER_ALTERNATIVE));

        // HeaderContentAlternative
        P036_data->display_header(); // Update Header

        if (P036_data->isInitialized() && P036_data->display_wifibars()) {
          // WiFi symbol was updated.
          P036_data->update_display();
        }
      }

      success = true;
      break;
    }

    case PLUGIN_TIMER_IN:
    {
      if (Settings.TaskDeviceEnabled[event->TaskIndex] == false) {
# ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_TIMER_IN Not enabled"));
# endif // PLUGIN_036_DEBUG
        return success;
      }
      P036_data_struct *P036_data =
        static_cast<P036_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P036_data) {
# ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_TIMER_IN NoData"));
# endif // PLUGIN_036_DEBUG
        return success;
      }

      if (!P036_data->isInitialized()) {
# ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_TIMER_IN Not initialized"));
# endif // PLUGIN_036_DEBUG
        return success;
      }

      # ifdef P036_SEND_EVENTS
      uint8_t currentFrame = P036_data->currentFrameToDisplay;
      # endif // ifdef P036_SEND_EVENTS

      if (P036_DisplayIsOn && P036_data->display_scroll_timer()) {                        // page scrolling only when the
        // display is on
        Scheduler.setPluginTaskTimer(P36_PageScrollTimer, event->TaskIndex, event->Par1); // calls next page scrollng tick
      }
      # ifdef P036_SEND_EVENTS

      if (bitRead(P036_FLAGS_0, P036_FLAG_SEND_EVENTS) &&
          bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE) &&
          (currentFrame != P036_data->currentFrameToDisplay)) { // Bit 28 Send Events && Bit 29 Send Events Frame & Line
        P036_SendEvent(event, P036_EVENT_FRAME, P036_data->currentFrameToDisplay + 1);
      }
      # endif // ifdef P036_SEND_EVENTS
      return success;
    }

    case PLUGIN_READ:
    {
      if (Settings.TaskDeviceEnabled[event->TaskIndex] == false) {
# ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_READ Not enabled"));
# endif // PLUGIN_036_DEBUG
        return success;
      }

      P036_data_struct *P036_data =
        static_cast<P036_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P036_data) {
# ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_READ NoData"));
# endif // PLUGIN_036_DEBUG
        return success;
      }

      if (!P036_data->isInitialized()) {
# ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_READ Not initialized"));
# endif // PLUGIN_036_DEBUG
        return success;
      }

      if (P036_data->disableFrameChangeCnt) {
        //  disable next page change after JumpToPage if PLUGIN_READ was already scheduled
# ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_READ disableFrameChangeCnt"));
# endif // PLUGIN_036_DEBUG
        return success;
      }

      if (P036_data->ScrollingPages.Scrolling == 0) { // page scrolling not running -> switch to next page is allowed
        // Define Scroll area layout
        P036_data->P036_DisplayPage(event);
      } else {
          # ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_READ Page scrolling running"));
          # endif // PLUGIN_036_DEBUG
      }

      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      P036_data_struct *P036_data =
        static_cast<P036_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P036_data) {
# ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_WRITE NoData"));
# endif // PLUGIN_036_DEBUG
        return success;
      }

      if (!P036_data->isInitialized()) {
        return success;
      }

# ifdef PLUGIN_036_DEBUG
      addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_WRITE ..."));
# endif // PLUGIN_036_DEBUG

      String command    = parseString(string, 1);
      String subcommand = parseString(string, 2);
      int    LineNo     = event->Par1;
      # ifdef P036_SEND_EVENTS
      bool sendEvents = bitRead(P036_FLAGS_0, P036_FLAG_SEND_EVENTS); // Bit 28 Send Events
      # endif // ifdef P036_SEND_EVENTS

      if ((command == F("oledframedcmd")) && P036_data->isInitialized()) {
        if (subcommand == F("display")) {
          // display functions
          String para1 = parseString(string, 3);

          if (para1 == F("on")) {
            success                 = true;
            P036_data->displayTimer = P036_TIMER;
            P036_data->display->displayOn();

            P036_SetDisplayOn(1); //  Save the fact that the display is now ON
            # ifdef P036_SEND_EVENTS

            if (sendEvents) {
              P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
            }
            # endif // ifdef P036_SEND_EVENTS
          }

          if (para1 == F("off")) {
            success                 = true;
            P036_data->displayTimer = 0;
            P036_data->display->displayOff();

            P036_SetDisplayOn(0); //  Save the fact that the display is now OFF
            # ifdef P036_SEND_EVENTS

            if (sendEvents) {
              P036_SendEvent(event, P036_EVENT_DISPLAY, 0);
            }
            # endif // ifdef P036_SEND_EVENTS
          }

          if (para1 == F("low")) {
            success = true;
            P036_data->setContrast(P36_CONTRAST_LOW);
            # ifdef P036_SEND_EVENTS

            if (sendEvents) {
              P036_SendEvent(event, P036_EVENT_CONTRAST, 0);

              if (!P036_DisplayIsOn) {
                P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
              }
            }
            # endif // ifdef P036_SEND_EVENTS
            P036_SetDisplayOn(1); //  Save the fact that the display is now ON
          }

          if (para1 == F("med")) {
            success = true;
            P036_data->setContrast(P36_CONTRAST_MED);
            # ifdef P036_SEND_EVENTS

            if (sendEvents) {
              P036_SendEvent(event, P036_EVENT_CONTRAST, 1);

              if (!P036_DisplayIsOn) {
                P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
              }
            }
            # endif // ifdef P036_SEND_EVENTS
            P036_SetDisplayOn(1); //  Save the fact that the display is now ON
          }

          if (para1 == F("high")) {
            success = true;
            P036_data->setContrast(P36_CONTRAST_HIGH);
            # ifdef P036_SEND_EVENTS

            if (sendEvents) {
              P036_SendEvent(event, P036_EVENT_CONTRAST, 2);

              if (!P036_DisplayIsOn) {
                P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
              }
            }
            # endif // ifdef P036_SEND_EVENTS
            P036_SetDisplayOn(1); //  Save the fact that the display is now ON
          }

          if ((para1 == F("user")) &&
              (event->Par3 >= 1) && (event->Par3 <= 255) && // contrast
              (event->Par4 >= 0) && (event->Par4 <= 255) && // precharge
              (event->Par5 >= 0) && (event->Par5 <= 255))   // comdetect
          {
            success = true;
            P036_data->display->setContrast(static_cast<uint8_t>(event->Par3), static_cast<uint8_t>(event->Par4),
                                            static_cast<uint8_t>(event->Par5));
            # ifdef P036_SEND_EVENTS

            if (sendEvents) {
              P036_SendEvent(event, P036_EVENT_CONTRAST, 3);

              if (!P036_DisplayIsOn) {
                P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
              }
            }
            # endif // ifdef P036_SEND_EVENTS
            P036_SetDisplayOn(1); //  Save the fact that the display is now ON
          }
        } else if ((subcommand == F("frame")) &&
                   (event->Par2 >= 0) &&
                   (event->Par2 <= P036_data->MaxFramesToDisplay + 1)) {
          success = true;

          if (!P036_DisplayIsOn) {
            // display was OFF, turn it ON
            P036_data->display->displayOn();
            P036_SetDisplayOn(1); //  Save the fact that the display is now ON
            # ifdef P036_SEND_EVENTS

            if (sendEvents) {
              P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
            }
            # endif // ifdef P036_SEND_EVENTS
          }
          uint8_t nextFrame = (event->Par2 == 0 ? 0xFF : event->Par2 - 1);
          P036_data->P036_JumpToPage(event, nextFrame);                           //  Start to display the selected page, function needs
                                                                                  // 65ms!
          # ifdef P036_SEND_EVENTS

          if (sendEvents && bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE)) { // Bit 29 Send Events Frame & Line
            P036_SendEvent(event, P036_EVENT_FRAME, P036_data->currentFrameToDisplay + 1);
          }
          # endif // ifdef P036_SEND_EVENTS
        }
        # ifdef P036_ENABLE_LINECOUNT
        else if ((subcommand == F("linecount")) &&
                 (event->Par2 >= 1) &&
                 (event->Par2 <= 4)) {
          success = true;

          if (P036_NLINES != event->Par2) {
            P036_NLINES = event->Par2;
            P036_data->setNrLines(P036_NLINES);
            #  ifdef P036_SEND_EVENTS

            if (sendEvents && bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE)) { // Bit 29 Send Events Frame & Line
              P036_SendEvent(event, P036_EVENT_LINECNT, P036_NLINES);
            }
            #  endif // ifdef P036_SEND_EVENTS
          }
        }
        # endif // P036_ENABLE_LINECOUNT
        # ifdef P036_ENABLE_LEFT_ALIGN
        else if ((subcommand == F("leftalign")) && // for compatibility
                 ((event->Par2 == 0) ||
                  (event->Par2 == 1))) {
          success = true;
          eAlignment aAlignment = (event->Par2 == 1 ? eAlignment::eLeft : eAlignment::eCenter);
          P036_data->setTextAlignment(aAlignment);
          uint32_t lSettings = P036_FLAGS_1;
          set2BitToUL(lSettings, P036_FLAG_LEFT_ALIGNED, static_cast<uint8_t>(aAlignment)); // Alignment
          P036_FLAGS_1 = lSettings;
        }
        else if ((subcommand == F("align")) &&
                 ((event->Par2 == 0) ||
                  (event->Par2 == 1) ||
                  (event->Par2 == 2))) {
          success = true;
          eAlignment aAlignment;

          switch (event->Par2) {
            case 1: aAlignment  = eAlignment::eLeft; break;
            case 2: aAlignment  = eAlignment::eRight; break;
            default: aAlignment = eAlignment::eCenter; break;
          }
          P036_data->setTextAlignment(aAlignment);
          uint32_t lSettings = P036_FLAGS_1;
          set2BitToUL(lSettings, P036_FLAG_LEFT_ALIGNED, static_cast<uint8_t>(aAlignment)); // Alignment
          P036_FLAGS_1 = lSettings;
        }
        # endif // ifdef P036_ENABLE_LEFT_ALIGN
        else if ((LineNo > 0) &&
                 (LineNo <= P36_Nlines)) {
          // content functions
          success = true;
          String* currentLine = &P036_data->LineContent->DisplayLinesV1[LineNo - 1].Content;
          *currentLine = parseStringKeepCase(string, 3);
          *currentLine = P036_data->P36_parseTemplate(*currentLine, LineNo - 1);

          // calculate Pix length of new Content
          uint16_t PixLength = P036_data->CalcPixLength(LineNo - 1);

          if (PixLength > 255) {
            String str_error = F("Pixel length of ");
            str_error += PixLength;
            str_error += F(" too long for line! Max. 255 pix!");
            addHtmlError(str_error);

            const int strlen = currentLine->length();
            if (strlen > 0) {
              const float fAvgPixPerChar = static_cast<float>(PixLength) / strlen;
              const int   iCharToRemove  = ceil((static_cast<float>(PixLength - 255)) / fAvgPixPerChar);

              // shorten string because OLED controller can not handle such long strings
              *currentLine = currentLine->substring(0, strlen - iCharToRemove);
            }
          }
          P036_data->MaxFramesToDisplay = 0xff; // update frame count

          # ifdef P036_SEND_EVENTS
          uint8_t currentFrame = P036_data->currentFrameToDisplay;
          # endif // ifdef P036_SEND_EVENTS

          if (!P036_DisplayIsOn &&
              !bitRead(P036_FLAGS_0, P036_FLAG_NODISPLAY_ONRECEIVE)) { // Bit 18 NoDisplayOnReceivedText
            // display was OFF, turn it ON
            P036_data->display->displayOn();
            P036_SetDisplayOn(1);                                      //  Save the fact that the display is now ON
            # ifdef P036_SEND_EVENTS

            if (sendEvents) {
              P036_SendEvent(event, P036_EVENT_DISPLAY, 1);

              if (bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE)) { // Bit 29 Send Events Frame & Line
                P036_SendEvent(event, P036_EVENT_LINE, LineNo);
              }
            }
            # endif // ifdef P036_SEND_EVENTS
          }

          if (P036_DisplayIsOn) {
            P036_data->P036_JumpToPageOfLine(event, LineNo - 1); // Start to display the selected page
            // function needs 65ms!
            # ifdef P036_SEND_EVENTS

            if (sendEvents && bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE) && (currentFrame != P036_data->currentFrameToDisplay)) {
              P036_SendEvent(event, P036_EVENT_FRAME, P036_data->currentFrameToDisplay + 1);
            }
            # endif // ifdef P036_SEND_EVENTS
          }

# ifdef PLUGIN_036_DEBUG
          String log;

          if (loglevelActiveFor(LOG_LEVEL_INFO) &&
              log.reserve(200)) { // estimated
            log += F("[P36] Line: ");
            log += LineNo;
            log += F(" NewContent:");
            log += NewContent;
            log += F(" Content:");
            log += P036_data->DisplayLinesV1[LineNo - 1].Content;
            log += F(" Length:");
            log += P036_data->DisplayLinesV1[LineNo - 1].Content.length();
            log += F(" Pix: ");
            log += P036_data->display->getStringWidth(P036_data->DisplayLinesV1[LineNo - 1].Content);
            log += F(" Reserved:");
            log += P036_data->DisplayLinesV1[LineNo - 1].reserved;
            addLogMove(LOG_LEVEL_INFO, log);
          }
# endif // PLUGIN_036_DEBUG
        }
      }
# ifdef PLUGIN_036_DEBUG

      if (!success && loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("[P36] Cmd: ");
        log += command;
        log += F(" SubCmd:");
        log += subcommand;
        log += F(" Success:");
        log += boolToString(success);
        addLogMove(LOG_LEVEL_INFO, log);
      }
# endif // PLUGIN_036_DEBUG
      break;
    }
  }
  return success;
}

# ifdef P036_SEND_EVENTS
void P036_SendEvent(struct EventStruct *event, uint8_t eventId, int16_t eventValue) {
  if (Settings.UseRules) {
    String RuleEvent;
    RuleEvent.reserve(32); // Guesstimate
    RuleEvent += getTaskDeviceName(event->TaskIndex);
    RuleEvent += '#';

    switch (eventId) {
      case P036_EVENT_DISPLAY:
      {
        RuleEvent += F("display");
        break;
      }
      case P036_EVENT_CONTRAST:
      {
        RuleEvent += F("contrast");
        break;
      }
      case P036_EVENT_FRAME:
      {
        RuleEvent += F("frame");
        break;
      }
      case P036_EVENT_LINE:
      {
        RuleEvent += F("line");
        break;
      }
      #  ifdef P036_ENABLE_LINECOUNT
      case P036_EVENT_LINECNT:
      {
        RuleEvent += F("linecount");
        break;
      }
      #  endif // P036_ENABLE_LINECOUNT
    }
    RuleEvent += '=';
    RuleEvent += eventValue;
    eventQueue.addMove(std::move(RuleEvent));
  }
}

# endif // ifdef P036_SEND_EVENTS

# ifdef P036_CHECK_HEAP
void P036_CheckHeap(String dbgtxt) {
  String log;

  log.reserve(80);
  log += dbgtxt;
  log += F(" FreeHeap:");
  log += ESP.getFreeHeap();
  log += F(" FreeStack:");
  log += getCurrentFreeStack();
  addLog(LOG_LEVEL_INFO, log);
}

# endif // ifdef P036_CHECK_HEAP

#endif // USES_P036
