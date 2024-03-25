#include "_Plugin_Helper.h"
#ifdef USES_P167

// #######################################################################################################
// ########################   Plugin 167 IKEA Vindstyrka I2C  Sensor (SEN5x)  ############################
// #######################################################################################################
// 19-06-2023 AndiBaciu creation based upon https://github.com/RobTillaart/SHT2x

# include "./src/PluginStructs/P167_data_struct.h"

#define PLUGIN_167
#define PLUGIN_ID_167     167               // plugin id
#define PLUGIN_NAME_167   "Environment - Sensirion SEN5x (IKEA Vindstyrka)" // What will be dislpayed in the selection list
#define PLUGIN_VALUENAME1_167 "Temperature" // variable output of the plugin. The label is in quotation marks
#define PLUGIN_VALUENAME2_167 "Humidity"    // multiple outputs are supporte
#define PLUGIN_VALUENAME3_167 "tVOC"    // multiple outputs are supported
#define PLUGIN_VALUENAME4_167 "NOx"    // multiple outputs are supported
#define PLUGIN_VALUENAME5_167 "PM 1.0"    // multiple outputs are supported
#define PLUGIN_VALUENAME6_167 "PM 2.5"    // multiple outputs are supported
#define PLUGIN_VALUENAME7_167 "PM 4.0"    // multiple outputs are supported
#define PLUGIN_VALUENAME8_167 "PM 10.0"    // multiple outputs are supported
#define PLUGIN_VALUENAME9_167 "DewPoint"    // multiple outputs are supported
#define PLUGIN_DEFAULT_NAME_1    "IKEA_Vindstyrka"
#define PLUGIN_DEFAULT_NAME_2    "Sensirion_SEN5x"

//   PIN/port configuration is stored in the following:
//   CONFIG_PIN1 - The first GPIO pin selected within the task
//   CONFIG_PIN2 - The second GPIO pin selected within the task
//   CONFIG_PIN3 - The third GPIO pin selected within the task
//   CONFIG_PORT - The port in case the device has multiple in/out pins
//
//   Custom configuration is stored in the following:
//   PCONFIG(x)
//   x can be between 1 - 8 and can store values between -32767 - 32768 (16 bit)
//
//   N.B. these are aliases for a longer less readable amount of code. See _Plugin_Helper.h
//
//   PCONFIG_LABEL(x) is a function to generate a unique label used as HTML id to be able to match 
//                    returned values when saving a configuration.

// Make accessing specific parameters more readable in the code
// #define Pxxx_OUTPUT_TYPE_INDEX  2
# define P167_I2C_ADDRESS           PCONFIG(0)
# define P167_I2C_ADDRESS_LABEL     PCONFIG_LABEL(0)
# define P167_MODEL                 PCONFIG(1)
# define P167_MODEL_LABEL           PCONFIG_LABEL(1)
# define P167_MON_SCL_PIN           PCONFIG(2)
# define P167_MON_SCL_PIN_LABEL     PCONFIG_LABEL(2)
# define P167_QUERY1   					    PCONFIG(3)
# define P167_QUERY2   					    PCONFIG(4)
# define P167_QUERY3   					    PCONFIG(5)
# define P167_QUERY4   					    PCONFIG(6)
# define P167_SEN_FIRST             PCONFIG(7)
# define P167_SEN_ATTEMPT           PCONFIG_LONG(1)


# define P167_I2C_ADDRESS_DFLT      0x69
# define P167_MON_SCL_PIN_DFLT      13
# define P167_MODEL_DFLT            0  // Vindstyrka or SEN54
# define P167_QUERY1_DFLT           0  // Temperature (C)
# define P167_QUERY2_DFLT           1  // Humidity (%)
# define P167_QUERY3_DFLT           5  // PM2.5 (ug/m3)
# define P167_QUERY4_DFLT           2  // tVOC (index)


# define P167_NR_OUTPUT_VALUES      4
# define P167_NR_OUTPUT_OPTIONS     10
# define P167_QUERY1_CONFIG_POS     3
# define P167_MAX_ATTEMPT           3  // Number of tentative before declaring NAN value

//# define LIMIT_BUILD_SIZE           1

// These pointers may be used among multiple instances of the same plugin,
// as long as the same settings are used.
P167_data_struct *Plugin_167_SEN              = nullptr;
boolean Plugin_167_init                       = false;

void IRAM_ATTR Plugin_167_interrupt();

// Forward declaration helper functions
const __FlashStringHelper* p167_getQueryString(uint8_t query);
const __FlashStringHelper* p167_getQueryValueString(uint8_t query);
unsigned int               p167_getRegister(uint8_t query, uint8_t model);
float                      p167_readVal(uint8_t      query, uint8_t      node, unsigned int model);


// A plugin has to implement the following function

boolean Plugin_167(uint8_t function, struct EventStruct *event, String& string)
{
  // function: reason the plugin was called
  // event: ??add description here??
  // string: ??add description here??

  boolean success = false;

  switch (function)
  {


    case PLUGIN_DEVICE_ADD:
    {
      // This case defines the device characteristics
      Device[++deviceCount].Number            = PLUGIN_ID_167;
      Device[deviceCount].Type                = DEVICE_TYPE_I2C;
      Device[deviceCount].VType               = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports               = 0;
      Device[deviceCount].PullUpOption        = true;
      Device[deviceCount].InverseLogicOption  = false;
      Device[deviceCount].FormulaOption       = true;
      Device[deviceCount].ValueCount          = P167_NR_OUTPUT_VALUES;
      Device[deviceCount].SendDataOption      = true;
      Device[deviceCount].TimerOption         = true;
      Device[deviceCount].I2CNoDeviceCheck    = true;
      Device[deviceCount].GlobalSyncOption    = true;
      Device[deviceCount].PluginStats         = true;
      Device[deviceCount].OutputDataType      = Output_Data_type_t::Simple;
      break;
    }


    case PLUGIN_GET_DEVICENAME:
    {
      // return the device name
      string = F(PLUGIN_NAME_167);
      break;
    }


    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      // called when the user opens the module configuration page
      // it allows to add a new row for each output variable of the plugin
      // For plugins able to choose output types, see P026_Sysinfo.ino.
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) 
      {
        if ( i < P167_NR_OUTPUT_VALUES) 
        {
          uint8_t choice = PCONFIG(i + P167_QUERY1_CONFIG_POS);
          safe_strncpy(ExtraTaskSettings.TaskDeviceValueNames[i], p167_getQueryValueString(choice), sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
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
      P167_I2C_ADDRESS = P167_I2C_ADDRESS_DFLT;
      P167_MODEL = P167_MODEL_DFLT;
      P167_QUERY1 = P167_QUERY1_DFLT;
      P167_QUERY2 = P167_QUERY2_DFLT;
      P167_QUERY3 = P167_QUERY3_DFLT;
      P167_QUERY4 = P167_QUERY4_DFLT;
      P167_MON_SCL_PIN = P167_MON_SCL_PIN_DFLT;
      P167_SEN_FIRST = 99;
      success = true;
      break;
    }


    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P167_I2C_ADDRESS_DFLT;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS


    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { P167_I2C_ADDRESS_DFLT };
      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) 
      {
        if (P167_SEN_FIRST == event->TaskIndex)                               // If first SEN, serial config available
        {
          //addFormSelectorI2C(P167_I2C_ADDRESS_LABEL, 3, i2cAddressValues, P167_I2C_ADDRESS);
          addFormSelectorI2C(F("i2c_addr"), 1, i2cAddressValues, P167_I2C_ADDRESS);
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
      if (P167_SEN_FIRST == event->TaskIndex)                               // If first SEN, serial config available
      {
        if(P167_MODEL==0)
        {
          string  = F("MonPin SCL: ");
          string += formatGpioLabel(P167_MON_SCL_PIN, false);
        }
      }
      success = true;
      break;
    }


    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      const __FlashStringHelper *options[P167_NR_OUTPUT_OPTIONS];
      for (int i = 0; i < P167_NR_OUTPUT_OPTIONS; ++i) 
      {
        options[i] = p167_getQueryString(i);
      }
      for (uint8_t i = 0; i < P167_NR_OUTPUT_VALUES; ++i) 
      {
        const uint8_t pconfigIndex = i + P167_QUERY1_CONFIG_POS;
        sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, P167_NR_OUTPUT_OPTIONS, options);
      }
      addFormNote(F("NOx is available ONLY on Sensirion SEN55 model"));
      break;
    }


    case PLUGIN_WEBFORM_LOAD:
    {
      // this case defines what should be displayed on the web form, when this plugin is selected
      // The user's selection will be stored in
      // PCONFIG(x) (custom configuration)

      if (Plugin_167_SEN == nullptr) 
      {
        P167_SEN_FIRST = event->TaskIndex; // To detect if first SEN or not
      }

      if (P167_SEN_FIRST == event->TaskIndex)                               // If first SEN, serial config available
      {
        addHtml(F("<br><B>This SEN5x is the first. Its configuration of Pins will affect next SEN5x.</B>"));
        addHtml(F("<span style=\"color:red\"> <br><B>If several SEN5x's foreseen, don't use other pins.</B></span>"));
        
        const __FlashStringHelper *options_model[3] = { F("IKEA Vindstyrka"), F("Sensirion SEN54"), F("Sensirion SEN55")};
        
        addFormSelector(F("Model Type"), P167_MODEL_LABEL, 3, options_model, nullptr, P167_MODEL);

        if(P167_MODEL==0)
        {
          addFormPinSelect(PinSelectPurpose::Generic_input, F("MonPin SCL"), F("taskdevicepin3"), P167_MON_SCL_PIN);
          addFormNote(F("Pin for monitoring i2c communication between Vindstyrka controller and SEN5x. (Only when Model - IKEA Vindstyrka is selected.)"));
        }

        if (Plugin_167_SEN != nullptr) 
        {
          addRowLabel(F("Device info"));
          String prodname;
          String sernum;
          uint8_t firmware;
          Plugin_167_SEN->getEID(prodname, sernum, firmware);
          String txt = F("ProdName: ");
          txt += prodname;
          txt += F("  Serial Number: ");
          txt += sernum;
          txt += F("  Firmware: ");
          txt += String (firmware);
          addHtml(txt);

          addRowLabel(F("Device status"));
          txt  = F("Speed warning: ");
          txt += String((bool)Plugin_167_SEN->getStatusInfo(sensor_speed));
          txt += F(" , Auto Cleaning: ");
          txt += String((bool)Plugin_167_SEN->getStatusInfo(sensor_autoclean));
          txt += F(" , GAS Error: ");
          txt += String((bool)Plugin_167_SEN->getStatusInfo(sensor_gas));
          txt += F(" , RHT Error: ");
          txt += String((bool)Plugin_167_SEN->getStatusInfo(sensor_rht));
          txt += F(" , LASER Error: ");
          txt += String((bool)Plugin_167_SEN->getStatusInfo(sensor_laser));
          txt += F(" , FAN Error: ");
          txt += String((bool)Plugin_167_SEN->getStatusInfo(sensor_fan));
          addHtml(txt);

          addRowLabel(F("Check (pass/fail/errCode)"));
          txt  = Plugin_167_SEN->getSuccCount();
          txt += '/';
          txt += Plugin_167_SEN->getErrCount();
          txt += '/';
          txt += Plugin_167_SEN->getErrCode();
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
        if(P167_MODEL==0)
        {
          allready_defined=findTaskIndexByName(PLUGIN_DEFAULT_NAME_1);
        }
        else
        {
          allready_defined=findTaskIndexByName(PLUGIN_DEFAULT_NAME_2);
        }
        P167_SEN_FIRST = allready_defined;
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
      for (uint8_t i = 0; i < P167_NR_OUTPUT_VALUES; ++i) 
      {
        const uint8_t pconfigIndex = i + P167_QUERY1_CONFIG_POS;
        const uint8_t choice = PCONFIG(pconfigIndex);
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, p167_getQueryValueString(choice));
      }
      P167_MODEL           = getFormItemInt(P167_MODEL_LABEL);
      P167_I2C_ADDRESS     = P167_I2C_ADDRESS_DFLT;
      if(P167_MODEL==0)
        P167_MON_SCL_PIN     = getFormItemInt(F("taskdevicepin3"));
      P167_SEN_FIRST       = P167_SEN_FIRST;

      if (P167_SEN_FIRST == event->TaskIndex)                              // For first task set default name
      {
        if(P167_MODEL==0)
        {
          strcpy(ExtraTaskSettings.TaskDeviceName, PLUGIN_DEFAULT_NAME_1); // populate default name.
        }
        else
        {
          strcpy(ExtraTaskSettings.TaskDeviceName, PLUGIN_DEFAULT_NAME_2); // populate default name.
        }
      }
      
      Plugin_167_init = false; // Force device setup next time
      success = true;
      break;
    }


    case PLUGIN_INIT:
    {
      // this case defines code to be executed when the plugin is initialised
      // This will fail if the set to be first taskindex is no longer enabled

      if (P167_SEN_FIRST == event->TaskIndex) // If first SEN5x, config available
      {
        if (Plugin_167_SEN != nullptr) 
        {
          delete Plugin_167_SEN;
          Plugin_167_SEN = nullptr;
        }

        Plugin_167_SEN = new (std::nothrow) P167_data_struct();

        if (Plugin_167_SEN != nullptr) 
        {
          Plugin_167_SEN->setupModel(P167_MODEL);
          Plugin_167_SEN->setupDevice(P167_I2C_ADDRESS);
          if(P167_MODEL==0)
          {
            Plugin_167_SEN->setupMonPin(P167_MON_SCL_PIN);
            pinMode(P167_MON_SCL_PIN, INPUT_PULLUP);
            attachInterrupt(P167_MON_SCL_PIN, Plugin_167_interrupt, RISING);
          }
          Plugin_167_SEN->reset();
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
      Plugin_167_init = true;

      break;
    }


    case PLUGIN_EXIT:
    {
      if (P167_SEN_FIRST == event->TaskIndex) // If first SEN5x, config available
      {
        if (Plugin_167_SEN != nullptr)
        {
          if(P167_MODEL==0)
          {
            Plugin_167_SEN->disableInterrupt_monpin();
          }
          delete Plugin_167_SEN;
          Plugin_167_SEN = nullptr;
        }
      }

      success = true;
      break;
    }


    case PLUGIN_READ:
    {
      // code to be executed to read data
      // It is executed according to the delay configured on the device configuration page, only once

      if(event->TaskIndex!=P167_SEN_FIRST)
      {
        //All the DATA are in the first task of IKEA_Vindstyrka
        //so all you have to do is to load this data in the current taskindex data
        initPluginTaskData(event->TaskIndex, getPluginTaskData(P167_SEN_FIRST) );
      }

      if (nullptr != Plugin_167_SEN) 
      {
        if (Plugin_167_SEN->inError()) 
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
          if(event->TaskIndex==P167_SEN_FIRST)
          {
            Plugin_167_SEN->startMeasurements(); // getting ready for another read cycle
          }

          //UserVar[event->BaseVarIndex]         = Plugin_167_SEN->getRequestedValue(P167_QUERY1);
          //UserVar[event->BaseVarIndex + 1]     = Plugin_167_SEN->getRequestedValue(P167_QUERY2);
          //UserVar[event->BaseVarIndex + 2]     = Plugin_167_SEN->getRequestedValue(P167_QUERY3);
          //UserVar[event->BaseVarIndex + 3]     = Plugin_167_SEN->getRequestedValue(P167_QUERY4);
          UserVar.setFloat(event->BaseVarIndex, 0, Plugin_167_SEN->getRequestedValue(P167_QUERY1));
          UserVar.setFloat(event->BaseVarIndex, 1, Plugin_167_SEN->getRequestedValue(P167_QUERY2));
          UserVar.setFloat(event->BaseVarIndex, 2, Plugin_167_SEN->getRequestedValue(P167_QUERY3));
          UserVar.setFloat(event->BaseVarIndex, 3, Plugin_167_SEN->getRequestedValue(P167_QUERY4));
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
      if(event->TaskIndex==P167_SEN_FIRST)
      {
        if (nullptr != Plugin_167_SEN) 
        {
          Plugin_167_SEN->monitorSCL();    // Vind / SEN5X FSM evaluation
          Plugin_167_SEN->update();
        }
      }
      success = true;
    }
  } // switch

  return success;
}   // function



/// @brief 
/// @param query 
/// @return 
const __FlashStringHelper*  p167_getQueryString(uint8_t query) 
{
  switch(query)
  {
    case 0: return F("Temperature (C)");
    case 1: return F("Humidity (% RH)");
    case 2: return F("tVOC (VOC index)");
    case 3: return F("NOx (NOx index)");
    case 4: return F("PM 1.0 (ug/m3)");
    case 5: return F("PM 2.5 (ug/m3)");
    case 6: return F("PM 4.0 (ug/m3)");
    case 7: return F("PM 10.0 (ug/m3)");
    case 8: return F("DewPoint (C)");
  }
  return F("");
}

/// @brief 
/// @param query 
/// @return 
const __FlashStringHelper* p167_getQueryValueString(uint8_t query) 
{
  switch(query)
  {
    case 0: return F("Temperature");
    case 1: return F("Humidity");
    case 2: return F("tVOC");
    case 3: return F("NOx");
    case 4: return F("PM1p0");
    case 5: return F("PM2p5");
    case 6: return F("PM4p0");
    case 7: return F("PM10p0");
    case 8: return F("DewPoint");
  }
  return F("");
}


// When using interrupts we have to call the library entry point
// whenever an interrupt is triggered
void IRAM_ATTR Plugin_167_interrupt() 
{
  //addLog(LOG_LEVEL_ERROR, F("********* SEN5X: interrupt apear!"));
  if (Plugin_167_SEN) 
  {
    Plugin_167_SEN->checkPin_interrupt();
  }
}



#endif  //USES_P167