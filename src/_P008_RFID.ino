#ifdef USES_P008
//#######################################################################################################
//################################# Plugin 008: Wiegand RFID Tag Reader #################################
//#######################################################################################################

#define PLUGIN_008
#define PLUGIN_ID_008         8
#define PLUGIN_NAME_008       "RFID - Wiegand"
#define PLUGIN_VALUENAME1_008 "Tag"

void Plugin_008_interrupt1() ICACHE_RAM_ATTR;
void Plugin_008_interrupt2() ICACHE_RAM_ATTR;

volatile byte Plugin_008_bitCount = 0;     // Count the number of bits received.
uint64_t Plugin_008_keyBuffer = 0;    // A 64-bit-long keyBuffer into which the number is stored.
byte Plugin_008_timeoutCount = 0;
byte Plugin_008_WiegandSize = 26;          // size of a tag via wiegand (26-bits or 36-bits)

boolean Plugin_008_init = false;

boolean Plugin_008(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_008;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].VType = SENSOR_TYPE_LONG;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_008);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_008));
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_008_init = true;
        Plugin_008_WiegandSize = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
        pinMode(Settings.TaskDevicePin2[event->TaskIndex], INPUT_PULLUP);
        attachInterrupt(Settings.TaskDevicePin1[event->TaskIndex], Plugin_008_interrupt1, FALLING);
        attachInterrupt(Settings.TaskDevicePin2[event->TaskIndex], Plugin_008_interrupt2, FALLING);
        success = true;
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        if (Plugin_008_init)
        {
          if (Plugin_008_bitCount > 0)
          {
            if (Plugin_008_bitCount % 4 == 0 && ((Plugin_008_keyBuffer & 0xF) == 11))
            {
              // a number of keys were pressed and finished by #
              Plugin_008_keyBuffer = Plugin_008_keyBuffer >> 4;  // Strip #
              UserVar[event->BaseVarIndex] = (Plugin_008_keyBuffer & 0xFFFF);
              UserVar[event->BaseVarIndex + 1] = ((Plugin_008_keyBuffer >> 16) & 0xFFFF);
            }
            else if (Plugin_008_bitCount == Plugin_008_WiegandSize)
            {
              // read a tag
              Plugin_008_keyBuffer = Plugin_008_keyBuffer >> 1;          // Strip leading and trailing parity bits from the keyBuffer
              if (Plugin_008_WiegandSize == 26)
                Plugin_008_keyBuffer &= 0xFFFFFF;
              else
                Plugin_008_keyBuffer &= 0xFFFFFFFF;
              UserVar[event->BaseVarIndex] = (Plugin_008_keyBuffer & 0xFFFF);
              UserVar[event->BaseVarIndex + 1] = ((Plugin_008_keyBuffer >> 16) & 0xFFFF);
            }
            else
            {
              // not enough bits, maybe next time
              Plugin_008_timeoutCount++;
              if (Plugin_008_timeoutCount > 5)
              {
                String log = F("RFID : reset bits: ");
                log += Plugin_008_bitCount;
                addLog(LOG_LEVEL_INFO, log );
                // reset after ~5 sec
                Plugin_008_keyBuffer = 0;
                Plugin_008_bitCount = 0;
                Plugin_008_timeoutCount = 0;
              }
              break;
            }
            // reset everything
            unsigned long bitCount = Plugin_008_bitCount;    // copy for log
            unsigned long keyBuffer = Plugin_008_keyBuffer;  // copy for log
            Plugin_008_keyBuffer = 0;
            Plugin_008_bitCount = 0;
            Plugin_008_timeoutCount = 0;
            // write log
            String log = F("RFID : Tag: ");
            log += keyBuffer;
            log += F(" Bits: ");
            log += bitCount;
            addLog(LOG_LEVEL_INFO, log);
            sendData(event);
          }
        }
        break;
      }
      case PLUGIN_WEBFORM_LOAD:
        {
          byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
          String options[2];
          options[0] = F("26 Bits");
          options[1] = F("34 Bits");
          int optionValues[2];
          optionValues[0] = 26;
          optionValues[1] = 34;
          addFormSelector(F("Wiegand Type"), F("plugin_008_type"), 2, options, optionValues, choice);
          success = true;
          break;
        }

      case PLUGIN_WEBFORM_SAVE:
        {
          String plugin1 = WebServer.arg(F("plugin_008_type"));
          Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
          success = true;
          break;
        }
  }
  return success;
}

/*********************************************************************/
void Plugin_008_interrupt1()
/*********************************************************************/
{
  // We've received a 1 bit. (bit 0 = high, bit 1 = low)
  Plugin_008_keyBuffer = Plugin_008_keyBuffer << 1;     // Left shift the number (effectively multiplying by 2)
  Plugin_008_keyBuffer += 1;         // Add the 1 (not necessary for the zeroes)
  Plugin_008_bitCount++;         // Increment the bit count
}

/*********************************************************************/
void Plugin_008_interrupt2()
/*********************************************************************/
{
  // We've received a 0 bit. (bit 0 = low, bit 1 = high)
  Plugin_008_keyBuffer = Plugin_008_keyBuffer << 1;     // Left shift the number (effectively multiplying by 2)
  Plugin_008_bitCount++;           // Increment the bit count
}
#endif // USES_P008
