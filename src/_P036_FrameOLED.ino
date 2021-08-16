#include "_Plugin_Helper.h"
#ifdef USES_P036

// #######################################################################################################
// #################################### Plugin 036: OLED SSD1306 display #################################
//
// This is a modification to Plugin_023 with graphics library provided from squix78 github
// https://github.com/squix78/esp8266-oled-ssd1306
//
// The OLED can display up to 12 strings in four frames - i.e. 12 frames with 1 line, 6 with 2 lines or 3 with 4 lines.
// The font size is adjusted according to the number of lines required per frame.
//
// Major work on this plugin has been done by 'Namirda'
// Added to the main repository with some optimizations and some limitations.
// Al long as the device is not selected, no RAM is waisted.
//
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


#include "src/PluginStructs/P036_data_struct.h"

#define PLUGIN_036
#define PLUGIN_ID_036         36
#define PLUGIN_NAME_036       "Display - OLED SSD1306/SH1106 Framed"
#define PLUGIN_VALUENAME1_036 "OLED"

#define P036_ADR         PCONFIG(0)
#define P036_ROTATE      PCONFIG(1)
#define P036_NLINES      PCONFIG(2)
#define P036_SCROLL      PCONFIG(3)
#define P036_TIMER       PCONFIG(4)
#define P036_CONTROLLER  PCONFIG(5)
#define P036_CONTRAST    PCONFIG(6)
#define P036_RESOLUTION  PCONFIG(7)

#define P036_FLAGS_0     PCONFIG_LONG(0)

#define P036_FLAG_HEADER_ALTERNATIVE   0 // Bit 7-0 HeaderContentAlternative
#define P036_FLAG_HEADER               8 // Bit15-8 HeaderContent
#define P036_FLAG_PIN3_INVERSE        16 // Bit 16 Pin3Invers
#define P036_FLAG_SCROLL_LINES        17 // Bit 17 ScrollLines
#define P036_FLAG_NODISPLAY_ONRECEIVE 18 // Bit 18 NoDisplayOnReceivingText
#define P036_FLAG_STEP_PAGES_BUTTON   19 // Bit 19 StepThroughPagesWithButton
#define P036_FLAG_SETTINGS_VERSION    20 // Bit23-20 Version CustomTaskSettings -> version V1
#define P036_FLAG_SCROLL_WITHOUTWIFI  24 // Bit 24 ScrollWithoutWifi
#define P036_FLAG_HIDE_HEADER         25 // Bit 25 Hide header
#define P036_FLAG_INPUT_PULLUP        26 // Bit 26 Input PullUp
#define P036_FLAG_INPUT_PULLDOWN      27 // Bit 27 Input PullDown
#define P036_FLAG_SEND_EVENTS         28 // Bit 28 SendEvents
#define P036_FLAG_EVENTS_FRAME_LINE   29 // Bit 29 SendEvents also on Frame & Line


#ifndef LIMIT_BUILD_SIZE
#define P036_SEND_EVENTS      // Enable sending events on Display On/Off, Contrast Low/Med/High, Frame and Line
#endif

#ifdef P036_SEND_EVENTS
#define P036_EVENT_DISPLAY  0 // event: <taskname>#display=0/1
#define P036_EVENT_CONTRAST 1 // event: <taskname>#contrast=0/1/2
#define P036_EVENT_FRAME    2 // event: <taskname>#frame=1..n
#define P036_EVENT_LINE     3 // event: <taskname>#line=1..n
void P036_SendEvent(struct EventStruct *event, uint8_t eventId, int16_t eventValue);
#endif

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
      const int i2cAddressValues[] = { 0x3c, 0x3d };
      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 2, i2cAddressValues, P036_ADR);
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
#ifdef PLUGIN_036_DEBUG
      addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_WEBFORM_LOAD ..."));
#endif // PLUGIN_036_DEBUG

      // Use number 5 to remain compatible with existing configurations,
      // but the item should be one of the first choices.
      {
        const __FlashStringHelper *  options[2];
        options[0] = F("SSD1306 (128x64 dot controller)");
        options[1] = F("SH1106 (132x64 dot controller)");
        int optionValues[2] = { 1, 2 };
        addFormSelector(F("Controller"), F("p036_controller"), 2, options, optionValues, P036_CONTROLLER);
      }

      {
        const __FlashStringHelper * options[P36_MaxSizesCount]      = { F("128x64"), F("128x32"), F("64x48") };
        int    optionValues[P36_MaxSizesCount] =
        { static_cast<int>(p036_resolution::pix128x64),
          static_cast<int>(p036_resolution::pix128x32),
          static_cast<int>(p036_resolution::pix64x48) };
        addFormSelector(F("Size"), F("p036_size"), P36_MaxSizesCount, options, optionValues, NULL, P036_RESOLUTION, true);
      }

      {
        const __FlashStringHelper *  options[2];
        options[0] = F("Normal");
        options[1] = F("Rotated");
        int optionValues[2] = { 1, 2 };
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
      addFormNote(F("Will be automatically reduced if there is no font to fit this setting."));

      {
        const __FlashStringHelper *  options[5];
        options[0] = F("Very Slow");
        options[1] = F("Slow");
        options[2] = F("Fast");
        options[3] = F("Very Fast");
        options[4] = F("Instant");
        int optionValues[5] =
        { static_cast<int>(ePageScrollSpeed::ePSS_VerySlow),
          static_cast<int>(ePageScrollSpeed::ePSS_Slow),
          static_cast<int>(ePageScrollSpeed::ePSS_Fast),
          static_cast<int>(ePageScrollSpeed::ePSS_VeryFast),
          static_cast<int>(ePageScrollSpeed::ePSS_Instant) };
        addFormSelector(F("Scroll"), F("p036_scroll"), 5, options, optionValues, P036_SCROLL);
      }

      // FIXME TD-er: Why is this using pin3 and not pin1? And why isn't this using the normal pin selection functions?
      addFormPinSelect(PinSelectPurpose::Generic_input, F("Display button"), F("taskdevicepin3"), CONFIG_PIN3);

      {
        uint8_t choice = uint8_t(bitRead(P036_FLAGS_0, P036_FLAG_INPUT_PULLUP)); // Bit 26 Input PullUp
        int Opcount = 2;
#ifdef INPUT_PULLDOWN
        choice += uint8_t(bitRead(P036_FLAGS_0, P036_FLAG_INPUT_PULLDOWN)) * 2;  // Bit 27 Input PullDown
        if (choice > 2) {
          choice = 2;
        }
        Opcount = 3;
#endif
        const __FlashStringHelper *  options[3];
        options[0] = F("Input");
        options[1] = F("Input pullup");
        options[2] = F("Input pulldown");
        int optionValues[3] =
        { static_cast<int>(eP036pinmode::ePPM_Input),
          static_cast<int>(eP036pinmode::ePPM_InputPullUp),
          static_cast<int>(eP036pinmode::ePPM_InputPullDown) };
        addFormSelector(F("Pin mode"), F("p036_pinmode"), Opcount, options, optionValues, choice);
      }

      addFormCheckBox(F("Inversed Logic"),                          F("p036_pin3invers"), bitRead(P036_FLAGS_0, P036_FLAG_PIN3_INVERSE)); // Bit 16

      addFormCheckBox(F("Step through frames with Display button"), F("p036_StepPages"),  bitRead(P036_FLAGS_0, P036_FLAG_STEP_PAGES_BUTTON)); // Bit 19

      addFormNumericBox(F("Display Timeout"), F("p036_timer"), P036_TIMER);

      {
        uint8_t choice = P036_CONTRAST;

        if (choice == 0) { choice = P36_CONTRAST_HIGH; }
        const __FlashStringHelper * options[3];
        options[0] = F("Low");
        options[1] = F("Medium");
        options[2] = F("High");
        int optionValues[3];
        optionValues[0] = P36_CONTRAST_LOW;
        optionValues[1] = P36_CONTRAST_MED;
        optionValues[2] = P36_CONTRAST_HIGH;
        addFormSelector(F("Contrast"), F("p036_contrast"), 3, options, optionValues, choice);
      }

      addFormCheckBox(F("Disable all scrolling while WiFi is disconnected"), F("p036_ScrollWithoutWifi"), !bitRead(P036_FLAGS_0, P036_FLAG_SCROLL_WITHOUTWIFI)); // Bit 24
      addFormNote(F("When checked, all scrollings (pages and lines) are disabled as long as WiFi is not connected."));

      #ifdef P036_SEND_EVENTS
      {
        uint8_t choice = 0;
        bitWrite(choice, 0, bitRead(P036_FLAGS_0, P036_FLAG_SEND_EVENTS));
        bitWrite(choice, 1, bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE));
        const __FlashStringHelper * options[3];
        options[0] = F("None");
        options[1] = F("Display &amp; Contrast");
        options[2] = F("Display, Contrast, Frame &amp; Line");
        int optionValues[3]; // Bitmap
        optionValues[0] = 0;
        optionValues[1] = 1;
        optionValues[2] = 3;
        addFormSelector(F("Generate events"), F("p036_generateEvents"), 3, options, optionValues, choice);
        addFormNote(F("Events named &lt;taskname&gt; #display=1/0 (on/off), #contrast=0/1/2 (low/med/high), #frame=&lt;framenr&gt; and #line=&lt;linenr&gt;"));
      }
      #endif

      addFormSubHeader(F("Content"));

      addFormCheckBox(F("Hide header"), F("p036_HideHeader"), bitRead(P036_FLAGS_0, P036_FLAG_HIDE_HEADER)); // Bit 25

      {
        const __FlashStringHelper *  options9[14] =
        { F("SSID"),         F("SysName"),         F("IP"),                 F("MAC"),                 F("RSSI"),                 F("BSSID"),
          F("WiFi channel"), F("Unit"),            F("SysLoad"),            F("SysHeap"),             F("SysStack"),             F("Date"),
          F("Time"),         F("PageNumbers") };
        int optionValues9[14] =
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
        addFormSelector(F("Header"),             F("p036_header"),          14, options9, optionValues9, get8BitFromUL(P036_FLAGS_0, P036_FLAG_HEADER)); // Bit15-8 HeaderContent
        addFormSelector(F("Header (alternate)"), F("p036_headerAlternate"), 14, options9, optionValues9, get8BitFromUL(P036_FLAGS_0, P036_FLAG_HEADER_ALTERNATIVE)); // Bit7-0 HeaderContentAlternative
      }

      addFormCheckBox(F("Scroll long lines"),              F("p036_ScrollLines"), bitRead(P036_FLAGS_0, P036_FLAG_SCROLL_LINES)); // Bit 17

      addFormCheckBox(F("Wake display on receiving text"), F("p036_NoDisplay"),   !bitRead(P036_FLAGS_0, P036_FLAG_NODISPLAY_ONRECEIVE)); // Bit 18
      addFormNote(F("When checked, the display wakes up at receiving remote updates."));

      {
        // For load and save of the display lines, we must not rely on the data in memory.
        // This data in memory can be altered through write commands.
        // Therefore we must read the lines from flash in a temporary object.
        P036_data_struct *P036_data = new (std::nothrow) P036_data_struct();

        if (nullptr != P036_data) {
          P036_data->loadDisplayLines(event->TaskIndex, get4BitFromUL(P036_FLAGS_0, P036_FLAG_SETTINGS_VERSION)); // Bit23-20 Version CustomTaskSettings

          String strLabel;

          for (uint8_t varNr = 0; varNr < P36_Nlines; varNr++)
          {
            strLabel = F("Line ");
            strLabel += (varNr + 1);
            addFormTextBox(strLabel,
                           getPluginCustomArgName(varNr),
                           String(P036_data->DisplayLinesV1[varNr].Content),
                           P36_NcharsV1 - 1);
          }

          // Need to delete the allocated object here
          delete P036_data;
        }
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
#ifdef PLUGIN_036_DEBUG
      addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_WEBFORM_SAVE ..."));
#endif // PLUGIN_036_DEBUG

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
      set8BitToUL(lSettings, P036_FLAG_HEADER, uint8_t(getFormItemInt(F("p036_header")) & 0xff));          // Bit15-8 HeaderContent
      set8BitToUL(lSettings, P036_FLAG_HEADER_ALTERNATIVE, uint8_t(getFormItemInt(F("p036_headerAlternate")) & 0xff)); // Bit 7-0 HeaderContentAlternative
      bitWrite(lSettings, P036_FLAG_PIN3_INVERSE, isFormItemChecked(F("p036_pin3invers")));                // Bit 16 Pin3Invers
      bitWrite(lSettings, P036_FLAG_SCROLL_LINES, isFormItemChecked(F("p036_ScrollLines")));               // Bit 17 ScrollLines
      bitWrite(lSettings, P036_FLAG_NODISPLAY_ONRECEIVE, !isFormItemChecked(F("p036_NoDisplay")));         // Bit 18 NoDisplayOnReceivingText
      bitWrite(lSettings, P036_FLAG_STEP_PAGES_BUTTON, isFormItemChecked(F("p036_StepPages")));            // Bit 19 StepThroughPagesWithButton
      // save CustomTaskSettings always in version V1
      set4BitToUL(lSettings, P036_FLAG_SETTINGS_VERSION, 0x01);                                            // Bit23-20 Version CustomTaskSettings ->
                                                                                                           // version V1
      bitWrite(lSettings, P036_FLAG_SCROLL_WITHOUTWIFI, !isFormItemChecked(F("p036_ScrollWithoutWifi")));  // Bit 24 ScrollWithoutWifi
      bitWrite(lSettings, P036_FLAG_HIDE_HEADER, isFormItemChecked(F("p036_HideHeader")));                 // Bit 25 Hide header

      int P036pinmode = getFormItemInt(F("p036_pinmode"));
      switch (P036pinmode) {
        case 1:
        {
          bitWrite(lSettings, P036_FLAG_INPUT_PULLUP, true);                                               // Bit 26 Input PullUp
          break;
        }
        case 2:
        {
          bitWrite(lSettings, P036_FLAG_INPUT_PULLDOWN, true);                                            // Bit 27 Input PullDown
          break;
        }
      }
      #ifdef P036_SEND_EVENTS
      uint8_t generateEvents = getFormItemInt(F("p036_generateEvents")) & 0xFF;
      bitWrite(lSettings, P036_FLAG_SEND_EVENTS,       bitRead(generateEvents, 0));                       // Bit 28 SendEvents
      bitWrite(lSettings, P036_FLAG_EVENTS_FRAME_LINE, bitRead(generateEvents, 1));                       // Bit 29 SendEventsFrameLine
      #endif

      P036_FLAGS_0 = lSettings;

      {
        // For load and save of the display lines, we must not rely on the data in memory.
        // This data in memory can be altered through write commands.
        // Therefore we must use a temporary version to store the settings.
        P036_data_struct *P036_data = new (std::nothrow) P036_data_struct();

        if (nullptr != P036_data) {
          String error;

          for (uint8_t varNr = 0; varNr < P36_Nlines; varNr++)
          {
            if (!safe_strncpy(P036_data->DisplayLinesV1[varNr].Content, webArg(getPluginCustomArgName(varNr)), P36_NcharsV1)) {
              error += getCustomTaskSettingsError(varNr);
            }
            P036_data->DisplayLinesV1[varNr].Content[P36_NcharsV1 - 1] = 0; // Terminate in case of uninitalized data
            P036_data->DisplayLinesV1[varNr].FontType                  = 0xff;
            P036_data->DisplayLinesV1[varNr].FontHeight                = 0xff;
            P036_data->DisplayLinesV1[varNr].FontSpace                 = 0xff;
            P036_data->DisplayLinesV1[varNr].reserved                  = 0xff;
          }

          if (error.length() > 0) {
            addHtmlError(error);
          }
          SaveCustomTaskSettings(event->TaskIndex, reinterpret_cast<const uint8_t *>(&(P036_data->DisplayLinesV1)), sizeof(P036_data->DisplayLinesV1));

          // Need to delete the allocated object here
          delete P036_data;
        }
      }

      P036_data_struct *P036_data =
        static_cast<P036_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P036_data) {
        // After saving, make sure the active lines are updated.
        P036_data->frameCounter       = 0;
        P036_data->MaxFramesToDisplay = 0xFF;
        P036_data->disp_resolution   = static_cast<p036_resolution>(P036_RESOLUTION);
        P036_data->loadDisplayLines(event->TaskIndex, 1);
      }

#ifdef PLUGIN_036_DEBUG
      addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_WEBFORM_SAVE Done"));
#endif // PLUGIN_036_DEBUG
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P036_data_struct());
      P036_data_struct *P036_data =
        static_cast<P036_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P036_data) {
        return success;
      }

      // Load the custom settings from flash

      P036_data->bHideHeader = bitRead(P036_FLAGS_0, P036_FLAG_HIDE_HEADER); // Bit 25 Hide header

      // Init the display and turn it on
      if (!(P036_data->init(event->TaskIndex,
                            get4BitFromUL(P036_FLAGS_0, P036_FLAG_SETTINGS_VERSION), // Bit23-20 Version CustomTaskSettings
                            P036_CONTROLLER,                               // Type
                            P036_ADR,                                      // I2C address
                            Settings.Pin_i2c_sda,
                            Settings.Pin_i2c_scl,
                            static_cast<p036_resolution>(P036_RESOLUTION), // OLED index
                            (P036_ROTATE == 2),                            // 1 = Normal, 2 = Rotated
                            P036_CONTRAST,
                            P036_TIMER,
                            P036_NLINES
                            ))) {
        clearPluginTaskData(event->TaskIndex);
        P036_data = nullptr;
        success   = true;
        break;
      }

      //      Set the initial value of OnOff to On
      UserVar[event->BaseVarIndex] = 1;
      #ifdef P036_SEND_EVENTS
      if (bitRead(P036_FLAGS_0, P036_FLAG_SEND_EVENTS)) {
        P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
        if (bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE)) {
          P036_SendEvent(event, P036_EVENT_FRAME, P036_data->currentFrameToDisplay + 1);
        }
      }
      #endif

      if (validGpio(CONFIG_PIN3)) // Button related setup
      {

#ifdef INPUT_PULLDOWN
        if (bitRead(P036_FLAGS_0, P036_FLAG_INPUT_PULLDOWN)) {      // Bit 27 Input PullDown
          pinMode(CONFIG_PIN3, INPUT_PULLDOWN); // Reset pinstate to PIN_MODE_INPUT_PULLDOWN
        }
        else
#endif
        {
          if (bitRead(P036_FLAGS_0, P036_FLAG_INPUT_PULLUP)) {      // Bit 26 Input PullUp
            pinMode(CONFIG_PIN3, INPUT_PULLUP);   // Reset pinstate to PIN_MODE_INPUT_PULLUP
          }
          else {
            pinMode(CONFIG_PIN3, INPUT);          // Reset pinstate to PIN_MODE_INPUT
          }
        }

        P036_data->DebounceCounter = 0;
        P036_data->RepeatCounter   = 0;
        P036_data->ButtonState     = false;
      }

#ifdef PLUGIN_036_DEBUG

      if (P036_data->isInitialized()) {
        addLog(LOG_LEVEL_INFO, F("P036_init Done"));
      } else {
        addLog(LOG_LEVEL_INFO, F("P036_init Not initialized"));
      }
#endif // PLUGIN_036_DEBUG

      success = true;
      break;
    }

    case PLUGIN_EXIT:
    {
#ifdef PLUGIN_036_DEBUG
      addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_EXIT ..."));
#endif // PLUGIN_036_DEBUG
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

      if (validGpio(CONFIG_PIN3))
      {
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

      if ((UserVar[event->BaseVarIndex] == 1) && (P036_data->disableFrameChangeCnt)) {
        // display is on
        //  disableFrameChangeCnt==0 enables next page change after JumpToPage
        P036_data->disableFrameChangeCnt--;
      }

      P036_data->bAlternativHeader = (++P036_data->HeaderCount > (Settings.TaskDeviceTimer[event->TaskIndex] * 5)); // change header after half of display time

      if ((validGpio(CONFIG_PIN3)) && P036_data->ButtonState)
      {
        if (bitRead(P036_FLAGS_0, P036_FLAG_STEP_PAGES_BUTTON) && (UserVar[event->BaseVarIndex] == 1)) { // Bit 19 When display already on, switch to next page when enabled
          if (P036_data->ScrollingPages.Scrolling == 0) {               // page scrolling not running -> switch to next page is allowed
            P036_data->P036_JumpToPage(event, 0xFF);                    //  Start to display the next page, function needs 65ms!
          }
        } else {
          P036_data->display->displayOn();
          UserVar[event->BaseVarIndex] = 1;     //  Save the fact that the display is now ON
          P036_data->P036_JumpToPage(event, 0); //  Start to display the first page, function needs 65ms!
          #ifdef P036_SEND_EVENTS
          if (bitRead(P036_FLAGS_0, P036_FLAG_SEND_EVENTS)) {
            P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
          }
          #endif
        }
        P036_data->markButtonStateProcessed();

#ifdef INPUT_PULLDOWN
        if (bitRead(P036_FLAGS_0, P036_FLAG_INPUT_PULLDOWN)) {      // Bit 27 Input PullDown
          pinMode(CONFIG_PIN3, INPUT_PULLDOWN); // Reset pinstate to PIN_MODE_INPUT_PULLDOWN
        }
        else
#endif
        {
          if (bitRead(P036_FLAGS_0, P036_FLAG_INPUT_PULLUP)) {      // Bit 26 Input PullUp
            pinMode(CONFIG_PIN3, INPUT_PULLUP);   // Reset pinstate to PIN_MODE_INPUT_PULLUP
          }
          else {
            pinMode(CONFIG_PIN3, INPUT);          // Reset pinstate to PIN_MODE_INPUT
          }
        }

      }

      if (P036_data->bLineScrollEnabled) {
        #ifdef P036_SEND_EVENTS
        uint8_t currentFrame = P036_data->currentFrameToDisplay;
        #endif
        if ((UserVar[event->BaseVarIndex] == 1) && (P036_data->ScrollingPages.Scrolling == 0)) {
          // Display is on.
          P036_data->display_scrolling_lines(); // line scrolling
        }
        #ifdef P036_SEND_EVENTS
        if (bitRead(P036_FLAGS_0, P036_FLAG_SEND_EVENTS) &&
            bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE) && 
            currentFrame != P036_data->currentFrameToDisplay) { // Bit 28 Send Events && Bit 29 Send Events Frame & Line
          P036_SendEvent(event, P036_EVENT_FRAME, P036_data->currentFrameToDisplay + 1);
        }
        #endif
      }
      success = true;
      break;
    }

    // Switch off display after displayTimer seconds, update header content
    case PLUGIN_ONCE_A_SECOND:
    {
      if (Settings.TaskDeviceEnabled[event->TaskIndex] == false) {
#ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_ONCE_A_SECOND Not enabled"));
#endif // PLUGIN_036_DEBUG
        return success;
      }
      P036_data_struct *P036_data =
        static_cast<P036_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P036_data) {
#ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_ONCE_A_SECOND NoData"));
#endif // PLUGIN_036_DEBUG
        return success;
      }

      if (!P036_data->isInitialized()) {
#ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_ONCE_A_SECOND Not initialized"));
#endif // PLUGIN_036_DEBUG
        return success;
      }

      if (P036_data->displayTimer > 0)
      {
        P036_data->displayTimer--;

        if (P036_data->displayTimer == 0)
        {
          P036_data->display->displayOff();
          UserVar[event->BaseVarIndex] = 0; //  Save the fact that the display is now OFF
          #ifdef P036_SEND_EVENTS
          if (bitRead(P036_FLAGS_0, P036_FLAG_SEND_EVENTS)) {
            P036_SendEvent(event, P036_EVENT_DISPLAY, 0);
          }
          #endif
        }
      }

      if (UserVar[event->BaseVarIndex] == 1) {
        // Display is on.

        P036_data->HeaderContent            = static_cast<eHeaderContent>(get8BitFromUL(P036_FLAGS_0, P036_FLAG_HEADER)); // Bit15-8 HeaderContent
        P036_data->HeaderContentAlternative = static_cast<eHeaderContent>(get8BitFromUL(P036_FLAGS_0, P036_FLAG_HEADER_ALTERNATIVE)); // Bit 7-0
                                                                                                              // HeaderContentAlternative
        P036_data->display_header();                                                                          // Update Header

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
#ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_TIMER_IN Not enabled"));
#endif // PLUGIN_036_DEBUG
        return success;
      }
      P036_data_struct *P036_data =
        static_cast<P036_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P036_data) {
#ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_TIMER_IN NoData"));
#endif // PLUGIN_036_DEBUG
        return success;
      }

      if (!P036_data->isInitialized()) {
#ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_TIMER_IN Not initialized"));
#endif // PLUGIN_036_DEBUG
        return success;
      }

      #ifdef P036_SEND_EVENTS
      uint8_t currentFrame = P036_data->currentFrameToDisplay;
      #endif
      if ((UserVar[event->BaseVarIndex] == 1) && P036_data->display_scroll_timer()) {     // page scrolling only when the display is on
        Scheduler.setPluginTaskTimer(P36_PageScrollTimer, event->TaskIndex, event->Par1); // calls next page scrollng tick
      }
      #ifdef P036_SEND_EVENTS
      if (bitRead(P036_FLAGS_0, P036_FLAG_SEND_EVENTS) &&
          bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE) &&
          currentFrame != P036_data->currentFrameToDisplay) { // Bit 28 Send Events && Bit 29 Send Events Frame & Line
        P036_SendEvent(event, P036_EVENT_FRAME, P036_data->currentFrameToDisplay + 1);
      }
      #endif
      return success;
    }

    case PLUGIN_READ:
    {
      if (Settings.TaskDeviceEnabled[event->TaskIndex] == false) {
#ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_READ Not enabled"));
#endif // PLUGIN_036_DEBUG
        return success;
      }

      P036_data_struct *P036_data =
        static_cast<P036_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P036_data) {
#ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_READ NoData"));
#endif // PLUGIN_036_DEBUG
        return success;
      }

      if (!P036_data->isInitialized()) {
#ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_READ Not initialized"));
#endif // PLUGIN_036_DEBUG
        return success;
      }

      if (P036_data->disableFrameChangeCnt) {
        //  disable next page change after JumpToPage if PLUGIN_READ was already scheduled
#ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_READ disableFrameChangeCnt"));
#endif // PLUGIN_036_DEBUG
        return success;
      }

      if (P036_data->ScrollingPages.Scrolling == 0) { // page scrolling not running -> switch to next page is allowed
        // Define Scroll area layout
        P036_data->P036_DisplayPage(event);
      } else {
          #ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_READ Page scrolling running"));
          #endif // PLUGIN_036_DEBUG
      }

      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      P036_data_struct *P036_data =
        static_cast<P036_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P036_data) {
#ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_WRITE NoData"));
#endif // PLUGIN_036_DEBUG
        return success;
      }

      if (!P036_data->isInitialized()) {
        return success;
      }

#ifdef PLUGIN_036_DEBUG
      addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_WRITE ..."));
#endif // PLUGIN_036_DEBUG

      String command    = parseString(string, 1);
      String subcommand = parseString(string, 2);
      int    LineNo     = event->Par1;
      #ifdef P036_SEND_EVENTS
      bool   sendEvents = bitRead(P036_FLAGS_0, P036_FLAG_SEND_EVENTS); // Bit 28 Send Events
      #endif

      if ((command == F("oledframedcmd")) && P036_data->isInitialized()) {
        if (subcommand == F("display"))
        {
          // display functions
          String para1 = parseString(string, 3);

          if (para1 == F("on")) {
            success                 = true;
            P036_data->displayTimer = P036_TIMER;
            P036_data->display->displayOn();
            UserVar[event->BaseVarIndex] = 1; //  Save the fact that the display is now ON
            #ifdef P036_SEND_EVENTS
            if (sendEvents) {
              P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
            }
            #endif
          }

          if (para1 == F("off")) {
            success                 = true;
            P036_data->displayTimer = 0;
            P036_data->display->displayOff();
            UserVar[event->BaseVarIndex] = 0; //  Save the fact that the display is now OFF
            #ifdef P036_SEND_EVENTS
            if (sendEvents) {
              P036_SendEvent(event, P036_EVENT_DISPLAY, 0);
            }
            #endif
          }

          if (para1 == F("low")) {
            success = true;
            P036_data->setContrast(P36_CONTRAST_LOW);
            #ifdef P036_SEND_EVENTS
            if (sendEvents) {
              P036_SendEvent(event, P036_EVENT_CONTRAST, 0);
              if (UserVar[event->BaseVarIndex] == 0) {
                P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
              }
            }
            #endif
            UserVar[event->BaseVarIndex] = 1; //  Save the fact that the display is now ON
          }

          if (para1 == F("med")) {
            success = true;
            P036_data->setContrast(P36_CONTRAST_MED);
            #ifdef P036_SEND_EVENTS
            if (sendEvents) {
              P036_SendEvent(event, P036_EVENT_CONTRAST, 1);
              if (UserVar[event->BaseVarIndex] == 0) {
                P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
              }
            }
            #endif
            UserVar[event->BaseVarIndex] = 1; //  Save the fact that the display is now ON
          }

          if (para1 == F("high")) {
            success = true;
            P036_data->setContrast(P36_CONTRAST_HIGH);
            #ifdef P036_SEND_EVENTS
            if (sendEvents) {
              P036_SendEvent(event, P036_EVENT_CONTRAST, 2);
              if (UserVar[event->BaseVarIndex] == 0) {
                P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
              }
            }
            #endif
            UserVar[event->BaseVarIndex] = 1; //  Save the fact that the display is now ON
          }
        }
        else if ((subcommand == F("frame")) && (event->Par2 >= 0) && (event->Par2 <= P036_data->MaxFramesToDisplay + 1))
        {
          success = true;

          if (UserVar[event->BaseVarIndex] == 0) {
            // display was OFF, turn it ON
            P036_data->display->displayOn();
            UserVar[event->BaseVarIndex] = 1;           //  Save the fact that the display is now ON
            #ifdef P036_SEND_EVENTS
            if (sendEvents) {
              P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
            }
            #endif
          }
          uint8_t nextFrame = (event->Par2 == 0 ? 0xFF : event->Par2 - 1);
          P036_data->P036_JumpToPage(event, nextFrame); //  Start to display the selected page, function needs 65ms!
          #ifdef P036_SEND_EVENTS
          if (sendEvents && bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE)) { // Bit 29 Send Events Frame & Line
            P036_SendEvent(event, P036_EVENT_FRAME, P036_data->currentFrameToDisplay + 1);
          }
          #endif
        }
        else if ((LineNo > 0) && (LineNo <= P36_Nlines))
        {
          // content functions
          success = true;
          String NewContent = parseStringKeepCase(string, 3);
          NewContent = P036_data->P36_parseTemplate(NewContent, 20);

          if (!safe_strncpy(P036_data->DisplayLinesV1[LineNo - 1].Content, NewContent, P36_NcharsV1)) {
            addHtmlError(getCustomTaskSettingsError(LineNo - 1));
          }
          P036_data->DisplayLinesV1[LineNo - 1].Content[P36_NcharsV1 - 1] = 0;                    // Terminate in case of uninitalized data
          P036_data->DisplayLinesV1[LineNo - 1].reserved                  = (event->Par3 & 0xFF); // not implemented yet

          // calculate Pix length of new Content
          P036_data->display->setFont(P036_data->ScrollingPages.Font);
          uint16_t PixLength = P036_data->display->getStringWidth(P036_data->DisplayLinesV1[LineNo - 1].Content);

          if (PixLength > 255) {
            String str_error = F("Pixel length of ");
            str_error += PixLength;
            str_error += F(" too long for line! Max. 255 pix!");
            addHtmlError(str_error);

            const int strlen = strnlen_P(P036_data->DisplayLinesV1[LineNo - 1].Content, sizeof(P036_data->DisplayLinesV1[LineNo - 1].Content));
            if (strlen > 0) {
              const float fAvgPixPerChar = static_cast<float>(PixLength) / strlen;
              const int   iCharToRemove  = ceil((static_cast<float>(PixLength - 255)) / fAvgPixPerChar);

              // shorten string because OLED controller can not handle such long strings
              P036_data->DisplayLinesV1[LineNo - 1].Content[strlen - iCharToRemove] = 0;
            }
          }
          P036_data->MaxFramesToDisplay = 0xff;                         // update frame count

          #ifdef P036_SEND_EVENTS
          uint8_t currentFrame = P036_data->currentFrameToDisplay;
          #endif
          if ((UserVar[event->BaseVarIndex] == 0) && !bitRead(P036_FLAGS_0, P036_FLAG_NODISPLAY_ONRECEIVE)) { // Bit 18 NoDisplayOnReceivedText
            // display was OFF, turn it ON
            P036_data->display->displayOn();
            UserVar[event->BaseVarIndex] = 1; //  Save the fact that the display is now ON
            #ifdef P036_SEND_EVENTS
            if (sendEvents) {
              P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
              if (bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE)) { // Bit 29 Send Events Frame & Line
                P036_SendEvent(event, P036_EVENT_LINE, LineNo);
              }
            }
            #endif
          }

          if (UserVar[event->BaseVarIndex] == 1) {
            uint8_t nextFrame = ceil((static_cast<float>(LineNo)) / P036_data->ScrollingPages.linesPerFrame) - 1; // next frame shows the new content,
                                                                                                     // 0-based
            P036_data->P036_JumpToPage(event, nextFrame);                                            //  Start to display the selected page,
            // function needs 65ms!
            #ifdef P036_SEND_EVENTS
            if (sendEvents && bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE) && currentFrame != P036_data->currentFrameToDisplay) {
              P036_SendEvent(event, P036_EVENT_FRAME, P036_data->currentFrameToDisplay + 1);
            }
            #endif
          }

#ifdef PLUGIN_036_DEBUG
          String log;
          log.reserve(200); // estimated
          log = F("[P36] Line: ");
          log += LineNo;
          log += F(" NewContent:");
          log += NewContent;
          log += F(" Content:");
          log += String(P036_data->DisplayLinesV1[LineNo - 1].Content);
          log += F(" Length:");
          log += P036_data->DisplayLinesV1[LineNo - 1].Content).length();
          log += F(" Pix: ");
          log += P036_data->display->getStringWidth(P036_data->DisplayLinesV1[LineNo - 1].Content);
          log += F(" Reserved:");
          log += P036_data->DisplayLinesV1[LineNo - 1].reserved;
          addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_036_DEBUG
        }
      }
#ifdef PLUGIN_036_DEBUG

      if (!success) {
        String log = F("[P36] Cmd: ");
        log += command;
        log += F(" SubCmd:");
        log += subcommand;
        log += F(" Success:"):
        log += jsonBool(success);
        addLog(LOG_LEVEL_INFO, log);
      }
#endif // PLUGIN_036_DEBUG
      break;
    }
  }
  return success;
}

#ifdef P036_SEND_EVENTS
void P036_SendEvent(struct EventStruct *event, uint8_t eventId, int16_t eventValue) {
  if (Settings.UseRules) {
    String RuleEvent;
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
    }
    RuleEvent += '=';
    RuleEvent += eventValue;
    eventQueue.addMove(std::move(RuleEvent));
  }
}
#endif

#endif // USES_P036
