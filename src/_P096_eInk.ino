#include "_Plugin_Helper.h"
#ifdef USES_P096
//#######################################################################################################
//#################################### Plugin 096: eInk display #################################
//#######################################################################################################

#define PLUGIN_096
#define PLUGIN_ID_096         96
#define PLUGIN_NAME_096       "Display - eInk with Lolin ePaper screen [TESTING]"
#define PLUGIN_VALUENAME1_096 "EINK"
#define PLUGIN_096_MAX_DISPLAY 1

/* README.MD

## INTRO

This plugin allow to control a eInk screen (ILI3897) through HTTP API

## Environment
Tested with Lolin d32 pro and Wemos ePaper 2.13 shield
Tested with ESPEasy 2.4.2  -tag mega-201902225)

ePaper Shield : https://www.wemos.cc/en/latest/d1_mini_shiled/epd_2_13.html
Price : ~ 10€/$ (https://fr.aliexpress.com/item/32981318996.html)


## Dependencies
Plugin lib_deps = Adafruit GFX, LOLIN_EPD

## API Documentation

This plugin is controlled by HTTP API, ie : http://<espeasy_ip>/control?cmd=epd,tx,HelloWorld

| command | details | description |
|-----|-----|-----|
| epd | `EPD,<epd_subcommand>,....` | Draw line, rect, circle, triangle and text |
|epdcmd | `EPDCMD,<epdcmd_subcommand>` | Control the screen (on, off, clear,..) |

EPD Subcommands:

| EPD Subcommands | details | description |
|-----|-----|-----|
| txt | txt,<text> | Write simple text (use last position, color and size) |
| txp | txp,<X>,<Y> | Set text position (move the cursor) |
| txc | txc,<foreColor>,<backgroundColor> | Set text color (background is transparent if not provided |
| txz | txz,<X>,<Y>,<text> | Write text at position (move the cursor + print) |
| txs | txs,<SIZE> | Set text size |
| txtfull | txtfull,<row>,<col>,<size=1>,<foreColor=white>,<backColor=black>,<text> | Write text with all options |
| l | l,<x1>,<y1>,<2>,<y2>,<color> | Draw a simple line |
| lh | lh,<y>,<width>,<color> | Draw an horizontal line (width = Line width in pixels (positive = right of first point, negative = point of first corner). |
| lv | lv,<x>,<height>,<color> | Draw a vertical line (height= Line height in pixels (positive = below first point, negative = above first point).|
| r | r,<x>,<y>,<width>,<height>,<color> | Draw a rectangle |
| rf | rf,<x>,<y>,<width>,<height>,<bordercolor>,<innercolor> | Draw a filled rectangle |
| c | c,<x>,<y>,<radius>,<color> | Draw a circle |
| cf | cf,<x>,<y>,<radius>,<bordercolor>,<innercolor> | Draw a filled circle |
| t | t,<x1>,<y1>,<x2>,<y2>,<x3>,<y3>,<color>| Draw a triangle |
| tf | tf,<x1>,<y1>,<x2>,<y2>,<x3>,<y3>,<bordercolor>,<innercolor> | Draw a filled triangle |
| rr | rr,<x>,<y>,<width>,<height>,<corner_radius>,<color> | Draw a round rectangle |
| rrf | rrf,<x>,<y>,<width>,<height>,<corner_radius>,<bordercolor>,<innercolor> | Draw a filled round rectangle |
| px | px,<x>,<y>,<color> | Print a single pixel |
  
EPDCMD Subcommands:

| EPD Subcommands | details | description |
|-----|-----|-----|
| clear | clear,<color> | Clear display |
| deepsleep | deepsleep | Make screen go to sleep |
| inv | inv,<value> | Invert the dispaly (value:0 normal display, 1 inverted display) |
| rot | rot,<value> | Rotate display (value from 0 to 3 inclusive) |


Examples:

	Write Text :
		http://<espeasy_ip>/control?cmd=epd,txtfull,0,0,HelloWorld
	
	Write Text another place:
		http://<espeasy_ip>/control?cmd=epd,txtfull,100,40,HelloWorld

	Write bigger Text :
		http://<espeasy_ip>/control?cmd=epd,txtfull,0,0,3,HelloWorld

	Write RED Text :
		http://<espeasy_ip>/control?cmd=epd,txtfull,0,0,3,HelloWorld

	Write RED Text (size is 1):
		http://<espeasy_ip>/control?cmd=epd,txtfull,0,0,1,RED,HelloWorld

	Write RED Text on YELLOW background (size is 1):
		http://<espeasy_ip>/control?cmd=epd,txtfull,0,0,1,RED,YELLOW,HelloWorld
		
	Clear whole display
		http://<espeasy_ip>/control?cmd=epdcmd,clear

	Deepsleep screen
		http://<espeasy_ip>/control?cmd=epdcmd,deepsleep

*/

//plugin dependency
#include <LOLIN_EPD.h>
#include <Adafruit_GFX.h>

//declare functions for using default value parameters
void Plugin_096_printText(const char *string, int X, int Y, unsigned int textSize = 1, unsigned short color = EPD_WHITE, unsigned short bkcolor = EPD_BLACK);

//Define the default values for both ESP32/lolin32 and D1 Mini 
#ifdef ESP32
//for D32 Pro with EPD connector
  #define EPD_CS 14
  #define EPD_CS_HSPI 26  // when connected to Hardware-SPI GPIO-14 is already used
  #define EPD_DC 27
  #define EPD_RST 33  // can set to -1 and share with microcontroller Reset!
  #define EPD_BUSY -1 // can set to -1 to not use a pin (will wait a fixed delay)
#else
  //for D1 Mini with shield connection
  #define EPD_CS D0
  #define EPD_DC D8
  #define EPD_RST -1  // can set to -1 and share with microcontroller Reset!
  #define EPD_BUSY -1 // can set to -1 to not use a pin (will wait a fixed delay)
#endif


//The setting structure
struct Plugin_096_EPD_SettingStruct
{
  Plugin_096_EPD_SettingStruct()
  : address_epd_cs(EPD_CS), address_epd_dc(EPD_DC), address_epd_rst(EPD_RST), address_epd_busy(EPD_BUSY), rotation(0), width(250), height(122)
  {

  }
  byte address_epd_cs;
  byte address_epd_dc;
  byte address_epd_rst;
  byte address_epd_busy;
  byte rotation;
  int width;
  int height;
} EPD_Settings;

//The display pointer
LOLIN_IL3897 *eInkScreen = NULL;
byte plugin_096_sequence_in_progress = false;

boolean Plugin_096(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_096;
        Device[deviceCount].Type = DEVICE_TYPE_SPI3;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_NONE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 0;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        success = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_096);
        success = true;
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_096));
        success = true;
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output(F("EPD CS"));
        event->String2 = formatGpioName_output(F("EPD DC"));
        event->String3 = formatGpioName_output(F("EPD RST"));
        break;
      }

    case PLUGIN_SET_DEFAULTS:
      {
        byte init = PCONFIG(0);

        //if already configured take it from settings, else use default values (only for pin values)
        if(init != 1)
        {
          #ifdef ESP32
          if (Settings.InitSPI == 2) {  // When using ESP32 H(ardware-)SPI
            EPD_Settings.address_epd_cs = EPD_CS_HSPI; 
          }
          #endif
          PIN(0) = EPD_Settings.address_epd_cs;
          PIN(1) = EPD_Settings.address_epd_dc;
          PIN(2) = EPD_Settings.address_epd_rst;
          PIN(3) = EPD_Settings.address_epd_busy;
        }
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte init = PCONFIG(0);

        //if already configured take it from settings, else use default values (only for pin values)
        if(init == 1)
        {
          EPD_Settings.address_epd_cs = PIN(0);
          EPD_Settings.address_epd_dc = PIN(1);
          EPD_Settings.address_epd_rst = PIN(2);
          EPD_Settings.address_epd_busy = PIN(3);
        }

        addFormPinSelect(formatGpioName_output(F("EPD BUSY")), F("p096_epd_busy"), EPD_Settings.address_epd_busy);

        byte choice2 = PCONFIG(1);
        String options2[4] = { F("Normal"), F("+90&deg;"), F("+180&deg;"), F("+270&deg;") };
        int optionValues2[4] = { 0, 1, 2, 3 };
        addFormSelector(F("Rotation"), F("p096_rotate"), 4, options2, optionValues2, choice2);

        uint16_t width_ = PCONFIG(2);
        if(width_ == 0)
          width_ = 250; //default value
        addFormNumericBox(F("Width (px)"), F("p096_width"), width_, 1, 65535);

        uint16_t height_ = PCONFIG(3);
        if(height_ == 0)
          height_ = 122; //default value
        addFormNumericBox(F("Height (px)"), F("p096_height"), height_, 1, 65535);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(0) = 1; //mark config as already saved (next time, will not use default values)
        // PIN(0)..(2) are already set
        PIN(3) = getFormItemInt(F("p096_epd_busy"));
        PCONFIG(1) = getFormItemInt(F("p096_rotate"));
        PCONFIG(2) = getFormItemInt(F("p096_width"));
        PCONFIG(3) = getFormItemInt(F("p096_height"));
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        byte init = PCONFIG(0);

        //if already configured take it from settings, else use default values (only for pin values)
        if(init != 1)
        {
          PIN(0) = EPD_Settings.address_epd_cs;
          PIN(1) = EPD_Settings.address_epd_dc;
          PIN(2) = EPD_Settings.address_epd_rst;
          PIN(3) = EPD_Settings.address_epd_busy;
        }

        EPD_Settings.address_epd_cs = PIN(0);
        EPD_Settings.address_epd_dc = PIN(1);
        EPD_Settings.address_epd_rst = PIN(2);
        EPD_Settings.address_epd_busy = PIN(3);
        EPD_Settings.rotation = PCONFIG(1);
        EPD_Settings.width = PCONFIG(2);
        EPD_Settings.height = PCONFIG(3);

        eInkScreen = new LOLIN_IL3897(EPD_Settings.width, EPD_Settings.height, EPD_Settings.address_epd_dc, EPD_Settings.address_epd_rst, EPD_Settings.address_epd_cs, EPD_Settings.address_epd_busy); //hardware SPI
        plugin_096_sequence_in_progress = false;
        eInkScreen->begin();
        eInkScreen->clearBuffer();

        eInkScreen->setTextColor(EPD_BLACK);
        eInkScreen->setTextSize(3);
        eInkScreen->println("ESP Easy");
        eInkScreen->setTextSize(2);
        eInkScreen->println("eInk shield");
        eInkScreen->display();
        delay(100);
        
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
#ifndef BUILD_NO_DEBUG
        String tmpString = String(string);
#endif
        String arguments = String(string);

        String command = F("");
        String subcommand = F("");

        int argIndex = arguments.indexOf(',');
        if (argIndex)
        {
          command = arguments.substring(0, argIndex);
          arguments = arguments.substring(argIndex+1);
          argIndex = arguments.indexOf(',');
          subcommand = arguments.substring(0, argIndex);
          success = true;

#ifndef BUILD_NO_DEBUG
          tmpString += "<br/> command= " + command;
          tmpString += "<br/> arguments= " + arguments;
          tmpString += "<br/> argIndex= " + String(argIndex);
          tmpString += "<br/> subcommand= " + subcommand;
#endif

          if (command.equalsIgnoreCase(F("EPDCMD")))
          {
            if(subcommand.equalsIgnoreCase(F("CLEAR")))
            {
              arguments = arguments.substring(argIndex + 1);
              eInkScreen->clearBuffer();
              eInkScreen->fillScreen(Plugin_096_ParseColor(arguments));
              eInkScreen->display();
              eInkScreen->clearBuffer();
            } 
            else if(subcommand.equalsIgnoreCase(F("DEEPSLEEP")))
            {
              eInkScreen->deepSleep();
            }        
            else if(subcommand.equalsIgnoreCase(F("SEQ_START")))
            {
              eInkScreen->clearBuffer();
              eInkScreen->fillScreen(Plugin_096_ParseColor(arguments));
              plugin_096_sequence_in_progress = true;
            } 
            else if(subcommand.equalsIgnoreCase(F("SEQ_END")))
            {
#ifndef BUILD_NO_DEBUG
              TimingStats s;
              const unsigned statisticsTimerStart(micros());
#endif
              eInkScreen->display();
              
#ifndef BUILD_NO_DEBUG
              s.add(usecPassedSince(statisticsTimerStart));
              tmpString += "<br/> Display timings = " + String(s.getAvg());
#endif              
              eInkScreen->clearBuffer();
              plugin_096_sequence_in_progress = false;
            } 
            else if(subcommand.equalsIgnoreCase(F("INV")))
            {
              arguments = arguments.substring(argIndex + 1);
              eInkScreen->invertDisplay(arguments.toInt() == 1);
              eInkScreen->display();
            } 
            else if(subcommand.equalsIgnoreCase(F("ROT")))
            {
              arguments = arguments.substring(argIndex + 1);
              eInkScreen->setRotation(arguments.toInt() % 4);
              eInkScreen->display();
            } 
            else 
            {
              success = false;
            }
          }
          else if (command.equalsIgnoreCase(F("EPD")))
          {
#ifndef BUILD_NO_DEBUG
            tmpString += "<br/> EPD  ";
#endif
            arguments = arguments.substring(argIndex + 1);
            String sParams[8];
            int argCount = Plugin_096_StringSplit(arguments, ',', sParams, 8);

#ifndef BUILD_NO_DEBUG
            for(int a=0; a < argCount && a < 8; a++)
            {
                tmpString += "<br/> ARGS[" + String(a) + "]=" + sParams[a];
            }
#endif

            if(plugin_096_sequence_in_progress == false)
            {
              eInkScreen->clearBuffer();
              eInkScreen->fillScreen(EPD_WHITE);
            }

            if(subcommand.equalsIgnoreCase(F("txt")))
            {
              Plugin_096_FixText(arguments);
              eInkScreen->println(arguments); //write all pending cars
            }
            else if(subcommand.equalsIgnoreCase(F("txz")))
            {
              eInkScreen->setCursor(sParams[0].toInt(), sParams[1].toInt());
              Plugin_096_FixText(sParams[2]);
              eInkScreen->println(sParams[2]); //write all pending cars
            }
            else if(subcommand.equalsIgnoreCase(F("txp")) && argCount == 2)
            {
              eInkScreen->setCursor(sParams[0].toInt(), sParams[1].toInt());
            }
            else if(subcommand.equalsIgnoreCase(F("txc")) && (argCount == 1 || argCount == 2) )
            {
              if(argCount == 1)
                eInkScreen->setTextColor(Plugin_096_ParseColor(sParams[0]));
              else //argCount=2
                eInkScreen->setTextColor(Plugin_096_ParseColor(sParams[0]), Plugin_096_ParseColor(sParams[1]));
            }
            else if(subcommand.equalsIgnoreCase(F("txs")) && argCount == 1)
            {
              eInkScreen->setTextSize(sParams[0].toInt());
            }
            else if(subcommand.equalsIgnoreCase(F("txtfull")) && argCount >= 3 && argCount <= 6)
            {
              switch (argCount)
              {
              case 3: //single text
                Plugin_096_printText(sParams[2].c_str(), sParams[0].toInt() - 1,sParams[1].toInt() - 1);  
                break;

              case 4: //text + size
                Plugin_096_printText(sParams[3].c_str(), sParams[0].toInt() - 1, sParams[1].toInt() - 1, sParams[2].toInt());  
                break;

              case 5: //text + size + color
                Plugin_096_printText(sParams[4].c_str(), sParams[0].toInt() - 1, sParams[1].toInt() - 1, sParams[2].toInt(), Plugin_096_ParseColor(sParams[3]));  
                break;
              
              case 6: //text + size + color
                Plugin_096_printText(sParams[5].c_str(), sParams[0].toInt() - 1, sParams[1].toInt() - 1, sParams[2].toInt(), Plugin_096_ParseColor(sParams[3]), Plugin_096_ParseColor(sParams[4]));  
                break;
              default:
                success = false;
                break;
              }            
            }
            else if(subcommand.equalsIgnoreCase(F("l")) && argCount == 5)
            {
              eInkScreen->drawLine(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), Plugin_096_ParseColor(sParams[4]));
            }          
            else if(subcommand.equalsIgnoreCase(F("lh")) && argCount == 3)
            {
              eInkScreen->drawFastHLine(0, sParams[0].toInt(), sParams[1].toInt(), Plugin_096_ParseColor(sParams[2]));
            }          
            else if(subcommand.equalsIgnoreCase(F("lv")) && argCount == 3)
            {
              eInkScreen->drawFastVLine(sParams[0].toInt(), 0, sParams[1].toInt(), Plugin_096_ParseColor(sParams[2]));
            }          
            else if(subcommand.equalsIgnoreCase(F("r")) && argCount == 5)
            {
              eInkScreen->drawRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), Plugin_096_ParseColor(sParams[4]));
            }          
            else if(subcommand.equalsIgnoreCase(F("rf")) && argCount == 6)
            {
              eInkScreen->fillRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), Plugin_096_ParseColor(sParams[5]));
              eInkScreen->drawRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), Plugin_096_ParseColor(sParams[4]));
            }          
            else if(subcommand.equalsIgnoreCase(F("c")) && argCount == 4)
            {
              eInkScreen->drawCircle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), Plugin_096_ParseColor(sParams[3]));
            }          
            else if(subcommand.equalsIgnoreCase(F("cf")) && argCount == 5)
            {
              eInkScreen->fillCircle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), Plugin_096_ParseColor(sParams[4]));
              eInkScreen->drawCircle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), Plugin_096_ParseColor(sParams[3]));
            }
            else if(subcommand.equalsIgnoreCase(F("t")) && argCount == 7)
            {
              eInkScreen->drawTriangle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), sParams[5].toInt(), Plugin_096_ParseColor(sParams[6]));
            }           
            else if(subcommand.equalsIgnoreCase(F("tf")) && argCount == 8)
            {
              eInkScreen->fillTriangle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), sParams[5].toInt(), Plugin_096_ParseColor(sParams[7]));
              eInkScreen->drawTriangle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), sParams[5].toInt(), Plugin_096_ParseColor(sParams[6]));
            }           
            else if(subcommand.equalsIgnoreCase(F("rr")) && argCount == 6)
            {
              eInkScreen->drawRoundRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), Plugin_096_ParseColor(sParams[5]));
            }          
            else if(subcommand.equalsIgnoreCase(F("rrf")) && argCount == 7)
            {
              eInkScreen->fillRoundRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), Plugin_096_ParseColor(sParams[6]));
              eInkScreen->drawRoundRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), Plugin_096_ParseColor(sParams[5]));
            } 
            else if(subcommand.equalsIgnoreCase(F("px")) && argCount == 3)
            {
              eInkScreen->drawPixel(sParams[0].toInt(), sParams[1].toInt(), Plugin_096_ParseColor(sParams[2]));
            } 
            else 
            {
              success = false;
            }
          }
          else 
          {
            success = false;
          }
        }
        else
        {
          //invalid arguments
          success = false;
        }

        //in case of command outside of sequence, then refresh screen
        if(success && !plugin_096_sequence_in_progress)
        {
          eInkScreen->display();
        }

#ifndef BUILD_NO_DEBUG
        String log;
        log.reserve(110);                           // Prevent re-allocation
        log = F("P096-eInk : WRITE = ");
        log += tmpString;
        SendStatus(event, log);             // Reply (echo) to sender. This will print message on browser.  
#endif
        break;        
      }
  }

  return success;
}


//Print some text
//param [in] string : The text to display
//param [in] X : The left position (X)
//param [in] Y : The top position (Y)
//param [in] textSize : The text size (default 1)
//param [in] color : The fore color (default ILI9341_WHITE)
//param [in] bkcolor : The background color (default ILI9341_BLACK)
void Plugin_096_printText(const char *string, int X, int Y, unsigned int textSize, unsigned short color, unsigned short bkcolor)
{
  eInkScreen->clearBuffer();
  eInkScreen->clearDisplay();
  eInkScreen->setCursor(X, Y);
  eInkScreen->setTextColor(color, bkcolor);
  eInkScreen->setTextSize(textSize);
  String fixString = string;
  Plugin_096_FixText(fixString);
  eInkScreen->println(fixString);
  eInkScreen->display();
}

//Parse color string to color
//param [in] colorString : The color string (white, red, ...)
//return : color (default EPD_WHITE)
unsigned short Plugin_096_ParseColor(const String & colorString)
{
  //copy to local var and ensure lowercase
  //this optimise the next equlaity checks
  String s = colorString;
  s.toLowerCase();

  if (s.equals(F("black")))
    return EPD_BLACK;
  if (s.equals(F("white")))
    return EPD_WHITE;
  if (s.equals(F("inverse")))
    return EPD_INVERSE;
  if (s.equals(F("red")))
    return EPD_RED;
  if (s.equals(F("dark")))
    return EPD_DARK;
  if (s.equals(F("light")))
    return EPD_LIGHT;
  return EPD_WHITE;
}

//Fix text with handling special characters (degrees and main monetary symbols)
//This is specific case for current AdafruitGfx standard fontused for eink screen
//param [in/out] s : The string to fix
void Plugin_096_FixText(String & s)
{
  const char degree[3] = {0xc2, 0xb0, 0};  // Unicode degree symbol
  const char degree_eink[2] = {0xf7, 0};  // eink degree symbol
  s.replace(degree, degree_eink);
  s.replace(F("{D}"), degree_eink);
  s.replace(F("&deg;"), degree_eink);
  
  const char euro[4]  = { 0xe2, 0x82, 0xac, 0 }; // Unicode euro symbol
  const char euro_eink[2] = {0xED, 0};  // eink degree symbol
  s.replace(euro, euro_eink);
  s.replace(F("{E}"), euro_eink);
  s.replace(F("&euro;"), euro_eink);

  const char pound[3] = { 0xc2, 0xa3, 0 };       // Unicode pound symbol
  const char pound_eink[2] = {0x9C, 0};  // eink pound symbol
  s.replace(pound, pound_eink);
  s.replace(F("{P}"), pound_eink);
  s.replace(F("&pound;"), pound_eink);

  const char yen[3]   = { 0xc2, 0xa5, 0 };       // Unicode yen symbol
  const char yen_eink[2] = {0x9D, 0};  // eink yen symbol
  s.replace(yen, yen_eink);
  s.replace(F("{Y}"), yen_eink);
  s.replace(F("&yen;"), yen_eink);

  const char cent[3]   = { 0xc2, 0xa2, 0 };       // Unicode yen symbol
  const char cent_eink[2] = {0x9B, 0};  // eink cent symbol
  s.replace(cent, cent_eink);
  s.replace(F("{c}"), cent_eink);
  s.replace(F("&cent;"), cent_eink);

}

//Split a string by delimiter
//param [in] s : The input string
//param [in] c : The delimiter
//param [out] op : The resulting string array
//param [in] limit : The maximum strings to find
//return : The string count
int Plugin_096_StringSplit(const String &s, char c, String op[], int limit)
{
  int count = 0;
  char * pch;
  String d = String(c);
  pch = strtok ((char*)(s.c_str()),d.c_str());
  while (pch != NULL && count < limit)
  {
    op[count] = String(pch);
    count++;
    pch = strtok (NULL, ",");
  }  
  return count;
}


#endif // USES_P096