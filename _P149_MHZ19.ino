#ifdef PLUGIN_BUILD_DEV
/*

  This plug in is written by Dmitry (rel22 ___ inbox.ru)
  Plugin is based upon SenseAir plugin by Daniel Tedenljung info__AT__tedenljungconsulting.com

  This plugin reads the CO2 value from MH-Z19 NDIR Sensor
  DevicePin1 - is RX for ESP
  DevicePin2 - is TX for ESP
*/


#define PLUGIN_149
#define PLUGIN_ID_149         149
#define PLUGIN_NAME_149       "NDIR CO2 Sensor MH-Z19 [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_149 "PPM"

boolean Plugin_149_init = false;

#include <SoftwareSerial.h>
SoftwareSerial *Plugin_149_S8;

// 9-bytes CMD PPM read command
byte mhzCmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
byte mhzResp[9];    // 9 bytes bytes response

boolean Plugin_149(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_149;
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
        string = F(PLUGIN_NAME_149);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_149));
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_149_init = true;
        Plugin_149_S8 = new SoftwareSerial(Settings.TaskDevicePin1[event->TaskIndex], Settings.TaskDevicePin2[event->TaskIndex]);
        success = true;
        break;
      }

    case PLUGIN_READ:
      {

        if (Plugin_149_init)
        {
          Plugin_149_S8->write(mhzCmd, 9);
          memset(mhzResp, 0, 9);
          Plugin_149_S8->readBytes(mhzResp, 9);
          int i;
          unsigned int ppm = 0;
          byte crc = 0;
          for (i = 1; i < 8; i++) crc+=mhzResp[i];
              crc = 255 - crc;
              crc++;

          if ( !(mhzResp[0] == 0xFF && mhzResp[1] == 0x86 && mhzResp[8] == crc) ) {
             String log = F("MHZ19: CRC error: ");
             log += String(crc); log += " / "; log += String(mhzResp[8]);
             addLog(LOG_LEVEL_ERROR, log);
             success = false;
             break;

          } else {

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
