#ifdef USES_P090

//#######################################################################################################
//################################ Plugin 090: Mitsubishi Heat Pump #####################################
//#######################################################################################################

#include <HeatPump.h>

//uncomment one of the following as needed
//#ifdef PLUGIN_BUILD_DEVELOPMENT
//#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_090
#define PLUGIN_ID_090         90
#define PLUGIN_NAME_090       "Mitsubishi Heat Pump"
#define PLUGIN_VALUENAME1_090 "Power"
#define PLUGIN_VALUENAME2_090 "Fan"
#define PLUGIN_VALUENAME3_090 "Mode"
#define PLUGIN_VALUENAME4_090 "Temperature"
#define PLUGIN_VALUENAME5_090 "Air direction (vertical)"
#define PLUGIN_VALUENAME6_090 "Air direction (horizontal)"

//#define PLUGIN_xxx_DEBUG  false             //set to true for extra log info in the debug

/*
PIN/port configuration is stored in the following:
CONFIG_PIN1 - The first GPIO pin selected within the task
CONFIG_PIN2 - The second GPIO pin selected within the task
CONFIG_PIN3 - The third GPIO pin selected within the task
CONFIG_PORT - The port in case the device has multiple in/out pins

Custom configuration is stored in the following:
PCONFIG(x)
x can be between 1 - 8 and can store values between -32767 - 32768 (16 bit)

N.B. these are aliases for a longer less readable amount of code. See _Plugin_Helper.h

*/

struct P090_data_struct : public PluginTaskData_base {
  P090_data_struct() : _heatPump(nullptr), _serial(nullptr) {}

  ~P090_data_struct() {
    reset();
  }

  void reset() {
    delete _heatPump;
    delete _serial;
    _heatPump = nullptr;
    _serial = nullptr;
  }

  bool init(const int16_t serial_rx, const int16_t serial_tx) {
    if ((serial_rx < 0) && (serial_tx < 0)) {
      return false;
    }

    reset();

    _serial = new ESPeasySerial(serial_rx, serial_tx);
    if (_serial == nullptr) {
      return false;
    }

    _heatPump = new HeatPump();
    if (_heatPump == nullptr || _heatPump->connect(_serial) == false) {
      reset();
      return false;
    }

    return true;
  }

  bool isInitialized() const {
    return _heatPump != nullptr;
  }

private:
  HeatPump *_heatPump;
  ESPeasySerial *_serial;
  /*String         sentence_part;
  uint16_t       max_length               = 550;
  uint32_t       sentences_received       = 0;
  uint32_t       sentences_received_error = 0;
  uint32_t       length_last_received     = 0;*/
};

boolean Plugin_090(byte function, struct EventStruct *event, String& string)
{
  //function: reason the plugin was called
  //event: ??add description here??
  // string: ??add description here??

  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
        //This case defines the device characteristics, edit appropriately

        Device[++deviceCount].Number = PLUGIN_ID_090;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;  //how the device is connected
        //Device[deviceCount].VType = SENSOR_TYPE_SWITCH; //type of value the plugin will return, used only for Domoticz
        //Device[deviceCount].Ports = 0;
        //Device[deviceCount].PullUpOption = false;
        //Device[deviceCount].InverseLogicOption = false;
        //Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 6;
        //Device[deviceCount].SendDataOption = false;
        //Device[deviceCount].TimerOption = false;
        //Device[deviceCount].TimerOptional = false;
        //Device[deviceCount].GlobalSyncOption = true;
        //Device[deviceCount].DecimalsOnly = true;
        break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      //return the device name
      string = F(PLUGIN_NAME_090);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      //called when the user opens the module configuration page
      //it allows to add a new row for each output variable of the plugin
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_090));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_090));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_090));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_090));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_090));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[5], PSTR(PLUGIN_VALUENAME6_090));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      serialHelper_getGpioNames(event);
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      serialHelper_webformLoad(event);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      serialHelper_webformSave(event);
      success = true;
      break;

    }
    case PLUGIN_INIT:
    {
      const int16_t serial_rx = CONFIG_PIN1;
      const int16_t serial_tx = CONFIG_PIN2;

      initPluginTaskData(event->TaskIndex, new P090_data_struct());
      P090_data_struct *P090_data = static_cast<P090_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P090_data) {
        return success;
      }

      if (P090_data->init(serial_rx, serial_tx)) {
        success = true;
        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("Mitsubishi HP: Init OK ESP GPIO-pin RX:");
          log += serial_rx;
          log += F(" TX:");
          log += serial_tx;
          addLog(LOG_LEVEL_DEBUG, log);
        }
      } else {
        clearPluginTaskData(event->TaskIndex);
      }
      break;
    }

    case PLUGIN_READ:
    {
      //code to be executed to read data
      //It is executed according to the delay configured on the device configuration page, only once

      //after the plugin has read data successfuly, set success and break
      success = true;
      break;

    }

    case PLUGIN_WRITE:
    {
      //this case defines code to be executed when the plugin executes an action (command).
      //Commands can be accessed via rules or via http.
      //As an example, http://192.168.1.12//control?cmd=dothis
      //implies that there exists the comamnd "dothis"

      /*if (plugin_not_initialised)
        break;

      // FIXME TD-er: This one is not using parseString* function
      //parse string to extract the command
      String tmpString  = string;
      int argIndex = tmpString.indexOf(',');
      if (argIndex)
        tmpString = tmpString.substring(0, argIndex);

      String tmpStr = string;
      int comma1 = tmpStr.indexOf(',');
      if (tmpString.equalsIgnoreCase(F("dothis"))) {
        //do something
        success = true;     //set to true only if plugin has executed a command successfully
      }*/

       break;
    }

	case PLUGIN_EXIT:
	{
    clearPluginTaskData(event->TaskIndex);
    success = true;
	  break;
	}

    case PLUGIN_ONCE_A_SECOND:
    {
      //code to be executed once a second. Tasks which do not require fast response can be added here

      //success = true;

    }

    case PLUGIN_TEN_PER_SECOND:
    {
      //code to be executed 10 times per second. Tasks which require fast response can be added here
      //be careful on what is added here. Heavy processing will result in slowing the module down!

      //success = true;

    }
  }   // switch
  return success;

}     //function

//implement plugin specific procedures and functions here
void pxxx_do_sth_useful()
{
  //code

}

#endif  // USES_P090
