#ifdef USES_P057
//#######################################################################################################
//#################################### Plugin 057: HT16K33 LED ##########################################
//#######################################################################################################

// ESPEasy Plugin to control a 16x8 LED matrix or 8 7-segment displays with chip HT16K33
// written by Jochen Krapf (jk@nerd2nerd.org)

// List of commands:
// (1) M,<param>,<param>,<param>, ...    with decimal values
// (2) MX,<param>,<param>,<param>, ...    with hexadecimal values
// (3) MNUM,<param>,<param>,<param>, ...    with decimal values for 7-segment displays
// (4) MPRINT,<text>    with decimal values for 7-segment displays
// (5) MBR,<0-15>    set display brightness, between 0 and 15

// List of M* params:
// (a) <value>
//     Writes a decimal / hexadecimal (0...0xFFFF) values to actual segment starting with 0
// (b) <seg>=<value>
//     Writes a decimal / hexadecimal (0...0xFFFF) values to given segment (0...7)
// (c) "CLEAR"
//     Set all LEDs to 0.
// (d) "TEST"
//     Set test pattern to LED buffer.
// (e) "LOG"
//     Print LED buffer to log output.

// Examples:
// MX,AA,55,AA,55,AA,55,AA,55   Set chess pattern to LED buffer
// MNUM,CLEAR,1,0   Clear the LED buffer and then set 0x06 to 1st segment and 0x3F to 2nd segment

// Connecting LEDs to HT16K33-board:
// Cathode for Column 0 = C0
// Cathode for Column 1 = C1
// ...
// Cathode for Column 7 = C7
//
// Anode for bit 0x0001 = A0
// Anode for bit 0x0002 = A1
// ...
// Anode for bit 0x0080 = A7
// ...
// Anode for bit 0x8000 = A15

// Note: The HT16K33-LED-plugin and the HT16K33-key-plugin can be used at the same time with the same I2C address

// Clock Display:
// This plugin also allows a "clock" mode. In clock mode the display will show
// the current system time. The "7-Seg. Clock" needs to be configured for this
// mode to work. Each segment number (0..5) needs to be set based on your
// display. 
//
// For my _Adafruit 0.56" 4-Digit 7-Segment FeatherWing Display_ these
// settings are as follows:
//    Xx:xx = 0, xX:xx = 1, 
//    xx:Xx = 3, xx:xX = 4
//    Seg. for Colon is 2 with a value of 2
//
// Any other data written to the display will show and be replaced at the next 
// clock cycle, e.g. when the plugin received 'PLUGIN_CLOCK_IN'.
//
// NOTE: The system time is set via NTP as part of the Core ESPEasy firmware.
// There is no configuration here to set or manipulate the time, only to
// display it.

#define PLUGIN_057
#define PLUGIN_ID_057         57
#define PLUGIN_NAME_057       "Display - HT16K33 [TESTING]"

#include <HT16K33.h>

CHT16K33* Plugin_057_M = NULL;

#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif


boolean Plugin_057(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_057;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = SENSOR_TYPE_NONE;
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
        string = F(PLUGIN_NAME_057);
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte addr = CONFIG(0);

        int optionValues[8] = { 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77 };
        addFormSelectorI2C(F("i2c_addr"), 8, optionValues, addr);


        addFormSubHeader(F("7-Seg. Clock"));

        int16_t choice = CONFIG(1);
        String options[3] = {F("none"), F("7-Seg. HH:MM (24 hour)"), F("7-Seg. HH:MM (12 hour)")};
        addFormSelector(F("Clock Type"), F("clocktype"), 3, options, NULL, choice);

        addFormNumericBox(F("Seg. for <b>X</b>x:xx"), F("clocksegh10"), CONFIG(2), 0, 7);
        addFormNumericBox(F("Seg. for x<b>X</b>:xx"), F("clocksegh1"), CONFIG(3), 0, 7);
        addFormNumericBox(F("Seg. for xx:<b>X</b>x"), F("clocksegm10"), CONFIG(4), 0, 7);
        addFormNumericBox(F("Seg. for xx:x<b>X</b>"), F("clocksegm1"), CONFIG(5), 0, 7);

        addFormNumericBox(F("Seg. for Colon"), F("clocksegcol"), CONFIG(6), -1, 7);
        addHtml(F(" Value "));
        addNumericBox(F("clocksegcolval"), CONFIG(7), 0, 255);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        CONFIG(0) = getFormItemInt(F("i2c_addr"));

        CONFIG(1) = getFormItemInt(F("clocktype"));

        CONFIG(2) = getFormItemInt(F("clocksegh10"));
        CONFIG(3) = getFormItemInt(F("clocksegh1"));
        CONFIG(4) = getFormItemInt(F("clocksegm10"));
        CONFIG(5) = getFormItemInt(F("clocksegm1"));
        CONFIG(6) = getFormItemInt(F("clocksegcol"));
        CONFIG(7) = getFormItemInt(F("clocksegcolval"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        byte addr = CONFIG(0);

        if (!Plugin_057_M)
          Plugin_057_M = new CHT16K33;

        Plugin_057_M->Init(addr);

        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        if (!Plugin_057_M)
          return false;

        String command = parseString(string, 1);

        if (command == F("mprint"))
        {
          String text = parseStringToEnd(string, 2);
          if (text.length() > 0) {
            byte seg = 0;

            Plugin_057_M->ClearRowBuffer();
            while (text[seg] && seg < 8)
            {
              // uint16_t value = 0;
              char c = text[seg];
              Plugin_057_M->SetDigit(seg, c);
              seg++;
            }
            Plugin_057_M->TransmitRowBuffer();
            success = true;
          }
        }
        else if (command == F("mbr")) {
          String param = parseString(string, 2);
          int brightness;
          if (validIntFromString(param, brightness)) {
            if (brightness >= 0 && brightness <= 255)
              Plugin_057_M->SetBrightness(brightness);
          }
          success = true;
        }
        else if (command == F("m") || command == F("mx") || command == F("mnum"))
        {
          String param;
          String paramKey;
          String paramVal;
          byte paramIdx = 2;
          uint8_t seg = 0;
          uint16_t value = 0;

          String lowerString=string;
          lowerString.toLowerCase();
          lowerString.replace(F("  "), " ");
          lowerString.replace(F(" ="), "=");
          lowerString.replace(F("= "), "=");

          param = parseString(lowerString, paramIdx++);
          if (param.length())
          {
            while (param.length())
            {
              addLog(LOG_LEVEL_DEBUG_MORE, param);

              if (param == F("log"))
              {
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  String log = F("MX   : ");
                  for (byte i = 0; i < 8; i++)
                  {
                    log += String(Plugin_057_M->GetRow(i), 16);
                    log += F("h, ");
                  }
                  addLog(LOG_LEVEL_INFO, log);
                }
                success = true;
              }

              else if (param == F("test"))
              {
                for (byte i = 0; i < 8; i++)
                  Plugin_057_M->SetRow(i, 1 << i);
                success = true;
              }

              else if (param == F("clear"))
              {
                Plugin_057_M->ClearRowBuffer();
                success = true;
              }

              else
              {
                int index = param.indexOf('=');
                if (index > 0)   //syntax: "<seg>=<value>"
                {
                  paramKey = param.substring(0, index);
                  paramVal = param.substring(index+1);
                  seg = paramKey.toInt();
                }
                else   //syntax: "<value>"
                {
                  paramVal = param;
                }

                if (command == F("mnum"))
                {
                  value = paramVal.toInt();
                  if (value < 16)
                    Plugin_057_M->SetDigit(seg, value);
                  else
                    Plugin_057_M->SetRow(seg, value);
                }
                else if (command == F("mx"))
                {
                  char* ep;
                  value = strtol(paramVal.c_str(), &ep, 16);
                  Plugin_057_M->SetRow(seg, value);
                }
                else
                {
                  value = paramVal.toInt();
                  Plugin_057_M->SetRow(seg, value);
                }

                success = true;
                seg++;
              }

              param = parseString(lowerString, paramIdx++);
            }
          }
          else
          {
            //??? no params
          }

          if (success)
            Plugin_057_M->TransmitRowBuffer();
          success = true;
        }

        break;
      }

    case PLUGIN_CLOCK_IN:
      {
        if (!Plugin_057_M || CONFIG(1) == 0)
          break;

        byte hours = hour();
        byte minutes = minute();

        // Plugin_057_M->ClearRowBuffer();
        Plugin_057_M->SetDigit(CONFIG(5), minutes % 10);
        Plugin_057_M->SetDigit(CONFIG(4), minutes / 10);

        if (CONFIG(1) == 1) {         // 24-hour clock
          // 24-hour clock shows leading zero
          Plugin_057_M->SetDigit(CONFIG(2), hours / 10);
          Plugin_057_M->SetDigit(CONFIG(3), hours % 10);
        } else if (CONFIG(1) == 2) {  // 12-hour clock
          if (hours < 12) {
            // to set AM marker, get buffer and add decimal to it.
            Plugin_057_M->SetRow(CONFIG(5), (Plugin_057_M->GetRow(CONFIG(5)) | 0x80));
          }

          hours = hours % 12;
          if (hours == 0) {
            hours = 12;
          }

          Plugin_057_M->SetDigit(CONFIG(3), hours % 10);

          if (hours < 10) {
            // 12-hour clock will show empty segment when hours < 10
            Plugin_057_M->SetRow(CONFIG(2), 0);
          } else {
            Plugin_057_M->SetDigit(CONFIG(2), hours / 10);
          }
        }

        //if (CONFIG(6) >= 0)
        //  Plugin_057_M->SetRow(CONFIG(6), CONFIG(7));
        Plugin_057_M->TransmitRowBuffer();

        success = true;

        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (!Plugin_057_M || CONFIG(1) == 0)   //clock enabled?
          break;

        if (CONFIG(6) >= 0)   //colon used?
        {
          uint8_t act = ((uint16_t)millis() >> 9) & 1;   //blink with about 2 Hz
          static uint8_t last = 0;
          if (act != last)
          {
            last = act;
            Plugin_057_M->SetRow(CONFIG(6), (act) ? CONFIG(7) : 0);
            Plugin_057_M->TransmitRowBuffer();
          }
        }
      }

  }
  return success;
}

#endif // USES_P057
