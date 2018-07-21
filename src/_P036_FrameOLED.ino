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

#define PLUGIN_036
#define PLUGIN_ID_036         36
#define PLUGIN_NAME_036       "Display - OLED SSD1306/SH1106 Framed"
#define PLUGIN_VALUENAME1_036 "OLED"

#define P36_Nlines 12        // The number of different lines which can be displayed - each line is 32 chars max
#define P36_Nchars 32

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

// Instantiate display here - does not work to do this within the INIT call

OLEDDisplay *display=NULL;

char P036_deviceTemplate[P36_Nlines][P36_Nchars];

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
        byte choice5 = Settings.TaskDevicePluginConfig[event->TaskIndex][5];
        String options5[2];
        options5[0] = F("SSD1306");
        options5[1] = F("SH1106");
        int optionValues5[2] = { 1, 2 };
        addFormSelector(F("Controler"), F("plugin_036_controler"), 2, options5, optionValues5, choice5);

        byte choice0 = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        /*
        String options0[2];
        options0[0] = F("3C");
        options0[1] = F("3D");
        */
        int optionValues0[2];
        optionValues0[0] = 0x3C;
        optionValues0[1] = 0x3D;
        addFormSelectorI2C(F("plugin_036_adr"), 2, optionValues0, choice0);

        byte choice1 = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        String options1[2];
        options1[0] = F("Normal");
        options1[1] = F("Rotated");
        int optionValues1[2] = { 1, 2 };
        addFormSelector(F("Rotation"), F("plugin_036_rotate"), 2, options1, optionValues1, choice1);

        byte choice2 = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        String options2[4];
        options2[0] = F("1");
        options2[1] = F("2");
        options2[2] = F("3");
        options2[3] = F("4");
        int optionValues2[4] = { 1, 2, 3, 4 };
        addFormSelector(F("Lines per Frame"), F("plugin_036_nlines"), 4, options2, optionValues2, choice2);

        byte choice3 = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
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
        addFormSelector(F("Scroll"), F("plugin_036_scroll"), 5, options3, optionValues3, choice3);

        LoadCustomTaskSettings(event->TaskIndex, (byte*)&P036_deviceTemplate, sizeof(P036_deviceTemplate));

        for (byte varNr = 0; varNr < P36_Nlines; varNr++)
        {
          addFormTextBox(String(F("Line ")) + (varNr + 1), String(F("Plugin_036_template")) + (varNr + 1), P036_deviceTemplate[varNr], P36_Nchars);
        }

        addFormPinSelect(F("Display button"), F("taskdevicepin3"), Settings.TaskDevicePin3[event->TaskIndex]);

        addFormNumericBox(F("Display Timeout"), F("plugin_036_timer"), Settings.TaskDevicePluginConfig[event->TaskIndex][4]);

        byte choice6 = Settings.TaskDevicePluginConfig[event->TaskIndex][6];
        if (choice6 == 0) choice6 = P36_CONTRAST_HIGH;
        String options6[3];
        options6[0] = F("Low");
        options6[1] = F("Medium");
        options6[2] = F("High");
        int optionValues6[3];
        optionValues6[0] = P36_CONTRAST_LOW;
        optionValues6[1] = P36_CONTRAST_MED;
        optionValues6[2] = P36_CONTRAST_HIGH;
        addFormSelector(F("Contrast"), F("plugin_036_contrast"), 3, options6, optionValues6, choice6);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        //update now
        schedule_task_device_timer(event->TaskIndex,
           millis() + (Settings.TaskDeviceTimer[event->TaskIndex] * 1000));
        frameCounter=0;

        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_036_adr"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_036_rotate"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("plugin_036_nlines"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = getFormItemInt(F("plugin_036_scroll"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][4] = getFormItemInt(F("plugin_036_timer"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][5] = getFormItemInt(F("plugin_036_controler"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][6] = getFormItemInt(F("plugin_036_contrast"));

        String argName;

        for (byte varNr = 0; varNr < P36_Nlines; varNr++)
        {
          argName = F("Plugin_036_template");
          argName += varNr + 1;
          strncpy(P036_deviceTemplate[varNr], WebServer.arg(argName).c_str(), sizeof(P036_deviceTemplate[varNr]));
        }

        SaveCustomTaskSettings(event->TaskIndex, (byte*)&P036_deviceTemplate, sizeof(P036_deviceTemplate));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        lastWiFiState = P36_WIFI_STATE_UNSET;
        // Load the custom settings from flash
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&P036_deviceTemplate, sizeof(P036_deviceTemplate));

        //      Init the display and turn it on
        if (display)
        {
          display->end();
          delete display;
        }

        uint8_t OLED_address = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][5] == 1) {
          display = new SSD1306Wire(OLED_address, Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);
        } else {
          display = new SH1106Wire(OLED_address, Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);
        }
        display->init();		// call to local override of init function
        display->displayOn();

        uint8_t OLED_contrast = Settings.TaskDevicePluginConfig[event->TaskIndex][6];
        P36_setContrast(OLED_contrast);

        //      Set the initial value of OnOff to On
        UserVar[event->BaseVarIndex] = 1;

        //      flip screen if required
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] == 2)display->flipScreenVertically();

        //      Display the device name, logo, time and wifi
        display_header();
        display_logo();
        display->display();

        //      Set up the display timer
        displayTimer = Settings.TaskDevicePluginConfig[event->TaskIndex][4];

        if (Settings.TaskDevicePin3[event->TaskIndex] != -1)
        {
          pinMode(Settings.TaskDevicePin3[event->TaskIndex], INPUT_PULLUP);
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
          break;
      }

    // Check frequently to see if we have a pin signal to switch on display
    case PLUGIN_TEN_PER_SECOND:
      {
        if (Settings.TaskDevicePin3[event->TaskIndex] != -1)
        {
          if (!digitalRead(Settings.TaskDevicePin3[event->TaskIndex]))
          {
            display->displayOn();
            UserVar[event->BaseVarIndex] = 1;      //  Save the fact that the display is now ON
            displayTimer = Settings.TaskDevicePluginConfig[event->TaskIndex][4];
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
        linesPerFrame = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        NFrames = P36_Nlines / linesPerFrame;

        //      Now create the string for the outgoing and incoming frames
        String tmpString;
        tmpString.reserve(P36_Nchars);
        String newString[4];
        String oldString[4];

        //      Construct the outgoing string
        for (byte i = 0; i < linesPerFrame; i++)
        {
          tmpString = P036_deviceTemplate[(linesPerFrame * frameCounter) + i];
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
            tmpString = P036_deviceTemplate[(linesPerFrame * frameCounter) + i];
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
        display_indicator(currentFrameToDisplay, nrFramesToDisplay);
//        display_indicator(frameCounter, NFrames);
        display->display();

        int scrollspeed = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
        display_scroll(oldString, newString, linesPerFrame, scrollspeed);

        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
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
      log += F(" ");
      log += String(tmpString[i], HEX);
    }
    log += F(" out hex:");
    for (int i = 0; i < result.length(); ++i) {
      log += F(" ");
      log += String(result[i], HEX);
    }
    addLog(LOG_LEVEL_INFO, log);
  }
*/
  return result;
}

// The screen is set up as 10 rows at the top for the header, 10 rows at the bottom for the footer and 44 rows in the middle for the scroll region

void display_header() {
  static boolean showWiFiName = true;
  if (showWiFiName && (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED) ) {
    String newString = WiFi.SSID();
    newString.trim();
    display_title(newString);
  } else {
    String dtime = F("%sysname%");
    String newString = parseTemplate(dtime, 10);
    newString.trim();
    display_title(newString);
  }
  showWiFiName = !showWiFiName;
  // Display time and wifibars both clear area below, so paint them after the title.
  display_time();
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
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->setColor(BLACK);
  display->fillRect(0, 0, 128, 13); // Underscores use a extra lines, clear also.
  display->setColor(WHITE);
  display->drawString(64, 0, title);
}

void display_logo() {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->setColor(BLACK);
  display->fillRect(0, 14, 128, 64);
  display->setColor(WHITE);
  display->drawString(65, 15, F("ESP"));
  display->drawString(65, 34, F("Easy"));
  display->drawXbm(24, 14, espeasy_logo_width, espeasy_logo_height, espeasy_logo_bits);
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
    display->setFont(ArialMT_Plain_24);
    ypos[0] = 20;
  }

  if (nlines == 2)
  {
    display->setFont(ArialMT_Plain_16);
    ypos[0] = 15;
    ypos[1] = 34;
  }

  if (nlines == 3)
  {
    display->setFont(Dialog_plain_12);
    ypos[0] = 13;
    ypos[1] = 25;
    ypos[2] = 37;
  }

  if (nlines == 4)
  {
    display->setFont(ArialMT_Plain_10);
    ypos[0] = 12;
    ypos[1] = 22;
    ypos[2] = 32;
    ypos[3] = 42;
  }

  display->setTextAlignment(TEXT_ALIGN_CENTER);

  for (byte i = 0; i < 33; i = i + scrollspeed)
  {

    //  Clear the scroll area

    display->setColor(BLACK);
    // We allow 12 pixels at the top because otherwise the wifi indicator gets too squashed!!
    display->fillRect(0, 12, 128, 42);   // scrolling window is 44 pixels high - ie 64 less margin of 10 at top and bottom
    display->setColor(WHITE);

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
  const bool connected = wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED;
  const int nbars_filled = (WiFi.RSSI() + 100) / 8;
  const int newState = connected ? nbars_filled : P36_WIFI_STATE_UNSET;
  if (newState == lastWiFiState)
    return false; // nothing to do.

  int x = 105;
  int y = 0;
  int size_x = 15;
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
  if (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED) {
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
    // Draw a not connected sign.
  }
  return true;
}
#endif // USES_P036
