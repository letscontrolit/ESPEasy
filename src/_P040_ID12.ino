#ifdef USES_P040
//#######################################################################################################
//#################################### Plugin 040: Serial RFID ID-12 ####################################
//#######################################################################################################

#define PLUGIN_040
#define PLUGIN_ID_040         40
#define PLUGIN_NAME_040       "RFID - ID12LA/RDM6300"
#define PLUGIN_VALUENAME1_040 "Tag"

boolean Plugin_040_init = false;

boolean Plugin_040(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_040;
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
        string = F(PLUGIN_NAME_040);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_040));
        break;
      }


    case PLUGIN_INIT:
      {
        Plugin_040_init = true;
        Serial.begin(9600);
        success = true;
        break;
      }


    case PLUGIN_SERIAL_IN:
      {
        if (Plugin_040_init)
        {
          byte val = 0;
          byte code[6];
          byte checksum = 0;
          byte bytesread = 0;
          byte tempbyte = 0;

          if ((val = Serial.read()) == 2)
          { // check for header
            bytesread = 0;
            while (bytesread < 12) {                        // read 10 digit code + 2 digit checksum
              if ( Serial.available() > 0) {
                val = Serial.read();
                if ((val == 0x0D) || (val == 0x0A) || (val == 0x03) || (val == 0x02)) {
                  // if header or stop bytes before the 10 digit reading
                  break;
                }

                // Do Ascii/Hex conversion:
                if ((val >= '0') && (val <= '9')) {
                  val = val - '0';
                }
                else if ((val >= 'A') && (val <= 'F')) {
                  val = 10 + val - 'A';
                }

                // Every two hex-digits, add byte to code:
                if ( (bytesread & 1) == 1) {
                  // make some space for this hex-digit by
                  // shifting the previous hex-digit with 4 bits to the left:
                  code[bytesread >> 1] = (val | (tempbyte << 4));

                  if (bytesread >> 1 != 5) {                // If we're at the checksum byte,
                    checksum ^= code[bytesread >> 1];       // Calculate the checksum... (XOR)
                  };
                }
                else {
                  tempbyte = val;                           // Store the first hex digit first...
                };
                bytesread++;                                // ready to read next digit
              }
            }
          }

          if (bytesread == 12)
          {
            if (code[5] == checksum)
            {
              // temp woraround, ESP Easy framework does not currently prepare this...
              byte index = 0;
              for (byte y = 0; y < TASKS_MAX; y++)
                if (Settings.TaskDeviceNumber[y] == PLUGIN_ID_040)
                  index = y;
              byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[index]);
              event->TaskIndex = index;
              event->BaseVarIndex = index * VARS_PER_TASK;
              event->sensorType = Device[DeviceIndex].VType;
              // endof workaround

              unsigned long key = 0;
              for (byte i = 1; i < 5; i++) key = key | (((unsigned long) code[i] << ((4 - i) * 8)));
              UserVar[event->BaseVarIndex] = (key & 0xFFFF);
              UserVar[event->BaseVarIndex + 1] = ((key >> 16) & 0xFFFF);
              String log = F("RFID : Tag: ");
              log += key;
              addLog(LOG_LEVEL_INFO, log);
              sendData(event);
            }
          }
          success = true;
        }
        break;
      }

  }
  return success;
}
#endif // USES_P040
