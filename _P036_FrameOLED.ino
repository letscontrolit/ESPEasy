//#######################################################################################################
//#################################### Plugin 036: OLED SSD1306 display #################################
//
// This is a modification to Plugin_023 with graphics library provided from squix78 github
// https://github.com/squix78/esp8266-oled-ssd1306
//
// The OLED can display up to 12 strings in four frames - ie 12 frames with 1 line, 6 with 2 lines or 3 with 4 lines.
// The font size is adjsted according to the number of lines required per frame.
//
// Major work on this plugin has been done by 'Namirda'
// Added to the main repository with some optimizations and some limitations.
// Al long as the device is not selected, no RAM is waisted.

#define PLUGIN_036
#define PLUGIN_ID_036         36
#define PLUGIN_NAME_036       "Display - OLED SSD1306 Framed"
#define PLUGIN_VALUENAME1_036 "OLED"

#define Nlines 12				// The number of different lines which can be displayed - each line is 32 chars max

#include "SSD1306.h"
#include "images.h"

// Instantiate display here - does not work to do this within the INIT call

SSD1306 *display=NULL;

boolean Plugin_036(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  static byte displayTimer = 0;
  static byte frameCounter = 0;				// need to keep track of framecounter from call to call
  static boolean firstcall = true;			// This is used to clear the init graphic on the first call to read

  int linesPerFrame;						// the number of lines in each frame
  int NFrames;								// the number of frames

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_036;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
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
        byte choice0 = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String options0[2];
        options0[0] = F("3C");
        options0[1] = F("3D");
        int optionValues0[2];
        optionValues0[0] = 0x3C;
        optionValues0[1] = 0x3D;
        string += F("<TR><TD>I2C Address:<TD><select name='plugin_036_adr'>");
        for (byte x = 0; x < 2; x++)
        {
          string += F("<option value='");
          string += optionValues0[x];
          string += "'";
          if (choice0 == optionValues0[x])
            string += F(" selected");
          string += ">";
          string += options0[x];
          string += F("</option>");
        }
        string += F("</select>");

        //

        byte choice1 = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        String options1[2];
        options1[0] = F("Normal");
        options1[1] = F("Rotated");
        int optionValues1[2];
        optionValues1[0] = 1;
        optionValues1[1] = 2;
        string += F("<TR><TD>Rotation:<TD><select name='plugin_036_rotate'>");
        for (byte x = 0; x < 2; x++)
        {
          string += F("<option value='");
          string += optionValues1[x];
          string += "'";
          if (choice1 == optionValues1[x])
            string += F(" selected");
          string += ">";
          string += options1[x];
          string += F("</option>");
        }
        string += F("</select>");

        byte choice2 = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        String options2[4];
        options2[0] = F("1");
        options2[1] = F("2");
        options2[2] = F("3");
        options2[3] = F("4");
        int optionValues2[4];
        optionValues2[0] = 1;
        optionValues2[1] = 2;
        optionValues2[2] = 3;
        optionValues2[3] = 4;
        string += F("<TR><TD>Lines per Frame:<TD><select name='plugin_036_nlines'>");
        for (byte x = 0; x < 4; x++)
        {
          string += F("<option value='");
          string += optionValues2[x];
          string += "'";
          if (choice2 == optionValues2[x])
            string += F(" selected");
          string += ">";
          string += options2[x];
          string += F("</option>");
        }
        string += F("</select>");

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

        string += F("<TR><TD>Scroll:<TD><select name='plugin_036_scroll'>");
        for (byte x = 0; x < 5; x++)
        {
          string += F("<option value='");
          string += optionValues3[x];
          string += F("'");
          if (choice3 == optionValues3[x])
            string += F(" selected");
          string += F(">");
          string += options3[x];
          string += F("</option>");
        }
        string += F("</select>");

        char deviceTemplate[Nlines][32];
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

        for (byte varNr = 0; varNr < Nlines; varNr++)
        {
          string += F("<TR><TD>Line ");
          string += varNr + 1;
          string += F(":<TD><input type='text' size='32' maxlength='32' name='Plugin_036_template");
          string += varNr + 1;
          string += F("' value='");
          string += deviceTemplate[varNr];
          string += F("'>");
        }

        string += F("<TR><TD>Display button:<TD>");
        addPinSelect(false, string, "taskdevicepin3", Settings.TaskDevicePin3[event->TaskIndex]);

        char tmpString[128];

        sprintf_P(tmpString, PSTR("<TR><TD>Display Timeout:<TD><input type='text' name='plugin_036_timer' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][4]);
        string += tmpString;

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {

        String plugin0 = WebServer.arg(F("plugin_036_adr"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin0.toInt();

        String plugin1 = WebServer.arg(F("plugin_036_rotate"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = plugin1.toInt();

        String plugin2 = WebServer.arg(F("plugin_036_nlines"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = plugin2.toInt();

        String plugin3 = WebServer.arg(F("plugin_036_scroll"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = plugin3.toInt();

        String plugin4 = WebServer.arg(F("plugin_036_timer"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][4] = plugin4.toInt();

        String argName;

        char deviceTemplate[Nlines][32];
        for (byte varNr = 0; varNr < Nlines; varNr++)
        {
          argName = F("Plugin_036_template");
          argName += varNr + 1;
          strncpy(deviceTemplate[varNr], WebServer.arg(argName).c_str(), sizeof(deviceTemplate[varNr]));
        }

        SaveCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        // Load the custom settings from flash
        char deviceTemplate[Nlines][32];
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

        int OLED_address = Settings.TaskDevicePluginConfig[event->TaskIndex][0];

        //      Init the display and turn it on
        if (!display)
          display = new SSD1306(0, 0, 0);
        display->init(OLED_address);		// call to local override of init function
        display->displayOn();

        //      Set the initial value of OnOff to On
        UserVar[event->BaseVarIndex] = 1;

        //      flip screen if required
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] == 2)display->flipScreenVertically();

        //      Display the device name, logo, time and wifi
        display_espname();
        display_logo();
        display_time();

        int nbars = (WiFi.RSSI() + 100) / 8;
        display_wifibars(105, 0, 15, 10, 5, nbars);

        display->display();

        //      Set up the display timer
        displayTimer = Settings.TaskDevicePluginConfig[event->TaskIndex][4];

        if (Settings.TaskDevicePin3[event->TaskIndex] != -1)
        {
          pinMode(Settings.TaskDevicePin3[event->TaskIndex], INPUT_PULLUP);
        }

        //		Initialize frame counter
        frameCounter = 0;

        success = true;
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

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        char deviceTemplate[Nlines][32];
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

        // Clear the init screen if this is the first call
        if (firstcall)
        {
          display->clear();
          firstcall = false;
        }

        //      Define Scroll area layout
        linesPerFrame = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        NFrames = Nlines / linesPerFrame;

        //      Now create the string for the outgoing and incoming frames
        String tmpString[4];
        String newString[4];
        String oldString[4];

        //      Construct the outgoing string
        for (byte i = 0; i < linesPerFrame; i++)
        {
          tmpString[i] = deviceTemplate[(linesPerFrame * frameCounter) + i];
          oldString[i] = parseTemplate(tmpString[i], 20);
          oldString[i].trim();
        }

        // now loop round looking for the next frame with some content
        int tlen = 0;
        int ntries = 0;
        while (tlen == 0) {

          //        Stop after framecount loops if no data found
          ntries += 1;
          if (ntries > NFrames) break;

          //        Increment the frame counter
          frameCounter = frameCounter + 1;
          if ( frameCounter > NFrames - 1) frameCounter = 0;

          //        Contruct incoming strings
          for (byte i = 0; i < linesPerFrame; i++)
          {
            tmpString[i] = deviceTemplate[(linesPerFrame * frameCounter) + i];
            newString[i] = parseTemplate(tmpString[i], 20);
            newString[i].trim();
          }

          //      skip this frame if all lines in frame are blank - we exit the while loop if tlen is not zero
          tlen = 0;
          for (byte i = 0; i < linesPerFrame; i++)
          {
            tlen += newString[i].length();
          }
        }

        //      Update display
        display_time();

        int nbars = (WiFi.RSSI() + 100) / 8;
        display_wifibars(105, 0, 15, 10, 5, nbars);

        display_espname();
        display_indicator(frameCounter, NFrames);
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
        if (tmpString.equalsIgnoreCase(F("OLEDFRAMEDCMD")))
        {
          success = true;
          argIndex = string.lastIndexOf(',');
          tmpString = string.substring(argIndex + 1);
          if (tmpString.equalsIgnoreCase(F("Off")))
            display->displayOff();
          else if (tmpString.equalsIgnoreCase(F("On")))
            display->displayOn();
        }
        break;
      }

  }
  return success;
}

// The screen is set up as 10 rows at the top for the header, 10 rows at the bottom for the footer and 44 rows in the middle for the scroll region

void display_time() {
  String dtime = "%systime%";
  String newString = parseTemplate(dtime, 10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->setColor(BLACK);
  display->fillRect(0, 0, 28, 10);
  display->setColor(WHITE);
  display->drawString(0, 0, newString.substring(0, 5));
}

void display_espname() {
  String dtime = "%sysname%";
  String newString = parseTemplate(dtime, 10);
  newString.trim();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 0, newString);
}

void display_logo() {
  // draw an xbm image.
  display->drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
}

// Draw the frame position

void display_indicator(int iframe, int frameCount) {

  //  Erase Indicator Area

  display->setColor(BLACK);
  display->fillRect(0, 54, 128, 10);
  display->setColor(WHITE);

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

    int margin = 20;
    int spacing = (128 - 2 * margin) / (frameCount - 1);

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
    display->setFont(Dialog_Plain_12);
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

    //delay(2);
    backgroundtasks();
  }
}

//Draw Signal Strength Bars
void display_wifibars(int x, int y, int size_x, int size_y, int nbars, int nbars_filled) {

  //	x,y are the x,y locations
  //	sizex,sizey are the sizes (should be a multiple of the number of bars)
  //	nbars is the number of bars and nbars_filled is the number of filled bars.

  //	We leave a 1 pixel gap between bars

  for (byte ibar = 1; ibar < nbars + 1; ibar++) {

    display->setColor(BLACK);
    display->fillRect(x + (ibar - 1)*size_x / nbars, y, size_x / nbars, size_y);
    display->setColor(WHITE);

    if (ibar <= nbars_filled) {
      display->fillRect(x + (ibar - 1)*size_x / nbars, y + (nbars - ibar)*size_y / nbars, (size_x / nbars) - 1, size_y * ibar / nbars);
    }
    else
    {
      display->drawRect(x + (ibar - 1)*size_x / nbars, y + (nbars - ibar)*size_y / nbars, (size_x / nbars) - 1, size_y * ibar / nbars);
    }
  }
}
