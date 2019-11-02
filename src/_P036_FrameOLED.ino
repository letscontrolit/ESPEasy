#ifdef USES_P036
//#######################################################################################################
//#################################### Plugin 036: OLED SSD1306 display #################################
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
// @uwekaditz: 2019-11-02
// NEW: more OLED sizes (128x32, 64x48) added to the original 128x64 size
// NEW: Display button can be inverted (saved as Bit16 in Settings.TaskDevicePluginConfigLong[event->TaskIndex][0])
// NEW: Content of header is adjustable, also the alternating function (saved as Bit 15-0 in Settings.TaskDevicePluginConfigLong[event->TaskIndex][0])
// CHG: Parameters sorted

#define PLUGIN_036
#define PLUGIN_ID_036         36
#define PLUGIN_NAME_036       "Display - OLED SSD1306/SH1106 Framed"
#define PLUGIN_VALUENAME1_036 "OLED"

#define P36_Nlines 12        // The number of different lines which can be displayed - each line is 32 chars max
#define P36_Nchars 32
#define P36_MaxSizesCount 3   // number of different OLED sizes
#define P36_MaxDisplayWidth 128

#define P36_CONTRAST_OFF    1
#define P36_CONTRAST_LOW    64
#define P36_CONTRAST_MED  0xCF
#define P36_CONTRAST_HIGH 0xFF


#include "SSD1306.h"
#include "SH1106Wire.h"
#include "OLED_SSD1306_SH1106_images.h"
#include "Dialog_Plain_12_font.h"

#define P36_WIFI_STATE_UNSET          -2
#define P36_WIFI_STATE_NOT_CONNECTED  -1

static int8_t lastWiFiState = P36_WIFI_STATE_UNSET;
static int OLEDIndex = 0;
static boolean Pin3Invers;

enum eHeaderContent {
    eSSID = 1,
    eSysName = 2,
    eIP = 3,
    eMAC = 4,
    eRSSI = 5,
    eBSSID = 6,
    eWiFiCh = 7,
    eUnit = 8,
    eSysLoad = 9,
    eSysHeap = 10,
    eSysStack = 11,
    eTime = 12,
    eDate = 13
};
static eHeaderContent HeaderContent=eSysName;
static eHeaderContent HeaderContentAlternative=eSysName;

typedef struct {
  byte          Top;                  // top in pix for this line setting
  const char    *fontData;            // font for this line setting
  byte          Space;                // space in pix between lines for this line setting
} tFontSettings;

typedef struct {
  byte          Width;                // width in pix
  byte          Height;               // height in pix
  byte          PixLeft;              // first left pix position
  byte          MaxLines;             // max. line count
  tFontSettings L1;                   // settings for 1 line
  tFontSettings L2;                   // settings for 2 lines
  tFontSettings L3;                   // settings for 3 lines
  tFontSettings L4;                   // settings for 4 lines
  byte          WiFiIndicatorWidth;   // width of WiFi indicator
} tSizeSettings;

 const tSizeSettings SizeSettings[P36_MaxSizesCount] = {
   { P36_MaxDisplayWidth, 64, 0,               // 128x64
     4,
     { 20, ArialMT_Plain_24,  0},  //  Width: 24 Height: 28
     { 15, ArialMT_Plain_16, 19},  //  Width: 16 Height: 19
     { 13, Dialog_plain_12,  12},  //  Width: 13 Height: 15
     { 12, ArialMT_Plain_10, 10},  //  Width: 10 Height: 13
     15
   },
   { P36_MaxDisplayWidth, 32, 0,               // 128x32
     2,
     { 14, Dialog_plain_12,   0},  //  Width: 13 Height: 15
     { 12, ArialMT_Plain_10, 10},  //  Width: 10 Height: 13
     {  0, ArialMT_Plain_10,  0},  //  Width: 10 Height: 13 not used!
     {  0, ArialMT_Plain_10,  0},  //  Width: 10 Height: 13 not used!
     10
   },
   { 64, 48, 32,               // 64x48
     3,
     { 20, ArialMT_Plain_24,  0},  //  Width: 24 Height: 28
     { 14, Dialog_plain_12,  16},  //  Width: 13 Height: 15
     { 13, ArialMT_Plain_10, 11},  //  Width: 10 Height: 13
     {  0, ArialMT_Plain_10,  0},  //  Width: 10 Height: 13 not used!
     10
   }
 };

// Instantiate display here - does not work to do this within the INIT call

OLEDDisplay *display=NULL;

String P036_displayLines[P36_Nlines];

void Plugin_036_loadDisplayLines(taskIndex_t taskIndex) {
  LoadCustomTaskSettings(taskIndex, P036_displayLines, P36_Nlines, P36_Nchars);
}

boolean Plugin_036(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  static byte displayTimer = 0;
  static byte frameCounter = 0;       // need to keep track of framecounter from call to call
  // static boolean firstcall = true;      // This is used to clear the init graphic on the first call to read
  static byte nrFramesToDisplay = 0;
  static byte currentFrameToDisplay = 0;

  int linesPerFrame;            // the number of lines in each frame
  int NFrames;                // the number of frames

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_036;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_NONE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 0;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_036);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_036));  // OnOff
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        // Use number 5 to remain compatible with existing configurations,
        // but the item should be one of the first choices.
        addFormSubHeader(F("Display"));
        byte choice5 = PCONFIG(5);
        String options5[2];
        options5[0] = F("SSD1306");
        options5[1] = F("SH1106");
        int optionValues5[2] = { 1, 2 };
        addFormSelector(F("Controller"), F("p036_controller"), 2, options5, optionValues5, choice5);

        byte choice0 = PCONFIG(0);
        /*
        String options0[2];
        options0[0] = F("3C");
        options0[1] = F("3D");
        */
        int optionValues0[2];
        optionValues0[0] = 0x3C;
        optionValues0[1] = 0x3D;
        addFormSelectorI2C(F("p036_adr"), 2, optionValues0, choice0);

        String options8[P36_MaxSizesCount] = { F("128x64"), F("128x32"), F("64x48") };
        int optionValues8[P36_MaxSizesCount] = { 0, 1, 2 };
        addFormSelector(F("Size"),F("p036_size"), P36_MaxSizesCount, options8, optionValues8, NULL, PCONFIG(7), true);

        byte choice1 = PCONFIG(1);
        String options1[2];
        options1[0] = F("Normal");
        options1[1] = F("Rotated");
        int optionValues1[2] = { 1, 2 };
        addFormSelector(F("Rotation"), F("p036_rotate"), 2, options1, optionValues1, choice1);

        OLEDIndex=PCONFIG(7);
        addFormNumericBox(F("Lines per Frame"), F("p036_nlines"), PCONFIG(2), 1, SizeSettings[OLEDIndex].MaxLines);

        byte choice3 = PCONFIG(3);
        String options3[5];
        options3[0] = F("Very Slow");
        options3[1] = F("Slow");
        options3[2] = F("Fast");
        options3[3] = F("Very Fast");
        options3[4] = F("Instant");
        int optionValues3[5];
        optionValues3[0] = 1;
        optionValues3[1] = 2;
        optionValues3[2] = 4;
        optionValues3[3] = 8;
        optionValues3[4] = 32;
        addFormSelector(F("Scroll"), F("p036_scroll"), 5, options3, optionValues3, choice3);
        Plugin_036_loadDisplayLines(event->TaskIndex);

        // FIXME TD-er: Why is this using pin3 and not pin1? And why isn't this using the normal pin selection functions?
        addFormPinSelect(F("Display button"), F("taskdevicepin3"), CONFIG_PIN3);
        Pin3Invers = ((Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] & 0x10000) == 0x10000);  // Bit 16
        addFormCheckBox(F("Inversed Logic"), F("p036_pin3invers"), Pin3Invers);

        addFormNumericBox(F("Display Timeout"), F("p036_timer"), PCONFIG(4));

        byte choice6 = PCONFIG(6);
        if (choice6 == 0) choice6 = P36_CONTRAST_HIGH;
        String options6[3];
        options6[0] = F("Low");
        options6[1] = F("Medium");
        options6[2] = F("High");
        int optionValues6[3];
        optionValues6[0] = P36_CONTRAST_LOW;
        optionValues6[1] = P36_CONTRAST_MED;
        optionValues6[2] = P36_CONTRAST_HIGH;
        addFormSelector(F("Contrast"), F("p036_contrast"), 3, options6, optionValues6, choice6);

        addFormSubHeader(F("Content"));

        byte choice9 = (Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] & 0xff00) >> 8;    // Bit15-8
        byte choice10 = Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] & 0xff; // Bit7-0
        byte MaxHeaderOption;
        String options9[13] = { F("SSID"), F("SysName"), F("IP"), F("MAC"), F("RSSI"), F("BSSID"), F("WiFi channel"), F("Unit"), F("SysLoad"), F("SysHeap"), F("SysStack"), F("Date"), F("Time") };
        int optionValues9[13] = { eSSID, eSysName, eIP, eMAC, eRSSI, eBSSID, eWiFiCh, eUnit, eSysLoad, eSysHeap, eSysStack, eDate, eTime };
        if (SizeSettings[OLEDIndex].Width == P36_MaxDisplayWidth) {
          MaxHeaderOption = 12;
//          String options9a[12] = { F("Date"), F("SSID"), F("SysName"), F("IP"), F("MAC"), F("RSSI"), F("BSSID"), F("WiFi channel"), F("Unit"), F("SysLoad"), F("SysHeap"), F("SysStack") };
          //int optionValues9a[12] = { eDate, eSSID, eSysName, eIP, eMAC, eRSSI, eBSSID, eWiFiCh, eUnit, eSysLoad, eSysHeap, eSysStack};
//          addFormSelector(F("Header"),F("p036_header"), 12, options9a, optionValues9a, choice9);
//          addFormSelector(F("Header (alternating)"),F("p036_headerAlternate"), 12, options9a, optionValues9a, choice10);
        }
        else {
          MaxHeaderOption = 13;
//          String options9b[13] = { F("Time"), F("Date"), F("SSID"), F("SysName"), F("IP"), F("MAC"), F("RSSI"), F("BSSID"), F("WiFi channel"), F("Unit"), F("SysLoad"), F("SysHeap"), F("SysStack") };
//          int optionValues9b[13] = { eTime, eDate, eSSID, eSysName, eIP, eMAC, eRSSI, eBSSID, eWiFiCh, eUnit, eSysLoad, eSysHeap, eSysStack};
        }
        addFormSelector(F("Header"),F("p036_header"), MaxHeaderOption, options9, optionValues9, choice9);
        addFormSelector(F("Header (alternating)"),F("p036_headerAlternate"), MaxHeaderOption, options9, optionValues9, choice10);

        for (byte varNr = 0; varNr < P36_Nlines; varNr++)
        {
          addFormTextBox(String(F("Line ")) + (varNr + 1), getPluginCustomArgName(varNr), P036_displayLines[varNr], P36_Nchars);
        }

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        //update now
        schedule_task_device_timer(event->TaskIndex,
           millis() + (Settings.TaskDeviceTimer[event->TaskIndex] * 1000));
        frameCounter=0;

        PCONFIG(0) = getFormItemInt(F("p036_adr"));
        PCONFIG(1) = getFormItemInt(F("p036_rotate"));
        PCONFIG(2) = getFormItemInt(F("p036_nlines"));
        PCONFIG(3) = getFormItemInt(F("p036_scroll"));
        PCONFIG(4) = getFormItemInt(F("p036_timer"));
        PCONFIG(5) = getFormItemInt(F("p036_controller"));
        PCONFIG(6) = getFormItemInt(F("p036_contrast"));
        PCONFIG(7) = getFormItemInt(F("p036_size"));
        Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] = getFormItemInt(F("p036_header"))*0x100;     // Bit 15-8
        Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] += getFormItemInt(F("p036_headerAlternate")); // Bit 7-0
        if (isFormItemChecked(F("p036_pin3invers")))
          Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] += 0x10000;           // Bit 16

        String error;
        char P036_deviceTemplate[P36_Nlines][P36_Nchars];
        for (byte varNr = 0; varNr < P36_Nlines; varNr++)
        {
          if (!safe_strncpy(P036_deviceTemplate[varNr], WebServer.arg(getPluginCustomArgName(varNr)), P36_Nchars)) {
            error += getCustomTaskSettingsError(varNr);
          }
        }
        if (error.length() > 0) {
          addHtmlError(error);
        }
        SaveCustomTaskSettings(event->TaskIndex, (byte*)&P036_deviceTemplate, sizeof(P036_deviceTemplate));
        // After saving, make sure the active lines are updated.
        Plugin_036_loadDisplayLines(event->TaskIndex);
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        lastWiFiState = P36_WIFI_STATE_UNSET;
        // Load the custom settings from flash
        Plugin_036_loadDisplayLines(event->TaskIndex);

        //      Init the display and turn it on
        if (display)
        {
          display->end();
          delete display;
        }

        uint8_t OLED_address = PCONFIG(0);
        if (PCONFIG(5) == 1) {
          display = new SSD1306Wire(OLED_address, Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);
        } else {
          display = new SH1106Wire(OLED_address, Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);
        }
        display->init();		// call to local override of init function
        display->displayOn();

        uint8_t OLED_contrast = PCONFIG(6);
        P36_setContrast(OLED_contrast);

        //      Set the initial value of OnOff to On
        UserVar[event->BaseVarIndex] = 1;

        //      flip screen if required
        if (PCONFIG(1) == 2)display->flipScreenVertically();

        //      Display the device name, logo, time and wifi
        display_header();
        display_logo();
        display->display();

        //      Set up the display timer
        displayTimer = PCONFIG(4);

        if (CONFIG_PIN3 != -1)
        {
          pinMode(CONFIG_PIN3, INPUT_PULLUP);
        }

        //    Initialize frame counter
        frameCounter = 0;
        nrFramesToDisplay = 1;
        currentFrameToDisplay = 0;

        success = true;
        break;
      }

    case PLUGIN_EXIT:
      {
          if (display)
          {
            display->end();
            delete display;
            display=NULL;
          }
          for (byte varNr = 0; varNr < P36_Nlines; varNr++) {
            P036_displayLines[varNr] = "";
          }
          break;
      }

    // Check frequently to see if we have a pin signal to switch on display
    case PLUGIN_TEN_PER_SECOND:
      {
        if (CONFIG_PIN3 != -1)
        {
          Pin3Invers = ((Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] & 0x10000) == 0x10000);  // Bit 16
          if ((!Pin3Invers && digitalRead(CONFIG_PIN3)) || (Pin3Invers && !digitalRead(CONFIG_PIN3)))
          {
            display->displayOn();
            UserVar[event->BaseVarIndex] = 1;      //  Save the fact that the display is now ON
            displayTimer = PCONFIG(4);
          }
        }
        break;
      }

    // Switch off display after displayTimer seconds
    case PLUGIN_ONCE_A_SECOND:
      {

        if ( displayTimer > 0)
        {
          displayTimer--;
          if (displayTimer == 0)
          {
            display->displayOff();
            UserVar[event->BaseVarIndex] = 0;      //  Save the fact that the display is now OFF
          }
        }
        if (UserVar[event->BaseVarIndex] == 1) {
          // Display is on.
          OLEDIndex = PCONFIG(7);
          HeaderContent = eHeaderContent((Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] & 0xff00) >> 8);     // Bit15-8
          HeaderContentAlternative = eHeaderContent(Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] & 0xff);   // Bit7-0
	        display_header();	// Update Header
          if (display && display_wifibars()) {
            // WiFi symbol was updated.
            display->display();
          }
        }

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        // Clear the init screen if this is the first call
        // if (firstcall)
        // {
        //   display->clear();
        //   firstcall = false;
        // }

        //      Define Scroll area layout
        if (UserVar[event->BaseVarIndex] == 1) {
          // Display is on.
          linesPerFrame = PCONFIG(2);
          NFrames = P36_Nlines / linesPerFrame;
          OLEDIndex = PCONFIG(7);
          HeaderContent = eHeaderContent((Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] & 0xff00) >> 8);     // Bit15-8
          HeaderContentAlternative = eHeaderContent(Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] & 0xff);   // Bit7-0

          //      Now create the string for the outgoing and incoming frames
          String tmpString;
          tmpString.reserve(P36_Nchars);
          String newString[4];
          String oldString[4];

          //      Construct the outgoing string
          for (byte i = 0; i < linesPerFrame; i++)
          {
            tmpString = P036_displayLines[(linesPerFrame * frameCounter) + i];
            oldString[i] = P36_parseTemplate(tmpString, 20);
            oldString[i].trim();
          }

          // now loop round looking for the next frame with some content
          //   skip this frame if all lines in frame are blank
          // - we exit the while loop if any line is not empty
          boolean foundText = false;
          int ntries = 0;
          while (!foundText) {

            //        Stop after framecount loops if no data found
            ntries += 1;
            if (ntries > NFrames) break;

            //        Increment the frame counter
            frameCounter = frameCounter + 1;
            if ( frameCounter > NFrames - 1) {
              frameCounter = 0;
              currentFrameToDisplay = 0;
            }

            //        Contruct incoming strings
            for (byte i = 0; i < linesPerFrame; i++)
            {
              tmpString = P036_displayLines[(linesPerFrame * frameCounter) + i];
              newString[i] = P36_parseTemplate(tmpString, 20);
              newString[i].trim();
              if (newString[i].length() > 0) foundText = true;
            }
            if (foundText) {
              if (frameCounter != 0) {
                ++currentFrameToDisplay;
              }
            }
          }
          if ((currentFrameToDisplay + 1) > nrFramesToDisplay) {
            nrFramesToDisplay = currentFrameToDisplay + 1;
          }

          //      Update display
          display_header();
          if (SizeSettings[OLEDIndex].Width == P36_MaxDisplayWidth) display_indicator(currentFrameToDisplay, nrFramesToDisplay);
  //        display_indicator(frameCounter, NFrames);
          display->display();

          int scrollspeed = PCONFIG(3);
          display_scroll(oldString, newString, linesPerFrame, scrollspeed);
				}
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        // FIXME TD-er: This one is not using parseString* function
        String tmpString  = string;
        int argIndex = tmpString.indexOf(',');
        if (argIndex)
          tmpString = tmpString.substring(0, argIndex);
        if (tmpString.equalsIgnoreCase(F("OLEDFRAMEDCMD")) && display)
        {
          success = true;
          argIndex = string.lastIndexOf(',');
          tmpString = string.substring(argIndex + 1);
          if (tmpString.equalsIgnoreCase(F("Off")))
            P36_setContrast(P36_CONTRAST_OFF);
          else if (tmpString.equalsIgnoreCase(F("On")))
            display->displayOn();
          else if (tmpString.equalsIgnoreCase(F("Low")))
            P36_setContrast(P36_CONTRAST_LOW);
          else if (tmpString.equalsIgnoreCase(F("Med")))
            P36_setContrast(P36_CONTRAST_MED);
          else if (tmpString.equalsIgnoreCase(F("High")))
            P36_setContrast(P36_CONTRAST_HIGH);
        }
        break;
      }

  }
  return success;
}

// Set the display contrast
// really low brightness & contrast: contrast = 10, precharge = 5, comdetect = 0
// normal brightness & contrast:  contrast = 100
void P36_setContrast(uint8_t OLED_contrast) {
  char contrast = 100;
  char precharge = 241;
  char comdetect = 64;
  switch (OLED_contrast) {
    case P36_CONTRAST_OFF:
      if (display) {
        display->displayOff();
      }
      return;
    case P36_CONTRAST_LOW:
      contrast = 10; precharge = 5; comdetect = 0;
      break;
    case P36_CONTRAST_MED:
      contrast = P36_CONTRAST_MED; precharge = 0x1F; comdetect = 64;
      break;
    case P36_CONTRAST_HIGH:
    default:
      contrast = P36_CONTRAST_HIGH; precharge = 241; comdetect = 64;
      break;
  }
  if (display) {
    display->displayOn();
    display->setContrast(contrast, precharge, comdetect);
  }
}

// Perform some specific changes for OLED display
String P36_parseTemplate(String &tmpString, byte lineSize) {
  String result = parseTemplate(tmpString, lineSize);
  // OLED lib uses this routine to convert UTF8 to extended ASCII
  // http://playground.arduino.cc/Main/Utf8ascii
  // Attempt to display euro sign (FIXME)
  /*
  const char euro[4] = {0xe2, 0x82, 0xac, 0}; // Unicode euro symbol
  const char euro_oled[3] = {0xc2, 0x80, 0}; // Euro symbol OLED display font
  result.replace(euro, euro_oled);
  */
/*
  if (tmpString.indexOf('{') != -1) {
    String log = F("Gijs: '");
    log += tmpString;
    log += F("'  hex:");
    for (int i = 0; i < tmpString.length(); ++i) {
      log += ' ';
      log += String(tmpString[i], HEX);
    }
    log += F(" out hex:");
    for (int i = 0; i < result.length(); ++i) {
      log += ' ';
      log += String(result[i], HEX);
    }
    addLog(LOG_LEVEL_INFO, log);
  }
*/
  return result;
}

// The screen is set up as 10 rows at the top for the header, 10 rows at the bottom for the footer and 44 rows in the middle for the scroll region

void display_header() {
  static boolean Alternative = true;
  eHeaderContent _HeaderContent;
  String newString, strHeader;
  if ((HeaderContentAlternative==HeaderContent) || Alternative) {
    _HeaderContent=HeaderContent;
  }
  else _HeaderContent=HeaderContentAlternative;
  switch (_HeaderContent) {
    case eSSID:
      if (WiFiConnected()) strHeader = WiFi.SSID();
      else {
        newString=F("%sysname%");
        strHeader = parseTemplate(newString, 10);
      }
    break;
    case eSysName:
      newString=F("%sysname%");
      strHeader = parseTemplate(newString, 10);
    break;
    case eTime:
      newString=F("%systime%");
      strHeader = parseTemplate(newString, 10);
    break;
    case eDate:
      newString = F("%sysday_0%.%sysmonth_0%.%sysyear%");
      strHeader = parseTemplate(newString, 10);
    break;
    case eIP:
      newString=F("%ip%");
      strHeader = parseTemplate(newString, 10);
    break;
    case eMAC:
      newString=F("%mac%");
      strHeader = parseTemplate(newString, 10);
    break;
    case eRSSI:
      newString=F("%rssi%dB");
      strHeader = parseTemplate(newString, 10);
    break;
    case eBSSID:
      newString=F("%bssid%");
      strHeader = parseTemplate(newString, 10);
    break;
    case eWiFiCh:
      newString=F("Channel: %wi_ch%");
      strHeader = parseTemplate(newString, 10);
    break;
    case eUnit:
      newString=F("Unit: %unit%");
      strHeader = parseTemplate(newString, 10);
    break;
    case eSysLoad:
      newString=F("Load: %sysload%%");
      strHeader = parseTemplate(newString, 10);
    break;
    case eSysHeap:
      newString=F("Mem: %sysheap%");
      strHeader = parseTemplate(newString, 10);
    break;
    case eSysStack:
      newString=F("Stack: %sysstack%");
      strHeader = parseTemplate(newString, 10);
    break;
    default:
      return;
  }
  Alternative = !Alternative;

  strHeader.trim();
  display_title(strHeader);
  // Display time and wifibars both clear area below, so paint them after the title.
  if (SizeSettings[OLEDIndex].Width == P36_MaxDisplayWidth) display_time(); // only for 128pix wide displays
  display_wifibars();
}

void display_time() {
  String dtime = F("%systime%");
  String newString = parseTemplate(dtime, 10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->setColor(BLACK);
  display->fillRect(0, 0, 28, 10);
  display->setColor(WHITE);
  display->drawString(0, 0, newString.substring(0, 5));
}

void display_title(String& title) {
  display->setFont(ArialMT_Plain_10);
  display->setColor(BLACK);
//  display->fillRect(0, 0, 128, 13); // Underscores use a extra lines, clear also.
  display->fillRect(0, 0, 128, 12);   // don't clean line under title.
  display->setColor(WHITE);
  if (SizeSettings[OLEDIndex].Width == P36_MaxDisplayWidth) {
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->drawString(64, 0, title);
  }
  else {
    display->setTextAlignment(TEXT_ALIGN_LEFT);  // Display right of WiFi bars
    display->drawString(SizeSettings[OLEDIndex].PixLeft + SizeSettings[OLEDIndex].WiFiIndicatorWidth + 1, 0, title);
  }
}

void display_logo() {
int left = 24;
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->setColor(BLACK);
  display->fillRect(0, 13, 128, 64);
  display->setColor(WHITE);
  display->drawString(65, 15, F("ESP"));
  display->drawString(65, 34, F("Easy"));
  if (SizeSettings[OLEDIndex].PixLeft<left) left = SizeSettings[OLEDIndex].PixLeft;
  display->drawXbm(left, 13, espeasy_logo_width, espeasy_logo_height, espeasy_logo_bits); // espeasy_logo_width=espeasy_logo_height=36
}

// Draw the frame position

void display_indicator(int iframe, int frameCount) {

  //  Erase Indicator Area

  display->setColor(BLACK);
  display->fillRect(0, 54, 128, 10);
  display->setColor(WHITE);

  // Only display when there is something to display.
  if (frameCount <= 1) return;

  // Display chars as required
  for (byte i = 0; i < frameCount; i++) {
    const char *image;
    if (iframe == i) {
      image = activeSymbole;
    } else {
      image = inactiveSymbole;
    }

    int x, y;

    y = 56;

    // I would like a margin of 20 pixels on each side of the indicator.
    // Therefore the width of the indicator should be 128-40=88 and so space between indicator dots is 88/(framecount-1)
    // The width of the display is 128 and so the first dot must be at x=20 if it is to be centred at 64
    const int number_spaces = frameCount - 1;
    if (number_spaces <= 0)
      return;
    int margin = 20;
    int spacing = (128 - 2 * margin) / number_spaces;
    // Now apply some max of 30 pixels between the indicators and center the row of indicators.
    if (spacing > 30) {
      spacing = 30;
      margin = (128 - number_spaces * spacing) / 2;
    }

    x = margin + (spacing * i);
    display->drawXbm(x, y, 8, 8, image);
  }
}

void display_scroll(String outString[], String inString[], int nlines, int scrollspeed)
{

  // outString contains the outgoing strings in this frame
  // inString contains the incomng strings in this frame
  // nlines is the number of lines in each frame

  int ypos[4]; // ypos contains the heights of the various lines - this depends on the font and the number of lines

  if (nlines == 1)
  {
    display->setFont(SizeSettings[OLEDIndex].L1.fontData);
    ypos[0] = SizeSettings[OLEDIndex].L1.Top;
  }

  if (nlines == 2)
  {
    display->setFont(SizeSettings[OLEDIndex].L2.fontData);
    ypos[0] = SizeSettings[OLEDIndex].L2.Top;
    ypos[1] = ypos[0]+SizeSettings[OLEDIndex].L2.Space;
  }

  if (nlines == 3)
  {
    display->setFont(SizeSettings[OLEDIndex].L3.fontData);
    ypos[0] = SizeSettings[OLEDIndex].L3.Top;
    ypos[1] = ypos[0]+SizeSettings[OLEDIndex].L3.Space;
    ypos[2] = ypos[1]+SizeSettings[OLEDIndex].L3.Space;
  }

  if (nlines == 4)
  {
    display->setFont(SizeSettings[OLEDIndex].L4.fontData);
    ypos[0] = SizeSettings[OLEDIndex].L4.Top;
    ypos[1] = ypos[0]+SizeSettings[OLEDIndex].L4.Space;
    ypos[2] = ypos[1]+SizeSettings[OLEDIndex].L4.Space;
    ypos[3] = ypos[2]+SizeSettings[OLEDIndex].L4.Space;
  }

  display->setTextAlignment(TEXT_ALIGN_CENTER);

  for (byte i = 0; i < 33; i = i + scrollspeed)
  {

    //  Clear the scroll area

    display->setColor(BLACK);
    // We allow 12 pixels at the top because otherwise the wifi indicator gets too squashed!!
    display->fillRect(0, 12, 128, 42);   // scrolling window is 44 pixels high - ie 64 less margin of 10 at top and bottom
    display->setColor(WHITE);
    display->drawLine(0, 12, 128, 12);   // line below title

    // Now draw the strings

    for (byte j = 0; j < nlines; j++)
    {

      display->drawString(64 + (4 * i), ypos[j], outString[j]);

      display->drawString(-64 + (4 * i), ypos[j], inString[j]);
    }

    display->display();

    delay(2);
    //NO, dont use background stuff, causes crashes in this plugin:
    // /backgroundtasks();
  }
}

//Draw Signal Strength Bars, return true when there was an update.
bool display_wifibars() {
  const bool connected = WiFiConnected();
  const int nbars_filled = (WiFi.RSSI() + 100) / 12;  // all bars filled if RSSI better than -46dB
  const int newState = connected ? nbars_filled : P36_WIFI_STATE_NOT_CONNECTED;
  if (newState == lastWiFiState)
    return false; // nothing to do.

  int x = SizeSettings[OLEDIndex].PixLeft;
  int y = 0;
  int size_x = SizeSettings[OLEDIndex].WiFiIndicatorWidth;
  int size_y = 10;
  int nbars = 5;
  int16_t width = (size_x / nbars);
  size_x = width * nbars - 1; // Correct for round errors.

  //  x,y are the x,y locations
  //  sizex,sizey are the sizes (should be a multiple of the number of bars)
  //  nbars is the number of bars and nbars_filled is the number of filled bars.

  //  We leave a 1 pixel gap between bars
  display->setColor(BLACK);
  display->fillRect(x , y, size_x, size_y);
  display->setColor(WHITE);
  if (WiFiConnected()) {
    for (byte ibar = 0; ibar < nbars; ibar++) {
      int16_t height = size_y * (ibar + 1) / nbars;
      int16_t xpos = x + ibar * width;
      int16_t ypos = y + size_y - height;
      if (ibar <= nbars_filled) {
        // Fill complete bar
        display->fillRect(xpos, ypos, width - 1, height);
      } else {
        // Only draw top and bottom.
        display->fillRect(xpos, ypos, width - 1, 1);
        display->fillRect(xpos, y + size_y - 1, width - 1, 1);
      }
    }
  } else {
    // Draw a not connected sign (empty rectangle)
  	display->drawRect(x , y, size_x, size_y-1);
  }
  return true;
}
#endif // USES_P036
