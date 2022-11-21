#include "_Plugin_Helper.h"
#ifdef USES_P014

// #######################################################################################################
// ######################## Plugin 014 SI70xx I2C Temperature Humidity Sensor  ###########################
// #######################################################################################################
// 12-10-2015 Charles-Henri Hallard, see my projects and blog at https://hallard.me
// 07-22-2022 MFD, Adding support for SI7013 with ADC and lots of refactoring


/*
 datasheet: https://www.silabs.com/sensors/humidity/si7006-13-20-21-34/device.si7013-a20-gm1
 If using the SI7013 to measure ADC make sure you implement one of the two thermistor (thermistor can we replaced with LRD for light level measurements) circuit diagrams:
  Figure 5. Typical Application Circuit for Thermistor Interface with AD0 = 1
  Figure 6. Typical Application Circuit for Thermistor Interface with AD0 = 0
Do not use the single ended circuit as the Vout is connected directly to ground:
  Figure 7. Typical Application Circuit for Single Ended 0 to 3 V Measurement (!DO NOT USE THIS CIRCUIT!)
*/



# include "src/PluginStructs/P014_data_struct.h"

# define PLUGIN_014
# define PLUGIN_ID_014        14
# define PLUGIN_NAME_014       "Environment - SI70xx/HTU21D"
# define PLUGIN_VALUENAME1_014 "Temperature"
# define PLUGIN_VALUENAME2_014 "Humidity"
# define PLUGIN_VALUENAME3_014 "ADC"

# define P014_I2C_ADDRESS         PCONFIG(1)
# define P014_RESOLUTION          PCONFIG(0)
# define P014_FILTER_POWER        PCONFIG(2)
# define P014_ERROR_STATE_OUTPUT  PCONFIG(3)
# define P014_VALUES_COUNT        PCONFIG(4) 

boolean Plugin_014(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_014;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      //Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      Device[deviceCount].OutputDataType     = Output_Data_type_t::All;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_014);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_014));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_014));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_014));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { SI70xx_I2C_ADDRESS, SI7013_I2C_ADDRESS_AD0_1 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 2, i2cAddressValues, P014_I2C_ADDRESS);
        addFormNote(F("ADO Low=0x40, High=0x41"));
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }

      break;
    }


    case PLUGIN_WEBFORM_LOAD:
    {
        # define P014_RESOLUTION_OPTIONS 4

      uint8_t choice = P014_RESOLUTION;
      const __FlashStringHelper *options[P014_RESOLUTION_OPTIONS];
      int optionValues[P014_RESOLUTION_OPTIONS];
      optionValues[0] = SI70xx_RESOLUTION_14T_12RH;
      options[0]      = F("Temp 14 bits / RH 12 bits");
      optionValues[1] = SI70xx_RESOLUTION_13T_10RH;
      options[1]      = F("Temp 13 bits / RH 10 bits");
      optionValues[2] = SI70xx_RESOLUTION_12T_08RH;
      options[2]      = F("Temp 12 bits / RH  8 bits");
      optionValues[3] = SI70xx_RESOLUTION_11T_11RH;
      options[3]      = F("Temp 11 bits / RH 11 bits");
      addFormSelector(F("Resolution"), F("p014_res"), P014_RESOLUTION_OPTIONS, options, optionValues, choice);

      addFormNumericBox("ADC Filter Power", F("p014_filter"), P014_FILTER_POWER, 0, 4);

      // addUnit(F("bits"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P014_I2C_ADDRESS  = getFormItemInt(F("i2c_addr"));
      P014_RESOLUTION   = getFormItemInt(F("p014_res"));
      P014_FILTER_POWER = getFormItemInt(F("p014_filter"));
        
      // Force device setup next time
      //P014_data_struct *P014_data = static_cast<P014_data_struct *>(getPluginTaskData(event->TaskIndex));
      //if (nullptr != P014_data) {
      //  P014_data->state = P014_state::Uninitialized;
      //}

      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = getValueCountFromSensorType(static_cast<Sensor_VType>(P014_VALUES_COUNT));
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(P014_VALUES_COUNT);
      event->idx        = 4;
      success           = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (P014_I2C_ADDRESS == 0) {
        P014_I2C_ADDRESS = SI70xx_I2C_ADDRESS; // Use default address if not (yet) set
      }
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P014_data_struct());
      P014_data_struct *P014_data = static_cast<P014_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P014_data) {
        return success;
      }
      //if (P014_data->init(P014_I2C_ADDRESS, P014_RESOLUTION)) {
      //  success = true;
      //}else{
        UserVar[event->BaseVarIndex]     = NAN;
        UserVar[event->BaseVarIndex + 1] = NAN;
        UserVar[event->BaseVarIndex + 2] = NAN;
      //}
      break;
    }


    case PLUGIN_READ:
    {
      event->sensorType = static_cast<Sensor_VType>(P014_VALUES_COUNT);

      P014_data_struct *P014_data = static_cast<P014_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P014_data) {
        if (P014_data->state == P014_state::Error){
          UserVar[event->BaseVarIndex]     = NAN;
          UserVar[event->BaseVarIndex + 1] = NAN;
          UserVar[event->BaseVarIndex + 2] = NAN;

          addLog(LOG_LEVEL_ERROR,F("SI70xx: in Error!"));
        
          return false;
        }
        
        if (P014_data->state != P014_state::New_Values_Available) {
          P014_data->update(P014_I2C_ADDRESS,P014_RESOLUTION,P014_FILTER_POWER); //run the state machine
          Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + SI70xx_DELAY);//keep going until we have New_values_available

          return false; //we are not ready to read the values
        }
        
        UserVar[event->BaseVarIndex]     = P014_data->temperature / 100.0f;
        UserVar[event->BaseVarIndex + 1] = P014_data->humidity / 10.0f;
        if (P014_data->chip_id == CHIP_ID_SI7013){
          UserVar[event->BaseVarIndex + 2] = (P014_data->adc) >> P014_FILTER_POWER;
        }
        
        String log = F("P014: Temperature: ");
        log += UserVar[event->BaseVarIndex + 0];
        log += F(" Humidity: ");
        log += UserVar[event->BaseVarIndex + 1];
        if (P014_data->chip_id == CHIP_ID_SI7013){
          log += F(" ADC: ");
          log += UserVar[event->BaseVarIndex + 2];
        }
        addLog(LOG_LEVEL_INFO, log);
      
        P014_data->state = P014_state::Ready; //getting ready for another read cycle
        success = true;
      }
      break;
    }
  }

  return success;
}

#endif // USES_P014
