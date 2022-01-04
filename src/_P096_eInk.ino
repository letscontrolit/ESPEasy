#include "_Plugin_Helper.h"
#ifdef USES_P096

#include "src/PluginStructs/P096_data_struct.h"

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
  #define EPD_CS 16   // D0
  #define EPD_DC 15   // D8
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
  uint8_t address_epd_cs;
  uint8_t address_epd_dc;
  uint8_t address_epd_rst;
  uint8_t address_epd_busy;
  uint8_t rotation;
  int width;
  int height;
} EPD_Settings;


boolean Plugin_096(uint8_t function, struct EventStruct *event, String& string)
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
        uint8_t init = PCONFIG(0);

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

    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      string  = F("EPD BUSY: ");
      string += formatGpioLabel(PIN(3), false);
      success = true;
      break;
    }


    case PLUGIN_WEBFORM_LOAD:
      {
        uint8_t init = PCONFIG(0);

        //if already configured take it from settings, else use default values (only for pin values)
        if(init == 1)
        {
          EPD_Settings.address_epd_cs = PIN(0);
          EPD_Settings.address_epd_dc = PIN(1);
          EPD_Settings.address_epd_rst = PIN(2);
          EPD_Settings.address_epd_busy = PIN(3);
        }

        addFormPinSelect(formatGpioName_output(F("EPD BUSY")), F("p096_epd_busy"), EPD_Settings.address_epd_busy);

        {
          uint8_t choice2 = PCONFIG(1);
          const __FlashStringHelper * options2[4] = { F("Normal"), F("+90&deg;"), F("+180&deg;"), F("+270&deg;") };
          int optionValues2[4] = { 0, 1, 2, 3 };
          addFormSelector(F("Rotation"), F("p096_rotate"), 4, options2, optionValues2, choice2);
        }

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
        uint8_t init = PCONFIG(0);

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


        initPluginTaskData(event->TaskIndex, 
          new (std::nothrow) P096_data_struct(
            EPD_Settings.width, 
            EPD_Settings.height, 
            EPD_Settings.address_epd_dc, 
            EPD_Settings.address_epd_rst, 
            EPD_Settings.address_epd_cs, 
            EPD_Settings.address_epd_busy)); // hardware SPI
        P096_data_struct *P096_data =
          static_cast<P096_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P096_data) {
          P096_data->eInkScreen.setTextColor(EPD_BLACK);
          P096_data->eInkScreen.setTextSize(3);
          P096_data->eInkScreen.println(F("ESP Easy"));
          P096_data->eInkScreen.setTextSize(2);
          P096_data->eInkScreen.println(F("eInk shield"));
          P096_data->eInkScreen.display();
          delay(100);
          
          success = true;
        }
        break;
      }

    case PLUGIN_WRITE:
      {
#ifndef BUILD_NO_DEBUG
        String tmpString = String(string);
#endif
        String arguments = String(string);

        String command;
        String subcommand;

        P096_data_struct *P096_data =
          static_cast<P096_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P096_data) {

          int argIndex = arguments.indexOf(',');
          if (argIndex)
          {
            command = arguments.substring(0, argIndex);
            arguments = arguments.substring(argIndex+1);
            argIndex = arguments.indexOf(',');
            subcommand = arguments.substring(0, argIndex);
            success = true;

  #ifndef BUILD_NO_DEBUG
            tmpString += F("<br/> command= ");
            tmpString += command;
            tmpString += F("<br/> arguments= ");
            tmpString += arguments;
            tmpString += F("<br/> argIndex= ");
            tmpString += String(argIndex);
            tmpString += F("<br/> subcommand= ");
            tmpString += subcommand;
  #endif

            if (command.equalsIgnoreCase(F("EPDCMD")))
            {
              if(subcommand.equalsIgnoreCase(F("CLEAR")))
              {
                arguments = arguments.substring(argIndex + 1);
                P096_data->eInkScreen.clearBuffer();
                P096_data->eInkScreen.fillScreen(P096_data->ParseColor(arguments));
                P096_data->eInkScreen.display();
                P096_data->eInkScreen.clearBuffer();
              } 
              else if(subcommand.equalsIgnoreCase(F("DEEPSLEEP")))
              {
                P096_data->eInkScreen.deepSleep();
              }        
              else if(subcommand.equalsIgnoreCase(F("SEQ_START")))
              {
                P096_data->eInkScreen.clearBuffer();
                P096_data->eInkScreen.fillScreen(P096_data->ParseColor(arguments));
                P096_data->plugin_096_sequence_in_progress = true;
              } 
              else if(subcommand.equalsIgnoreCase(F("SEQ_END")))
              {
  #ifndef BUILD_NO_DEBUG
                TimingStats s;
                const unsigned statisticsTimerStart(micros());
  #endif
                P096_data->eInkScreen.display();
                
  #ifndef BUILD_NO_DEBUG
                s.add(usecPassedSince(statisticsTimerStart));
                tmpString += F("<br/> Display timings = ");
                tmpString += toString(s.getAvg());
  #endif              
                P096_data->eInkScreen.clearBuffer();
                P096_data->plugin_096_sequence_in_progress = false;
              } 
              else if(subcommand.equalsIgnoreCase(F("INV")))
              {
                arguments = arguments.substring(argIndex + 1);
                P096_data->eInkScreen.invertDisplay(arguments.toInt() == 1);
                P096_data->eInkScreen.display();
              } 
              else if(subcommand.equalsIgnoreCase(F("ROT")))
              {
                arguments = arguments.substring(argIndex + 1);
                P096_data->eInkScreen.setRotation(arguments.toInt() % 4);
                P096_data->eInkScreen.display();
              } 
              else 
              {
                success = false;
              }
            }
            else if (command.equalsIgnoreCase(F("EPD")))
            {
  #ifndef BUILD_NO_DEBUG
              tmpString += F("<br/> EPD  ");
  #endif
              arguments = arguments.substring(argIndex + 1);
              String sParams[8];
              int argCount = P096_data->StringSplit(arguments, ',', sParams, 8);

  #ifndef BUILD_NO_DEBUG
              for(int a=0; a < argCount && a < 8; a++)
              {
                  tmpString += F("<br/> ARGS[");
                  tmpString += a;
                  tmpString += F("]=");
                  tmpString += sParams[a];
              }
  #endif

              if(P096_data->plugin_096_sequence_in_progress == false)
              {
                P096_data->eInkScreen.clearBuffer();
                P096_data->eInkScreen.fillScreen(EPD_WHITE);
              }

              if(subcommand.equalsIgnoreCase(F("txt")))
              {
                P096_data->FixText(arguments);
                P096_data->eInkScreen.println(arguments); //write all pending cars
              }
              else if(subcommand.equalsIgnoreCase(F("txz")))
              {
                P096_data->eInkScreen.setCursor(sParams[0].toInt(), sParams[1].toInt());
                P096_data->FixText(sParams[2]);
                P096_data->eInkScreen.println(sParams[2]); //write all pending cars
              }
              else if(subcommand.equalsIgnoreCase(F("txp")) && argCount == 2)
              {
                P096_data->eInkScreen.setCursor(sParams[0].toInt(), sParams[1].toInt());
              }
              else if(subcommand.equalsIgnoreCase(F("txc")) && (argCount == 1 || argCount == 2) )
              {
                if(argCount == 1)
                  P096_data->eInkScreen.setTextColor(P096_data->ParseColor(sParams[0]));
                else //argCount=2
                  P096_data->eInkScreen.setTextColor(P096_data->ParseColor(sParams[0]), P096_data->ParseColor(sParams[1]));
              }
              else if(subcommand.equalsIgnoreCase(F("txs")) && argCount == 1)
              {
                P096_data->eInkScreen.setTextSize(sParams[0].toInt());
              }
              else if(subcommand.equalsIgnoreCase(F("txtfull")) && argCount >= 3 && argCount <= 6)
              {
                switch (argCount)
                {
                case 3: //single text
                  P096_data->printText(sParams[2].c_str(), sParams[0].toInt() - 1,sParams[1].toInt() - 1);  
                  break;

                case 4: //text + size
                  P096_data->printText(sParams[3].c_str(), sParams[0].toInt() - 1, sParams[1].toInt() - 1, sParams[2].toInt());  
                  break;

                case 5: //text + size + color
                  P096_data->printText(sParams[4].c_str(), sParams[0].toInt() - 1, sParams[1].toInt() - 1, sParams[2].toInt(), P096_data->ParseColor(sParams[3]));  
                  break;
                
                case 6: //text + size + color
                  P096_data->printText(sParams[5].c_str(), sParams[0].toInt() - 1, sParams[1].toInt() - 1, sParams[2].toInt(), P096_data->ParseColor(sParams[3]), P096_data->ParseColor(sParams[4]));  
                  break;
                default:
                  success = false;
                  break;
                }            
              }
              else if(subcommand.equalsIgnoreCase(F("l")) && argCount == 5)
              {
                P096_data->eInkScreen.drawLine(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), P096_data->ParseColor(sParams[4]));
              }          
              else if(subcommand.equalsIgnoreCase(F("lh")) && argCount == 3)
              {
                P096_data->eInkScreen.drawFastHLine(0, sParams[0].toInt(), sParams[1].toInt(), P096_data->ParseColor(sParams[2]));
              }          
              else if(subcommand.equalsIgnoreCase(F("lv")) && argCount == 3)
              {
                P096_data->eInkScreen.drawFastVLine(sParams[0].toInt(), 0, sParams[1].toInt(), P096_data->ParseColor(sParams[2]));
              }          
              else if(subcommand.equalsIgnoreCase(F("r")) && argCount == 5)
              {
                P096_data->eInkScreen.drawRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), P096_data->ParseColor(sParams[4]));
              }          
              else if(subcommand.equalsIgnoreCase(F("rf")) && argCount == 6)
              {
                P096_data->eInkScreen.fillRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), P096_data->ParseColor(sParams[5]));
                P096_data->eInkScreen.drawRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), P096_data->ParseColor(sParams[4]));
              }          
              else if(subcommand.equalsIgnoreCase(F("c")) && argCount == 4)
              {
                P096_data->eInkScreen.drawCircle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), P096_data->ParseColor(sParams[3]));
              }          
              else if(subcommand.equalsIgnoreCase(F("cf")) && argCount == 5)
              {
                P096_data->eInkScreen.fillCircle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), P096_data->ParseColor(sParams[4]));
                P096_data->eInkScreen.drawCircle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), P096_data->ParseColor(sParams[3]));
              }
              else if(subcommand.equalsIgnoreCase(F("t")) && argCount == 7)
              {
                P096_data->eInkScreen.drawTriangle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), sParams[5].toInt(), P096_data->ParseColor(sParams[6]));
              }           
              else if(subcommand.equalsIgnoreCase(F("tf")) && argCount == 8)
              {
                P096_data->eInkScreen.fillTriangle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), sParams[5].toInt(), P096_data->ParseColor(sParams[7]));
                P096_data->eInkScreen.drawTriangle(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), sParams[5].toInt(), P096_data->ParseColor(sParams[6]));
              }           
              else if(subcommand.equalsIgnoreCase(F("rr")) && argCount == 6)
              {
                P096_data->eInkScreen.drawRoundRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), P096_data->ParseColor(sParams[5]));
              }          
              else if(subcommand.equalsIgnoreCase(F("rrf")) && argCount == 7)
              {
                P096_data->eInkScreen.fillRoundRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), P096_data->ParseColor(sParams[6]));
                P096_data->eInkScreen.drawRoundRect(sParams[0].toInt(), sParams[1].toInt(), sParams[2].toInt(), sParams[3].toInt(), sParams[4].toInt(), P096_data->ParseColor(sParams[5]));
              } 
              else if(subcommand.equalsIgnoreCase(F("px")) && argCount == 3)
              {
                P096_data->eInkScreen.drawPixel(sParams[0].toInt(), sParams[1].toInt(), P096_data->ParseColor(sParams[2]));
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
          if(success && !P096_data->plugin_096_sequence_in_progress)
          {
            P096_data->eInkScreen.display();
          }
        }
#ifndef BUILD_NO_DEBUG
        String log;
        if (log.reserve(20 + tmpString.length())) { // Prevent re-allocation
          log = F("P096-eInk : WRITE = ");
          log += tmpString;
          SendStatus(event, log);             // Reply (echo) to sender. This will print message on browser.  
        } else {
          SendStatus(event, F("P096-eInk : WRITE = "));
        }
#endif
        break;        
      }
  }

  return success;
}




#endif // USES_P096