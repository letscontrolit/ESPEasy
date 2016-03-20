//#######################################################################################################
//################################# Plugin 008: Wiegand RFID Tag Reader #################################
//#######################################################################################################

#define PLUGIN_008
#define PLUGIN_ID_008         8
#define PLUGIN_NAME_008       "RFID Reader - Wiegand"
#define PLUGIN_VALUENAME1_008 "Tag"

#define PLUGIN_008_WGSIZE 26

volatile byte Plugin_008_bitCount = 0;             // Count the number of bits received.
volatile unsigned long Plugin_008_keyBuffer = 0;   // A 32-bit-long keyBuffer into which the number is stored.
byte Plugin_008_bitCountPrev = 0;                  // to detect noise
byte Plugin_008_Unit = 0;

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
          if ((Plugin_008_bitCount != PLUGIN_008_WGSIZE) && (Plugin_008_bitCount == Plugin_008_bitCountPrev))
          {
            // must be noise
            Plugin_008_bitCount = 0;
            Plugin_008_keyBuffer = 0;
          }

          if (Plugin_008_bitCount == PLUGIN_008_WGSIZE)
          {
            Plugin_008_bitCount = 0;          // Read in the current key and reset everything so that the interrupts can

            Plugin_008_keyBuffer = Plugin_008_keyBuffer >> 1;          // Strip leading and trailing parity bits from the keyBuffer
            Plugin_008_keyBuffer &= 0xFFFFFF;
            UserVar[event->BaseVarIndex] = (Plugin_008_keyBuffer & 0xFFFF);
            UserVar[event->BaseVarIndex + 1] = ((Plugin_008_keyBuffer >> 16) & 0xFFFF);
            String log = F("RFID : Tag: ");
            log += Plugin_008_keyBuffer;
            addLog(LOG_LEVEL_INFO, log);
            sendData(event);
          }

          Plugin_008_bitCountPrev = Plugin_008_bitCount; // store this value for next check, detect noise
        }
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

