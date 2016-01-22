//#######################################################################################################
//#################### Plugin 029 SRF01 Ultrasonic Distance Sensor ######################################
//#######################################################################################################

#include <SoftwareSerial.h>

#define PLUGIN_029
#define PLUGIN_ID_029        29
#define PLUGIN_NAME_029       "Ultrasonic Sensor - SRF01"
#define PLUGIN_VALUENAME1_029 "Distance"

#define PLUGIN_029_srfAddress 0x01             // Address of the SFR01, default ist 0x01
#define PLUGIN_029_getSoft    0x5D             // Byte to tell SRF01 we wish to read software version
#define PLUGIN_029_getRange   0x54             // Byte used to get range from SRF01 in cm
#define PLUGIN_029_getStatus  0x5F             // Byte used to get the status of the transducer

SoftwareSerial *Plugin_029_SRF;

uint8_t Plugin_029_SRF_Pin;

boolean Plugin_029(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  byte timeout;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_029;
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
        string = F(PLUGIN_NAME_029);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_029));
        break;
      }


    case PLUGIN_INIT:
      {
        Plugin_029_SRF_Pin = Settings.TaskDevicePin1[event->TaskIndex];
        Plugin_029_SRF = new SoftwareSerial(Plugin_029_SRF_Pin, Plugin_029_SRF_Pin);

        Plugin_029_SRF01_Cmd(PLUGIN_029_srfAddress, PLUGIN_029_getSoft);
        for (timeout = 0; timeout < 5; timeout++) {
          if (Plugin_029_SRF->available() < 1 ) {
            delay(10);
          } else {
            int softVer = Plugin_029_SRF->read();
            String log = F("SRF01 : Firmware-Version: ");
            log += softVer;
            addLog(LOG_LEVEL_INFO, log);
            success = true;
            break;
          };
        };
        success = false;
        break;
      }

    case PLUGIN_READ:
      {
        Plugin_029_SRF01_Cmd(PLUGIN_029_srfAddress, PLUGIN_029_getRange);
        for (timeout = 0; timeout < 5; timeout++) {
          if (Plugin_029_SRF->available() < 2 ) {
            delay(10);
          } else {
            byte highByte = Plugin_029_SRF->read();
            byte lowByte = Plugin_029_SRF->read();
            int dist = ((highByte << 8) + lowByte);
            UserVar[event->BaseVarIndex] = dist;
            
            String log = F("SRF1 : Distance: ");
            log += dist;
            addLog(LOG_LEVEL_INFO,log);
            
            success = true;
            break;
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
void Plugin_029_SRF01_Cmd(byte Address, byte cmd) {
  pinMode(Plugin_029_SRF_Pin, OUTPUT);
  digitalWrite(Plugin_029_SRF_Pin, LOW);
  delay(2);
  digitalWrite(Plugin_029_SRF_Pin, HIGH);
  delay(1);
  Plugin_029_SRF->write(Address);
  Plugin_029_SRF->write(cmd);
  pinMode(Plugin_029_SRF_Pin, INPUT);
  Plugin_029_SRF->flush();
}

