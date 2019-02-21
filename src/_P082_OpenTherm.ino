#ifdef USES_P082
//#######################################################################################################
//#################################### Plugin 082: OpenTherm Device######################################
//#######################################################################################################

// поддерживается только один плагин и один котел
#include "OpenTherm.h"

#define PLUGIN_082
#define PLUGIN_ID_082         82
#define PLUGIN_NAME_082       "Generic - OpenTherm boiler device"

// (RW) заданная пользователем температура. Хоть 100500 градусов
#define PLUGIN_VALUENAME1_082 "TSetUser"

// (RO) выставленая котлом температура.
// Например, котел Bosch GAS 6000 выставляет:
//    0 если TSetUser < 40
//    TSetUser если TSetUser <= MaxTSet
//    MaxTSet если TSetUser > MaxTSet
//    (для других котлов возможно другая логика)
// Если в статусе отопление отключено - возвращаем 0.
#define PLUGIN_VALUENAME2_082 "TSet"

// (RO) ограничение максимальной температуры регулирования.
// Задается на экране котла.
// Если замкнуть контакты термостата - котел будет поддерживать это значение
// Если в статусе отопление отключено - возвращаем 0.
#define PLUGIN_VALUENAME3_082 "MaxTSet"

// температура подачи (на выходе из контура отопления)
#define PLUGIN_VALUENAME4_082 "Tboiler"

// fault flags - запрашиваем тоько если fault в статусе
// Значения ASFflags:
// 0 - нет ошибки
// -1 - timeout
// -2 - неверный ответ от котла
// -3 - ошибка инициализации протокола
// 1 - ошибка. Код ошибки запросить не удалось
// другое значение - код ошибки, возвращенный котлом на запрос ASFflags (ID 5)
#define PLUGIN_VALUENAME5_082 "ASFflags"

#define P82_Nlines 1 //количество форм для ввода данных пользователем (сейчас только TSet)
#define P82_Nchars 3 //количество символов web-формы для ввода значения пользователем

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
        addFormTextBox(String(F("Default TSetUser")), argName, String(webFormValue), P82_Nchars);
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
        // поддерживается только один плагин и один котел
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

        // обогрев включен если заданная пользователем температура != 0
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
            // запрашиваем у котла расширенный статус ошибки
            request = ot->buildRequest(OpenThermRequestType::READ, OpenThermMessageID::ASFflags, 0);
            response = ot->sendRequest(request) & 0xFFFF;
            // если не удалось получить расширенный статус - возвращаем код ошибки 1
            errorCode = (response == 0) ? 1 : response;
          }

          // запрашиваем у котла текущую температуру контура отопления
          request = ot->buildRequest(OpenThermRequestType::READ, OpenThermMessageID::Tboiler, 0);
          UserVar[event->BaseVarIndex + Plugin_082_values::vTboiler] = ot->getTemperature(ot->sendRequest(request));

          // устанавливаем в котле заданную пользователем температуру. Смотрим что ответит
          request = ot->buildRequest(OpenThermRequestType::WRITE, OpenThermMessageID::TSet, ot->temperatureToData(userTemperature));
          response = ot->sendRequest(request);
          if (ot->isCentralHeatingEnabled(status)){
            UserVar[event->BaseVarIndex + Plugin_082_values::vTSet] = ot->getTemperature(response);
            // запрашиваем у котла максимальную установку температуры
            request = ot->buildRequest(OpenThermRequestType::READ, OpenThermMessageID::MaxTSet, 0);
            UserVar[event->BaseVarIndex + Plugin_082_values::vMaxTSet] = ot->getTemperature(ot->sendRequest(request));
          }
          else {
            // Если отопление отключено
            UserVar[event->BaseVarIndex + Plugin_082_values::vTSet] = 0;
            UserVar[event->BaseVarIndex + Plugin_082_values::vMaxTSet] = 0;
          }
          log += F("OT device polling ok");
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
        }
        UserVar[event->BaseVarIndex + Plugin_082_values::vASFflags] = errorCode;
        log += F(" (rx pin ");
        log += rxPin;
        log += F(", tx pin ");
        log += txPin;
        log += F(")");
        addLog(LOG_LEVEL_INFO,log);

        success = true;
        break;
      }

  }
  return success;
}
#endif // USES_P082
