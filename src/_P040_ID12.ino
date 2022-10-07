#include "_Plugin_Helper.h"
#ifdef USES_P040


//#######################################################################################################
//#################################### Plugin 040: Serial RFID ID-12 ####################################
//#######################################################################################################

#define PLUGIN_040
#define PLUGIN_ID_040         40
#define PLUGIN_NAME_040       "RFID - ID12LA/RDM6300"
#define PLUGIN_VALUENAME1_040 "Tag"

boolean Plugin_040_init = false;

boolean Plugin_040(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_040;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_LONG;
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

    case PLUGIN_TASKTIMER_IN:
      {
        if (Plugin_040_init) {
            // Reset card id on timeout
            UserVar.setSensorTypeLong(event->TaskIndex, 0);
            addLog(LOG_LEVEL_INFO, F("RFID : Removed Tag"));
            sendData(event);
            success = true;
        }
        break;
      }

    case PLUGIN_SERIAL_IN:
      {
        if (Plugin_040_init)
        {
          uint8_t val = 0;
          uint8_t code[6];
          uint8_t checksum = 0;
          uint8_t bytesread = 0;
          uint8_t tempbyte = 0;

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
                if (isDigit(val)) {
                  val = val - '0';
                }
                else if ((val >= 'A') && (val <= 'F')) {
                  val = 10 + val - 'A';
                }

                // Every two hex-digits, add uint8_t to code:
                if ( (bytesread & 1) == 1) {
                  // make some space for this hex-digit by
                  // shifting the previous hex-digit with 4 bits to the left:
                  code[bytesread >> 1] = (val | (tempbyte << 4));

                  if (bytesread >> 1 != 5) {                // If we're at the checksum uint8_t,
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
              taskIndex_t index = INVALID_TASK_INDEX;
              for (taskIndex_t y = 0; y < TASKS_MAX; y++)
                if (Settings.TaskDeviceNumber[y] == PLUGIN_ID_040)
                  index = y;
              const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(index);
              if (!validDeviceIndex(DeviceIndex)) {
                break;
              }
              event->setTaskIndex(index);
              if (!validUserVarIndex(event->BaseVarIndex)) {
                break;
              }
              checkDeviceVTypeForTask(event);
              // endof workaround

              unsigned long key = 0, old_key = 0;
              old_key = UserVar.getSensorTypeLong(event->TaskIndex);
              for (uint8_t i = 1; i < 5; i++) key = key | (((unsigned long) code[i] << ((4 - i) * 8)));
              bool new_key = false;              
              if (old_key != key) {
                UserVar.setSensorTypeLong(event->TaskIndex, key);
                new_key = true;
              }

              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                String log = F("RFID : ");
                if (new_key) {
                  log += F("New Tag: ");
                } else {
                  log += F("Old Tag: "); 
                }
                log += key;
                addLogMove(LOG_LEVEL_INFO, log);
              }
              
              if (new_key) sendData(event);
              Scheduler.setPluginTaskTimer(500, event->TaskIndex, event->Par1);
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
