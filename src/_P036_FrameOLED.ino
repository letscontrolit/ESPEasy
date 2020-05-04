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
// @uwekaditz: 2020-05-04
// BUG: temporary pointer to P036_data_struct was not deleted in PLUGIN_WEBFORM_LOAD
// CHG: PLUGIN_WEBFORM_SAVE does not use a temporary pointer to P036_data_struct
// @uwekaditz: 2020-05-03
// CHG: Memory optimization (issue #2799)
// CHG: Added setting for 'Disable scrolling while WiFi is not connected', scrolling is not smooth as long as the ESP tries to connect Wifi (Load 100%!)
// BUG: Wifi-Bars not fully right for 128 pixel wide displays
// BUG: clearing window for scrolling lines was too high and to narrow
// BUG: content of lines with pixel width > 255 had to be shortened
// CHG: scrolling lines improved
// CHG: variables renamed for better understanding
// @tonhuisman: 2020-03-05
// CHG: Added setting for 'Wake display on receiving text', when unticked doesn't enable the display if it is off by time-out
// @uwekaditz: 2019-11-22
// CHG: Each line can now have 64 characters (version is saved as Bit23-20 in PCONFIG_LONG(0)))
// FIX: Overlapping while page scrolling (size of line content for scrolling pages limited to 128 pixel)
// CHG: Using a calculation to reduce line content for scrolling pages instead of a while loop
// CHG: Using SetBit and GetBit functions to change the content of PCONFIG_LONG(0)
// CHG: Memory usage reduced (only P036_DisplayLinesV1 is now used)
// CHG: using uint8_t and uint16_t instead of byte and word
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

#include "_Plugin_Helper.h"

#define PLUGIN_036
#define PLUGIN_ID_036         36
#define PLUGIN_NAME_036       "Display - OLED SSD1306/SH1106 Framed"
#define PLUGIN_VALUENAME1_036 "OLED"
//#define PLUGIN_036_DEBUG    // additional debug messages in the log

#define P36_Nlines 12         // The number of different lines which can be displayed - each line is 64 chars max
#define P36_NcharsV0 32       // max chars per line up to 22.11.2019 (V0)
#define P36_NcharsV1 64       // max chars per line from 22.11.2019 (V1)
#define P36_MaxSizesCount 3   // number of different OLED sizes

#define P36_MaxDisplayWidth 128
#define P36_MaxDisplayHeight 64
#define P36_DisplayCentre 64
#define P36_HeaderHeight 12
#define P036_IndicatorTop 54
#define P036_IndicatorHeight 10

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
#define P36_MAX_LinesPerPage          4
#define P36_WaitScrollLines           5   // wait 0.5s before and after scrolling line
#define P36_PageScrollTimer           25  // timer for page Scrolling
#define P36_PageScrollTick            (P36_PageScrollTimer+20)  // total time for one PageScrollTick (including the handling time of 20ms in PLUGIN_TIMER_IN)
#define P36_PageScrollPix             4  // min pixel change while page scrolling

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
    eDate = 13,
    ePageNo = 14,
};

typedef struct {
  uint8_t       Top;                  // top in pix for this line setting
  const char    *fontData;            // font for this line setting
  uint8_t       Space;                // space in pix between lines for this line setting
} tFontSettings;

typedef struct {
  uint8_t       Width;                // width in pix
  uint8_t       Height;               // height in pix
  uint8_t       PixLeft;              // first left pix position
  uint8_t       MaxLines;             // max. line count
  tFontSettings L1;                   // settings for 1 line
  tFontSettings L2;                   // settings for 2 lines
  tFontSettings L3;                   // settings for 3 lines
  tFontSettings L4;                   // settings for 4 lines
  uint8_t       WiFiIndicatorLeft;    // left of WiFi indicator
  uint8_t       WiFiIndicatorWidth;   // width of WiFi indicator
} tSizeSettings;

const tSizeSettings SizeSettings[P36_MaxSizesCount] = {
   { P36_MaxDisplayWidth, P36_MaxDisplayHeight, 0,   // 128x64
     4,
     // page scrolling height = 42
     { 19, ArialMT_Plain_24, 28},  //  Width: 24 Height: 28
     { 15, ArialMT_Plain_16, 19},  //  Width: 16 Height: 19
     { 12, Dialog_plain_12,  14},  //  Width: 13 Height: 15
     { 12, ArialMT_Plain_10, 10},  //  Width: 10 Height: 13
     113,
     15
   },
   { P36_MaxDisplayWidth, 32, 0,               // 128x32
     2,
     // page scrolling height = 20
     { 14, Dialog_plain_12,  15},  //  Width: 13 Height: 15
     { 12, ArialMT_Plain_10, 10},  //  Width: 10 Height: 13
     {  0, ArialMT_Plain_10,  0},  //  Width: 10 Height: 13 not used!
     {  0, ArialMT_Plain_10,  0},  //  Width: 10 Height: 13 not used!
     113,
     15
   },
   { 64, 48, 32,               // 64x48
     3,
     // page scrolling height = 42
     { 20, ArialMT_Plain_24, 28},  //  Width: 24 Height: 28
     { 14, Dialog_plain_12,  19},  //  Width: 13 Height: 15
     { 13, ArialMT_Plain_10, 14},  //  Width: 10 Height: 13
     {  0, ArialMT_Plain_10,  0},  //  Width: 10 Height: 13 not used!
     32,
     10
   }
 };

 enum ePageScrollSpeed {
   ePSS_VerySlow = 1,   // 800ms
   ePSS_Slow = 2,       // 400ms
   ePSS_Fast = 4,       // 200ms
   ePSS_VeryFast = 8,   // 100ms
   ePSS_Instant = 32    // 20ms
};

typedef struct {
   String        LineContent;          // content
   uint16_t      LastWidth;            // width of last line in pix
   uint16_t      Width;                // width in pix
   uint8_t       Height;               // Height in Pix
   uint8_t       ypos;                 // y position in pix
   int           CurrentLeft;          // current left pix position
   float         dPix;                 // pix change per scroll time (100ms)
   float         fPixSum;              // pix sum while scrolling (100ms)
} tScrollLine;

typedef struct {
  const char    *Font;                 // font for this line setting
  uint8_t       Space;                 // space in pix between lines for this line setting
  uint16_t      wait;                  // waiting time before scrolling
  tScrollLine   Line[P36_MAX_LinesPerPage];
} tScrollingLines;

typedef struct {
  uint8_t       Scrolling;                    // 0=Ready, 1=Scrolling
  const char    *Font;                        // font for this line setting
  uint8_t       dPix;                         // pix change per scroll time (25ms)
  int           dPixSum;                      // act pix change
  uint8_t       linesPerFrame;                // the number of lines in each frame
  int           ypos[P36_MAX_LinesPerPage];   // ypos contains the heights of the various lines - this depends on the font and the number of lines
  String        LineIn[P36_MAX_LinesPerPage];
  String        LineOut[P36_MAX_LinesPerPage];
} tScrollingPages;

typedef struct {
  char         Content[P36_NcharsV1];
  uint8_t      FontType;
  uint8_t      FontHeight;
  uint8_t      FontSpace;
  uint8_t      reserved;
} tDisplayLines;

struct P036_data_struct: public PluginTaskData_base {
  P036_data_struct() : display(nullptr) {}
  ~P036_data_struct() {
    reset();
  }

  void reset() {
    if (display!=nullptr) {
      display->displayOff();
      display->end();
      delete display;
      display=nullptr;
    }
  }

  bool init(uint8_t _type, uint8_t _address, uint8_t _sda, uint8_t _scl) {
    reset();

    lastWiFiState = P36_WIFI_STATE_UNSET;
    OLEDIndex = 0;
    bAlternativHeader = false;
    HeaderCount = 0;
    bPageScrollDisabled = true;   // first page after INIT without scrolling
    TopLineOffset = 0;   // Offset for top line, used for rotated image while using displays < P36_MaxDisplayHeight lines

    HeaderContent=eSysName;
    HeaderContentAlternative=eSysName;
    MaxFramesToDisplay = 0xFF;
    currentFrameToDisplay = 0;
    nextFrameToDisplay = 0xFF;  // next frame because content changed in PLUGIN_WRITE

    switch (_type) {
      case 1:
        display = new SSD1306Wire(_address, _sda, _scl);
        break;
      case 2:
        display = new SH1106Wire(_address, _sda, _scl);
        break;
      default:
        return false;
    }

    display->init();		// call to local override of init function
    display->displayOn();

  return isInitialized();
  }


  bool isInitialized() const {
    return display != nullptr;
  }

// Instantiate display here - does not work to do this within the INIT call
OLEDDisplay *display=nullptr;

tScrollingLines ScrollingLines;
tScrollingPages ScrollingPages;

// CustomTaskSettings
tDisplayLines DisplayLinesV1[P36_Nlines];    // holds the CustomTaskSettings for V1

int8_t lastWiFiState;
uint8_t OLEDIndex;
boolean bPin3Invers;
boolean bScrollLines;
boolean bLineScrollEnabled;
boolean bNoDisplayOnReceivedText;
boolean bScrollWithoutWifi;
boolean bAlternativHeader;
uint16_t HeaderCount;
boolean bPageScrollDisabled;   // first page after INIT without scrolling
uint8_t TopLineOffset;   // Offset for top line, used for rotated image while using displays < P36_MaxDisplayHeight lines

eHeaderContent HeaderContent;
eHeaderContent HeaderContentAlternative;
uint8_t MaxFramesToDisplay;
uint8_t currentFrameToDisplay;
uint8_t nextFrameToDisplay;  // next frame because content changed in PLUGIN_WRITE
};

P036_data_struct *P036_data = nullptr;
void Plugin_036_loadDisplayLines(taskIndex_t taskIndex, uint8_t LoadVersion, P036_data_struct* P036_data);


void P036_setBitToUL(uint32_t& number, byte bitnr, bool value) {
  uint32_t mask = (0x01UL << bitnr);
  uint32_t newbit = (value ? 1UL : 0UL) << bitnr;
  number = (number & ~mask) | newbit;
}
uint8_t get8BitFromUL(uint32_t number, byte bitnr) {
  return (number >> bitnr) & 0xFF;
}
void set8BitToUL(uint32_t& number, byte bitnr, uint8_t value) {
  uint32_t mask = (0xFFUL << bitnr);
  uint32_t newvalue = ((value << bitnr) & mask);
  number = (number & ~mask) | newvalue;
}
uint8_t get4BitFromUL(uint32_t number, byte bitnr) {
  return (number >> bitnr) &  0x0F;
}
void set4BitToUL(uint32_t& number, byte bitnr, uint8_t value) {
  uint32_t mask = (0x0FUL << bitnr);
  uint32_t newvalue = ((value << bitnr) & mask);
  number = (number & ~mask) | newvalue;
}

void Plugin_036_loadDisplayLines(taskIndex_t taskIndex, uint8_t LoadVersion, P036_data_struct* P036_data) {
String DisplayLinesV0[P36_Nlines];                // used to load the CustomTaskSettings for V0

  if (LoadVersion == 0) {
      // read data of version 0 (up to 22.11.2019)
      LoadCustomTaskSettings(taskIndex, DisplayLinesV0, P36_Nlines, P36_NcharsV0); // max. length 1024 Byte  (DAT_TASKS_CUSTOM_SIZE)
      for (int i = 0; i < P36_Nlines; ++i) {
        safe_strncpy(P036_data->DisplayLinesV1[i].Content, DisplayLinesV0[i], P36_NcharsV1);
        P036_data->DisplayLinesV1[i].Content[P36_NcharsV1-1] = 0; // Terminate in case of uninitalized data
        P036_data->DisplayLinesV1[i].FontType = 0xff;
        P036_data->DisplayLinesV1[i].FontHeight = 0xff;
        P036_data->DisplayLinesV1[i].FontSpace = 0xff;
        P036_data->DisplayLinesV1[i].reserved = 0xff;
      }
    }
    else {
      // read data of version 1 (beginning from 22.11.2019)
      LoadCustomTaskSettings(taskIndex, (uint8_t*)&(P036_data->DisplayLinesV1), sizeof(P036_data->DisplayLinesV1));
      for (int i = 0; i < P36_Nlines; ++i) {
        P036_data->DisplayLinesV1[i].Content[P36_NcharsV1-1] = 0; // Terminate in case of uninitalized data
      }
  }
}

boolean Plugin_036(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  static uint8_t displayTimer = 0;
  static uint8_t frameCounter = 0;       // need to keep track of framecounter from call to call
  static uint8_t nrFramesToDisplay = 0;

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
#ifdef PLUGIN_036_DEBUG
          addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_WEBFORM_LOAD ..."));
#endif  // PLUGIN_036_DEBUG
        P036_data_struct *P036_data_temp = new P036_data_struct();  // just temporary for Plugin_036_loadDisplayLines()

        // Use number 5 to remain compatible with existing configurations,
        // but the item should be one of the first choices.
        addFormSubHeader(F("Display"));
        uint8_t choice5 = PCONFIG(5);
        String options5[2];
        options5[0] = F("SSD1306 (128x64 dot controller)");
        options5[1] = F("SH1106 (132x64 dot controller)");
        int optionValues5[2] = { 1, 2 };
        addFormSelector(F("Controller"), F("p036_controller"), 2, options5, optionValues5, choice5);

        uint8_t choice0 = PCONFIG(0);
        int optionValues0[2];
        optionValues0[0] = 0x3C;
        optionValues0[1] = 0x3D;
        addFormSelectorI2C(F("p036_adr"), 2, optionValues0, choice0);

        String options8[P36_MaxSizesCount] = { F("128x64"), F("128x32"), F("64x48") };
        int optionValues8[P36_MaxSizesCount] = { 0, 1, 2 };
        addFormSelector(F("Size"),F("p036_size"), P36_MaxSizesCount, options8, optionValues8, NULL, PCONFIG(7), true);

        uint8_t choice1 = PCONFIG(1);
        String options1[2];
        options1[0] = F("Normal");
        options1[1] = F("Rotated");
        int optionValues1[2] = { 1, 2 };
        addFormSelector(F("Rotation"), F("p036_rotate"), 2, options1, optionValues1, choice1);

        uint8_t tOLEDIndex=PCONFIG(7);
        addFormNumericBox(F("Lines per Frame"), F("p036_nlines"), PCONFIG(2), 1, SizeSettings[tOLEDIndex].MaxLines);

        uint8_t choice3 = PCONFIG(3);
        String options3[5];
        options3[0] = F("Very Slow");
        options3[1] = F("Slow");
        options3[2] = F("Fast");
        options3[3] = F("Very Fast");
        options3[4] = F("Instant");
        int optionValues3[5] = {ePSS_VerySlow, ePSS_Slow, ePSS_Fast, ePSS_VeryFast, ePSS_Instant};
        addFormSelector(F("Scroll"), F("p036_scroll"), 5, options3, optionValues3, choice3);
        uint8_t version = get4BitFromUL(PCONFIG_LONG(0), 20);    // Bit23-20 Version CustomTaskSettings

        Plugin_036_loadDisplayLines(event->TaskIndex, version, P036_data_temp);

        // FIXME TD-er: Why is this using pin3 and not pin1? And why isn't this using the normal pin selection functions?
        addFormPinSelect(F("Display button"), F("taskdevicepin3"), CONFIG_PIN3);
        boolean tbPin3Invers = getBitFromUL(PCONFIG_LONG(0), 16);  // Bit 16
        addFormCheckBox(F("Inversed Logic"), F("p036_pin3invers"), tbPin3Invers);

        addFormNumericBox(F("Display Timeout"), F("p036_timer"), PCONFIG(4));

        uint8_t choice6 = PCONFIG(6);
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

        boolean tbScrollWithoutWifi = getBitFromUL(PCONFIG_LONG(0), 19);  // Bit 19
        addFormCheckBox(F("Disable all scrolling while WiFi is disconnected"), F("p036_ScrollWithoutWifi"), !tbScrollWithoutWifi);
        addFormNote(F("When checked, all scrollings (pages and lines) are disabled as long as WiFi is not connected."));

        addFormSubHeader(F("Content"));

        uint8_t choice9 = get8BitFromUL(PCONFIG_LONG(0), 8);    // Bit15-8 HeaderContent
        uint8_t choice10 = get8BitFromUL(PCONFIG_LONG(0), 0);   // Bit7-0 HeaderContentAlternative
        String options9[14] = { F("SSID"), F("SysName"), F("IP"), F("MAC"), F("RSSI"), F("BSSID"), F("WiFi channel"), F("Unit"), F("SysLoad"), F("SysHeap"), F("SysStack"), F("Date"), F("Time"), F("PageNumbers") };
        int optionValues9[14] = { eSSID, eSysName, eIP, eMAC, eRSSI, eBSSID, eWiFiCh, eUnit, eSysLoad, eSysHeap, eSysStack, eDate, eTime , ePageNo};
        addFormSelector(F("Header"),F("p036_header"), 14, options9, optionValues9, choice9);
        addFormSelector(F("Header (alternating)"),F("p036_headerAlternate"), 14, options9, optionValues9, choice10);

        boolean tbScrollLines = getBitFromUL(PCONFIG_LONG(0), 17);  // Bit 17
        addFormCheckBox(F("Scroll long lines"), F("p036_ScrollLines"), tbScrollLines);

        boolean tbNoDisplayOnReceivedText = getBitFromUL(PCONFIG_LONG(0), 18);  // Bit 18
        addFormCheckBox(F("Wake display on receiving text"), F("p036_NoDisplay"), !tbNoDisplayOnReceivedText);
        addFormNote(F("When checked, the display wakes up at receiving remote updates."));

        for (uint8_t varNr = 0; varNr < P36_Nlines; varNr++)
        {
          addFormTextBox(String(F("Line ")) + (varNr + 1), getPluginCustomArgName(varNr), String(P036_data_temp->DisplayLinesV1[varNr].Content), P36_NcharsV1-1);
        }

        delete P036_data_temp;
        P036_data_temp = nullptr;

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
#ifdef PLUGIN_036_DEBUG
          addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_WEBFORM_SAVE ..."));
#endif  // PLUGIN_036_DEBUG

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

        uint32_t lSettings = 0;
        set8BitToUL(lSettings, 8, uint8_t(getFormItemInt(F("p036_header")) & 0xff));            // Bit15-8 HeaderContent
        set8BitToUL(lSettings, 0, uint8_t(getFormItemInt(F("p036_headerAlternate")) & 0xff));   // Bit 7-0 HeaderContentAlternative
        P036_setBitToUL(lSettings, 16, isFormItemChecked(F("p036_pin3invers")));                // Bit 16 Pin3Invers
        P036_setBitToUL(lSettings, 17, isFormItemChecked(F("p036_ScrollLines")));               // Bit 17 ScrollLines
        P036_setBitToUL(lSettings, 18, !isFormItemChecked(F("p036_NoDisplay")));                // Bit 18 NoDisplayOnReceivingText
        P036_setBitToUL(lSettings, 19, !isFormItemChecked(F("p036_ScrollWithoutWifi")));        // Bit 19 ScrollWithoutWifi
        // save CustomTaskSettings always in version V1
        set4BitToUL(lSettings, 20, 0x01);                                                       // Bit23-20 Version CustomTaskSettings -> version V1

        PCONFIG_LONG(0) = lSettings;

        String error;
        tDisplayLines tDisplayLinesV1[P36_Nlines];    // holds the CustomTaskSettings for V1
        for (uint8_t varNr = 0; varNr < P36_Nlines; varNr++)
        {
          if (!safe_strncpy(tDisplayLinesV1[varNr].Content, web_server.arg(getPluginCustomArgName(varNr)), P36_NcharsV1)) {
            error += getCustomTaskSettingsError(varNr);
          }
          tDisplayLinesV1[varNr].Content[P36_NcharsV1-1] = 0; // Terminate in case of uninitalized data
          tDisplayLinesV1[varNr].FontType = 0xff;
          tDisplayLinesV1[varNr].FontHeight = 0xff;
          tDisplayLinesV1[varNr].FontSpace = 0xff;
          tDisplayLinesV1[varNr].reserved = 0xff;
        }
        if (error.length() > 0) {
          addHtmlError(error);
        }
        SaveCustomTaskSettings(event->TaskIndex, (uint8_t*)&(tDisplayLinesV1), sizeof(tDisplayLinesV1));

        if (nullptr != P036_data) {
          // After saving, make sure the active lines are updated.
          P036_data->MaxFramesToDisplay = 0xFF;
          Plugin_036_loadDisplayLines(event->TaskIndex, 1, P036_data);
        }

#ifdef PLUGIN_036_DEBUG
          addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_WEBFORM_SAVE Done"));
#endif  // PLUGIN_036_DEBUG
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (nullptr == P036_data) {
#ifdef PLUGIN_036_DEBUG
          addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_INIT NoData -> Create ..."));
#endif  // PLUGIN_036_DEBUG
          initPluginTaskData(event->TaskIndex, new P036_data_struct());
          P036_data = static_cast<P036_data_struct *>(getPluginTaskData(event->TaskIndex));
        }
        if (nullptr == P036_data) {
          return success;
        }
#ifdef PLUGIN_036_DEBUG
        if (P036_data->isInitialized()) {
          addLog(LOG_LEVEL_INFO, F("P036_init -> Already done!"));
        }
        else {
          addLog(LOG_LEVEL_INFO, F("P036_init ..."));
        }
#endif  // PLUGIN_036_DEBUG

        if (!(P036_data->init(PCONFIG(5), PCONFIG(0), Settings.Pin_i2c_sda, Settings.Pin_i2c_scl))) { // Init the display and turn it on
          clearPluginTaskData(event->TaskIndex);
          P036_data = nullptr;
          success = true;
          break;
        }

        P036_data->lastWiFiState = P36_WIFI_STATE_UNSET;
        // Load the custom settings from flash
        uint8_t version = get4BitFromUL(PCONFIG_LONG(0), 20);    // Bit23-20 Version CustomTaskSettings
        Plugin_036_loadDisplayLines(event->TaskIndex, version, P036_data);

        uint8_t OLED_contrast = PCONFIG(6);
        P36_setContrast(OLED_contrast);

        //      Set the initial value of OnOff to On
        UserVar[event->BaseVarIndex] = 1;

        //      flip screen if required
        P036_data->OLEDIndex = PCONFIG(7);
        if (PCONFIG(1) == 2) {
          P036_data->display->flipScreenVertically();
          P036_data->TopLineOffset = P36_MaxDisplayHeight - SizeSettings[P036_data->OLEDIndex].Height;
        }
        else P036_data->TopLineOffset = 0;

        //      Display the device name, logo, time and wifi
        P036_data->bAlternativHeader = false;  // start with first header content
        P036_data->HeaderCount = 0;            // reset header count
        display_header();
        display_logo();
        P036_data->display->display();

        //      Set up the display timer
        displayTimer = PCONFIG(4);

        if (CONFIG_PIN3 != -1)
        {
          pinMode(CONFIG_PIN3, INPUT_PULLUP);
        }

        //    Initialize frame counter
        frameCounter = 0;
        nrFramesToDisplay = 1;
        P036_data->currentFrameToDisplay = 0;
        P036_data->bPageScrollDisabled = true;  // first page after INIT without scrolling
        P036_data->ScrollingPages.linesPerFrame = PCONFIG(2);
        P036_data->bLineScrollEnabled = false;  // start without line scrolling

        //    Clear scrolling line data
        for (uint8_t i=0; i<P36_MAX_LinesPerPage ; i++) {
          P036_data->ScrollingLines.Line[i].Width = 0;
          P036_data->ScrollingLines.Line[i].LastWidth = 0;
        }

        //    prepare font and positions for page and line scrolling
        prepare_pagescrolling();
#ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_init Done"));
#endif  // PLUGIN_036_DEBUG

        success = true;
        break;
      }

    case PLUGIN_EXIT:
      {
#ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_EXIT ..."));
#endif  // PLUGIN_036_DEBUG
        clearPluginTaskData(event->TaskIndex);
        P036_data = nullptr;
        success = true;
        break;
      }

    // Check frequently to see if we have a pin signal to switch on display
    case PLUGIN_TEN_PER_SECOND:
      {
        if (Settings.TaskDeviceEnabled[event->TaskIndex] == false) {
          return success;
        }

        if (nullptr == P036_data) {
          return success;
        }

        if (P036_data->isInitialized() == false) {
          return success;
        }

        int lTaskTimer = Settings.TaskDeviceTimer[event->TaskIndex];
        P036_data->bAlternativHeader = (++P036_data->HeaderCount > (lTaskTimer*5)); // change header after half of display time
        if (CONFIG_PIN3 != -1)
        {
          P036_data->bPin3Invers = getBitFromUL(PCONFIG_LONG(0), 16);  // Bit 16
          if ((!P036_data->bPin3Invers && digitalRead(CONFIG_PIN3)) || (P036_data->bPin3Invers && !digitalRead(CONFIG_PIN3)))
          {
            P036_data->display->displayOn();
            UserVar[event->BaseVarIndex] = 1;      //  Save the fact that the display is now ON
            displayTimer = PCONFIG(4);
          }
        }
        if (P036_data->bLineScrollEnabled) {
          if ((UserVar[event->BaseVarIndex] == 1) && (P036_data->ScrollingPages.Scrolling == 0)) {
            // Display is on.
            P036_data->OLEDIndex = PCONFIG(7);
            display_scrolling_lines(); // line scrolling
          }
        }
        success = true;
        break;
      }

    // Switch off display after displayTimer seconds
    case PLUGIN_ONCE_A_SECOND:
      {
        if (Settings.TaskDeviceEnabled[event->TaskIndex] == false) {
#ifdef PLUGIN_036_DEBUG
          addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_ONCE_A_SECOND Not enabled"));
#endif  // PLUGIN_036_DEBUG
          return success;
        }

        if (nullptr == P036_data) {
#ifdef PLUGIN_036_DEBUG
          addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_ONCE_A_SECOND NoData"));
#endif  // PLUGIN_036_DEBUG
          return success;
        }

        if (P036_data->isInitialized() == false) {
#ifdef PLUGIN_036_DEBUG
          addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_ONCE_A_SECOND Not initialized"));
#endif  // PLUGIN_036_DEBUG
          return success;
        }

        if ( displayTimer > 0)
        {
          displayTimer--;
          if (displayTimer == 0)
          {
            P036_data->display->displayOff();
            UserVar[event->BaseVarIndex] = 0;      //  Save the fact that the display is now OFF
          }
        }
        if (UserVar[event->BaseVarIndex] == 1) {
          // Display is on.
          P036_data->OLEDIndex = PCONFIG(7);
          P036_data->HeaderContent = static_cast<eHeaderContent>(get8BitFromUL(PCONFIG_LONG(0), 8));             // Bit15-8 HeaderContent
          P036_data->HeaderContentAlternative = static_cast<eHeaderContent>(get8BitFromUL(PCONFIG_LONG(0), 0));  // Bit 7-0 HeaderContentAlternative
	        display_header();	// Update Header
          if (P036_data->isInitialized() && display_wifibars()) {
            // WiFi symbol was updated.
            P036_data->display->display();
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
#endif  // PLUGIN_036_DEBUG
        return success;
      }

      if (nullptr == P036_data) {
#ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_TIMER_IN NoData"));
#endif  // PLUGIN_036_DEBUG
        return success;
      }

      if (P036_data->isInitialized() == false) {
#ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_TIMER_IN Not initialized"));
#endif  // PLUGIN_036_DEBUG
        return success;
      }

      P036_data->OLEDIndex = PCONFIG(7);
      if (display_scroll_timer()) // page scrolling
        setPluginTaskTimer(P36_PageScrollTimer, event->TaskIndex, event->Par1);  // calls next page scrollng tick

      return success;
    }
    case PLUGIN_READ:
      {
        if (Settings.TaskDeviceEnabled[event->TaskIndex] == false) {
#ifdef PLUGIN_036_DEBUG
        addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_READ Not enabled"));
#endif  // PLUGIN_036_DEBUG
          return success;
        }

        if (nullptr == P036_data) {
#ifdef PLUGIN_036_DEBUG
          addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_READ NoData"));
#endif  // PLUGIN_036_DEBUG
          return success;
        }

        if (P036_data->isInitialized() == false) {
#ifdef PLUGIN_036_DEBUG
          addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_READ Not initialized"));
#endif  // PLUGIN_036_DEBUG
          return success;
        }

        //      Define Scroll area layout
        if (UserVar[event->BaseVarIndex] == 1) {
          // Display is on.
          P036_data->ScrollingPages.Scrolling = 1; // page scrolling running -> no line scrolling allowed
          NFrames = P36_Nlines / P036_data->ScrollingPages.linesPerFrame;
          P036_data->OLEDIndex = PCONFIG(7);
          P036_data->HeaderContent = static_cast<eHeaderContent>(get8BitFromUL(PCONFIG_LONG(0), 8));             // Bit15-8 HeaderContent
          P036_data->HeaderContentAlternative = static_cast<eHeaderContent>(get8BitFromUL(PCONFIG_LONG(0), 0));  // Bit 7-0 HeaderContentAlternative

          //      Now create the string for the outgoing and incoming frames
          String tmpString;
          tmpString.reserve(P36_NcharsV1);

          //      Construct the outgoing string
          for (uint8_t i = 0; i < P036_data->ScrollingPages.linesPerFrame; i++)
          {
            tmpString = String(P036_data->DisplayLinesV1[(P036_data->ScrollingPages.linesPerFrame * frameCounter) + i].Content);
            P036_data->ScrollingPages.LineOut[i] = P36_parseTemplate(tmpString, 20);
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

            if (P036_data->nextFrameToDisplay == 0xff) {
              // Increment the frame counter
              frameCounter = frameCounter + 1;
              if ( frameCounter > NFrames - 1) {
                frameCounter = 0;
                P036_data->currentFrameToDisplay = 0;
              }
            }
            else {
              // next frame because content changed in PLUGIN_WRITE
              frameCounter = P036_data->nextFrameToDisplay;
            }

            //        Contruct incoming strings
            for (uint8_t i = 0; i < P036_data->ScrollingPages.linesPerFrame; i++)
            {
              tmpString = String(P036_data->DisplayLinesV1[(P036_data->ScrollingPages.linesPerFrame * frameCounter) + i].Content);
              P036_data->ScrollingPages.LineIn[i] = P36_parseTemplate(tmpString, 20);
              if (P036_data->ScrollingPages.LineIn[i].length() > 0) foundText = true;
            }
            if (foundText) {
              if (P036_data->nextFrameToDisplay == 0xff) {
                if (frameCounter != 0) {
                  ++P036_data->currentFrameToDisplay;
                }
              }
              else P036_data->currentFrameToDisplay = P036_data->nextFrameToDisplay;
            }
          }
          P036_data->nextFrameToDisplay = 0xFF;

          if ((P036_data->currentFrameToDisplay + 1) > nrFramesToDisplay) {
            nrFramesToDisplay = P036_data->currentFrameToDisplay + 1;
          }

          // Update max page count
          if (P036_data->MaxFramesToDisplay == 0xFF) {
            // not updated yet
            for (uint8_t i = 0; i < NFrames; i++) {
              for (uint8_t k = 0; k < P036_data->ScrollingPages.linesPerFrame; k++)
              {
                tmpString = String(P036_data->DisplayLinesV1[(P036_data->ScrollingPages.linesPerFrame * i) + k].Content);
                tmpString = P36_parseTemplate(tmpString, 20);
                if (tmpString.length() > 0) {
                  // page not empty
                  P036_data->MaxFramesToDisplay ++;
                  break;
                }
              }
            }
          }
          //      Update display
          P036_data->bAlternativHeader = false;  // start with first header content
          P036_data->HeaderCount = 0;            // reset header count
          display_header();
          if (SizeSettings[P036_data->OLEDIndex].Width == P36_MaxDisplayWidth) display_indicator(nrFramesToDisplay);

          P036_data->display->display();

          P036_data->bScrollWithoutWifi = getBitFromUL(PCONFIG_LONG(0), 19);  // Bit 19
          P036_data->bScrollLines = getBitFromUL(PCONFIG_LONG(0), 17);  // Bit 17
          P036_data->bLineScrollEnabled = (P036_data->bScrollLines && (WiFiConnected() || P036_data->bScrollWithoutWifi)); // scroll lines only if WifiIsConnected, otherwise too slow

          int lscrollspeed = PCONFIG(3);
          if (P036_data->bPageScrollDisabled) lscrollspeed = ePSS_Instant; // first page after INIT without scrolling
          int lTaskTimer = Settings.TaskDeviceTimer[event->TaskIndex];
          if (display_scroll(lscrollspeed, lTaskTimer))
            setPluginTaskTimer(P36_PageScrollTimer, event->TaskIndex, event->Par1); // calls next page scrollng tick

          if (WiFiConnected() || P036_data->bScrollWithoutWifi) {
            // scroll lines only if WifiIsConnected, otherwise too slow
            P036_data->bPageScrollDisabled = false;    // next PLUGIN_READ will do page scrolling
          }

				}
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        if (nullptr == P036_data) {
#ifdef PLUGIN_036_DEBUG
          addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_WRITE NoData"));
#endif  // PLUGIN_036_DEBUG
          return success;
        }
#ifdef PLUGIN_036_DEBUG
          addLog(LOG_LEVEL_INFO, F("P036_PLUGIN_WRITE ..."));
#endif  // PLUGIN_036_DEBUG

        String command = parseString(string, 1);
        String subcommand = parseString(string, 2);
        int LineNo = event->Par1;

        if ((command == F("oledframedcmd")) && P036_data->isInitialized()) {
          if (subcommand == F("display"))
          {
            // display functions
            String para1 = parseString(string, 3);
            if (para1 == F("on")) {
              success = true;
              displayTimer = PCONFIG(4);
              P036_data->display->displayOn();
              UserVar[event->BaseVarIndex] = 1;      //  Save the fact that the display is now ON
            }
            if (para1 == F("off")) {
              success = true;
              displayTimer = 0;
              P036_data->display->displayOff();
              UserVar[event->BaseVarIndex] = 0;      //  Save the fact that the display is now OFF
            }
            if (para1 == F("low")) {
              success = true;
              P36_setContrast(P36_CONTRAST_LOW);
            }
            if (para1 == F("med")) {
              success = true;
              P36_setContrast(P36_CONTRAST_MED);
            }
            if (para1 == F("high")) {
              success = true;
              P36_setContrast(P36_CONTRAST_HIGH);
            }
        }
        else if ((LineNo > 0) && (LineNo <= P36_Nlines))
          {
            // content functions
            success = true;
            String NewContent = parseStringKeepCase(string, 3);
            NewContent = P36_parseTemplate(NewContent, 20);
            if (!safe_strncpy(P036_data->DisplayLinesV1[LineNo-1].Content, NewContent, P36_NcharsV1)) {
              addHtmlError(getCustomTaskSettingsError(LineNo-1));
            }
            P036_data->DisplayLinesV1[LineNo-1].Content[P36_NcharsV1-1] = 0;      // Terminate in case of uninitalized data
            P036_data->DisplayLinesV1[LineNo-1].reserved = (event->Par3 & 0xFF);  // not implemented yet

            // calculate Pix length of new Content
            P036_data->display->setFont(P036_data->ScrollingPages.Font);
            uint16_t PixLength = P036_data->display->getStringWidth(String(P036_data->DisplayLinesV1[LineNo-1].Content));
            if (PixLength > 255) {
              addHtmlError(String(F("Pixel length of ")) + String(PixLength) + String(F(" too long for line! Max. 255 pix!")));

              int strlen = String(P036_data->DisplayLinesV1[LineNo-1].Content).length();
              float fAvgPixPerChar = ((float) PixLength)/strlen;
              int iCharToRemove = ceil(((float) (PixLength-255))/fAvgPixPerChar);
              // shorten string because OLED controller can not handle such long strings
              P036_data->DisplayLinesV1[LineNo-1].Content[strlen-iCharToRemove] = 0;
            }

            P036_data->nextFrameToDisplay = LineNo / P036_data->ScrollingPages.linesPerFrame; // next frame shows the new content

            P036_data->bNoDisplayOnReceivedText = getBitFromUL(PCONFIG_LONG(0), 18);  // Bit 18 NoDisplayOnReceivedText
            if (UserVar[event->BaseVarIndex] == 0 && !P036_data->bNoDisplayOnReceivedText) {
              // display was OFF, turn it ON
              displayTimer = PCONFIG(4);
              P036_data->display->displayOn();
              UserVar[event->BaseVarIndex] = 1;      //  Save the fact that the display is now ON
            }

#ifdef PLUGIN_036_DEBUG
            String log = String(F("[P36] Line: ")) + String(LineNo);
            log += String(F(" NewContent:")) + String(NewContent);
            log += String(F(" Content:")) + String(P036_data->DisplayLinesV1[LineNo-1].Content);
            log += String(F(" Length:")) + String(String(P036_data->DisplayLinesV1[LineNo-1].Content).length());
            log += String(F(" Pix: ")) + String(P036_data->display->getStringWidth(String(P036_data->DisplayLinesV1[LineNo-1].Content)));
            log += String(F(" Reserved:")) + String(P036_data->DisplayLinesV1[LineNo-1].reserved);
            addLog(LOG_LEVEL_INFO, log);
#endif  // PLUGIN_036_DEBUG
          }
      }
#ifdef PLUGIN_036_DEBUG
      if (!success) {
        String log = String(F("[P36] Cmd: ")) + String(command);
        log += String(F(" SubCmd:")) + String(subcommand);
        log += String(F(" Success:")) + String(success);
        addLog(LOG_LEVEL_INFO, log);
      }
#endif  // PLUGIN_036_DEBUG
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
      if (P036_data->isInitialized()) {
        P036_data->display->displayOff();
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
  if (P036_data->isInitialized()) {
    P036_data->display->displayOn();
    P036_data->display->setContrast(contrast, precharge, comdetect);
  }
}

// Perform some specific changes for OLED display
String P36_parseTemplate(String &tmpString, uint8_t lineSize) {
  String result = parseTemplate_padded(tmpString, lineSize);
  // OLED lib uses this routine to convert UTF8 to extended ASCII
  // http://playground.arduino.cc/Main/Utf8ascii
  // Attempt to display euro sign (FIXME)
  /*
  const char euro[4] = {0xe2, 0x82, 0xac, 0}; // Unicode euro symbol
  const char euro_oled[3] = {0xc2, 0x80, 0}; // Euro symbol OLED display font
  result.replace(euro, euro_oled);
  */
  result.trim();
  return result;
}

// The screen is set up as 10 rows at the top for the header, 10 rows at the bottom for the footer and 44 rows in the middle for the scroll region

void display_header() {
  eHeaderContent _HeaderContent;
  String newString, strHeader;
  if ((P036_data->HeaderContentAlternative==P036_data->HeaderContent) || !P036_data->bAlternativHeader) {
    _HeaderContent=P036_data->HeaderContent;
  }
  else
  {
    _HeaderContent=P036_data->HeaderContentAlternative;
  }
  switch (_HeaderContent) {
    case eSSID:
      if (WiFiConnected()) {
        strHeader = WiFi.SSID();
      }
      else {
        newString=F("%sysname%");
      }
    break;
    case eSysName:
      newString=F("%sysname%");
    break;
    case eTime:
      newString=F("%systime%");
    break;
    case eDate:
      newString = F("%sysday_0%.%sysmonth_0%.%sysyear%");
    break;
    case eIP:
      newString=F("%ip%");
    break;
    case eMAC:
      newString=F("%mac%");
    break;
    case eRSSI:
      newString=F("%rssi%dB");
    break;
    case eBSSID:
      newString=F("%bssid%");
    break;
    case eWiFiCh:
      newString=F("Channel: %wi_ch%");
    break;
    case eUnit:
      newString=F("Unit: %unit%");
    break;
    case eSysLoad:
      newString=F("Load: %sysload%%");
    break;
    case eSysHeap:
      newString=F("Mem: %sysheap%");
    break;
    case eSysStack:
      newString=F("Stack: %sysstack%");
    break;
    case ePageNo:
      strHeader = F("page ");
      strHeader += (P036_data->currentFrameToDisplay+1);
      if (P036_data->MaxFramesToDisplay != 0xFF) {
        strHeader += F("/");
        strHeader += (P036_data->MaxFramesToDisplay+1);
      }
    break;
    default:
      return;
  }
  if (newString.length() > 0) {
    // Right now only systemvariables have been used, so we don't have to call the parseTemplate.
    parseSystemVariables(newString, false);
    strHeader = newString;
  }

  strHeader.trim();
  display_title(strHeader);
  // Display time and wifibars both clear area below, so paint them after the title.
  if (SizeSettings[P036_data->OLEDIndex].Width == P36_MaxDisplayWidth) display_time(); // only for 128pix wide displays
  display_wifibars();
}

void display_time() {
  String dtime = F("%systime%");
  parseSystemVariables(dtime, false);
  P036_data->display->setTextAlignment(TEXT_ALIGN_LEFT);
  P036_data->display->setFont(ArialMT_Plain_10);
  P036_data->display->setColor(BLACK);
  P036_data->display->fillRect(0, P036_data->TopLineOffset, 28, P36_HeaderHeight-2);
  P036_data->display->setColor(WHITE);
  P036_data->display->drawString(0, P036_data->TopLineOffset, dtime.substring(0, 5));
}

void display_title(String& title) {
  P036_data->display->setFont(ArialMT_Plain_10);
  P036_data->display->setColor(BLACK);
  P036_data->display->fillRect(0, P036_data->TopLineOffset, P36_MaxDisplayWidth, P36_HeaderHeight);   // don't clear line under title.
  P036_data->display->setColor(WHITE);
  if (SizeSettings[P036_data->OLEDIndex].Width == P36_MaxDisplayWidth) {
    P036_data->display->setTextAlignment(TEXT_ALIGN_CENTER);
    P036_data->display->drawString(P36_DisplayCentre, P036_data->TopLineOffset, title);
  }
  else {
    P036_data->display->setTextAlignment(TEXT_ALIGN_LEFT);  // Display right of WiFi bars
    P036_data->display->drawString(SizeSettings[P036_data->OLEDIndex].PixLeft + SizeSettings[P036_data->OLEDIndex].WiFiIndicatorWidth + 3, P036_data->TopLineOffset, title);
  }
}

void display_logo() {
int left = 24;
  P036_data->display->setTextAlignment(TEXT_ALIGN_LEFT);
  P036_data->display->setFont(ArialMT_Plain_16);
  P036_data->display->setColor(BLACK);
  P036_data->display->fillRect(0, P36_HeaderHeight+1+P036_data->TopLineOffset, P36_MaxDisplayWidth, P36_MaxDisplayHeight);
  P036_data->display->setColor(WHITE);
  P036_data->display->drawString(65, 15+P036_data->TopLineOffset, F("ESP"));
  P036_data->display->drawString(65, 34+P036_data->TopLineOffset, F("Easy"));
  if (SizeSettings[P036_data->OLEDIndex].PixLeft<left) left = SizeSettings[P036_data->OLEDIndex].PixLeft;
  P036_data->display->drawXbm(left, P36_HeaderHeight+1+P036_data->TopLineOffset, espeasy_logo_width, espeasy_logo_height, espeasy_logo_bits); // espeasy_logo_width=espeasy_logo_height=36
}

// Draw the frame position

void display_indicator(int frameCount) {

  //  Erase Indicator Area

  P036_data->display->setColor(BLACK);
  P036_data->display->fillRect(0, P036_IndicatorTop+P036_data->TopLineOffset, P36_MaxDisplayWidth, P036_IndicatorHeight);
  P036_data->display->setColor(WHITE);

  // Only display when there is something to display.
  if (frameCount <= 1) return;

  // Display chars as required
  for (uint8_t i = 0; i < frameCount; i++) {
    const char *image;
    if (P036_data->currentFrameToDisplay == i) {
      image = activeSymbole;
    } else {
      image = inactiveSymbole;
    }

    int x, y;

    y = P036_IndicatorTop+2+P036_data->TopLineOffset;

    // I would like a margin of 20 pixels on each side of the indicator.
    // Therefore the width of the indicator should be 128-40=88 and so space between indicator dots is 88/(framecount-1)
    // The width of the display is 128 and so the first dot must be at x=20 if it is to be centred at 64
    const int number_spaces = frameCount - 1;
    if (number_spaces <= 0)
      return;
    int margin = 20;
    int spacing = (P36_MaxDisplayWidth - 2 * margin) / number_spaces;
    // Now apply some max of 30 pixels between the indicators and center the row of indicators.
    if (spacing > 30) {
      spacing = 30;
      margin = (P36_MaxDisplayWidth - number_spaces * spacing) / 2;
    }

    x = margin + (spacing * i);
    P036_data->display->drawXbm(x, y, 8, 8, image);
  }
}

void prepare_pagescrolling()
{
  switch (P036_data->ScrollingPages.linesPerFrame) {
  case 1:
    P036_data->ScrollingPages.Font = SizeSettings[P036_data->OLEDIndex].L1.fontData;
    P036_data->ScrollingPages.ypos[0] = SizeSettings[P036_data->OLEDIndex].L1.Top+P036_data->TopLineOffset;
    P036_data->ScrollingLines.Space = SizeSettings[P036_data->OLEDIndex].L1.Space+1;
  break;
  case 2:
    P036_data->ScrollingPages.Font = SizeSettings[P036_data->OLEDIndex].L2.fontData;
    P036_data->ScrollingPages.ypos[0] = SizeSettings[P036_data->OLEDIndex].L2.Top+P036_data->TopLineOffset;
    P036_data->ScrollingPages.ypos[1] = P036_data->ScrollingPages.ypos[0]+SizeSettings[P036_data->OLEDIndex].L2.Space;
    P036_data->ScrollingLines.Space = SizeSettings[P036_data->OLEDIndex].L2.Space+1;
  break;
  case 3:
    P036_data->ScrollingPages.Font = SizeSettings[P036_data->OLEDIndex].L3.fontData;
    P036_data->ScrollingPages.ypos[0] = SizeSettings[P036_data->OLEDIndex].L3.Top+P036_data->TopLineOffset;
    P036_data->ScrollingPages.ypos[1] = P036_data->ScrollingPages.ypos[0]+SizeSettings[P036_data->OLEDIndex].L3.Space;
    P036_data->ScrollingPages.ypos[2] = P036_data->ScrollingPages.ypos[1]+SizeSettings[P036_data->OLEDIndex].L3.Space;
    P036_data->ScrollingLines.Space = SizeSettings[P036_data->OLEDIndex].L3.Space+1;
  break;
  default:
    P036_data->ScrollingPages.linesPerFrame = 4;
    P036_data->ScrollingPages.Font = SizeSettings[P036_data->OLEDIndex].L4.fontData;
    P036_data->ScrollingPages.ypos[0] = SizeSettings[P036_data->OLEDIndex].L4.Top+P036_data->TopLineOffset;
    P036_data->ScrollingPages.ypos[1] = P036_data->ScrollingPages.ypos[0]+SizeSettings[P036_data->OLEDIndex].L4.Space;
    P036_data->ScrollingPages.ypos[2] = P036_data->ScrollingPages.ypos[1]+SizeSettings[P036_data->OLEDIndex].L4.Space;
    P036_data->ScrollingPages.ypos[3] = P036_data->ScrollingPages.ypos[2]+SizeSettings[P036_data->OLEDIndex].L4.Space;
    P036_data->ScrollingLines.Space = SizeSettings[P036_data->OLEDIndex].L4.Space+1;
  }
  P036_data->ScrollingLines.Font = P036_data->ScrollingPages.Font;
  for (uint8_t i=0; i<P36_MAX_LinesPerPage ; i++) {
    P036_data->ScrollingLines.Line[i].ypos = P036_data->ScrollingPages.ypos[i];
  }

}

uint8_t display_scroll(int lscrollspeed, int lTaskTimer)
{
  int iPageScrollTime;
  int iCharToRemove;

#ifdef PLUGIN_036_DEBUG
  String log = F("Start Scrolling: Speed: ");
  log += lscrollspeed;
  addLog(LOG_LEVEL_INFO, log);
#endif  // PLUGIN_036_DEBUG

  P036_data->display->setFont(P036_data->ScrollingPages.Font);

  P036_data->ScrollingLines.wait = 0;

  // calculate total page scrolling time
  if (lscrollspeed == ePSS_Instant) iPageScrollTime = P36_PageScrollTick-P36_PageScrollTimer; // no scrolling, just the handling time to build the new page
  else iPageScrollTime = (P36_MaxDisplayWidth /(P36_PageScrollPix * lscrollspeed)) * P36_PageScrollTick;
  float fScrollTime = (float)(lTaskTimer*1000 - iPageScrollTime - 2*P36_WaitScrollLines*100)/100.0;

#ifdef PLUGIN_036_DEBUG
  log = F(" PageScrollTime: ");
  log += iPageScrollTime;
  addLog(LOG_LEVEL_INFO, log);
#endif  // PLUGIN_036_DEBUG

    uint16_t MaxPixWidthForPageScrolling = P36_MaxDisplayWidth;
    if (P036_data->bLineScrollEnabled) {
      // Reduced scrolling width because line is displayed left or right aligned
      MaxPixWidthForPageScrolling -= SizeSettings[P036_data->OLEDIndex].PixLeft;
    }

    for (uint8_t j = 0; j < P036_data->ScrollingPages.linesPerFrame; j++)
    {
      // default no line scrolling and strings are centered
      P036_data->ScrollingLines.Line[j].LastWidth = 0;
      P036_data->ScrollingLines.Line[j].Width = 0;

      // get last and new line width
      uint16_t PixLengthLineOut = P036_data->display->getStringWidth(P036_data->ScrollingPages.LineOut[j]);
      uint16_t PixLengthLineIn = P036_data->display->getStringWidth(P036_data->ScrollingPages.LineIn[j]);

      if (PixLengthLineIn > 255) {
        // shorten string because OLED controller can not handle such long strings
        int strlen = P036_data->ScrollingPages.LineIn[j].length();
        float fAvgPixPerChar = ((float) PixLengthLineIn)/strlen;
        iCharToRemove = ceil(((float) (PixLengthLineIn-255))/fAvgPixPerChar);
        P036_data->ScrollingPages.LineIn[j] = P036_data->ScrollingPages.LineIn[j].substring(0, strlen-iCharToRemove);
        PixLengthLineIn = P036_data->display->getStringWidth(P036_data->ScrollingPages.LineIn[j]);
      }
      if (PixLengthLineOut > 255) {
        // shorten string because OLED controller can not handle such long strings
        int strlen = P036_data->ScrollingPages.LineOut[j].length();
        float fAvgPixPerChar = ((float) PixLengthLineOut)/strlen;
        iCharToRemove = ceil(((float) (PixLengthLineOut-255))/fAvgPixPerChar);
        P036_data->ScrollingPages.LineOut[j] = P036_data->ScrollingPages.LineOut[j].substring(0, strlen-iCharToRemove);
        PixLengthLineOut = P036_data->display->getStringWidth(P036_data->ScrollingPages.LineOut[j]);
      }

      if (P036_data->bLineScrollEnabled) {
        // settings for following line scrolling
        if (PixLengthLineOut > SizeSettings[P036_data->OLEDIndex].Width)
          P036_data->ScrollingLines.Line[j].LastWidth = PixLengthLineOut;   // while page scrolling this line is right aligned

        if ((PixLengthLineIn > SizeSettings[P036_data->OLEDIndex].Width) && (fScrollTime > 0.0))
        {
          // width of the line > display width -> scroll line
          P036_data->ScrollingLines.Line[j].LineContent = P036_data->ScrollingPages.LineIn[j];
          P036_data->ScrollingLines.Line[j].Width = PixLengthLineIn;   // while page scrolling this line is left aligned
          P036_data->ScrollingLines.Line[j].CurrentLeft = SizeSettings[P036_data->OLEDIndex].PixLeft;
          P036_data->ScrollingLines.Line[j].fPixSum = (float) SizeSettings[P036_data->OLEDIndex].PixLeft;
          P036_data->ScrollingLines.Line[j].dPix = ((float)(PixLengthLineIn-SizeSettings[P036_data->OLEDIndex].Width))/fScrollTime; // pix change per scrolling line tick

#ifdef PLUGIN_036_DEBUG
          log = String(F("Line: ")) + String(j+1);
          log += F(" width: ");
          log += P036_data->ScrollingLines.Line[j].Width;
          log += F(" dPix: ");
          log += P036_data->ScrollingLines.Line[j].dPix;
          addLog(LOG_LEVEL_INFO, log);
#endif  // PLUGIN_036_DEBUG
        }
      }

      // reduce line content for page scrolling to max width
      if (PixLengthLineIn > MaxPixWidthForPageScrolling) {
        int strlen = P036_data->ScrollingPages.LineIn[j].length();
#ifdef PLUGIN_036_DEBUG
        String LineInStr=P036_data->ScrollingPages.LineIn[j];
#endif  // PLUGIN_036_DEBUG
        float fAvgPixPerChar = ((float) PixLengthLineIn)/strlen;
        if (P036_data->bLineScrollEnabled) {
          // shorten string on right side because line is displayed left aligned while scrolling
          // using floor() because otherwise empty space on right side
          iCharToRemove = floor(((float) (PixLengthLineIn-MaxPixWidthForPageScrolling))/fAvgPixPerChar);
          P036_data->ScrollingPages.LineIn[j] = P036_data->ScrollingPages.LineIn[j].substring(0, strlen-iCharToRemove);
        }
        else {
          // shorten string on both sides because line is displayed centered
          // using floor() because otherwise empty space on both sides
          iCharToRemove = floor(((float) (PixLengthLineIn-MaxPixWidthForPageScrolling))/(2*fAvgPixPerChar));
          P036_data->ScrollingPages.LineIn[j] = P036_data->ScrollingPages.LineIn[j].substring(0, strlen-iCharToRemove);
          P036_data->ScrollingPages.LineIn[j] = P036_data->ScrollingPages.LineIn[j].substring(iCharToRemove);
        }
#ifdef PLUGIN_036_DEBUG
        log = String(F("Line: ")) + String(j+1);
        log += String(F(" LineIn: ")) + String(LineInStr);
        log += String(F(" Length: ")) + String(strlen);
        log += String(F(" PixLength: ")) + String(PixLengthLineIn);
        log += String(F(" AvgPixPerChar: ")) + String(fAvgPixPerChar);
        log += String(F(" CharsRemoved: ")) + String(iCharToRemove);
        addLog(LOG_LEVEL_INFO, log);
        log = String(F(" -> Changed to: ")) + String(P036_data->ScrollingPages.LineIn[j]);
        log += String(F(" Length: ")) + String(P036_data->ScrollingPages.LineIn[j].length());
        log += String(F(" PixLength: ")) + String(P036_data->display->getStringWidth(P036_data->ScrollingPages.LineIn[j]));
        addLog(LOG_LEVEL_INFO, log);
#endif  // PLUGIN_036_DEBUG
      }

      // reduce line content for page scrolling to max width
      if (PixLengthLineOut > MaxPixWidthForPageScrolling) {
        int strlen = P036_data->ScrollingPages.LineOut[j].length();
#ifdef PLUGIN_036_DEBUG
        String LineOutStr=P036_data->ScrollingPages.LineOut[j];
#endif  // PLUGIN_036_DEBUG
        float fAvgPixPerChar = ((float) PixLengthLineOut)/strlen;
        if (P036_data->bLineScrollEnabled) {
          // shorten string on left side because line is displayed right aligned while scrolling
          // using ceil() because otherwise overlapping the new text
          iCharToRemove = ceil(((float) (PixLengthLineOut-MaxPixWidthForPageScrolling))/fAvgPixPerChar);
          P036_data->ScrollingPages.LineOut[j] = P036_data->ScrollingPages.LineOut[j].substring(iCharToRemove);
          if (P036_data->display->getStringWidth(P036_data->ScrollingPages.LineOut[j])>MaxPixWidthForPageScrolling) {
            // remove one more character because still overlapping the new text
            P036_data->ScrollingPages.LineOut[j] = P036_data->ScrollingPages.LineOut[j].substring(1, iCharToRemove-1);
          }
        }
        else {
          // shorten string on both sides because line is displayed centered
          // using ceil() because otherwise overlapping the new text
          iCharToRemove = ceil(((float) (PixLengthLineOut-MaxPixWidthForPageScrolling))/(2*fAvgPixPerChar));
          P036_data->ScrollingPages.LineOut[j] = P036_data->ScrollingPages.LineOut[j].substring(0, strlen-iCharToRemove);
          P036_data->ScrollingPages.LineOut[j] = P036_data->ScrollingPages.LineOut[j].substring(iCharToRemove);
        }
#ifdef PLUGIN_036_DEBUG
        log = String(F("Line: ")) + String(j+1);
        log += String(F(" LineOut: ")) + String(LineOutStr);
        log += String(F(" Length: ")) + String(strlen);
        log += String(F(" PixLength: ")) + String(PixLengthLineOut);
        log += String(F(" AvgPixPerChar: ")) + String(fAvgPixPerChar);
        log += String(F(" CharsRemoved: ")) + String(iCharToRemove);
        addLog(LOG_LEVEL_INFO, log);
        log = String(F(" -> Changed to: ")) + String(P036_data->ScrollingPages.LineOut[j]);
        log += String(F(" Length: ")) + String(P036_data->ScrollingPages.LineOut[j].length());
        log += String(F(" PixLength: ")) + String(P036_data->display->getStringWidth(P036_data->ScrollingPages.LineOut[j]));
        addLog(LOG_LEVEL_INFO, log);
#endif  // PLUGIN_036_DEBUG
      }
  }

  P036_data->ScrollingPages.dPix = P36_PageScrollPix * lscrollspeed; // pix change per scrolling page tick
  P036_data->ScrollingPages.dPixSum = P036_data->ScrollingPages.dPix;

  P036_data->display->setColor(BLACK);
   // We allow 12 pixels at the top because otherwise the wifi indicator gets too squashed!!
  P036_data->display->fillRect(0, P36_HeaderHeight+P036_data->TopLineOffset, P36_MaxDisplayWidth, P036_IndicatorTop-P36_HeaderHeight);   // scrolling window is 42 pixels high - ie 64 less margin of 12 at top and 10 at bottom
  P036_data->display->setColor(WHITE);
  P036_data->display->drawLine(0, P36_HeaderHeight+P036_data->TopLineOffset, P36_MaxDisplayWidth, P36_HeaderHeight+P036_data->TopLineOffset);   // line below title

  for (uint8_t j = 0; j < P036_data->ScrollingPages.linesPerFrame; j++)
  {
    if (lscrollspeed < ePSS_Instant ) { // scrolling
      if (P036_data->ScrollingLines.Line[j].LastWidth > 0 ) {
        // width of LineOut[j] > display width -> line at beginning of scrolling page is right aligned
        P036_data->display->setTextAlignment(TEXT_ALIGN_RIGHT);
        P036_data->display->drawString(P36_MaxDisplayWidth - SizeSettings[P036_data->OLEDIndex].PixLeft + P036_data->ScrollingPages.dPixSum, P036_data->ScrollingPages.ypos[j], P036_data->ScrollingPages.LineOut[j]);
      }
      else {
        // line at beginning of scrolling page is centered
        P036_data->display->setTextAlignment(TEXT_ALIGN_CENTER);
        P036_data->display->drawString(P36_DisplayCentre + P036_data->ScrollingPages.dPixSum, P036_data->ScrollingPages.ypos[j], P036_data->ScrollingPages.LineOut[j]);
      }
    }

    if (P036_data->ScrollingLines.Line[j].Width > 0 ) {
      // width of LineIn[j] > display width -> line at end of scrolling page should be left aligned
      P036_data->display->setTextAlignment(TEXT_ALIGN_LEFT);
      P036_data->display->drawString(-P36_MaxDisplayWidth + SizeSettings[P036_data->OLEDIndex].PixLeft + P036_data->ScrollingPages.dPixSum, P036_data->ScrollingPages.ypos[j], P036_data->ScrollingPages.LineIn[j]);
    }
    else {
      // line at end of scrolling page should be centered
      P036_data->display->setTextAlignment(TEXT_ALIGN_CENTER);
      P036_data->display->drawString(-P36_DisplayCentre + P036_data->ScrollingPages.dPixSum, P036_data->ScrollingPages.ypos[j], P036_data->ScrollingPages.LineIn[j]);
    }
  }

  P036_data->display->display();

  if (lscrollspeed < ePSS_Instant ) {
    // page scrolling (using PLUGIN_TIMER_IN)
    P036_data->ScrollingPages.dPixSum += P036_data->ScrollingPages.dPix;
  }
  else {
    // no page scrolling
    P036_data->ScrollingPages.Scrolling = 0; // allow following line scrolling
  }
#ifdef PLUGIN_036_DEBUG
  log = F("Scrolling finished");
  addLog(LOG_LEVEL_INFO, log);
#endif  // PLUGIN_036_DEBUG
  return (P036_data->ScrollingPages.Scrolling);
}

uint8_t display_scroll_timer() {

  // page scrolling (using PLUGIN_TIMER_IN)
  P036_data->display->setColor(BLACK);
   // We allow 13 pixels (including underline) at the top because otherwise the wifi indicator gets too squashed!!
  P036_data->display->fillRect(0, P36_HeaderHeight+1+P036_data->TopLineOffset, P36_MaxDisplayWidth, P036_IndicatorTop-P36_HeaderHeight);   // scrolling window is 42 pixels high - ie 64 less margin of 12 at top and 10 at bottom
  P036_data->display->setColor(WHITE);
  P036_data->display->setFont(P036_data->ScrollingPages.Font);

  for (uint8_t j = 0; j < P036_data->ScrollingPages.linesPerFrame; j++)
  {
    if (P036_data->ScrollingLines.Line[j].LastWidth > 0 ) {
      // width of LineOut[j] > display width -> line is right aligned while scrolling page
      P036_data->display->setTextAlignment(TEXT_ALIGN_RIGHT);
      P036_data->display->drawString(P36_MaxDisplayWidth - SizeSettings[P036_data->OLEDIndex].PixLeft + P036_data->ScrollingPages.dPixSum, P036_data->ScrollingPages.ypos[j], P036_data->ScrollingPages.LineOut[j]);
    }
    else {
      // line is centered while scrolling page
      P036_data->display->setTextAlignment(TEXT_ALIGN_CENTER);
      P036_data->display->drawString(P36_DisplayCentre + P036_data->ScrollingPages.dPixSum, P036_data->ScrollingPages.ypos[j], P036_data->ScrollingPages.LineOut[j]);
    }

    if (P036_data->ScrollingLines.Line[j].Width > 0 ) {
      // width of LineIn[j] > display width -> line is left aligned while scrolling page
      P036_data->display->setTextAlignment(TEXT_ALIGN_LEFT);
      P036_data->display->drawString(-P36_MaxDisplayWidth + SizeSettings[P036_data->OLEDIndex].PixLeft + P036_data->ScrollingPages.dPixSum, P036_data->ScrollingPages.ypos[j], P036_data->ScrollingPages.LineIn[j]);
    }
    else {
      // line is centered while scrolling page
      P036_data->display->setTextAlignment(TEXT_ALIGN_CENTER);
      P036_data->display->drawString(-P36_DisplayCentre + P036_data->ScrollingPages.dPixSum, P036_data->ScrollingPages.ypos[j], P036_data->ScrollingPages.LineIn[j]);
    }
  }

  P036_data->display->display();

  if (P036_data->ScrollingPages.dPixSum < P36_MaxDisplayWidth ) { // scrolling
    // page still scrolling
    P036_data->ScrollingPages.dPixSum += P036_data->ScrollingPages.dPix;
  }
  else {
    // page scrolling finished
    P036_data->ScrollingPages.Scrolling = 0; // allow following line scrolling
// String log = F("Scrolling finished");
// addLog(LOG_LEVEL_INFO, log);
  }
  return (P036_data->ScrollingPages.Scrolling);
}

//Draw scrolling line (1pix/s)
void display_scrolling_lines() {
  // line scrolling (using PLUGIN_TEN_PER_SECOND)

  int i;
  boolean bscroll = false;
  boolean updateDisplay = false;
  int iCurrentLeft;

  for (i=0; i<P036_data->ScrollingPages.linesPerFrame; i++) {
    if (P036_data->ScrollingLines.Line[i].Width !=0 ) {
      P036_data->display->setFont(P036_data->ScrollingLines.Font);
      bscroll = true;
      break;
    }
  }
  if (bscroll) {
    P036_data->ScrollingLines.wait++;
    if (P036_data->ScrollingLines.wait < P36_WaitScrollLines)
      return; // wait before scrolling line not finished

    for (i=0; i<P036_data->ScrollingPages.linesPerFrame; i++) {
      if (P036_data->ScrollingLines.Line[i].Width !=0 ) {
        // scroll this line
        P036_data->ScrollingLines.Line[i].fPixSum -= P036_data->ScrollingLines.Line[i].dPix;
        iCurrentLeft = round(P036_data->ScrollingLines.Line[i].fPixSum);
        if (iCurrentLeft != P036_data->ScrollingLines.Line[i].CurrentLeft) {
          // still scrolling
          P036_data->ScrollingLines.Line[i].CurrentLeft = iCurrentLeft;
          updateDisplay = true;
          P036_data->display->setColor(BLACK);
          P036_data->display->fillRect(0 , P036_data->ScrollingLines.Line[i].ypos+1, P36_MaxDisplayWidth, P036_data->ScrollingLines.Space+2); // clearing window was too high and to narrow
          P036_data->display->setColor(WHITE);
          if (((P036_data->ScrollingLines.Line[i].CurrentLeft-SizeSettings[P036_data->OLEDIndex].PixLeft)+P036_data->ScrollingLines.Line[i].Width) >= SizeSettings[P036_data->OLEDIndex].Width) {
            P036_data->display->setTextAlignment(TEXT_ALIGN_LEFT);
            P036_data->display->drawString(P036_data->ScrollingLines.Line[i].CurrentLeft, P036_data->ScrollingLines.Line[i].ypos, P036_data->ScrollingLines.Line[i].LineContent);
          }
          else {
            // line scrolling finished -> line is shown as aligned right
            P036_data->display->setTextAlignment(TEXT_ALIGN_RIGHT);
            P036_data->display->drawString(P36_MaxDisplayWidth - SizeSettings[P036_data->OLEDIndex].PixLeft, P036_data->ScrollingPages.ypos[i], P036_data->ScrollingLines.Line[i].LineContent);
            P036_data->ScrollingLines.Line[i].Width = 0; // Stop scrolling
          }
        }
      }
    }
    if (updateDisplay && (P036_data->ScrollingPages.Scrolling == 0)) P036_data->display->display();
  }
}

//Draw Signal Strength Bars, return true when there was an update.
bool display_wifibars() {
  const bool connected = WiFiConnected();
  const int nbars_filled = (WiFi.RSSI() + 100) / 12;  // all bars filled if RSSI better than -46dB
  const int newState = connected ? nbars_filled : P36_WIFI_STATE_NOT_CONNECTED;
  if (newState == P036_data->lastWiFiState)
    return false; // nothing to do.

  int x = SizeSettings[P036_data->OLEDIndex].WiFiIndicatorLeft;
  int y = P036_data->TopLineOffset;
  int size_x = SizeSettings[P036_data->OLEDIndex].WiFiIndicatorWidth;
  int size_y = P36_HeaderHeight-2;
  int nbars = 5;
  int16_t width = (size_x / nbars);
  size_x = width * nbars - 1; // Correct for round errors.

  //  x,y are the x,y locations
  //  sizex,sizey are the sizes (should be a multiple of the number of bars)
  //  nbars is the number of bars and nbars_filled is the number of filled bars.

  //  We leave a 1 pixel gap between bars
  P036_data->display->setColor(BLACK);
  P036_data->display->fillRect(x , y, size_x, size_y);
  P036_data->display->setColor(WHITE);
  if (WiFiConnected()) {
    for (uint8_t ibar = 0; ibar < nbars; ibar++) {
      int16_t height = size_y * (ibar + 1) / nbars;
      int16_t xpos = x + ibar * width;
      int16_t ypos = y + size_y - height;
      if (ibar <= nbars_filled) {
        // Fill complete bar
        P036_data->display->fillRect(xpos, ypos, width - 1, height);
      } else {
        // Only draw top and bottom.
        P036_data->display->fillRect(xpos, ypos, width - 1, 1);
        P036_data->display->fillRect(xpos, y + size_y - 1, width - 1, 1);
      }
    }
  } else {
    // Draw a not connected sign (empty rectangle)
  	P036_data->display->drawRect(x , y, size_x, size_y-1);
  }
  return true;
}
#endif // USES_P036
