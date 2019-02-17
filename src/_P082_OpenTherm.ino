#ifdef USES_P082
//#######################################################################################################
//#################################### Plugin 082: Dummy ################################################
//#######################################################################################################

#include "OpenTherm.h"

#define PLUGIN_082
#define PLUGIN_ID_082         82
#define PLUGIN_NAME_082       "Generic - OpenTherm boiler device"
#define PLUGIN_VALUENAME1_082 "TSet" // программная установка температуры. Изменяется программно <= MaxTSet. Если в статусе отопление отключено - возвращаем 0.
#define PLUGIN_VALUENAME2_082 "Tboiler" // температура подачи
#define PLUGIN_VALUENAME3_082 "MaxTSet" //ограничение установки температуры (RO). Задается на экране котла.
#define PLUGIN_VALUENAME4_082 "CHPressure" // давление в системе отопления, бар
#define PLUGIN_VALUENAME5_082 "ASFflags" // fault flags - запрашиваем тоько если fault в статусе
// Значения ASFflags:
// 0 - нет ошибки
// -1 - timeout
// -2 - неверный ответ от котла
// -3 - ошибка инициализации протокола
// 1 - ошибка. Код ошибки запросить не удалось
// другое значение - код ошибки, возвращенный котлом на запрос ASFflags (ID 5)

#define P82_Nlines 1 //количество форм для ввода данных пользователем (сейчас только TSet)
#define P82_Nchars 3 //количество символов web-формы для ввода значения пользователем

enum  Plugin_082_values{
  vTSet,
  vTboiler,
  vMaxTSet,
  vCHPressure,
  vASFflags
};

// TSET (если < 40 - устанавливаем в 0 и отключаем отопление)

OpenTherm ot(4,14);

void Plugin_082_handleInterrupt() {
	ot.handleInterrupt();
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
        addFormTextBox(String(F(PLUGIN_VALUENAME1_082)), argName, String(webFormValue), P82_Nchars);
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
        // LoadTaskSettings(event->TaskIndex);
        // if (Settings.TaskDevicePin1[event->TaskIndex] < 0 ) || (Settings.TaskDevicePin1[event->TaskIndex] < 0 ){
        //   break;
        // }
        // int rx =4;
        // int tx=14;
        // OpenTherm otLocal(rx, tx);
        // ot=otLocal;
        // char deviceTemplate[P82_Nlines][P82_Nchars];
        // LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
        // for (byte varNr = 0; varNr < P82_Nlines; varNr++)
        // {
        //   float temp;
        //   temp=atof(deviceTemplate[varNr]);
        //   UserVar[event->BaseVarIndex + varNr] = temp;
        // }
        ot.begin(Plugin_082_handleInterrupt);
        success = true;
        break;
      }

    case PLUGIN_READ:
      {

        LoadTaskSettings(event->TaskIndex);
        int16_t userTemperature = ExtraTaskSettings.TaskDevicePluginConfig[0];

        // обогрев включен если заданная пользователем температура != 0
        bool enableCentralHeating = userTemperature;
      	bool enableHotWater = true;
      	bool enableCooling = false;
        String log = F("*** ");
        float errorCode = 0;

        unsigned int request;
        unsigned int response;
        unsigned long status = ot.setBoilerStatus(enableCentralHeating, enableHotWater, enableCooling);
      	OpenThermResponseStatus responseStatus = ot.getLastResponseStatus();
      	if (responseStatus == OpenThermResponseStatus::SUCCESS) {
          if (ot.isFault(status)){
            // запрашиваем у котла расширенный статус ошибки
            request = ot.buildRequest(OpenThermRequestType::READ, OpenThermMessageID::ASFflags, 0);
            response = ot.sendRequest(request) & 0xFFFF;
            // если не удалось получить расширенный статус - возвращаем код ошибки 1
            errorCode = (response == 0) ? 1 : response;
          }

          // запрашиваем у котла давление в контуре отопления
          request = ot.buildRequest(OpenThermRequestType::READ, OpenThermMessageID::CHPressure, 0);
          UserVar[event->BaseVarIndex + Plugin_082_values::vCHPressure] = ot.getTemperature(ot.sendRequest(request));

          // запрашиваем у котла текущую температуру контура отопления
          request = ot.buildRequest(OpenThermRequestType::READ, OpenThermMessageID::Tboiler, 0);
          UserVar[event->BaseVarIndex + Plugin_082_values::vTboiler] = ot.getTemperature(ot.sendRequest(request));


          // устанавливаем в котле заданную температуру. Смотрим что ответит
          request = ot.buildRequest(OpenThermRequestType::WRITE, OpenThermMessageID::TSet, ot.temperatureToData(userTemperature));
          response = ot.sendRequest(request);
          if (ot.isCentralHeatingEnabled(status))
            UserVar[event->BaseVarIndex + Plugin_082_values::vTSet] = ot.getTemperature(response);
          else
            UserVar[event->BaseVarIndex + Plugin_082_values::vTSet] = 0;

          if (ot.isCentralHeatingEnabled(status)){
            // запрашиваем у котла максимальную установку температуры
            request = ot.buildRequest(OpenThermRequestType::READ, OpenThermMessageID::MaxTSet, 0);
            UserVar[event->BaseVarIndex + Plugin_082_values::vMaxTSet] = ot.getTemperature(ot.sendRequest(request));
          }
          else
            UserVar[event->BaseVarIndex + Plugin_082_values::vMaxTSet] = 0;


      	}
        else {
          // ошибка обмена данными с котлом
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
          UserVar[event->BaseVarIndex + Plugin_082_values::vTSet] = NAN;
          UserVar[event->BaseVarIndex + Plugin_082_values::vTboiler] = NAN;
          UserVar[event->BaseVarIndex + Plugin_082_values::vMaxTSet] = NAN;
          UserVar[event->BaseVarIndex + Plugin_082_values::vCHPressure] = NAN;
        }
        UserVar[event->BaseVarIndex + Plugin_082_values::vASFflags] = errorCode;
        addLog(LOG_LEVEL_INFO,log);

        // unsigned int data = ot.temperatureToData(50);
        // unsigned long req=ot.buildRequest(OpenThermRequestType::READ, OpenThermMessageID::RelModLevel, 0);
        // unsigned long res = ot.sendRequest(req);
        // Serial.println("set status: "+ String(ot.getLastResponseStatus()));
        // Serial.println("set result: " + String(res)+" "+String(res & 0x1)+" "+String(res & 0x2));
        // Serial.println("set result: " + String(ot.getTemperature(res)));


        // event->sensorType = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        // char deviceTemplate[P82_Nlines][P82_Nchars];
        // LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
        // boolean valueChanged = false;
        //
        // for (byte varNr = 0; varNr < P82_Nlines; varNr++)
        // {
        //   float ramValue = UserVar[event->BaseVarIndex + varNr];
        //   String log = F("Dummy: value ");
        //   log += varNr + 1;
        //   log += F(": ");
        //   log += ramValue;
        //
        //   float persistentValue;
        //   persistentValue = atof(deviceTemplate[varNr]);
        //
        //   if (ramValue != persistentValue){
        //     log += F(" (changed)");
        //     valueChanged = true;
        //   }
        //   addLog(LOG_LEVEL_INFO,log);
        // }
        //
        // byte keepValue = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        // if (valueChanged && keepValue){
        //   for (byte varNr = 0; varNr < P82_Nlines; varNr++)
        //   {
        //     float value=UserVar[event->BaseVarIndex + varNr];
        //       String tmp = F("");
        //     tmp += value;
        //     safe_strncpy(deviceTemplate[varNr], tmp, P82_Nchars);
        //   }
        //   SaveCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
        //
        //   String log = F("persistent values saved for task ");
        //   log += (event->TaskIndex + 1);
        //
        // }

        success = true;
        break;
      }

  }
  return success;
}
#endif // USES_P082
