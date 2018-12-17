#ifdef USES_P073
//#######################################################################################################
//###################   Plugin 073 - 7-segment display plugin TM1637/MAX7219       ######################
//#######################################################################################################
//
// Chips/displays supported:
//  0 - TM1637     -- 2 pins - 4 digits and colon in the middle (XX:XX)
//  1 - TM1637     -- 2 pins - 4 digits and dot on each digit (X.X.X.X.)
//  2 - TM1637     -- 2 pins - 6 digits and dot on each digit (X.X.X.X.X.X.)
//  3 - MAX7219/21 -- 3 pins - 8 digits and dot on each digit (X.X.X.X.X.X.X.X.)
//
// Plugin can be setup as:
//  - Manual        -- display is manually updated sending commands
//                     "7dn,<number>"        (number can be negative or positive, even with decimal)
//                     "7dt,<temperature>"   (temperature can be negative or positive and containing decimals)
//                     "7dst,<hh>,<mm>,<ss>" (show manual time -not current-, no checks done on numbers validity!)
//                     "7dsd,<dd>,<mm>,<yy>" (show manual date -not current-, no checks done on numbers validity!)
//                     "7dtext,<text>"       (show free text - supported chars 0-9,a-z,A-Z," ","-","=","_","/","^")
//  - Clock-Blink     -- display is automatically updated with current time and blinking dot/lines
//  - Clock-NoBlink   -- display is automatically updated with current time and steady dot/lines
//  - Clock12-Blink   -- display is automatically updated with current time (12h clock) and blinking dot/lines
//  - Clock12-NoBlink -- display is automatically updated with current time (12h clock) and steady dot/lines
//  - Date            -- display is automatically updated with current date
//
// Generic commands:
//  - "7don"      -- turn ON the display
//  - "7doff"     -- turn OFF the display
//  - "7db,<0-15> -- set brightness to specific value between 0 and 15
//


#define PLUGIN_073
#define PLUGIN_ID_073        73
#define PLUGIN_NAME_073      "Display - 7-segment display"
#define PLUGIN_073_DEBUG     false    //activate extra log info in the debug

#define P073_TM1637_4DGTCOLON   0
#define P073_TM1637_4DGTDOTS    1
#define P073_TM1637_6DGT        2
#define P073_MAX7219_8DGT       3

#define P073_DISP_MANUAL        0
#define P073_DISP_CLOCK24BLNK   1
#define P073_DISP_CLOCK24       2
#define P073_DISP_CLOCK12BLNK   3
#define P073_DISP_CLOCK12       4
#define P073_DISP_DATE          5

//---------------------------------------------------
// Class used by plugin
//---------------------------------------------------
class p073_7dgt
{
  public:
    uint8_t pin1, pin2, pin3;
    byte type;
    byte output;
    byte brightness;
    boolean timesep;
};
p073_7dgt *Plugin_073_7dgt = NULL;
//---------------------------------------------------

uint8_t p073_showbuffer[8];
byte    p073_spidata[2];
int     p073_dotpos;
bool    p073_shift;

#define TM1637_POWER_ON   B10001000
#define TM1637_POWER_OFF  B10000000
#define TM1637_CLOCKDELAY 40
#define TM1637_4DIGIT     4
#define TM1637_6DIGIT     2

// each char table is specific for each display and maps all numbers/symbols needed:
//   - pos 0-9   - Numbers from 0 to 9
//   - pos 10    - Space " "
//   - pos 11    - minus symbol "-"
//   - pos 12    - degree symbol "°"
//   - pos 13    - equal "="
//   - pos 14    - triple lines "/"
//   - pos 15    - underscore "_"
//   - pos 16-41 - Letters from A to Z
const byte CharTableTM1637  [42] = {B00111111,B00000110,B01011011,B01001111,B01100110,B01101101,B01111101,B00000111,B01111111,B01101111,
                                    B00000000,B01000000,B01100011,B01001000,B01001001,B00001000,
                                    B01110111,B01111100,B00111001,B01011110,B01111001,B01110001,B00111101,B01110110,B00110000,B00011110,
                                    B01110101,B00111000,B00010101,B00110111,B00111111,B01110011,B01101011,B00110011,B01101101,B01111000,
                                    B00111110,B00111110,B00101010,B01110110,B01101110,B01011011};
const byte CharTableMAX7219 [42] = {B01111110,B00110000,B01101101,B01111001,B00110011,B01011011,B01011111,B01110000,B01111111,B01111011,
                                    B00000000,B00000001,B01100011,B00001001,B01001001,B00001000,
                                    B01110111,B00011111,B01001110,B00111101,B01001111,B01000111,B01011110,B00110111,B00000110,B00111100,
                                    B01010111,B00001110,B01010100,B01110110,B01111110,B01100111,B01101011,B01100110,B01011011,B00001111,
                                    B00111110,B00111110,B00101010,B00110111,B00111011,B01101101};

boolean Plugin_073(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_073;
        Device[deviceCount].Type = DEVICE_TYPE_TRIPLE;
        Device[deviceCount].VType = SENSOR_TYPE_NONE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 0;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].TimerOptional = false;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_073);
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormNote(F("TM1637:  1st=CLK-Pin, 2nd=DIO-Pin"));
        addFormNote(F("MAX7219: 1st=DIN-Pin, 2nd=CLK-Pin, 3rd=CS-Pin"));
        String displtype[5] = { F("TM1637 - 4 digit (colon)"), F("TM1637 - 4 digit (dots)"), F("TM1637 - 6 digit"), F("MAX7219 - 8 digit")};
        addFormSelector(F("Display Type"), F("plugin_073_displtype"), 4, displtype, NULL, Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
        String displout[6] = { F("Manual"), F("Clock 24h - Blink"), F("Clock 24h - No Blink"), F("Clock 12h - Blink"), F("Clock 12h - No Blink"), F("Date")  };
        addFormSelector(F("Display Output"), F("plugin_073_displout"), 6, displout, NULL, Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
        addFormNumericBox(F("Brightness"), F("plugin_073_brightness"), Settings.TaskDevicePluginConfig[event->TaskIndex][2], 0, 15);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_073_displtype"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_073_displout"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("plugin_073_brightness"));
        if (Plugin_073_7dgt) {
          Plugin_073_7dgt->pin1 = Settings.TaskDevicePin1[event->TaskIndex];
          Plugin_073_7dgt->pin2 = Settings.TaskDevicePin2[event->TaskIndex];
          Plugin_073_7dgt->pin3 = Settings.TaskDevicePin3[event->TaskIndex];
          Plugin_073_7dgt->type = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
          Plugin_073_7dgt->output = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
          Plugin_073_7dgt->brightness = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
          Plugin_073_7dgt->timesep = true;
          switch (Settings.TaskDevicePluginConfig[event->TaskIndex][0])
          {
            case P073_TM1637_4DGTCOLON:    // set brightness of TM1637
            case P073_TM1637_4DGTDOTS:
            case P073_TM1637_6DGT:
              {
                int tm1637_bright = Settings.TaskDevicePluginConfig[event->TaskIndex][2] / 2;
                tm1637_SetPowerBrightness(Settings.TaskDevicePin1[event->TaskIndex], Settings.TaskDevicePin2[event->TaskIndex], tm1637_bright, true);
                if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] == P073_DISP_MANUAL)
                  tm1637_ClearDisplay(Settings.TaskDevicePin1[event->TaskIndex], Settings.TaskDevicePin2[event->TaskIndex]);
                break;
              }
            case P073_MAX7219_8DGT:        // set brightness of MAX7219
              {
                max7219_SetPowerBrightness(Settings.TaskDevicePin1[event->TaskIndex], Settings.TaskDevicePin2[event->TaskIndex], Settings.TaskDevicePin3[event->TaskIndex], Settings.TaskDevicePluginConfig[event->TaskIndex][2], true);
                if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] == P073_DISP_MANUAL)
                  max7219_ClearDisplay(Settings.TaskDevicePin1[event->TaskIndex], Settings.TaskDevicePin2[event->TaskIndex], Settings.TaskDevicePin3[event->TaskIndex]);
                break;
              }
          }
        }
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (!Plugin_073_7dgt) {
          Plugin_073_7dgt = new p073_7dgt;
            Plugin_073_7dgt->pin1 = Settings.TaskDevicePin1[event->TaskIndex];
            Plugin_073_7dgt->pin2 = Settings.TaskDevicePin2[event->TaskIndex];
            Plugin_073_7dgt->pin3 = Settings.TaskDevicePin3[event->TaskIndex];
            Plugin_073_7dgt->type = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
            Plugin_073_7dgt->output = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
            Plugin_073_7dgt->brightness = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
          switch (Settings.TaskDevicePluginConfig[event->TaskIndex][0])
          {
            case P073_TM1637_4DGTCOLON:
            case P073_TM1637_4DGTDOTS:
            case P073_TM1637_6DGT:
              {
                tm1637_InitDisplay(Settings.TaskDevicePin1[event->TaskIndex], Settings.TaskDevicePin2[event->TaskIndex]);
                int tm1637_bright = Settings.TaskDevicePluginConfig[event->TaskIndex][2] / 2;
                tm1637_SetPowerBrightness(Settings.TaskDevicePin1[event->TaskIndex], Settings.TaskDevicePin2[event->TaskIndex], tm1637_bright, true);
                break;
              }
            case P073_MAX7219_8DGT:
              {
                max7219_InitDisplay(Settings.TaskDevicePin1[event->TaskIndex], Settings.TaskDevicePin2[event->TaskIndex], Settings.TaskDevicePin3[event->TaskIndex]);
                max7219_SetPowerBrightness(Settings.TaskDevicePin1[event->TaskIndex], Settings.TaskDevicePin2[event->TaskIndex], Settings.TaskDevicePin3[event->TaskIndex], Settings.TaskDevicePluginConfig[event->TaskIndex][2], true);
                break;
              }
          }
        }
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        if (!Plugin_073_7dgt)
          break;

        String tmpString  = string;
        int argIndex = tmpString.indexOf(',');
        if (argIndex)
          tmpString = tmpString.substring(0, argIndex);

        String tmpStr = string;
        int comma1 = tmpStr.indexOf(',');

//----------------------------------------------------------------------------------------------------------------------
        if (tmpString.equalsIgnoreCase(F("7dn"))) {
          if (Plugin_073_7dgt->output != P073_DISP_MANUAL)
            break;
          String log = F("7DGT : Show Number=");
          log += event->Par1;
          addLog(LOG_LEVEL_INFO, log);
          switch (Plugin_073_7dgt->type)
          {
            case P073_TM1637_4DGTCOLON:
            {
              if (event->Par1 > -1000 && event->Par1 < 10000)
                p073_FillBufferWithNumber(String(int(event->Par1)));
              else
                p073_FillBufferWithDash();
              tm1637_ShowBuffer(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, TM1637_4DIGIT, 8);
              break;
            }
            case P073_TM1637_4DGTDOTS:
            {
              if (event->Par1 > -1000 && event->Par1 < 10000)
                p073_FillBufferWithNumber(tmpStr.substring(comma1+1).c_str());
              else
                p073_FillBufferWithDash();
              tm1637_ShowBuffer(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, TM1637_4DIGIT, 8);
              break;
            }
            case P073_TM1637_6DGT:
            {
              if (event->Par1 > -100000 && event->Par1 < 1000000)
                p073_FillBufferWithNumber(tmpStr.substring(comma1+1).c_str());
              else
                p073_FillBufferWithDash();
              tm1637_SwapDigitInBuffer(2);     // only needed for 6-digits displays
              tm1637_ShowBuffer(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, TM1637_6DIGIT, 8);
              break;
            }
            case P073_MAX7219_8DGT:
            {
              if (comma1 > 0) {
                if (event->Par1 > -10000000 && event->Par1 < 100000000) {
                  p073_FillBufferWithNumber(tmpStr.substring(comma1+1).c_str());
                }
                else
                  p073_FillBufferWithDash();
                max7219_ShowBuffer(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, Plugin_073_7dgt->pin3);
              }
              break;
            }
          }
          success = true;
//----------------------------------------------------------------------------------------------------------------------
        } else if (tmpString.equalsIgnoreCase(F("7dt"))) {
          if (Plugin_073_7dgt->output != P073_DISP_MANUAL)
            break;
          double p073_temptemp = 0;
          bool p073_tempflagdot = false;
          if (comma1 > 0)
            p073_temptemp  = atof(tmpStr.substring(comma1+1).c_str());
          String log = F("7DGT : Show Temperature=");
          log += p073_temptemp;
          addLog(LOG_LEVEL_INFO, log);
          switch (Plugin_073_7dgt->type)
          {
            case P073_TM1637_4DGTCOLON:
            case P073_TM1637_4DGTDOTS:
            {
              if (p073_temptemp > 999 || p073_temptemp < -99.9)
                p073_FillBufferWithDash();
              else {
                if (p073_temptemp < 100 && p073_temptemp > -10) {
                  p073_temptemp = int(p073_temptemp*10);
                  p073_tempflagdot = true;
                }
                p073_FillBufferWithTemp(p073_temptemp);
                if (p073_temptemp == 0 && p073_tempflagdot)
                  p073_showbuffer[5] = 0;
              }
              tm1637_ShowTimeTemp4(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, p073_tempflagdot, 4);
              break;
            }
            case P073_TM1637_6DGT:
            {
              if (p073_temptemp > 999 || p073_temptemp < -99.9)
                p073_FillBufferWithDash();
              else {
                if (p073_temptemp < 100 && p073_temptemp > -10) {
                  p073_temptemp = int(p073_temptemp*10);
                  p073_tempflagdot = true;
                }
                p073_FillBufferWithTemp(p073_temptemp);
                if (p073_temptemp == 0 && p073_tempflagdot)
                  p073_showbuffer[5] = 0;
              }
              tm1637_ShowTemp6(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, p073_tempflagdot);
              break;
            }
            case P073_MAX7219_8DGT:
            {
              p073_temptemp = int(p073_temptemp*10);
              p073_FillBufferWithTemp(p073_temptemp);
              if (p073_temptemp == 0)
                p073_showbuffer[5] = 0;
              max7219_ShowTemp(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, Plugin_073_7dgt->pin3);
              break;
            }
          }
          success = true;
//----------------------------------------------------------------------------------------------------------------------
        } else if (tmpString.equalsIgnoreCase(F("7dst"))) {
          if (Plugin_073_7dgt->output != P073_DISP_MANUAL)
            break;
          String log = F("7DGT : Show Time=");
          log += event->Par1; log += ":";
          log += event->Par2; log += ":";
          log += event->Par3;
          addLog(LOG_LEVEL_INFO, log);
          Plugin_073_7dgt->timesep = true;
          p073_FillBufferWithTime(false, event->Par1, event->Par2, event->Par3, false);
          switch (Plugin_073_7dgt->type)
          {
            case P073_TM1637_4DGTCOLON:
            case P073_TM1637_4DGTDOTS:
            {
              tm1637_ShowTimeTemp4(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, Plugin_073_7dgt->timesep, 0);
              break;
            }
            case P073_TM1637_6DGT:
            {
              tm1637_ShowTime6(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, Plugin_073_7dgt->timesep);
              break;
            }
            case P073_MAX7219_8DGT:
            {
              max7219_ShowTime(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, Plugin_073_7dgt->pin3, Plugin_073_7dgt->timesep);
              break;
            }
          };
          success = true;
//----------------------------------------------------------------------------------------------------------------------
        } else if (tmpString.equalsIgnoreCase(F("7dsd"))) {
          if (Plugin_073_7dgt->output != P073_DISP_MANUAL)
            break;
          String log = F("7DGT : Show Date=");
          log += event->Par1; log += "-";
          log += event->Par2; log += "-";
          log += event->Par3;
          addLog(LOG_LEVEL_INFO, log);
          p073_FillBufferWithDate(false, event->Par1, event->Par2, event->Par3);
          switch (Plugin_073_7dgt->type)
          {
            case P073_TM1637_4DGTCOLON:
            case P073_TM1637_4DGTDOTS:
            {
              tm1637_ShowTimeTemp4(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, Plugin_073_7dgt->timesep, 0);
              break;
            }
            case P073_TM1637_6DGT:
            {
              tm1637_ShowDate6(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, Plugin_073_7dgt->timesep);
              break;
            }
            case P073_MAX7219_8DGT:
            {
              max7219_ShowDate(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, Plugin_073_7dgt->pin3);
              break;
            }
          }
          success = true;
//----------------------------------------------------------------------------------------------------------------------
        } else if (tmpString.equalsIgnoreCase(F("7dtext"))) {
          if (Plugin_073_7dgt->output != P073_DISP_MANUAL)
            break;
          String tmpString = string;
          int argIndex = tmpString.indexOf(',');
          if (argIndex)
            tmpString = tmpString.substring(argIndex+1);
          else
            tmpString = "";
          String log = F("7DGT : Show Text=");
          log += tmpString;
          addLog(LOG_LEVEL_INFO, log);
          p073_FillBufferWithString(tmpString);
          switch (Plugin_073_7dgt->type)
          {
            case P073_TM1637_4DGTCOLON:
            case P073_TM1637_4DGTDOTS:
            {
              tm1637_ShowBuffer(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, 0, 4);
              break;
            }
            case P073_TM1637_6DGT:
            {
              tm1637_SwapDigitInBuffer(0);     // only needed for 6-digits displays
              tm1637_ShowBuffer(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, 0, 6);
              break;
            }
            case P073_MAX7219_8DGT:
            {
              p073_dotpos = -1;     // avoid to display the dot
              max7219_ShowBuffer(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, Plugin_073_7dgt->pin3);
              break;
            }
          }
          success = true;
//----------------------------------------------------------------------------------------------------------------------
        } else {
          bool p073_validcmd = false;
          bool p073_displayon;
          if (tmpString.equalsIgnoreCase(F("7don"))) {
            String log = F("7DGT : Display ON");
            addLog(LOG_LEVEL_INFO, log);
            p073_displayon = true;
            p073_validcmd = true;
          }
          else if (tmpString.equalsIgnoreCase(F("7doff"))) {
            String log = F("7DGT : Display OFF");
            addLog(LOG_LEVEL_INFO, log);
            p073_displayon = false;
            p073_validcmd = true;
          }
          else if (tmpString.equalsIgnoreCase(F("7db"))) {
            if (event->Par1 >= 0 && event->Par1 < 16) {
              String log = F("7DGT : Brightness=");
              log += event->Par1;
              addLog(LOG_LEVEL_INFO, log);
              Plugin_073_7dgt->brightness = event->Par1;
              p073_displayon = true;
              p073_validcmd = true;
            }
          }
          if (p073_validcmd) {
            success = true;
            switch (Plugin_073_7dgt->type)
            {
              case P073_TM1637_4DGTCOLON:
              case P073_TM1637_4DGTDOTS:
              case P073_TM1637_6DGT:
              { int tm1637_bright = Plugin_073_7dgt->brightness / 2;
                tm1637_SetPowerBrightness(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, tm1637_bright, p073_displayon);
                break; }
              case P073_MAX7219_8DGT:
              { max7219_SetPowerBrightness(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, Plugin_073_7dgt->pin3, Plugin_073_7dgt->brightness, p073_displayon);
                break; }
            }
          }
        }
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        if (Plugin_073_7dgt->output == P073_DISP_MANUAL)
          break;

        if (Plugin_073_7dgt->output == P073_DISP_CLOCK24BLNK or Plugin_073_7dgt->output == P073_DISP_CLOCK12BLNK)
          { Plugin_073_7dgt->timesep = !Plugin_073_7dgt->timesep; }
        else
          { Plugin_073_7dgt->timesep = true; }

        if (Plugin_073_7dgt->output == P073_DISP_DATE)
          p073_FillBufferWithDate(true,0,0,0);
        else if (Plugin_073_7dgt->output == P073_DISP_CLOCK24BLNK or Plugin_073_7dgt->output == P073_DISP_CLOCK24)
          p073_FillBufferWithTime(true,0,0,0, false);
        else
          p073_FillBufferWithTime(true,0,0,0, true);

        switch (Plugin_073_7dgt->type)
        {
          case P073_TM1637_4DGTCOLON:
          case P073_TM1637_4DGTDOTS:
          {
            tm1637_ShowTimeTemp4(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, Plugin_073_7dgt->timesep, 0);
            break;
          }
          case P073_TM1637_6DGT:
          {
            if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] == P073_DISP_DATE)
              tm1637_ShowDate6(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, Plugin_073_7dgt->timesep);
            else
              tm1637_ShowTime6(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, Plugin_073_7dgt->timesep);
            break;
          }
          case P073_MAX7219_8DGT:
          {
            if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] == P073_DISP_DATE)
              max7219_ShowDate(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, Plugin_073_7dgt->pin3);
            else
              max7219_ShowTime(Plugin_073_7dgt->pin1, Plugin_073_7dgt->pin2, Plugin_073_7dgt->pin3, Plugin_073_7dgt->timesep);
            break;
          }
        }
      }

  }
  return success;
}

void p073_FillBufferWithTime(boolean sevendgt_now, byte sevendgt_hours, byte sevendgt_minutes, byte sevendgt_seconds, boolean flag12h)
{
  memset(p073_showbuffer,0,sizeof(p073_showbuffer));
  if (sevendgt_now) {
    sevendgt_hours = hour();
    sevendgt_minutes = minute();
    sevendgt_seconds = second();
  }
  if (flag12h and sevendgt_hours > 12) sevendgt_hours -= 12;  // if flag 12h is TRUE and h>12 adjust subtracting 12
  if (flag12h and sevendgt_hours == 0) sevendgt_hours =  12;  // if flag 12h is TRUE and h=0  adjust to h=12

  uint8_t p073_digit1, p073_digit2;
  p073_digit1 = (uint8_t)(sevendgt_hours / 10);
  p073_digit2 = sevendgt_hours - p073_digit1*10;
  p073_showbuffer[0] = p073_digit1; p073_showbuffer[1] = p073_digit2;
  p073_digit1 = (uint8_t)(sevendgt_minutes / 10);
  p073_digit2 = sevendgt_minutes - p073_digit1*10;
  p073_showbuffer[2] = p073_digit1; p073_showbuffer[3] = p073_digit2;
  p073_digit1 = (uint8_t)(sevendgt_seconds / 10);
  p073_digit2 = sevendgt_seconds - p073_digit1*10;
  p073_showbuffer[4] = p073_digit1; p073_showbuffer[5] = p073_digit2;
}

void p073_FillBufferWithDate(boolean sevendgt_now, byte sevendgt_day, byte sevendgt_month, int sevendgt_year)
{
  memset(p073_showbuffer,0,sizeof(p073_showbuffer));
  int  sevendgt_year0 = sevendgt_year;
  byte sevendgt_year1 = 0;
  byte sevendgt_year2 = 0;
  if (sevendgt_now) {
    sevendgt_day = day();
    sevendgt_month = month();
    sevendgt_year1 = uint8_t(year()/100);
    sevendgt_year2 = uint8_t(year()-(sevendgt_year1*100));
  } else {
    if (sevendgt_year0 < 100) { sevendgt_year0 += 2000; }
    sevendgt_year1 = uint8_t(sevendgt_year0/100);
    sevendgt_year2 = uint8_t(sevendgt_year0-(sevendgt_year1*100));
  }
  uint8_t p073_digit1, p073_digit2;
  p073_digit1 = (uint8_t)(sevendgt_day / 10);
  p073_digit2 = sevendgt_day - p073_digit1*10;
  p073_showbuffer[0] = p073_digit1; p073_showbuffer[1] = p073_digit2;
  p073_digit1 = (uint8_t)(sevendgt_month / 10);
  p073_digit2 = sevendgt_month - p073_digit1*10;
  p073_showbuffer[2] = p073_digit1; p073_showbuffer[3] = p073_digit2;
  p073_digit1 = (uint8_t)(sevendgt_year1 / 10);
  p073_digit2 = sevendgt_year1 - p073_digit1*10;
  p073_showbuffer[4] = p073_digit1; p073_showbuffer[5] = p073_digit2;
  p073_digit1 = (uint8_t)(sevendgt_year2 / 10);
  p073_digit2 = sevendgt_year2 - p073_digit1*10;
  p073_showbuffer[6] = p073_digit1; p073_showbuffer[7] = p073_digit2;
}

void p073_FillBufferWithNumber(const String& number)
{
  memset(p073_showbuffer,10,sizeof(p073_showbuffer));
  byte p073_numlenght = number.length();
  byte p073_dispdigit = 10;
  byte p073_index = 7;
  p073_dotpos = -1;     // -1 means no dot to display
  for (int i=p073_numlenght;i>0;i--) {
    char p073_tmpchar = number.charAt(i-1);
    p073_dispdigit = 10;           // default is space
    if (p073_tmpchar > 47 && p073_tmpchar < 58)
      p073_dispdigit = p073_tmpchar-48;
    else if (p073_tmpchar == 32)  // space
      p073_dispdigit = 10;
    else if (p073_tmpchar == 45)  // minus
      p073_dispdigit = 11;
    if (p073_tmpchar == 46)  // dot
      p073_dotpos = p073_index;
    else {
      p073_showbuffer[p073_index] = p073_dispdigit;
      p073_index--; }
  }
}

void p073_FillBufferWithTemp(long temperature)
{
  memset(p073_showbuffer,10,sizeof(p073_showbuffer));
  char p073_digit[8];
  sprintf(p073_digit, "%7d", static_cast<int>(temperature));
  int p073_numlenght = strlen(p073_digit);
  byte p073_dispdigit = 10;
  for (int i=0;i<p073_numlenght;i++) {
    p073_dispdigit = 10;           // default is space
    if (p073_digit[i] > 47 && p073_digit[i] < 58)
      p073_dispdigit = p073_digit[i]-48;
    else if (p073_digit[i] == 32)  // space
      p073_dispdigit = 10;
    else if (p073_digit[i] == 45)  // minus
      p073_dispdigit = 11;
    p073_showbuffer[i] = p073_dispdigit;
  }
  p073_showbuffer[7] = 12;  // degree "°"
}

void p073_FillBufferWithString(const String& textToShow)
{
  memset(p073_showbuffer,10,sizeof(p073_showbuffer));
  String tmpText;
  byte p073_dispdigit = 10;
  int p073_txtlength = textToShow.length();
  if (p073_txtlength > 8) p073_txtlength = 8;
  tmpText = textToShow.substring(0, p073_txtlength);
  for (int i=0;i<p073_txtlength;i++) {
    char p073_tmpchar = tmpText.charAt(i);
    p073_dispdigit = 10;           // default is space
    if (p073_tmpchar > 47 && p073_tmpchar < 58)
      p073_dispdigit = p073_tmpchar-48;
    else if (p073_tmpchar > 64 && p073_tmpchar < 91)
      p073_dispdigit = p073_tmpchar-49;
    else if (p073_tmpchar > 96 && p073_tmpchar < 123)
      p073_dispdigit = p073_tmpchar-81;
    else if (p073_tmpchar == 32)  // space
      p073_dispdigit = 10;
    else if (p073_tmpchar == 45)  // minus  "-"
      p073_dispdigit = 11;
    else if (p073_tmpchar == 94)  // degree "^"
      p073_dispdigit = 12;
    else if (p073_tmpchar == 61)  // equal  "="
      p073_dispdigit = 13;
    else if (p073_tmpchar == 47)  // three lines "/"
      p073_dispdigit = 14;
    else if (p073_tmpchar == 95)  // underscore "_"
      p073_dispdigit = 15;
    p073_showbuffer[i] = p073_dispdigit;
  }
}

void p073_FillBufferWithDash()  // in case of error show all dashes
{
  memset(p073_showbuffer,11,sizeof(p073_showbuffer));
}

//===================================
//---- TM1637 specific functions ----
//===================================

#define CLK_HIGH()  digitalWrite(clk_pin, HIGH)
#define CLK_LOW()   digitalWrite(clk_pin, LOW)
#define DIO_HIGH()  pinMode(dio_pin, INPUT)
#define DIO_LOW()   pinMode(dio_pin, OUTPUT)

void tm1637_i2cStart (uint8_t clk_pin, uint8_t dio_pin)
{
  if (PLUGIN_073_DEBUG) {
    String log = F("7DGT : Comm Start");
    addLog(LOG_LEVEL_INFO, log);
  }
  DIO_HIGH();
  CLK_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_LOW();
}

void tm1637_i2cStop (uint8_t clk_pin, uint8_t dio_pin)
{
  if (PLUGIN_073_DEBUG) {
    String log = F("7DGT : Comm Stop");
    addLog(LOG_LEVEL_INFO, log);
  }
  CLK_LOW();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_LOW();
  delayMicroseconds(TM1637_CLOCKDELAY);
  CLK_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_HIGH();
}

void tm1637_i2cAck (uint8_t clk_pin, uint8_t dio_pin)
{
  bool dummyAck = false;
  CLK_LOW();
  pinMode(dio_pin, INPUT_PULLUP);
  //DIO_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  //while(digitalRead(dio_pin));
  dummyAck = digitalRead(dio_pin);
  if (PLUGIN_073_DEBUG) {
    String log = F("7DGT : Comm ACK=");
    if (dummyAck == 0) { log += F("TRUE"); } else { log += F("FALSE"); }
    addLog(LOG_LEVEL_INFO, log);
  }
  CLK_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  CLK_LOW();
  pinMode(dio_pin, OUTPUT);
}

void tm1637_i2cWrite (uint8_t clk_pin, uint8_t dio_pin, uint8_t bytetoprint)
{
  if (PLUGIN_073_DEBUG) {
    String log = F("7DGT : WriteByte");
    addLog(LOG_LEVEL_INFO, log);
  }
  uint8_t i;
  for(i=0; i<8; i++)
  {
    CLK_LOW();
    (bytetoprint & B00000001)? DIO_HIGH() : DIO_LOW();
    delayMicroseconds(TM1637_CLOCKDELAY);
    bytetoprint = bytetoprint >> 1;
    CLK_HIGH();
    delayMicroseconds(TM1637_CLOCKDELAY);
  }
}

void tm1637_ClearDisplay (uint8_t clk_pin, uint8_t dio_pin)
{
  tm1637_i2cStart(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, 0xC0);   tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, 0);      tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, 0);      tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, 0);      tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, 0);      tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, 0);      tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, 0);      tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cStop(clk_pin, dio_pin);
}

void tm1637_SetPowerBrightness (uint8_t clk_pin, uint8_t dio_pin, uint8_t brightlvl, bool poweron)
{
  if (PLUGIN_073_DEBUG) {
    String log = F("7DGT : Set BRIGHT");
    addLog(LOG_LEVEL_INFO, log);
  }
  uint8_t brightvalue = (brightlvl & 0b111);
  if (poweron)
    brightvalue = TM1637_POWER_ON  | brightvalue;
  else
    brightvalue = TM1637_POWER_OFF | brightvalue;
  tm1637_i2cStart(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, brightvalue);
  tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cStop(clk_pin, dio_pin);
}

void tm1637_InitDisplay(uint8_t clk_pin, uint8_t dio_pin)
{
  pinMode(clk_pin, OUTPUT);
  pinMode(dio_pin, OUTPUT);
  CLK_HIGH();
  DIO_HIGH();
//  pinMode(dio_pin, INPUT_PULLUP);
//  pinMode(clk_pin, OUTPUT);
  tm1637_i2cStart(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, 0x40);
  tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cStop(clk_pin, dio_pin);
  tm1637_ClearDisplay(clk_pin, dio_pin);
}

void tm1637_ShowTime6(uint8_t clk_pin, uint8_t dio_pin, bool sep)
{
  byte p073_datashowpos1;
  tm1637_i2cStart(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, 0xC0);                                  tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, CharTableTM1637[p073_showbuffer[2]]);   tm1637_i2cAck(clk_pin, dio_pin);
  // add bit for colon on second digit if required
    p073_datashowpos1 = CharTableTM1637[p073_showbuffer[1]];
    if (sep) p073_datashowpos1 |= 0b10000000;
  tm1637_i2cWrite(clk_pin, dio_pin, p073_datashowpos1);                     tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, CharTableTM1637[p073_showbuffer[0]]);   tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, CharTableTM1637[p073_showbuffer[5]]);   tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, CharTableTM1637[p073_showbuffer[4]]);   tm1637_i2cAck(clk_pin, dio_pin);
  // add bit for colon on fourth digit if required
    p073_datashowpos1 = CharTableTM1637[p073_showbuffer[3]];
    if (sep) p073_datashowpos1 |= 0b10000000;
  tm1637_i2cWrite(clk_pin, dio_pin, p073_datashowpos1);                     tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cStop(clk_pin, dio_pin);
}

void tm1637_ShowDate6(uint8_t clk_pin, uint8_t dio_pin, bool sep)
{
  byte p073_datashowpos1;
  tm1637_i2cStart(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, 0xC0);                                  tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, CharTableTM1637[p073_showbuffer[2]]);   tm1637_i2cAck(clk_pin, dio_pin);
  // add bit for colon on second digit if required
    p073_datashowpos1 = CharTableTM1637[p073_showbuffer[1]];
    if (sep) p073_datashowpos1 |= 0b10000000;
  tm1637_i2cWrite(clk_pin, dio_pin, p073_datashowpos1);                     tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, CharTableTM1637[p073_showbuffer[0]]);   tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, CharTableTM1637[p073_showbuffer[7]]);   tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, CharTableTM1637[p073_showbuffer[6]]);   tm1637_i2cAck(clk_pin, dio_pin);
  // add bit for colon on fourth digit if required
    p073_datashowpos1 = CharTableTM1637[p073_showbuffer[3]];
    if (sep) p073_datashowpos1 |= 0b10000000;
  tm1637_i2cWrite(clk_pin, dio_pin, p073_datashowpos1);                     tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cStop(clk_pin, dio_pin);
}

void tm1637_ShowTemp6(uint8_t clk_pin, uint8_t dio_pin, bool sep)
{
  tm1637_i2cStart(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, 0xC0);                                  tm1637_i2cAck(clk_pin, dio_pin);
  // add bit for colon on second digit if required
    byte p073_datashowpos1 = CharTableTM1637[p073_showbuffer[5]];
    if (sep) p073_datashowpos1 |= 0b10000000;
  tm1637_i2cWrite(clk_pin, dio_pin, p073_datashowpos1);                     tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, CharTableTM1637[p073_showbuffer[4]]);   tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, CharTableTM1637[10]);                   tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, CharTableTM1637[10]);                   tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, CharTableTM1637[p073_showbuffer[7]]);   tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, CharTableTM1637[p073_showbuffer[6]]);   tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cStop(clk_pin, dio_pin);
}

void tm1637_ShowTimeTemp4(uint8_t clk_pin, uint8_t dio_pin, bool sep, byte bufoffset)
{
  tm1637_i2cStart(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, 0xC0);                                            tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, CharTableTM1637[p073_showbuffer[0+bufoffset]]);   tm1637_i2cAck(clk_pin, dio_pin);
  // add bit for colon on second digit if required
    byte p073_datashowpos1 = CharTableTM1637[p073_showbuffer[1+bufoffset]];
    if (sep) p073_datashowpos1 |= 0b10000000;
  tm1637_i2cWrite(clk_pin, dio_pin, p073_datashowpos1);                               tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, CharTableTM1637[p073_showbuffer[2+bufoffset]]);   tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, CharTableTM1637[p073_showbuffer[3+bufoffset]]);   tm1637_i2cAck(clk_pin, dio_pin);
  tm1637_i2cStop(clk_pin, dio_pin);
}

void tm1637_SwapDigitInBuffer(byte startPos) {
  uint8_t p073_temp;
  p073_temp = p073_showbuffer[2+startPos]; p073_showbuffer[2+startPos] = p073_showbuffer[0+startPos]; p073_showbuffer[0+startPos] = p073_temp;
  p073_temp = p073_showbuffer[3+startPos]; p073_showbuffer[3+startPos] = p073_showbuffer[5+startPos]; p073_showbuffer[5+startPos] = p073_temp;
  switch (p073_dotpos)
  {
    case 2: { p073_dotpos = 4; break; }
    case 4: { p073_dotpos = 2; break; }
    case 5: { p073_dotpos = 7; break; }
    case 7: { p073_dotpos = 5; break; }
  }
}

void tm1637_ShowBuffer(uint8_t clk_pin, uint8_t dio_pin, byte firstPos, byte lastPos)
{
  byte p073_datashowpos1;
  tm1637_i2cStart(clk_pin, dio_pin);
  tm1637_i2cWrite(clk_pin, dio_pin, 0xC0);                            tm1637_i2cAck(clk_pin, dio_pin);
    for(int i=firstPos;i<lastPos;i++) {
    p073_datashowpos1 = CharTableTM1637[p073_showbuffer[i]];
    if (p073_dotpos == i) p073_datashowpos1 |= 0b10000000;
    tm1637_i2cWrite(clk_pin, dio_pin, p073_datashowpos1);             tm1637_i2cAck(clk_pin, dio_pin);
  }
  tm1637_i2cStop(clk_pin, dio_pin);
}

//====================================
//---- MAX7219 specific functions ----
//====================================

#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15

void max7219_spiTransfer (uint8_t din_pin, uint8_t clk_pin, uint8_t cs_pin, volatile byte opcode, volatile byte data)
{
  p073_spidata[0]=(byte)0;  p073_spidata[1]=(byte)0;
  p073_spidata[1]=opcode;   p073_spidata[0]=data;
  digitalWrite(cs_pin,LOW);
  shiftOut(din_pin,clk_pin,MSBFIRST,p073_spidata[1]);
  shiftOut(din_pin,clk_pin,MSBFIRST,p073_spidata[0]);
  digitalWrite(cs_pin,HIGH);
}

void max7219_ClearDisplay (uint8_t din_pin, uint8_t clk_pin, uint8_t cs_pin)
{
  for(int i=0;i<8;i++) {
    max7219_spiTransfer(din_pin, clk_pin, cs_pin, i+1, 0);
  }
}

void max7219_SetPowerBrightness (uint8_t din_pin, uint8_t clk_pin, uint8_t cs_pin, uint8_t brightlvl, bool poweron)
{
  max7219_spiTransfer(din_pin, clk_pin, cs_pin, OP_INTENSITY, brightlvl);
  if (poweron)
    max7219_spiTransfer(din_pin, clk_pin, cs_pin, OP_SHUTDOWN, 1);
  else
    max7219_spiTransfer(din_pin, clk_pin, cs_pin, OP_SHUTDOWN, 0);
}

void max7219_SetDigit(uint8_t din_pin, uint8_t clk_pin, uint8_t cs_pin, int dgtpos, byte dgtvalue, boolean showdot)
{
  byte p073_tempvalue;
  p073_tempvalue = CharTableMAX7219[dgtvalue];
  if(showdot)
    p073_tempvalue |= 0b10000000;
  max7219_spiTransfer(din_pin, clk_pin, cs_pin, dgtpos+1, p073_tempvalue);
}

void max7219_InitDisplay(uint8_t din_pin, uint8_t clk_pin, uint8_t cs_pin)
{
  pinMode(din_pin, OUTPUT);
  pinMode(clk_pin, OUTPUT);
  pinMode(cs_pin, OUTPUT);
  digitalWrite(cs_pin, HIGH);
  max7219_spiTransfer(din_pin, clk_pin, cs_pin, OP_DISPLAYTEST, 0);
  max7219_spiTransfer(din_pin, clk_pin, cs_pin, OP_SCANLIMIT, 7);    // scanlimit setup to max at Init
  max7219_spiTransfer(din_pin, clk_pin, cs_pin, OP_DECODEMODE, 0);
  max7219_ClearDisplay(din_pin, clk_pin, cs_pin);
  max7219_SetPowerBrightness(din_pin, clk_pin, cs_pin,0,false);
}

void max7219_ShowTime(uint8_t din_pin, uint8_t clk_pin, uint8_t cs_pin, bool sep)
{
  max7219_SetDigit(din_pin, clk_pin, cs_pin, 0, p073_showbuffer[5], false);
  max7219_SetDigit(din_pin, clk_pin, cs_pin, 1, p073_showbuffer[4], false);
  max7219_SetDigit(din_pin, clk_pin, cs_pin, 3, p073_showbuffer[3], false);
  max7219_SetDigit(din_pin, clk_pin, cs_pin, 4, p073_showbuffer[2], false);
  max7219_SetDigit(din_pin, clk_pin, cs_pin, 6, p073_showbuffer[1], false);
  max7219_SetDigit(din_pin, clk_pin, cs_pin, 7, p073_showbuffer[0], false);
  if (sep) {
    max7219_SetDigit(din_pin, clk_pin, cs_pin, 2, 11, false);
    max7219_SetDigit(din_pin, clk_pin, cs_pin, 5, 11, false);
  }
  else {
    max7219_SetDigit(din_pin, clk_pin, cs_pin, 2, 10, false);
    max7219_SetDigit(din_pin, clk_pin, cs_pin, 5, 10, false);
  }
}

void max7219_ShowTemp(uint8_t din_pin, uint8_t clk_pin, uint8_t cs_pin)
{
  max7219_SetDigit(din_pin, clk_pin, cs_pin, 0, 10, false);
  byte dotflags[8] = {false,false,false,false,false,true,false,false};
  for(int i=1;i<8;i++)
    max7219_SetDigit(din_pin, clk_pin, cs_pin, i, p073_showbuffer[8-i], dotflags[8-i]);
}

void max7219_ShowDate(uint8_t din_pin, uint8_t clk_pin, uint8_t cs_pin)
{
  byte dotflags[8] = {false,true,false,true,false,false,false,false};
  for(int i=0;i<8;i++)
    max7219_SetDigit(din_pin, clk_pin, cs_pin, i, p073_showbuffer[7-i], dotflags[7-i]);
}

void max7219_ShowBuffer(uint8_t din_pin, uint8_t clk_pin, uint8_t cs_pin)
{
  byte dotflags[8] = {false,false,false,false,false,false,false,false};
  if (p073_dotpos >= 0) dotflags[p073_dotpos] = true;
  for(int i=0;i<8;i++)
    max7219_SetDigit(din_pin, clk_pin, cs_pin, i, p073_showbuffer[7-i], dotflags[7-i]);
}

#endif // USES_P073
