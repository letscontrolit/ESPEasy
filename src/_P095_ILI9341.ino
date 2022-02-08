#include "_Plugin_Helper.h"
#ifdef USES_P095

#include "src/PluginStructs/P095_data_struct.h"

//#######################################################################################################
//#################################### Plugin 095: ILI9341 TFT 2.4inches display #################################
//#######################################################################################################

#define PLUGIN_095
#define PLUGIN_ID_095         95
#define PLUGIN_NAME_095       "Display - TFT 2.4 inches ILI9341 [TESTING]"
#define PLUGIN_VALUENAME1_095 "TFT"
#define PLUGIN_095_MAX_DISPLAY 1


/**
 * Changelog:
 * 2020-08-29 tonhuisman: Removed TS (Touchscreen) related stuff, XPT2046 will be a separate plugin
 *                        Changed initial text from '--cdt--' to 'ESPEasy'
 * 2020-08 tonhuisman: SPI improvements
 * 2020-04 TD-er: Pull plugin into main repository and rework according to new plugin standards
 * 2019 Jean-Michel Decoret: Initial plugin work
 */
/* README.MD

## INTRO

This plugin allow to control a TFT screen (ILI9341) through HTTP API

## Environment
Tested with WEMOS D1 Mini Pro and Wemos TDFT 2.4
Tested with ESPEasy 2.4.2  -tag mega-201902225)

TFT Shield : https://docs.wemos.cc/en/latest/d1_mini_shiled/tft_2_4.html
Price : ~ 5.40€/$ (https://fr.aliexpress.com/item/32919729730.html)


## Dependencies
Plugin lib_deps = Adafruit GFX, Adafruit ILI9341

## API Documentation

This plugin is controlled by HTTP API, ie : http://<espeasy_ip>/control?cmd=tft,tx,HelloWorld

| command | details | description |
|-----|-----|-----|
| tft | `TFT,<tft_subcommand>,....` | Draw line, rect, circle, triangle and text |
|tftcmd | `TFTCMD,<tftcmd_subcommand>` | Control the screen (on, off, clear,..) |

TFT Subcommands:

| TFT Subcommands | details | description |
|-----|-----|-----|
| txt | txt,<text> | Write simple text (use last position, color and size) |
| txp | txp,<X>,<Y> | Set text position (move the cursor) |
| txc | txc,<foreColor>,<backgroundColor> | Set text color (background is transparent if not provided |
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
| font| font,<fontname>  | Switch to font - SEVENSEG24, SEVENSEG18, FREESANS, DEFAULT |

TFTCMD Subcommands:

| TFT Subcommands | details | description |
|-----|-----|-----|
| on | on | Display ON |
| off | off | Display OFF |
| clear | clear,<color> | Clear display |
| inv | inv,<value> | Invert the dispaly (value:0 normal display, 1 inverted display) |
| rot | rot,<value> | Rotate display (value from 0 to 3 inclusive) |


Examples:

	Write Text :
		http://<espeasy_ip>/control?cmd=tft,txtfull,0,0,HelloWorld

	Write Text another place:
		http://<espeasy_ip>/control?cmd=tft,txtfull,100,40,HelloWorld

	Write bigger Text :
		http://<espeasy_ip>/control?cmd=tft,txtfull,0,0,3,HelloWorld

	Write RED Text :
		http://<espeasy_ip>/control?cmd=tft,txtfull,0,0,3,HelloWorld

	Write RED Text (size is 1):
		http://<espeasy_ip>/control?cmd=tft,txtfull,0,0,1,RED,HelloWorld

	Write RED Text on YELLOW background (size is 1):
		http://<espeasy_ip>/control?cmd=tft,txtfull,0,0,1,RED,YELLOW,HelloWorld

	Switch display ON
		http://<espeasy_ip>/control?cmd=tftcmd,on

	Switch display OFF
		http://<espeasy_ip>/control?cmd=tftcmd,off

	Clear whole display
		http://<espeasy_ip>/control?cmd=tftcmd,clear

	Clear GREEN whole display
		http://<espeasy_ip>/control?cmd=tftcmd,clear,green
*/


//Define the default values for both ESP32/lolin32 and D1 Mini
#ifdef ESP32
//for D32 Pro with TFT connector
  #define TFT_CS 14
  #define TFT_CS_HSPI 26  // when connected to Hardware-SPI GPIO-14 is already used
  #define TFT_DC 27
  #define TFT_RST 33
#else
 //for D1 Mini with shield connection
  #define TFT_CS 16   // D0
  #define TFT_DC 15   // D8
  #define TFT_RST -1
#endif

//The setting structure
struct Plugin_095_TFT_SettingStruct
{
  Plugin_095_TFT_SettingStruct()
  : address_tft_cs(TFT_CS), address_tft_dc(TFT_DC), address_tft_rst(TFT_RST), rotation(0)
  {

  }
  uint8_t address_tft_cs;
  uint8_t address_tft_dc;
  uint8_t address_tft_rst;
  uint8_t rotation;
} TFT_Settings;


boolean Plugin_095(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_095;
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
        string = F(PLUGIN_NAME_095);
        success = true;
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_095));
        success = true;
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output(F("TFT CS"));
        event->String2 = formatGpioName_output(F("TFT DC"));
        event->String3 = formatGpioName_output(F("TFT RST"));
        break;
      }

    case PLUGIN_SET_DEFAULTS:
      {
        uint8_t init = PCONFIG(0);

        //if already configured take it from settings, else use default values (only for pin values)
        if(init != 1)
        {
          #ifdef ESP32
          if (Settings.InitSPI == 2) {  // When using ESP32 H(ardware-)SPI
            TFT_Settings.address_tft_cs = TFT_CS_HSPI; 
          }
          #endif
          PIN(0) = TFT_Settings.address_tft_cs;
          PIN(1) = TFT_Settings.address_tft_dc;
          PIN(2) = TFT_Settings.address_tft_rst;
        }
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        uint8_t init = PCONFIG(0);

        //if already configured take it from settings, else use default values (only for pin values)
        if(init == 1)
        {
          TFT_Settings.address_tft_cs = PIN(0);
          TFT_Settings.address_tft_dc = PIN(1);
          TFT_Settings.address_tft_rst = PIN(2);
        }

        uint8_t choice2 = PCONFIG(1);
        const __FlashStringHelper * options2[4] = { F("Normal"), F("+90&deg;"), F("+180&deg;"), F("+270&deg;") };
        int optionValues2[4] = { 0, 1, 2, 3 };
        addFormSelector(F("Rotation"), F("p095_rotate"), 4, options2, optionValues2, choice2);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(0) = 1; //mark config as already saved (next time, will not use default values)
        // PIN(0)..(2) are already set
        PCONFIG(1) = getFormItemInt(F("p095_rotate"));
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {

        TFT_Settings.address_tft_cs = PIN(0);
        TFT_Settings.address_tft_dc = PIN(1);
        TFT_Settings.address_tft_rst = PIN(2);
        TFT_Settings.rotation = PCONFIG(1);

        initPluginTaskData(event->TaskIndex, 
        new (std::nothrow) P095_data_struct(
          TFT_Settings.address_tft_cs, 
          TFT_Settings.address_tft_dc, 
          TFT_Settings.address_tft_rst));
        P095_data_struct *P095_data =
          static_cast<P095_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P095_data) {
          P095_data->tft.setRotation(TFT_Settings.rotation);
          P095_data->tft.fillScreen(ILI9341_WHITE);
          P095_data->printText(F("ESPEasy"), 1, 1);
          success = true;
        }
        break;
      }

    case PLUGIN_WRITE:
      {
        String tmpString = String(string);
        String arguments = String(string);

        String command;
        String subcommand;

        int argIndex = arguments.indexOf(',');
        if (argIndex)
        {
          P095_data_struct *P095_data =
            static_cast<P095_data_struct *>(getPluginTaskData(event->TaskIndex));

          if (nullptr != P095_data) {

            command = arguments.substring(0, argIndex);
            arguments = arguments.substring(argIndex+1);
            argIndex = arguments.indexOf(',');
            subcommand = arguments.substring(0, argIndex);
            success = true;

            tmpString += F("<br/> command= ");
            tmpString += command;
            tmpString += F("<br/> arguments= ");
            tmpString += arguments;
            tmpString += F("<br/> argIndex= ");
            tmpString += String(argIndex);
            tmpString += F("<br/> subcommand= ");
            tmpString += subcommand;


            if (command.equalsIgnoreCase(F("TFTCMD")))
            {
              if(subcommand.equalsIgnoreCase(F("ON")))
              {
                P095_data->tft.sendCommand(ILI9341_DISPON);
              }
              else if(subcommand.equalsIgnoreCase(F("OFF")))
              {
                P095_data->tft.sendCommand(ILI9341_DISPOFF);
              }
              else if(subcommand.equalsIgnoreCase(F("CLEAR")))
              {
                arguments = arguments.substring(argIndex + 1);
                P095_data->tft.fillScreen(P095_data->ParseColor(arguments));
              }
              else if(subcommand.equalsIgnoreCase(F("INV")))
              {
                arguments = arguments.substring(argIndex + 1);
                P095_data->tft.invertDisplay(arguments.toInt() == 1);
              }
              else if(subcommand.equalsIgnoreCase(F("ROT")))
              {
                ///control?cmd=tftcmd,rot,0
                //not working to verify
                arguments = arguments.substring(argIndex + 1);
                P095_data->tft.setRotation(arguments.toInt() % 4);
              }
              else
              {
                success = false;
              }
            }
            else if (command.equalsIgnoreCase(F("TFT")))
            {
              tmpString += F("<br/> TFT  ");

              arguments = arguments.substring(argIndex + 1);
              String sParams[8];
              int argCount = P095_data->StringSplit(arguments, ',', sParams, 8);

              for(int a=0; a < argCount && a < 8; a++)
              {
                  tmpString += F("<br/> ARGS[");
                  tmpString += a;
                  tmpString += F("]=");
                  tmpString += sParams[a];
              }

              if(subcommand.equalsIgnoreCase(F("txt")))
              {
                P095_data->tft.println(arguments); //write all pending cars
              }
              else if(subcommand.equalsIgnoreCase(F("txp")) && argCount == 2)
              {
                P095_data->tft.setCursor(sParams[0].toInt(), sParams[1].toInt());
              }
              else if(subcommand.equalsIgnoreCase(F("txc")) && (argCount == 1 || argCount == 2) )
              {
                if(argCount == 1)
                  P095_data->tft.setTextColor(P095_data->ParseColor(sParams[0]));
                else //argCount=2
                  P095_data->tft.setTextColor(P095_data->ParseColor(sParams[0]), P095_data->ParseColor(sParams[1]));
              }
              else if(subcommand.equalsIgnoreCase(F("txs")) && argCount == 1)
              {
                P095_data->tft.setTextSize(sParams[0].toInt());
              }
              #ifdef PLUGIN_095_FONT_INCLUDED
                  else if(subcommand.equalsIgnoreCase(F("font")) && argCount == 1) {
                    if (sParams[0].equalsIgnoreCase(F("SEVENSEG24"))) {
                        P095_data->tft.setFont(&Seven_Segment24pt7b);
                    } else if (sParams[0].equalsIgnoreCase(F("SEVENSEG18"))) {
                        P095_data->tft.setFont(&Seven_Segment18pt7b);
                    } else if (sParams[0].equalsIgnoreCase(F("FREESANS"))) {
                        P095_data->tft.setFont(&FreeSans9pt7b);
                    } else if (sParams[0].equalsIgnoreCase(F("DEFAULT"))) {
                        P095_data->tft.setFont(); 
                    } else {
                        success = false;
                    }
                  }
              #endif
              else if(subcommand.equalsIgnoreCase(F("txtfull")) && argCount >= 3 && argCount <= 6)
              {
                switch (argCount)
                {
                case 3: //single text
                  P095_data->printText(sParams[2], sParams[0].toInt() - 1,sParams[1].toInt() - 1);
                  break;

                case 4: //text + size
                  P095_data->printText(sParams[3], sParams[0].toInt() - 1, sParams[1].toInt() - 1, sParams[2].toInt());
                  break;

                case 5: //text + size + color
                  P095_data->printText(sParams[4], sParams[0].toInt() - 1, sParams[1].toInt() - 1, sParams[2].toInt(), P095_data->ParseColor(sParams[3]));
                  break;

                case 6: //text + size + color
                  P095_data->printText(sParams[5], sParams[0].toInt() - 1, sParams[1].toInt() - 1, sParams[2].toInt(), P095_data->ParseColor(sParams[3]), P095_data->ParseColor(sParams[4]));
                  break;
                default:
                  success = false;
                  break;
                }
              }
              else if(subcommand.equalsIgnoreCase(F("l")) && argCount == 5)
              {
                P095_data->tft.drawLine(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), P095_data->ParseColor(sParams[4]));
              }
              else if(subcommand.equalsIgnoreCase(F("lh")) && argCount == 3)
              {
                P095_data->tft.drawFastHLine(0, sParams[0].toInt(), sParams[1].toInt(), P095_data->ParseColor(sParams[2]));
              }
              else if(subcommand.equalsIgnoreCase(F("lv")) && argCount == 3)
              {
                P095_data->tft.drawFastVLine(sParams[0].toInt(), 0, sParams[1].toInt(), P095_data->ParseColor(sParams[2]));
              }
              else if(subcommand.equalsIgnoreCase(F("r")) && argCount == 5)
              {
                P095_data->tft.drawRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), P095_data->ParseColor(sParams[4]));
              }
              else if(subcommand.equalsIgnoreCase(F("rf")) && argCount == 6)
              {
                P095_data->tft.fillRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), P095_data->ParseColor(sParams[5]));
                P095_data->tft.drawRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), P095_data->ParseColor(sParams[4]));
              }
              else if(subcommand.equalsIgnoreCase(F("c")) && argCount == 4)
              {
                P095_data->tft.drawCircle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), P095_data->ParseColor(sParams[3]));
              }
              else if(subcommand.equalsIgnoreCase(F("cf")) && argCount == 5)
              {
                P095_data->tft.fillCircle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), P095_data->ParseColor(sParams[4]));
                P095_data->tft.drawCircle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), P095_data->ParseColor(sParams[3]));
              }
              else if(subcommand.equalsIgnoreCase(F("t")) && argCount == 7)
              {
                P095_data->tft.drawTriangle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), sParams[5].toInt(), P095_data->ParseColor(sParams[6]));
              }
              else if(subcommand.equalsIgnoreCase(F("tf")) && argCount == 8)
              {
                P095_data->tft.fillTriangle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), sParams[5].toInt(), P095_data->ParseColor(sParams[7]));
                P095_data->tft.drawTriangle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), sParams[5].toInt(), P095_data->ParseColor(sParams[6]));
              }
              else if(subcommand.equalsIgnoreCase(F("rr")) && argCount == 6)
              {
                P095_data->tft.drawRoundRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), P095_data->ParseColor(sParams[5]));
              }
              else if(subcommand.equalsIgnoreCase(F("rrf")) && argCount == 7)
              {
                P095_data->tft.fillRoundRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), P095_data->ParseColor(sParams[6]));
                P095_data->tft.drawRoundRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), P095_data->ParseColor(sParams[5]));
              }
              else if(subcommand.equalsIgnoreCase(F("px")) && argCount == 3)
              {
                P095_data->tft.drawPixel(sParams[0].toInt(), sParams[1].toInt(), P095_data->ParseColor(sParams[2]));
              }
              else
              {
                success = false;
              }
            }
            else {
              success = false;
            }                 
          }
        }
        else
        {
          //invalid arguments
          success = false;
        }

        if(!success)
        {
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLog(LOG_LEVEL_INFO, F("Fail to parse command correctly; please check API documentation"));
            String log2  = F("Parsed command = \"");
            log2 += string;
            log2 += F("\"");
            addLog(LOG_LEVEL_INFO, log2);
          }
        } 
        else 
        {
            String log;
            if (log.reserve(24 + tmpString.length())) { // Prevent re-allocation
              log = F("P095-ILI9341 : WRITE = ");
              log += tmpString;
              SendStatus(event, log);             // Reply (echo) to sender. This will print message on browser.
            } else {
              SendStatus(event, F("P095-ILI9341 : WRITE = "));
            }
        }
        break;
      }
  }



  return success;
}

#endif // USES_P095