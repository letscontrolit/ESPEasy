#include "_Plugin_Helper.h"
#ifdef USES_P164

// #######################################################################################################
// #################################### Plugin-164: Gases - ENS160 (tvoc,eco2) ###########################
// #######################################################################################################

# include "src/PluginStructs/P164_data_struct.h"

# define PLUGIN_164
# define PLUGIN_ID_164         164
# define PLUGIN_NAME_164       "Gases - ENS160"
# define PLUGIN_VALUENAME1_164 "TVOC"
# define PLUGIN_VALUENAME2_164 "eCO2"

boolean Plugin_164(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_164;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_DUAL;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      Device[deviceCount].I2CNoDeviceCheck   = true;  //TODO
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_164);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_164));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_164));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { ENS160_I2CADDR_0, ENS160_I2CADDR_1 };
      constexpr int nrAddressOptions   = NR_ELEMENTS(i2cAddressValues);

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), nrAddressOptions, i2cAddressValues, P164_I2C_ADDR);
        addFormNote(F("ADDR Low=0x52, High=0x53"));
        Serial.print("ENS16x: plugin_webform_show_i2c_params() = ");
        Serial.println(success);
      } else {
        success = intArrayContains(nrAddressOptions, i2cAddressValues, event->Par1);
        Serial.print("ENS16x: plugin_i2c_has_address() = ");
        Serial.println(success);       
       }

      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P164_I2C_ADDR;
      success     = true;
      Serial.print("ENS16x: plugin_i2c_get_addr() = ");
      Serial.println(success);      
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P164_I2C_ADDR = ENS160_I2CADDR_1;
      success = true;
      Serial.print("ENS16x: plugin_set_defaults() = ");
      Serial.println(success);
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P164_data_struct(event));
      P164_data_struct *P164_data = static_cast<P164_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P164_data && P164_data->begin());
      Serial.print("ENS16x: plugin_init() = ");
      Serial.println(success);
      break;
    }

    case PLUGIN_READ:
    {
      P164_data_struct *P164_data = static_cast<P164_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P164_data) {
        Serial.print("ENS16x: plugin_read NULLPTR");
        break;
      }

      success = P164_data->read(UserVar[event->BaseVarIndex], UserVar[event->BaseVarIndex + 1]);
      Serial.print("ENS16x: plugin_read() = ");
      Serial.print(success);
      Serial.print(" val1= ");
      Serial.print(UserVar[event->BaseVarIndex]);
      Serial.print(" val2= ");
      Serial.println(UserVar[event->BaseVarIndex+1]);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      success = P164_data_struct::webformLoad(event);
      Serial.print("ENS16x: plugin_webform_load() = ");
      Serial.println(success);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      success = P164_data_struct::webformSave(event);
      Serial.print(F("ENS16x: plugin_webform_save() = "));
      Serial.println(success);
      Serial.print(F("ENS16x: I2C_ADDR = 0x"));
      Serial.println(P164_I2C_ADDR, HEX);
      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P164_data_struct *P164_data =
        static_cast<P164_data_struct *>(getPluginTaskData(event->TaskIndex));
      if (nullptr == P164_data) {
        break;
      }
      success = P164_data->tenPerSecond(event);
      success = true;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    case PLUGIN_ONCE_A_SECOND:
    {
      break;
    }
    default:
    {
      // Serial.print(F("ENS160: Unhandled plugin call: "));
      // Serial.println(function);
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P164
