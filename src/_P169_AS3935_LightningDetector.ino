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
      // called when the user opens the module configuration page
      // it allows to add a new row for each output variable of the plugin
      // For plugins able to choose output types, see P026_Sysinfo.ino.
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) 
      {
        if ( i < P169_NR_OUTPUT_VALUES) 
        {
          uint8_t choice = PCONFIG(i + P169_QUERY1_CONFIG_POS);
          safe_strncpy(ExtraTaskSettings.TaskDeviceValueNames[i], p169_getQueryValueString(choice), sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
        }
        else
        {
          ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
        }
      }
      break;
    }


    case PLUGIN_SET_DEFAULTS:
    {
      // Set a default config here, which will be called when a plugin is assigned to a task.
      P169_I2C_ADDRESS = P169_I2C_ADDRESS_DFLT;
      P169_MODEL = P169_MODEL_DFLT;
      P169_QUERY1 = P169_QUERY1_DFLT;
      P169_QUERY2 = P169_QUERY2_DFLT;
      P169_QUERY3 = P169_QUERY3_DFLT;
      P169_QUERY4 = P169_QUERY4_DFLT;
      P169_MON_SCL_PIN = P169_MON_SCL_PIN_DFLT;
      P169_SEN_FIRST = 99;
      success = true;
      break;
    }


    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P169_I2C_ADDRESS_DFLT;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS


    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { P169_I2C_ADDRESS_DFLT };
      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) 
      {
        if (P169_SEN_FIRST == event->TaskIndex)                               // If first SEN, serial config available
        {
          //addFormSelectorI2C(P169_I2C_ADDRESS_LABEL, 3, i2cAddressValues, P169_I2C_ADDRESS);
          addFormSelectorI2C(F("i2c_addr"), 1, i2cAddressValues, P169_I2C_ADDRESS);
          addFormNote(F("Vindstyrka, SEN54, SEN55 default i2c address: 0x69"));
        }
      } 
      else 
      {
        success = intArrayContains(1, i2cAddressValues, event->Par1);
      }
      break;
    }


    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      if (P169_SEN_FIRST == event->TaskIndex)                               // If first SEN, serial config available
      {
        if(P169_MODEL==0)
        {
          string  = F("MonPin SCL: ");
          string += formatGpioLabel(P169_MON_SCL_PIN, false);
        }
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // this case defines what should be displayed on the web form, when this plugin is selected
      // The user's selection will be stored in
      // PCONFIG(x) (custom configuration)

      if (Plugin_169_SEN == nullptr) 
      {
        P169_SEN_FIRST = event->TaskIndex; // To detect if first SEN or not
      }

      if (P169_SEN_FIRST == event->TaskIndex)                               // If first SEN, serial config available
      {
        addHtml(F("<br><B>This SEN5x is the first. Its configuration of Pins will affect next SEN5x.</B>"));
        addHtml(F("<span style=\"color:red\"> <br><B>If several SEN5x's foreseen, don't use other pins.</B></span>"));
        
        const __FlashStringHelper *options_model[3] = { F("IKEA Vindstyrka"), F("Sensirion SEN54"), F("Sensirion SEN55")};
        
        addFormSelector(F("Model Type"), P169_MODEL_LABEL, 3, options_model, nullptr, P169_MODEL);

        if(P169_MODEL==0)
        {
          addFormPinSelect(PinSelectPurpose::Generic_input, F("MonPin SCL"), F("taskdevicepin3"), P169_MON_SCL_PIN);
          addFormNote(F("Pin for monitoring i2c communication between Vindstyrka controller and SEN5x. (Only when Model - IKEA Vindstyrka is selected.)"));
        }

        if (Plugin_169_SEN != nullptr) 
        {
          addRowLabel(F("Device info"));
          String prodname;
          String sernum;
          uint8_t firmware;
          Plugin_169_SEN->getEID(prodname, sernum, firmware);
          String txt = F("ProdName: ");
          txt += prodname;
          txt += F("  Serial Number: ");
          txt += sernum;
          txt += F("  Firmware: ");
          txt += String (firmware);
          addHtml(txt);

          addRowLabel(F("Device status"));
          txt  = F("Speed warning: ");
          txt += String((bool)Plugin_169_SEN->getStatusInfo(sensor_speed));
          txt += F(" , Auto Cleaning: ");
          txt += String((bool)Plugin_169_SEN->getStatusInfo(sensor_autoclean));
          txt += F(" , GAS Error: ");
          txt += String((bool)Plugin_169_SEN->getStatusInfo(sensor_gas));
          txt += F(" , RHT Error: ");
          txt += String((bool)Plugin_169_SEN->getStatusInfo(sensor_rht));
          txt += F(" , LASER Error: ");
          txt += String((bool)Plugin_169_SEN->getStatusInfo(sensor_laser));
          txt += F(" , FAN Error: ");
          txt += String((bool)Plugin_169_SEN->getStatusInfo(sensor_fan));
          addHtml(txt);

          addRowLabel(F("Check (pass/fail/errCode)"));
          txt  = Plugin_169_SEN->getSuccCount();
          txt += '/';
          txt += Plugin_169_SEN->getErrCount();
          txt += '/';
          txt += Plugin_169_SEN->getErrCode();
          addHtml(txt);

        }
      }
      else
      {
        addHtml(F("<br><B>This SEN5x is the NOT the first. Model and Pins config are DISABLED. Configuration is available in the first SEN5x plugin.</B>"));
        addHtml(F("<span style=\"color:red\"> <br><B>Only output value can be configured.</B></span>"));

        //looking for FIRST task Named "IKEA_Vindstyrka or Sensirion_SEN5x"
        //Cache.taskIndexName
        uint8_t allready_defined=88;
        if(P169_MODEL==0)
        {
          allready_defined=findTaskIndexByName(PLUGIN_DEFAULT_NAME_1);
        }
        else
        {
          allready_defined=findTaskIndexByName(PLUGIN_DEFAULT_NAME_2);
        }
        P169_SEN_FIRST = allready_defined;
      }

      success = true;
      break;
    }


    case PLUGIN_WEBFORM_SAVE:
    {
      // this case defines the code to be executed when the form is submitted
      // the plugin settings should be saved to PCONFIG(x)
      // ping configuration should be read from CONFIG_PIN1 and stored

      // Save output selector parameters.
      for (uint8_t i = 0; i < P169_NR_OUTPUT_VALUES; ++i) 
      {
        const uint8_t pconfigIndex = i + P169_QUERY1_CONFIG_POS;
        const uint8_t choice = PCONFIG(pconfigIndex);
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, p169_getQueryValueString(choice));
      }
      P169_MODEL           = getFormItemInt(P169_MODEL_LABEL);
      P169_I2C_ADDRESS     = P169_I2C_ADDRESS_DFLT;
      if(P169_MODEL==0)
        P169_MON_SCL_PIN     = getFormItemInt(F("taskdevicepin3"));
      P169_SEN_FIRST       = P169_SEN_FIRST;

      if (P169_SEN_FIRST == event->TaskIndex)                              // For first task set default name
      {
        if(P169_MODEL==0)
        {
          strcpy(ExtraTaskSettings.TaskDeviceName, PLUGIN_DEFAULT_NAME_1); // populate default name.
        }
        else
        {
          strcpy(ExtraTaskSettings.TaskDeviceName, PLUGIN_DEFAULT_NAME_2); // populate default name.
        }
      }
      
      Plugin_169_init = false; // Force device setup next time
      success = true;
      break;
    }


    case PLUGIN_INIT:
    {
      // this case defines code to be executed when the plugin is initialised
      // This will fail if the set to be first taskindex is no longer enabled

      if (P169_SEN_FIRST == event->TaskIndex) // If first SEN5x, config available
      {
        if (Plugin_169_SEN != nullptr) 
        {
          delete Plugin_169_SEN;
          Plugin_169_SEN = nullptr;
        }

        Plugin_169_SEN = new (std::nothrow) P169_data_struct();

        if (Plugin_169_SEN != nullptr) 
        {
          Plugin_169_SEN->setupModel(P169_MODEL);
          Plugin_169_SEN->setupDevice(P169_I2C_ADDRESS);
          if(P169_MODEL==0)
          {
            Plugin_169_SEN->setupMonPin(P169_MON_SCL_PIN);
            pinMode(P169_MON_SCL_PIN, INPUT_PULLUP);
            attachInterrupt(P169_MON_SCL_PIN, Plugin_169_interrupt, RISING);
          }
          Plugin_169_SEN->reset();
        }
      }
      
      //UserVar[event->BaseVarIndex]     = NAN;
      //UserVar[event->BaseVarIndex + 1] = NAN;
      //UserVar[event->BaseVarIndex + 2] = NAN;
      //UserVar[event->BaseVarIndex + 3] = NAN;
      UserVar.setFloat(event->BaseVarIndex, 0, NAN);
      UserVar.setFloat(event->BaseVarIndex, 1, NAN);
      UserVar.setFloat(event->BaseVarIndex, 2, NAN);
      UserVar.setFloat(event->BaseVarIndex, 3, NAN);

      success = true;
      Plugin_169_init = true;

      break;
    }


    case PLUGIN_EXIT:
    {
      if (P169_SEN_FIRST == event->TaskIndex) // If first SEN5x, config available
      {
        if (Plugin_169_SEN != nullptr)
        {
          if(P169_MODEL==0)
          {
            Plugin_169_SEN->disableInterrupt_monpin();
          }
          delete Plugin_169_SEN;
          Plugin_169_SEN = nullptr;
        }
      }

      success = true;
      break;
    }


    case PLUGIN_READ:
    {
      // code to be executed to read data
      // It is executed according to the delay configured on the device configuration page, only once

      if(event->TaskIndex!=P169_SEN_FIRST)
      {
        //All the DATA are in the first task of IKEA_Vindstyrka
        //so all you have to do is to load this data in the current taskindex data
        initPluginTaskData(event->TaskIndex, getPluginTaskData(P169_SEN_FIRST) );
      }

      if (nullptr != Plugin_169_SEN) 
      {
        if (Plugin_169_SEN->inError()) 
        {
          //UserVar[event->BaseVarIndex]     = NAN;
          //UserVar[event->BaseVarIndex + 1] = NAN;
          //UserVar[event->BaseVarIndex + 2] = NAN;
          //UserVar[event->BaseVarIndex + 3] = NAN;
          UserVar.setFloat(event->BaseVarIndex, 0, NAN);
          UserVar.setFloat(event->BaseVarIndex, 1, NAN);
          UserVar.setFloat(event->BaseVarIndex, 2, NAN);
          UserVar.setFloat(event->BaseVarIndex, 3, NAN);
          addLog(LOG_LEVEL_ERROR, F("Vindstyrka / SEN5X: in Error!"));
        }
        else
        {
          if(event->TaskIndex==P169_SEN_FIRST)
          {
            Plugin_169_SEN->startMeasurements(); // getting ready for another read cycle
          }

          //UserVar[event->BaseVarIndex]         = Plugin_169_SEN->getRequestedValue(P169_QUERY1);
          //UserVar[event->BaseVarIndex + 1]     = Plugin_169_SEN->getRequestedValue(P169_QUERY2);
          //UserVar[event->BaseVarIndex + 2]     = Plugin_169_SEN->getRequestedValue(P169_QUERY3);
          //UserVar[event->BaseVarIndex + 3]     = Plugin_169_SEN->getRequestedValue(P169_QUERY4);
          UserVar.setFloat(event->BaseVarIndex, 0, Plugin_169_SEN->getRequestedValue(P169_QUERY1));
          UserVar.setFloat(event->BaseVarIndex, 1, Plugin_169_SEN->getRequestedValue(P169_QUERY2));
          UserVar.setFloat(event->BaseVarIndex, 2, Plugin_169_SEN->getRequestedValue(P169_QUERY3));
          UserVar.setFloat(event->BaseVarIndex, 3, Plugin_169_SEN->getRequestedValue(P169_QUERY4));
        }
      }

      success = true;
      break;
    }


    case PLUGIN_ONCE_A_SECOND:
    {
      // code to be executed once a second. Tasks which do not require fast response can be added here
      success = true;
    }


    case PLUGIN_TEN_PER_SECOND:
    {
      // code to be executed 10 times per second. Tasks which require fast response can be added here
      // be careful on what is added here. Heavy processing will result in slowing the module down!
      success = true;
    }


    case PLUGIN_FIFTY_PER_SECOND:
    {
      // code to be executed 10 times per second. Tasks which require fast response can be added here
      // be careful on what is added here. Heavy processing will result in slowing the module down!
      if(event->TaskIndex==P169_SEN_FIRST)
      {
        if (nullptr != Plugin_169_SEN) 
        {
          Plugin_169_SEN->monitorSCL();    // Vind / SEN5X FSM evaluation
          Plugin_169_SEN->update();
        }
      }
      success = true;
    }
  } // switch

  return success;
}   // function


// When using interrupts we have to call the library entry point
// whenever an interrupt is triggered
void IRAM_ATTR Plugin_169_interrupt() 
{
  //addLog(LOG_LEVEL_ERROR, F("********* SEN5X: interrupt apear!"));
  if (Plugin_169_SEN) 
  {
    Plugin_169_SEN->checkPin_interrupt();
  }
}



#endif  //USES_P169