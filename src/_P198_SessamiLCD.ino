//#######################################################################################################
//####################################### Plugin 198: SessamiUI ##############################################
//#######################################################################################################
#ifdef PLUGIN_BUILD_DEV

#include <SessamiUI.h>
#include <Sessami_Page_ButtonSen.h>

#define PLUGIN_198
#define PLUGIN_ID_198         198
#define PLUGIN_NAME_198       "SessamiUI"
#define PLUGIN_VALUENAME1_198 "UI"

#define TFT_DC 16
#define TFT_CS 4

uint8_t state = 0;
bool ui_rst = true;
SessamiUI *ui[2];

boolean Plugin_198(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte displayTimer = 0;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_198;
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
        string = F(PLUGIN_NAME_198);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_198));
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
        ui[0] = (SessamiUI *)new Page_ButtonSen;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        /*switch (state) {
          case 0 :
            if (*button == B_PROX) {
              //led->SetDutyCycle(B1111, B110);
              state = 1;
              ui_rst = true;
            }
            break;
          case 1 :
            if (button->GetHeldT() > 3) {
              //led->SetDutyCycle(B1111, B000);
              state = 0;
              ui_rst = true;
            }
            break;
        }*/

        ui[state]->UIStateMachine(ui_rst);
        ui_rst = false;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        Plugin_198_ButTest();
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

void Plugin_198_ButTest() {
  //if (*button == B_PROX)
  //Serial.println("PROX");
  if (*button == B_UP)
    Serial.println("UP");
  if (*button == B_DOWN)
    Serial.println("DOWN");
  if (*button == B_POWER)
    Serial.println("POWER");
  if (*button == B_LEFT)
    Serial.println("LEFT");
  if (*button == B_MID)
    Serial.println("MID");
  if (*button == B_RIGHT)
    Serial.println("RIGHT");

  if (*button == S_LEFT)
    Serial.println("Slider LEFT");
  if (*button == S_RIGHT)
    Serial.println("Slider RIGHT");
}

#endif
