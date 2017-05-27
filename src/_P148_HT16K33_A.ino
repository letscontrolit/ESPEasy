//######################################## DMX512 TX ####################################################
//#######################################################################################################

// ESPEasy Plugin to control DMX-512 Devices (DMX 512/1990; DIN 56930-2) like Dimmer-Packs, LED-Bars, Moving-Heads, Event-Lighting
// written by Jochen Krapf (jk@nerd2nerd.org)

// List of commands:
// (1) DMX,<param>
// (2) DMX,<param>,<param>,<param>, ...

// List of DMX params:
// (a) <value>
//     DMX-value (0...255) to write to the next channel address (1...512) starting with 1
// (b) <channel>=<value>
//     DMX-value (0...255) to write to the given channel address (1...512).
// (c) <channel>=<value>,<value>,<value>,...
//     List of DMX-values (0...255) to write beginning with the given channel address (1...512).
// (d) "OFF"
//     Set DMX-values of all channels to 0.
// (e) "ON"
//     Set DMX-values of all channels to 255.
// (f) "LOG"
//     Print DMX-values of all channels to log output.

// Examples:
// DMX,123"   Set channel 1 to value 123
// DMX,123,22,33,44"   Set channel 1 to value 123, channel 2 to 22, channel 3 to33, channel 4 to 44
// DMX,5=123"   Set channel 5 to value 123
// DMX,5=123,22,33,44"   Set channel 5 to value 123, channel 6 to 22, channel 7 to33, channel 8 to 44
// DMX,5=123,8=44"   Set channel 5 to value 123, channel 8 to 44
// DMX,OFF"   Pitch Black

// Transeiver:
// SN75176 or MAX485 or LT1785 or ...
// Pin 5: GND
// Pin 2, 3, 5: +5V
// Pin 4: to ESP D4
// Pin 6: DMX+ (hot)
// Pin 7: DMX- (cold)

// XLR Plug:
// Pin 1: GND, Shield
// Pin 2: DMX- (cold)
// Pin 3: DMX+ (hot)

// Note: The ESP serial FIFO has size of 128 byte. Therefore it is rcommented to use DMX buffer sizes below 128


//#include <*.h>   //no lib needed

#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_148
#define PLUGIN_ID_148         148
#define PLUGIN_NAME_148       "HT16K33"

byte* Plugin_148_DMXBuffer = 0;
int Plugin_148_DMXSize = 32;


boolean Plugin_148(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_148;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
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
        string = F(PLUGIN_NAME_148);
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte addr = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        /*
        String options[2];
        options[0] = F("0x76 - default settings (SDO Low)");
        options[1] = F("0x77 - alternate settings (SDO HIGH)");
        */
        int optionValues[2] = { 0x76, 0x77 };
        addFormSelectorI2C(string, F("i2c_addr"), 2, optionValues, addr);



        Settings.TaskDevicePin1[event->TaskIndex] = 2;
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = Plugin_148_DMXSize;
        addFormNote(string, F("Only GPIO-2 (D4) can be used as TX1!"));
        addFormNumericBox(string, F("Channels"), F("channels"), Plugin_148_DMXSize, 1, 512);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("i2c_addr"));

        Settings.TaskDevicePin1[event->TaskIndex] = 2;
        if (Settings.Pin_status_led == 2)
          Settings.Pin_status_led = -1;
        Plugin_148_DMXSize = getFormItemInt(F("channels"));
        Limit (Plugin_148_DMXSize, 1, 512);
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = Plugin_148_DMXSize;
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Settings.TaskDevicePin1[event->TaskIndex] = 2;   //TX1 fix to GPIO2 (D4) == onboard LED
        Plugin_148_DMXSize = Settings.TaskDevicePluginConfig[event->TaskIndex][0];

        if (Plugin_148_DMXBuffer)
          delete [] Plugin_148_DMXBuffer;
        Plugin_148_DMXBuffer = new byte[Plugin_148_DMXSize];
        memset(Plugin_148_DMXBuffer, 0, Plugin_148_DMXSize);

        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        string.toLowerCase();
        String command = parseString(string, 1);

        if (command == F("dmx"))
        {
          String param;
          String paramKey;
          String paramVal;
          byte paramIdx = 2;
          int channel = 1;
          int value = 0;

          string.replace("  ", " ");
          string.replace(" =", "=");
          string.replace("= ", "=");

          param = parseString(string, paramIdx++);
          if (param.length())
          {
            while (param.length())
            {
              addLog(LOG_LEVEL_DEBUG_MORE, param);

              if (param == F("log"))
              {
                String log = F("DMX  : ");
                for (int i = 0; i < Plugin_148_DMXSize; i++)
                {
                  log += Plugin_148_DMXBuffer[i];
                  log += F(", ");
                }
                addLog(LOG_LEVEL_INFO, log);
                success = true;
              }

              else if (param == F("test"))
              {
                for (int i = 0; i < Plugin_148_DMXSize; i++)
                  //Plugin_148_DMXBuffer[i] = i+1;
                  Plugin_148_DMXBuffer[i] = rand()&255;
                success = true;
              }

              else if (param == F("on"))
              {
                memset(Plugin_148_DMXBuffer, 255, Plugin_148_DMXSize);
                success = true;
              }

              else if (param == F("off"))
              {
                memset(Plugin_148_DMXBuffer, 0, Plugin_148_DMXSize);
                success = true;
              }

              else
              {
                int index = param.indexOf('=');
                if (index > 0)   //syntax: "<channel>=<value>"
                {
                  paramKey = param.substring(0, index);
                  paramVal = param.substring(index+1);
                  channel = paramKey.toInt();
                }
                else   //syntax: "<value>"
                {
                  paramVal = param;
                }

                value = paramVal.toInt();
                Limit (value, 0, 255);

                if (channel > 0 && channel <= Plugin_148_DMXSize)
                  Plugin_148_DMXBuffer[channel-1] = value;
                channel++;
              }

              param = parseString(string, paramIdx++);
            }
          }
          else
          {
            //??? no params
          }

          success = true;
        }

        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (Plugin_148_DMXBuffer)
        {
          int sendPin = 2;   //TX1 fix to GPIO2 (D4) == onboard LED

          //empty serial from prev. transmit
          Serial1.flush();

          //send break
          Serial1.end();
          pinMode(sendPin, OUTPUT);
          digitalWrite(sendPin, LOW);
          delayMicroseconds(120);   //88µs ... inf
          digitalWrite(sendPin, HIGH);
          delayMicroseconds(12);   //8µs ... 1s

          //send DMX data
          Serial1.begin(250000, SERIAL_8N2);
          Serial1.write(0);   //start byte
          Serial1.write(Plugin_148_DMXBuffer, Plugin_148_DMXSize);
        }
        break;
      }

    case PLUGIN_READ:
      {
        //no values
        success = true;
        break;
      }

  }
  return success;
}

#endif
