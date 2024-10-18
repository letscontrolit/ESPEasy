#include "_Plugin_Helper.h"
#ifdef USES_P130

// #######################################################################################################
// ############################# Plugin 130: Irms ADS1015 I2C (base : 0x48)  #############################
// #######################################################################################################


#include "src/PluginStructs/P130_data_struct.h"

#define PLUGIN_130
#define PLUGIN_ID_130 130
#define PLUGIN_NAME_130 "Current Sensor Irms - ADS1015"
#define PLUGIN_VALUENAME1_130 "current1"
#define PLUGIN_VALUENAME2_130 "power1"
#define PLUGIN_VALUENAME3_130 "current2"
#define PLUGIN_VALUENAME4_130 "power2"


boolean Plugin_130(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  // static uint8_t portValue = 0;
  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_130;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_130);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_130));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_130));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_130));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_130));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      #define ADS1015_I2C_ADDR_COUNT 4
      const uint8_t i2cAddressValues[] = { 0x48, 0x49, 0x4A, 0x4B };
      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), ADS1015_I2C_ADDR_COUNT, i2cAddressValues, PCONFIG(0));
      } else {
        success = intArrayContains(ADS1015_I2C_ADDR_COUNT, i2cAddressValues, event->Par1);
      }
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      #define ADS1015_SPS_AVAILABLE_COUNT 3
      const String spsAvailableStr[] = {String(1600), String(2400), String(3300)};
      const int spsAvailableVal[] = {P130_ADS1015_RATE_1600SPS,
                                      P130_ADS1015_RATE_2400SPS,
                                      P130_ADS1015_RATE_3300SPS};
      addFormSubHeader(F("Calibration - General"));
      addFormSelector(F("SPS"),F("p130_sps"), ADS1015_SPS_AVAILABLE_COUNT, spsAvailableStr,spsAvailableVal, PCONFIG_LONG(3));
      addFormNumericBox(F("Current Frequency"), F("p130_frequency"), PCONFIG_LONG(0), 50, 60);
      addFormNumericBox(F("Nb Sinus to read"), F("p130_nb_sinus"), PCONFIG_LONG(1), 1, 25);
      addFormNote("Take care! Reading sinus is a blocking process. With a SCT013 (60A/50A/30A - 1V), reading 2 sinus on 50Hz giving stable values.");
      addFormCheckBox(F("ADC Conversion Mode Continous"), F("p130_adc_continous"), PCONFIG_LONG(2));
      addFormCheckBox(F("Small Debug to INFO Log"), F("p130_debug"), PCONFIG(3));
      addFormFloatNumberBox(F("Voltage Estimated"), F("p130_voltageEstimated"), PCONFIG_FLOAT(0), 0.0, 380.0, 2);
      addFormSubHeader(F("Calibration"));
      addFormNumericBox(F("Canal 1 - Max Current"), F("p130_calCurrent1"), PCONFIG(1), 1, 250);
      addFormNumericBox(F("Canal 2 - Max Current"), F("p130_calCurrent2"), PCONFIG(2), 1, 250);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c_addr"));
      PCONFIG(1) = getFormItemInt(F("p130_calCurrent1"));
      PCONFIG(2) = getFormItemInt(F("p130_calCurrent2"));
      PCONFIG(3) = (true == isFormItemChecked(F("p130_debug")))?1:0;

      PCONFIG_LONG(0) = getFormItemInt(F("p130_frequency"));
      PCONFIG_LONG(1) = getFormItemInt(F("p130_nb_sinus"));
      PCONFIG_LONG(2) = (true == isFormItemChecked(F("p130_adc_continous")))?1:0;
      PCONFIG_LONG(3) = getFormItemInt(F("p130_sps"));

      PCONFIG_FLOAT(0) = getFormItemFloat(F("p130_voltageEstimated"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      const uint8_t address = PCONFIG(0);
      const uint8_t calCurrent1 = PCONFIG(1);
      const uint8_t calCurrent2 = PCONFIG(2);
      const uint8_t debug = PCONFIG(3);
      const uint8_t currentFreq = PCONFIG_LONG(0);
      const uint8_t nbSinus = PCONFIG_LONG(1);
      const uint8_t adc_continous = PCONFIG_LONG(2);
      const uint8_t sps = PCONFIG_LONG(3);
      const float_t voltageEstimated = PCONFIG_FLOAT(0);

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P130_data_struct(address, calCurrent1, calCurrent2, voltageEstimated, currentFreq, nbSinus, adc_continous, sps));
      P130_data_struct *P130_data =
        static_cast<P130_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P130_data) {
        P130_data->setDebug(debug);
        success = true;
      }
      break;
    }

    case PLUGIN_READ:
    {
      P130_data_struct *P130_data =
        static_cast<P130_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P130_data) {
        float_t value = 0.;
        P130_data->readCurrent(1, value);
        UserVar[event->BaseVarIndex+0] = value;
        UserVar[event->BaseVarIndex+1] = P130_data->estimatePower(value);
        P130_data->readCurrent(2, value);
        UserVar[event->BaseVarIndex+2] = value;
        UserVar[event->BaseVarIndex+3] = P130_data->estimatePower(value);

        {
          String log;
          log  = F("Irms - ADS1015 : Canal1{ current="); log += UserVar[event->BaseVarIndex+0];
          log += F("; power="); log += UserVar[event->BaseVarIndex+1];
          log += F(";} Canal2{ current="); log += UserVar[event->BaseVarIndex+2];
          log += F("; power=");  log += UserVar[event->BaseVarIndex+3];
          log += F(";}");
          addLog(LOG_LEVEL_INFO, log);
        }
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P130
