//#######################################################################################################
//################################ Plugin 197: Capactive Touch ##########################################
//#######################################################################################################
#ifdef PLUGIN_BUILD_DEV

#include <CAP1114_Driver.h>
#include <CAP1114_Button.h>

#define PLUGIN_197
#define PLUGIN_ID_197         197
#define PLUGIN_NAME_197       "CapTouch and LED Driver - CAP1114"
#define PLUGIN_VALUENAME1_197 "CAP"

boolean Plugin_197(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte displayTimer = 0;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_197;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 0;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_197);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_197));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
      }

    case PLUGIN_WEBFORM_SAVE:
      {
      }

    case PLUGIN_INIT:
      {
        button = new Sessami_Button;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        button->UpdateBut();
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        button->HeldCount();
        button->HoldCount();
      }

    case PLUGIN_READ:
      {
      }

    case PLUGIN_WRITE:
      {
      }

  }
  return success;
}

#endif
