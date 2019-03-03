#ifdef USES_P082
//#######################################################################################################
//#################################### Plugin 082: OpenTherm Device #####################################
//#######################################################################################################

// only one boiler device is supported now
#include "OpenTherm.h"

#define PLUGIN_082
#define PLUGIN_ID_082         82
#define PLUGIN_NAME_082       "Generic - OpenTherm boiler device"

// (RW) any user specified temperature value. Even 100500 degrees :)
// can be specified via GUI or calling TaskValueSet command
#define PLUGIN_VALUENAME1_082 "TSetUser"

// (RO) boiler accepted temperature.
// Tested Bosch GAS 6000 boiler will accept:
//    0 if TSetUser < 40
//    TSetUser if TSetUser <= MaxTSet
//    MaxTSet if TSetUser > MaxTSet
//    (this logic is depend on boiler model)
// Also will return 0 if heating (CH) is disabled
#define PLUGIN_VALUENAME2_082 "TSet"

// (RO) maximum boiler temperature we can set.
// This value is specified via hardware control panel on boiler's front.
// This value will be used by boiler in the case of short-circuiting OpenTherm wires.
// Also will return 0 if heating (CH) is disabled
#define PLUGIN_VALUENAME3_082 "MaxTSet"

// boiler measured temperature of heating (CH) circuit
#define PLUGIN_VALUENAME4_082 "Tboiler"

// fault flags:
// 0 - no error
// -1 - timeout comminicating with boiler device
// -2 - wrong boiler reply
// -3 - OpenTherm protocol initialization error
// 1 - some error. Request of error code was unsuccessful
// another value - boiler fault flags returned by OpenTherm ASFflags request (ID 5)
#define PLUGIN_VALUENAME5_082 "ASFflags"

#define P82_Nchars 3 // temperature input webform digits count

enum  Plugin_082_values{
  vTSetUser,
  vTSet,
  vMaxTSet,
  vTboiler,
  vASFflags
};

OpenTherm* ot=NULL;

void Plugin_082_handleInterrupt() {
  ot->handleInterrupt();
}


boolean Plugin_082(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_082;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].VType = SENSOR_TYPE_PENTA;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].DecimalsOnly = true;
        Device[deviceCount].ValueCount = 5;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        // Device[deviceCount].TimerOptional = true;         // When taskdevice timer is not set and not optional, use default "Interval" delay (Settings.Delay)
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_082);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_082));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_082));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_082));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_082));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_082));
        break;
      }

      case PLUGIN_GET_DEVICEGPIONAMES: {
        event->String1 = formatGpioName_RX(false);
        event->String2 = formatGpioName_TX(false);
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        LoadTaskSettings(event->TaskIndex);
        String argName = F("p082_template");
        int16_t webFormValue = ExtraTaskSettings.TaskDevicePluginConfig[0];
        addFormTextBox(String(F("TSetUser")), argName, String(webFormValue), P82_Nchars);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String error;
        String argName = F("p082_template");
        char webFormValue[P82_Nchars];
        if (!safe_strncpy(webFormValue, WebServer.arg(argName), P82_Nchars)) {
          error = F("Settings save error");
        }
        if (error.length() > 0) {
          addHtmlError(error);
          success = false;
        }
        else {
          ExtraTaskSettings.TaskDevicePluginConfig[0] = atoi(webFormValue);
          SaveTaskSettings(event->TaskIndex);
          success = true;
        }
        break;
      }

    case PLUGIN_INIT:
      {
        LoadTaskSettings(event->TaskIndex);
        UserVar[event->BaseVarIndex + Plugin_082_values::vTSetUser] = ExtraTaskSettings.TaskDevicePluginConfig[0];
        int rxPin = Settings.TaskDevicePin1[event->TaskIndex];
        int txPin = Settings.TaskDevicePin2[event->TaskIndex];
        // only one boiler is supported!
        static OpenTherm otObj(rxPin, txPin);
        ot=&otObj;
        ot->begin(Plugin_082_handleInterrupt);
        String log = F("OT plugin initialized at pins ");
        log += rxPin;
        log +=F(", ");
        log += txPin;
        addLog(LOG_LEVEL_INFO,log);
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        if (ot == NULL)
          return false;
        int rxPin = Settings.TaskDevicePin1[event->TaskIndex];
        int txPin = Settings.TaskDevicePin2[event->TaskIndex];

        int16_t userTemperature = UserVar[event->BaseVarIndex + Plugin_082_values::vTSetUser];

        // enable heating if userTemperature != 0
        bool enableCentralHeating = userTemperature;
      	bool enableHotWater = true;
      	bool enableCooling = false;
        String log = F("");
        float errorCode = 0;

        unsigned int request;
        unsigned int response;
        unsigned long status = ot->setBoilerStatus(enableCentralHeating, enableHotWater, enableCooling);
      	OpenThermResponseStatus responseStatus = ot->getLastResponseStatus();
      	if (responseStatus == OpenThermResponseStatus::SUCCESS) {
          if (ot->isFault(status)){
            // request boiler for fault flags
            request = ot->buildRequest(OpenThermRequestType::READ, OpenThermMessageID::ASFflags, 0);
            response = ot->sendRequest(request) & 0xFFFF;
            // request boiler for fault flags was unsuccessful - returning error 1
            errorCode = (response == 0) ? 1 : response;
          }

          // request boiler for current heating (CH) temperature
          request = ot->buildRequest(OpenThermRequestType::READ, OpenThermMessageID::Tboiler, 0);
          UserVar[event->BaseVarIndex + Plugin_082_values::vTboiler] = ot->getTemperature(ot->sendRequest(request));

          // set userTemperature for boiler. Analyzing reply.
          request = ot->buildRequest(OpenThermRequestType::WRITE, OpenThermMessageID::TSet, ot->temperatureToData(userTemperature));
          response = ot->sendRequest(request);
          if (ot->isCentralHeatingEnabled(status)){
            UserVar[event->BaseVarIndex + Plugin_082_values::vTSet] = ot->getTemperature(response);
            // request boiler for MaxTSet
            request = ot->buildRequest(OpenThermRequestType::READ, OpenThermMessageID::MaxTSet, 0);
            UserVar[event->BaseVarIndex + Plugin_082_values::vMaxTSet] = ot->getTemperature(ot->sendRequest(request));
          }
          else {
            // heating (CH) is disabled
            UserVar[event->BaseVarIndex + Plugin_082_values::vTSet] = 0;
            UserVar[event->BaseVarIndex + Plugin_082_values::vMaxTSet] = 0;
          }
          log += F("OT device polling ok");
      	}
        else {
          // some communication errors
        	if (responseStatus == OpenThermResponseStatus::NONE) {
        		log += F("OT Error: OpenTherm is not initialized");
            errorCode = -3;
        	}
        	else if (responseStatus == OpenThermResponseStatus::INVALID) {
        		log += F("OT Error: Invalid response ");
            errorCode = -2;
        	}
        	else if (responseStatus == OpenThermResponseStatus::TIMEOUT) {
        		log += F("OT Error: Response timeout");
            errorCode = -1;
        	}
          UserVar[event->BaseVarIndex + Plugin_082_values::vTSet] = -1;
          UserVar[event->BaseVarIndex + Plugin_082_values::vTboiler] = -1;
          UserVar[event->BaseVarIndex + Plugin_082_values::vMaxTSet] = -1;
        }
        UserVar[event->BaseVarIndex + Plugin_082_values::vASFflags] = errorCode;
        log += F(" (rx pin ");
        log += rxPin;
        log += F(", tx pin ");
        log += txPin;
        log += F(")");
        addLog(LOG_LEVEL_INFO,log);

        // check if userTemperature was changed or not
        LoadTaskSettings(event->TaskIndex);
        int16_t savedUserTemperature = ExtraTaskSettings.TaskDevicePluginConfig[0];

        if (savedUserTemperature != userTemperature){
          addLog(LOG_LEVEL_INFO,F("OT userTemperature has been changed. Commiting new value."));
          ExtraTaskSettings.TaskDevicePluginConfig[0] = userTemperature;
          SaveTaskSettings(event->TaskIndex);
        }

        success = true;
        break;
      }

  }
  return success;
}
#endif // USES_P082
