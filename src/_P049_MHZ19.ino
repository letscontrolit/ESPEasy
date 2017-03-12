/*

  This plug in is written by Dmitry (rel22 ___ inbox.ru)
  Plugin is based upon SenseAir plugin by Daniel Tedenljung info__AT__tedenljungconsulting.com

  This plugin reads the CO2 value from MH-Z19 NDIR Sensor
  DevicePin1 - is RX for ESP
  DevicePin2 - is TX for ESP
*/

#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_049
#define PLUGIN_ID_049         49
#define PLUGIN_NAME_049       "NDIR CO2 Sensor MH-Z19 [TESTING]"
#define PLUGIN_VALUENAME1_049 "PPM"
#define PLUGIN_READ_TIMEOUT   3000

boolean Plugin_049_init = false;

#include <SoftwareSerial.h>
SoftwareSerial *Plugin_049_S8;

// 9-bytes CMD PPM read command
byte mhzCmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
byte mhzResp[9];    // 9 bytes bytes response

boolean Plugin_049(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_049;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_049);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_049));
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_049_S8 = new SoftwareSerial(Settings.TaskDevicePin1[event->TaskIndex], Settings.TaskDevicePin2[event->TaskIndex]);
        Plugin_049_S8->begin(9600);
        String log = F("MHZ19: Init OK ");
        addLog(LOG_LEVEL_INFO, log);

        //delay first read, because hardware needs to initialize on cold boot
        //otherwise we get a weird value or read error
        timerSensor[event->TaskIndex] = millis() + 15000;

        Plugin_049_init = true;
        success = true;
        break;
      }

    case PLUGIN_READ:
      {

        if (Plugin_049_init)
        {
          //send read PPM command
          int nbBytesSent = Plugin_049_S8->write(mhzCmd, 9);
          if (nbBytesSent != 9) {
            String log = F("MHZ19: Error, nb bytes sent != 9 : ");
              log += nbBytesSent;
              addLog(LOG_LEVEL_INFO, log);
          }

          // get response
          memset(mhzResp, 0, sizeof(mhzResp));

          long start = millis();
          int counter = 0;
          while (((millis() - start) < PLUGIN_READ_TIMEOUT) && (counter < 9)) {
            if (Plugin_049_S8->available() > 0) {
              mhzResp[counter++] = Plugin_049_S8->read();
            } else {
              delay(10);
            }
          }
          if (counter < 9){
              String log = F("MHZ19: Error, timeout while trying to read");
              addLog(LOG_LEVEL_INFO, log);
          }

          unsigned int ppm = 0;
          int i;
          byte crc = 0;
          for (i = 1; i < 8; i++) crc+=mhzResp[i];
              crc = 255 - crc;
              crc++;

          if ( !(mhzResp[0] == 0xFF && mhzResp[1] == 0x86 && mhzResp[8] == crc) ) {
             String log = F("MHZ19: Read error : CRC = ");
             log += String(crc); log += " / "; log += String(mhzResp[8]);
             log += " bytes read  => ";for (i = 0; i < 9; i++) {log += mhzResp[i];log += "/" ;}
             addLog(LOG_LEVEL_ERROR, log);

             success = false;
             break;

          } else {
              //calculate CO2 PPM
              unsigned int mhzRespHigh = (unsigned int) mhzResp[2];
              unsigned int mhzRespLow = (unsigned int) mhzResp[3];
              ppm = (256*mhzRespHigh) + mhzRespLow;
          }

          UserVar[event->BaseVarIndex] = (float)ppm;
          String log = F("MHZ19: PPM value: ");
          log += ppm;
          addLog(LOG_LEVEL_INFO, log);
          success = true;
          break;
        }
        break;
      }
  }
  return success;
}

#endif
