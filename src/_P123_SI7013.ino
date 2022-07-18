#include "_Plugin_Helper.h"

#ifdef USES_P123
#include "src/PluginStructs/P123_data_struct.h"
//#######################################################################################################
//######################## Plugin 123 SI7013 I2C Temperature Humidity Sensor  ###########################
//#######################################################################################################
// 08-06-2019 Florin Muntean

#define PLUGIN_123
#define PLUGIN_ID_123        123
#define PLUGIN_NAME_123       "Environment - SI7013"
#define PLUGIN_VALUENAME1_123 "Temperature"
#define PLUGIN_VALUENAME2_123 "Humidity"
#define PLUGIN_VALUENAME3_123 "ADC"


# define P123_RESOLUTION      PCONFIG(0)
# define P123_I2CADDR         PCONFIG(1)
# define P123_FILTER_POWER    PCONFIG(2)


//convert from I2C address to index
uint8_t Plugin_123_device_index(const uint8_t i2caddr) {
  switch(i2caddr) {
    case SI7021_I2C_ADDRESS:  return 0u;
    case SI7013_I2C_ADDRESS:  return 1u;
  }
  return 1u; // Some default
}

const uint8_t Plugin_123_i2c_addresses[2] = { SI7021_I2C_ADDRESS, SI7013_I2C_ADDRESS };

boolean Plugin_123(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_123;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO; //using this type in order to send the 3rd value which is the ADC 
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_123);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_123));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_123));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_123));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        #define SI7013_RESOLUTION_OPTIONS 4

        byte choice = P123_RESOLUTION;
        String options[SI7013_RESOLUTION_OPTIONS];
        int optionValues[SI7013_RESOLUTION_OPTIONS];
        optionValues[0] = SI7013_RESOLUTION_14T_12RH;
        options[0] = F("Temp 14 bits / RH 12 bits");
        optionValues[1] = SI7013_RESOLUTION_13T_10RH;
        options[1] = F("Temp 13 bits / RH 10 bits");
        optionValues[2] = SI7013_RESOLUTION_12T_08RH;
        options[2] = F("Temp 12 bits / RH  8 bits");
        optionValues[3] = SI7013_RESOLUTION_11T_11RH;
        options[3] = F("Temp 11 bits / RH 11 bits");
        addFormSelector(F("Resolution"), F("p014_res"), SI7013_RESOLUTION_OPTIONS, options, optionValues, choice);
        //addUnit(F("bits"));

        addFormSelectorI2C(F("p123_i2c"), 2, Plugin_123_i2c_addresses, P123_I2CADDR);
 
        addFormNumericBox("Filter Power", F("p123_filter"), P123_FILTER_POWER, 0, 4);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        P123_RESOLUTION   = getFormItemInt(F("p123_res"));
        P123_I2CADDR      = getFormItemInt(F("p123_i2c"));
        P123_FILTER_POWER = getFormItemInt(F("p123_filter"));
        
         // Force device setup next time
        P123_data_struct *P123_data = static_cast<P123_data_struct *>(getPluginTaskData(event->TaskIndex));
        if (P123_data) {
          P123_data->state = P123_state::Uninitialized;
        }

        success = true;
        break;
      }
    
    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P123_data_struct() );
      P123_data_struct *P123_data = static_cast<P123_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P123_data) {
        return success;
      }

      success = true;
      break;
    }

    case PLUGIN_READ:
      {
        P123_data_struct *P123_data = static_cast<P123_data_struct *>(getPluginTaskData(event->TaskIndex));
        uint8_t i2caddress   = P123_I2CADDR;
        uint8_t resolution   = P123_RESOLUTION;
        uint8_t filter_power = P123_FILTER_POWER;
            
        if (nullptr != P123_data) {
          if (P123_data->state != P123_state::New_values_available) {
            P123_data->update(i2caddress,resolution,filter_power); //run the state machine
            Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + SI7013_DELAY);//keep going until we have New_values_available
            success = false;
            break;
          }
        }
        
        P123_data->state = P123_state::Ready;
      
        UserVar[event->BaseVarIndex]     = P123_data->last_temp_val;
        UserVar[event->BaseVarIndex + 1] = P123_data->last_hum_val;
        UserVar[event->BaseVarIndex + 2] = (P123_data->last_adc_val) >>filter_power;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("SI7013: Temperature: ");
          log += UserVar[event->BaseVarIndex + 0];
          log += F(" Humidity: ");
          log += UserVar[event->BaseVarIndex + 1];
          log += F(" ADC: ");
          log += UserVar[event->BaseVarIndex + 2];
          addLog(LOG_LEVEL_INFO, log);
        }

        success = true;
        break;
      }

  }
  return success;
}

#endif // USES_P123
