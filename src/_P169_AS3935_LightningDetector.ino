#include "_Plugin_Helper.h"
#ifdef USES_P169

// #######################################################################################################
// ########################   Plugin 169 AS3935 Lightning Detector I2C  ##################################
// #######################################################################################################

# include "./src/PluginStructs/P169_data_struct.h"

#define PLUGIN_169
#define PLUGIN_ID_169     169        
#define PLUGIN_NAME_169   "Environment - AS3935 Lightning Detector"
#define PLUGIN_VALUENAME1_169 "Distance"
#define PLUGIN_VALUENAME2_169 "Energy"
#define PLUGIN_VALUENAME3_169 "Lightning"




boolean Plugin_169(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number            = PLUGIN_ID_169;
      Device[deviceCount].Type                = DEVICE_TYPE_I2C;
      Device[deviceCount].VType               = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports               = 0;
      Device[deviceCount].PullUpOption        = false;
      Device[deviceCount].InverseLogicOption  = false;
      Device[deviceCount].FormulaOption       = true;
      Device[deviceCount].ValueCount          = 3;
      Device[deviceCount].SendDataOption      = true;
      Device[deviceCount].TimerOption         = true;
      Device[deviceCount].I2CNoDeviceCheck    = true;
      Device[deviceCount].GlobalSyncOption    = true;
      Device[deviceCount].PluginStats         = true;
      break;
    }


    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_169);
      break;
    }


    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_169));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_169));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_169));
      break;
    }


    case PLUGIN_SET_DEFAULTS:
    {
      // Set a default config here, which will be called when a plugin is assigned to a task.
      P169_I2C_ADDRESS = P169_I2C_ADDRESS_DFLT;
      P169_NOISE = AS3935MI::AS3935_NFL_2;
      P169_WATCHDOG = AS3935MI::AS3935_WDTH_2;
      P169_SPIKE_REJECTION = AS3935MI::AS3935_SREJ_2;
      P169_LIGHTNING_THRESHOLD = AS3935MI::AS3935_MNL_1;
      P169_SET_INDOOR(true);
      P169_SET_MASK_DISTURBANCE(false);
      success = true;
      break;
    }


    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P169_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS


    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x01, 0x02, 0x03 };
      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) 
      {
        //addFormSelectorI2C(P169_I2C_ADDRESS_LABEL, 3, i2cAddressValues, P169_I2C_ADDRESS);
        addFormSelectorI2C(F("i2c_addr"), NR_ELEMENTS(i2cAddressValues), i2cAddressValues, P169_I2C_ADDRESS);
        addFormNote(F("Addr: 0-0-0-0-0-A1-A0. Both A0 & A1 low is not valid."));
      } 
      else 
      {
        success = intArrayContains(NR_ELEMENTS(i2cAddressValues), i2cAddressValues, event->Par1);
      }
      break;
    }


    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      string  = F("IRQ: ");
      string += formatGpioLabel(P169_IRQ_PIN, false);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormPinSelect(PinSelectPurpose::Generic, F("IRQ"), F(P169_IRQ_PIN_LABEL), P169_IRQ_PIN);

      addFormNumericBox(F("Noise Floor Threshold"), P169_NOISE_LABEL, P169_NOISE, 0, AS3935MI::AS3935_NFL_7);
      addFormNumericBox(F("Watchdog Threshold"), P169_WATCHDOG_LABEL, P169_WATCHDOG, 0, AS3935MI::AS3935_WDTH_15);
      addFormNumericBox(F("Spike Rejection"), P169_SPIKE_REJECTION_LABEL, P169_SPIKE_REJECTION, 0, AS3935MI::AS3935_SREJ_15);

      {
        const __FlashStringHelper *options[] = { F("1"), F("5"), F("9"), F("16")};
        int optionValues[]                   = { 
          AS3935MI::AS3935_MNL_1,
          AS3935MI::AS3935_MNL_5,
          AS3935MI::AS3935_MNL_9,
          AS3935MI::AS3935_MNL_16 };
        addFormSelector(F("Lightning Threshold"), P169_LIGHTNING_THRESHOLD_LABEL, NR_ELEMENTS(optionValues), options, optionValues, P169_LIGHTNING_THRESHOLD);
        addFormNote(F("Minimum number of lightning strikes in the last 15 minutes"));
      }
      {
        const __FlashStringHelper *options[] = { F("Indoor"), F("Outdoor")};
        int optionValues[]                   = { 0, 1 };
        addFormSelector(F("Mode"), F(P169_INDOOR_LABEL), NR_ELEMENTS(optionValues), options, optionValues, P169_GET_INDOOR);
      }


      addFormCheckBox(F("Ignore Disturbance"), F(P169_MASK_DISTURBANCE_LABEL), P169_GET_MASK_DISTURBANCE);

      success = true;
      break;
    }


    case PLUGIN_WEBFORM_SAVE:
    {
      P169_I2C_ADDRESS = getFormItemInt(F("i2c_addr"));

      P169_NOISE = getFormItemInt(P169_NOISE_LABEL);
      P169_WATCHDOG = getFormItemInt(P169_WATCHDOG_LABEL);
      P169_SPIKE_REJECTION = getFormItemInt(P169_SPIKE_REJECTION_LABEL);
      P169_LIGHTNING_THRESHOLD = getFormItemInt(P169_LIGHTNING_THRESHOLD_LABEL);
      P169_SET_INDOOR(getFormItemInt(F(P169_INDOOR_LABEL)) == 0);
      P169_SET_MASK_DISTURBANCE(isFormItemChecked(F(P169_MASK_DISTURBANCE_LABEL)));
      success = true;
      break;
    }


    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P169_data_struct(event));
      P169_data_struct *P169_data = static_cast<P169_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P169_data) {
        success = P169_data->plugin_init(event);
      }


      break;
    }

    case PLUGIN_READ:
    {
      P169_data_struct *P169_data = static_cast<P169_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P169_data) {
        const int distance = P169_data->getDistance();
        UserVar.setFloat(event->TaskIndex, 0, distance);
        UserVar.setFloat(event->TaskIndex, 1, P169_data->getEnergy());
        // FIXME TD-er: Must add some counter of nr of lightning strikes per time unit

        if (distance > 0) {
          success = true;
        }
      }

      break;
    }


    case PLUGIN_TEN_PER_SECOND:
    {
      P169_data_struct *P169_data = static_cast<P169_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P169_data) {
        if (P169_data->loop()) {
          // Lightning detected, schedule PLUGIN_READ
          Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
        }
        success = true;
      }
      break;
    }


  } 

  return success;
}   // function


#endif  //USES_P169