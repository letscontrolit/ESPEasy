#ifdef PLUGIN_BUILD_DEV

//#######################################################################################################
//############################# Plugin 111: SenseAir CO2 Sensors ########################################
//#######################################################################################################
/*
  Plugin written by: Daniel Tedenljung info__AT__tedenljungconsulting.com

  This plugin reads the co2 value of SenseAir Co2 Sensors. (S8 works, K30 is ongoing)
  Datasheet can be found here for S8: http://www.senseair.com/products/oem-modules/senseair-s8/
  Datasheet can be found here for K30: http://www.senseair.com/products/oem-modules/k30/

  You can buy sensor from m.nu in Sweden:
  S8 https://www.m.nu/co2matare-fran-senseair-p-1440.html
  K30 https://www.m.nu/k30-co2matare-p-302.html
*/


#define PLUGIN_111
#define PLUGIN_ID_111         111
#define PLUGIN_NAME_111       "SenseAir CO2 Sensor [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_111 "PPM"

boolean Plugin_111_init = false;

#include <SoftwareSerial.h>
SoftwareSerial *Plugin_111_S8;

// 0xFE=Any address 0x04=Read input registers 0x0003=Starting address 0x0001=Number of registers to read 0xD5C5=CRC in reverse order
byte cmdReadPPM[] = {0xFE, 0x04, 0x00, 0x03, 0x00, 0x01, 0xD5, 0xC5};

byte ReciveBuffer[7];

byte Data[5];
byte co2[2];
long ppm;

boolean Plugin_111(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_111;
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
        string = F(PLUGIN_NAME_111);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_111));
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_111_init = true;
        Plugin_111_S8 = new SoftwareSerial(Settings.TaskDevicePin1[event->TaskIndex], Settings.TaskDevicePin2[event->TaskIndex]); // TODO: Explain this in plugin description RX=GPIO Setting 1, TX=GPIO Setting 2, Use 1kOhm in serie on datapins!
        success = true;
        break;
      }

    case PLUGIN_READ:
      {

        if (Plugin_111_init)
        {
          Plugin_111_S8->write(cmdReadPPM, sizeof(cmdReadPPM)); //Send the byte array
          delay(50);

          // Read answer from S8
          int ByteCounter = 0;
          while (Plugin_111_S8->available()) {
            ReciveBuffer[ByteCounter] = Plugin_111_S8->read();
            ByteCounter++;
          }

          // Divide recived chunk in different registers
          for (int i = 0 ; i < sizeof(ReciveBuffer) - 2 ; i++)
          {
            Data[i] = ReciveBuffer[i];
          }

          co2[0] = Data[3];
          co2[1] = Data[4];

          ppm = (co2[0] << 8) | (co2[1]);

          int value = ppm;
          UserVar[event->BaseVarIndex] = (float)value;
          String log = F("S8  : PPM value: "); //TODO: Different string if K30 is used.
          log += value;
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
