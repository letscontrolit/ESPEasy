#ifdef PLUGIN_BUILD_DEV

//#######################################################################################################
//#################### Plugin 100 SRF01 Ultrasonic Distance Sensor ######################################
//#######################################################################################################

#include <SoftwareSerial.h>

#define PLUGIN_100
#define PLUGIN_ID_100        100
#define PLUGIN_NAME_100       "Ultrasonic Sensor - SRF01 [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_100 "Distance"

#define PLUGIN_100_srfAddress 0x01             // Address of the SFR01, default ist 0x01
#define PLUGIN_100_getSoft    0x5D             // Byte to tell SRF01 we wish to read software version
#define PLUGIN_100_getRange   0x54             // Byte used to get range from SRF01 in cm
#define PLUGIN_100_getStatus  0x5F             // Byte used to get the status of the transducer

SoftwareSerial *Plugin_100_SRF;

uint8_t Plugin_100_SRF_Pin;

boolean Plugin_100(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  byte timeout;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_100;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_100);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_100));
        break;
      }


    case PLUGIN_INIT:
      {
        addLog(LOG_LEVEL_INFO, (char*)"INIT : SRF01");
        Plugin_100_SRF_Pin = Settings.TaskDevicePin1[event->TaskIndex];
        Plugin_100_SRF = new SoftwareSerial(Plugin_100_SRF_Pin, Plugin_100_SRF_Pin);
        Plugin_100_SRF01_Cmd(PLUGIN_100_srfAddress, PLUGIN_100_getSoft);
        for (timeout = 0; timeout < 10; timeout++) {
          if (Plugin_100_SRF->available() < 1 ) {
            delay(10);
          } else {
            int softVer = Plugin_100_SRF->read();
            String log = F("SRF01 : Firmware-Version: ");
            log += softVer;
            addLog(LOG_LEVEL_INFO, log);
            success = true;
            return success;
          };
        };
        addLog(LOG_LEVEL_ERROR, (char*)"SRF01-Init: protocol timeout!");
        success = false;
        break;
      }

    case PLUGIN_READ:
      {
        Plugin_100_SRF01_Cmd(PLUGIN_100_srfAddress, PLUGIN_100_getRange);
        for (timeout = 0; timeout < 10; timeout++) {
          if (Plugin_100_SRF->available() < 2 ) {
            delay(10);
          } else {
            byte highByte = Plugin_100_SRF->read();
            byte lowByte = Plugin_100_SRF->read();
            int dist = ((highByte << 8) + lowByte);
            UserVar[event->BaseVarIndex] = dist;
            String log = F("SRF1 : Distance: ");
            log += dist;
            addLog(LOG_LEVEL_INFO, log);
            success = true;
            return success;
          };
        };
        addLog(LOG_LEVEL_ERROR, (char*)"SRF01  : protocol timeout!");
        UserVar[event->BaseVarIndex] = NAN;
        success = false;
        break;
      }

  }
  return success;
}

//**************************************************************************/
// Send SRF01 Command
//**************************************************************************/
void Plugin_100_SRF01_Cmd(byte Address, byte cmd) {
  Plugin_100_SRF->flush();
  pinMode(Plugin_100_SRF_Pin, OUTPUT);
  digitalWrite(Plugin_100_SRF_Pin, LOW);
  delay(2);
  digitalWrite(Plugin_100_SRF_Pin, HIGH);
  delay(1);
  Plugin_100_SRF->write(Address);
  Plugin_100_SRF->write(cmd);
  pinMode(Plugin_100_SRF_Pin, INPUT);
  Plugin_100_SRF->flush();
}

#endif
