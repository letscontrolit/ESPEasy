#include "_Plugin_Helper.h"
#ifdef USES_P018

#include "src/Helpers/Hardware.h"
# include <GPIO_Direct_Access.h>

//#######################################################################################################
//#################################### Plugin 018: GP2Y10 ###############################################
//#######################################################################################################

#define PLUGIN_018
#define PLUGIN_ID_018 18
#define PLUGIN_NAME_018 "Dust - Sharp GP2Y10"
#define PLUGIN_VALUENAME1_018 "Dust"

boolean Plugin_018_init = false;

boolean Plugin_018(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_018;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        Device[deviceCount].PluginStats = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_018);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_018));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output(F("LED"));
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_018_init = true;
        pinMode(CONFIG_PIN1, OUTPUT);
        DIRECT_pinWrite(CONFIG_PIN1, HIGH);
        success = true;
        break;
      }


    case PLUGIN_READ:
      {
        ISR_noInterrupts();
        int value = 0;

        for (uint8_t x = 0; x < 25; x++) {
          DIRECT_pinWrite(CONFIG_PIN1, LOW);
          delayMicroseconds(280);
          value = value + espeasy_analogRead(A0);
          delayMicroseconds(40);
          DIRECT_pinWrite(CONFIG_PIN1, HIGH);
          delayMicroseconds(9680);
        }
        ISR_interrupts();
        UserVar[event->BaseVarIndex] = value;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLogMove(LOG_LEVEL_INFO, concat(F("GPY  : Dust value: "), value));
        }

        success = true;
        break;
      }
  }
  return success;
}
#endif // USES_P018
