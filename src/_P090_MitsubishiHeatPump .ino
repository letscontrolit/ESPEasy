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
#define PLUGIN_VALUENAME1_090 "Room-Temperature"
#define PLUGIN_VALUENAME2_090 "Power"
#define PLUGIN_VALUENAME3_090 "Temperature"
#define PLUGIN_VALUENAME4_090 "Fan"
//#define PLUGIN_VALUENAME3_090 "Fan"
//#define PLUGIN_VALUENAME4_090 "Mode"
//#define PLUGIN_VALUENAME5_090 "Vane-Vertical"
//#define PLUGIN_VALUENAME6_090 "Vane-Horizontal"
//#define PLUGIN_VALUENAME7_090 "Set-Temperature"

//#define PLUGIN_xxx_DEBUG  false             //set to true for extra log info in the debug

struct P090_data_struct : public PluginTaskData_base {
  P090_data_struct(const int16_t serialRx, const int16_t serialTx);

  void init();
  void sync();

  bool isConnected() const;
  heatpumpSettings settings() const;
  heatpumpStatus status() const;

private:
  ESPeasySerial _serial;
  HeatPump _heatPump;
};

boolean Plugin_090(byte function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {

    case PLUGIN_DEVICE_ADD: {

      addLog(LOG_LEVEL_INFO, F("MHP - ADD"));


        Device[++deviceCount].Number = PLUGIN_ID_090;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].VType = SENSOR_TYPE_QUAD;
        //Device[deviceCount].Ports = 0;
        //Device[deviceCount].PullUpOption = false;
        //Device[deviceCount].InverseLogicOption = false;
        //Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 4;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = true;
        //Device[deviceCount].GlobalSyncOption = true;
        Device[deviceCount].DecimalsOnly = true;
        break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_090);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_090));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_090));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_090));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_090));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES: {
      serialHelper_getGpioNames(event);
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG: {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      serialHelper_webformLoad(event);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      serialHelper_webformSave(event);
      success = true;
      break;
    }

    case PLUGIN_INIT: {
      const int16_t serialRx = CONFIG_PIN1;
      const int16_t serialTx = CONFIG_PIN2;

      addLog(LOG_LEVEL_INFO, F("MHP - init"));

      P090_data_struct* heatPump = new P090_data_struct(serialRx, serialTx);
      if (heatPump == nullptr) {
        return success;
      }

      addLog(LOG_LEVEL_INFO, F("MHP - init 1"));

      initPluginTaskData(event->TaskIndex, heatPump);
      heatPump->init();
      success = true;

      break;
    }

    case PLUGIN_READ: {
      /*addLog(LOG_LEVEL_INFO, F("MHP - plugin read"));

      P090_data_struct* heatPump = static_cast<P090_data_struct *>(getPluginTaskData(event->TaskIndex));
      if (heatPump == nullptr) {
        return success;
      }

      if (heatPump->isConnected()) {
        UserVar[event->BaseVarIndex] = heatPump->roomTemperature();
        UserVar[event->BaseVarIndex + 1] = heatPump->powerSetting();
        UserVar[event->BaseVarIndex + 2] = heatPump->temperature();
        UserVar[event->BaseVarIndex + 3] = heatPump->isConnected() ? 1 : 0;  // TODO: remove
      } else {
        UserVar[event->BaseVarIndex] = NAN;
        UserVar[event->BaseVarIndex + 1] = NAN;
        UserVar[event->BaseVarIndex + 2] = NAN;
        UserVar[event->BaseVarIndex + 3] = NAN;
      }*/

      success = true;
      break;
    }

    case PLUGIN_WRITE: {
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

    case PLUGIN_EXIT: {
      clearPluginTaskData(event->TaskIndex);
      success = true;
      break;
    }

    case PLUGIN_ONCE_A_SECOND: {

      //addLog(LOG_LEVEL_INFO, F("MHP - PLUGIN_ONCE_A_SECOND"));

      P090_data_struct* heatPump = static_cast<P090_data_struct*>(getPluginTaskData(event->TaskIndex));
      if (heatPump == nullptr) {
        return success;
      }

      heatPump->sync();

      if (!heatPump->isConnected()) {
        return success;
      }

      static heatpumpSettings lastHeatpumpSettings;
      static heatpumpStatus lastHeatpumpStatus;

      heatpumpSettings settings = heatPump->settings();
      heatpumpStatus status = heatPump->status();

      if (settings != lastHeatpumpSettings || status.roomTemperature != lastHeatpumpStatus.roomTemperature) {
        lastHeatpumpSettings = settings;
        lastHeatpumpStatus = status;

        UserVar[event->BaseVarIndex] = status.roomTemperature;
        //UserVar[event->BaseVarIndex + 1] = settings.powerSetting;
        UserVar[event->BaseVarIndex + 2] = settings.temperature;
        //UserVar[event->BaseVarIndex + 3] = settings.isConnected() ? 1 : 0;  // TODO: remove

        sendData(event);
      }

      success = true;
      break;
    }
  }

  return success;
}

static void hpPacketDebug(byte* packet, unsigned int length, char* packetDirection) {
  String message = "MHP - ";
  for (unsigned int idx = 0; idx < length; idx++) {
    if (packet[idx] < 16) {
      message += "0"; // pad single hex digits with a 0
    }
    message += String(packet[idx], HEX) + " ";
  }
  addLog(LOG_LEVEL_INFO, message);
}

P090_data_struct::P090_data_struct(const int16_t serialRx, const int16_t serialTx) : _serial(serialRx, serialTx) {
  _heatPump.setPacketCallback(hpPacketDebug);
}

void P090_data_struct::init() {
  _heatPump.connect(&_serial);
}

void P090_data_struct::sync() {
  _heatPump.sync();
}

heatpumpSettings P090_data_struct::settings() const {
  return _heatPump.getSettings();
}

heatpumpStatus P090_data_struct::status() const {
  return _heatPump.getStatus();
}

bool P090_data_struct::isConnected() const {
  return _heatPump.isConnected();
}

#endif  // USES_P090
