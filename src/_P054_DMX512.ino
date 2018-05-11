#ifdef USES_P054
//#######################################################################################################
//######################################## Plugin 054: DMX512 TX ########################################
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

// Transceiver:
// SN75176 or MAX485 or LT1785 or ...
// Pin 5: GND
// Pin 2, 3, 8: +5V
// Pin 4: to ESP D4
// Pin 6: DMX+ (hot)
// Pin 7: DMX- (cold)

// XLR Plug:
// Pin 1: GND, Shield
// Pin 2: DMX- (cold)
// Pin 3: DMX+ (hot)

// Note: The ESP serial FIFO has size of 128 byte. Therefore it is rcommented to use DMX buffer sizes below 128


//#include <*.h>   //no lib needed


#define PLUGIN_054
#define PLUGIN_ID_054         54
#define PLUGIN_NAME_054       "Communication - DMX512 TX [TESTING]"

byte* Plugin_054_DMXBuffer = 0;
int16_t Plugin_054_DMXSize = 32;

static inline void PLUGIN_054_Limit(int16_t& value, int16_t min, int16_t max)
{
  if (value < min)
    value = min;
  if (value > max)
    value = max;
}


boolean Plugin_054(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_054;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
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
        string = F(PLUGIN_NAME_054);
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        Settings.TaskDevicePin1[event->TaskIndex] = 2;
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = Plugin_054_DMXSize;
        addFormNote(F("Only GPIO-2 (D4) can be used as TX1!"));
        addFormNumericBox(F("Channels"), F("channels"), Plugin_054_DMXSize, 1, 512);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePin1[event->TaskIndex] = 2;
        if (Settings.Pin_status_led == 2)   //Status LED assigned to TX1?
          Settings.Pin_status_led = -1;
        Plugin_054_DMXSize = getFormItemInt(F("channels"));
        PLUGIN_054_Limit (Plugin_054_DMXSize, 1, 512);
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = Plugin_054_DMXSize;
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Settings.TaskDevicePin1[event->TaskIndex] = 2;   //TX1 fix to GPIO2 (D4) == onboard LED
        Plugin_054_DMXSize = Settings.TaskDevicePluginConfig[event->TaskIndex][0];

        if (Plugin_054_DMXBuffer)
          delete [] Plugin_054_DMXBuffer;
        Plugin_054_DMXBuffer = new byte[Plugin_054_DMXSize];
        memset(Plugin_054_DMXBuffer, 0, Plugin_054_DMXSize);

        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String lowerString=string;
        lowerString.toLowerCase();
        String command = parseString(lowerString, 1);

        if (command == F("dmx"))
        {
          String param;
          String paramKey;
          String paramVal;
          byte paramIdx = 2;
          int16_t channel = 1;
          int16_t value = 0;

          lowerString.replace("  ", " ");
          lowerString.replace(" =", "=");
          lowerString.replace("= ", "=");

          param = parseString(lowerString, paramIdx++);
          if (param.length())
          {
            while (param.length())
            {
              addLog(LOG_LEVEL_DEBUG_MORE, param);

              if (param == F("log"))
              {
                String log = F("DMX  : ");
                for (int16_t i = 0; i < Plugin_054_DMXSize; i++)
                {
                  log += Plugin_054_DMXBuffer[i];
                  log += F(", ");
                }
                addLog(LOG_LEVEL_INFO, log);
                success = true;
              }

              else if (param == F("test"))
              {
                for (int16_t i = 0; i < Plugin_054_DMXSize; i++)
                  //Plugin_054_DMXBuffer[i] = i+1;
                  Plugin_054_DMXBuffer[i] = rand()&255;
                success = true;
              }

              else if (param == F("on"))
              {
                memset(Plugin_054_DMXBuffer, 255, Plugin_054_DMXSize);
                success = true;
              }

              else if (param == F("off"))
              {
                memset(Plugin_054_DMXBuffer, 0, Plugin_054_DMXSize);
                success = true;
              }

              else
              {
                int16_t index = param.indexOf('=');
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
                PLUGIN_054_Limit (value, 0, 255);

                if (channel > 0 && channel <= Plugin_054_DMXSize)
                  Plugin_054_DMXBuffer[channel-1] = value;
                channel++;
              }

              param = parseString(lowerString, paramIdx++);
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
        if (Plugin_054_DMXBuffer)
        {
          int16_t sendPin = 2;   //TX1 fix to GPIO2 (D4) == onboard LED

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
          Serial1.write(Plugin_054_DMXBuffer, Plugin_054_DMXSize);
        }
        break;
      }

  }
  return success;
}

#endif // USES_P054
