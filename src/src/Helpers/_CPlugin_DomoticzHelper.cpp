#include "../Helpers/_CPlugin_DomoticzHelper.h"

#if FEATURE_DOMOTICZ

# include "../DataStructs/ESPEasy_EventStruct.h"
# include "../DataTypes/TaskIndex.h"

# include "../ESPEasyCore/ESPEasy_Log.h"
# include "../ESPEasyCore/ESPEasyWifi.h"

# include "../Globals/Cache.h"

# include "../Helpers/Convert.h"
# include "../Helpers/StringConverter.h"

# include "../../ESPEasy-Globals.h"

# ifdef USES_C002
#  include <ArduinoJson.h>
# endif // ifdef USES_C002


// HUM_STAT can be one of:

// 0=Normal
// 1=Comfortable
// 2=Dry
// 3=Wet
int humStatDomoticz(struct EventStruct *event, uint8_t rel_index) {
  userVarIndex_t userVarIndex = event->BaseVarIndex + rel_index;

  if (validTaskVarIndex(rel_index) && validUserVarIndex(userVarIndex)) {
    const int hum = UserVar[userVarIndex];

    if (hum < 30) { return 2; }

    if (hum < 40) { return 0; }

    if (hum < 59) { return 1; }
  }
  return 3;
}

int mapRSSItoDomoticz() { 
  return GetRSSI_quality(); 
}

int mapVccToDomoticz() {
  # if FEATURE_ADC_VCC

  // Voltage range from 2.6V .. 3.6V => 0..100%
  if (vcc < 2.6f) { return 0; }
  return (vcc - 2.6f) * 100;
  # else // if FEATURE_ADC_VCC
  return 255;
  # endif // if FEATURE_ADC_VCC
}

String formatDomoticzSensorType(struct EventStruct *event) {
  String values;

  const Sensor_VType sensorType = event->getSensorType();

  if (isSimpleOutputDataType(sensorType) 
      || isIntegerOutputDataType(sensorType)
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
      || isDoubleOutputDataType(sensorType)
#endif
      ) {
    const uint8_t valueCount = getValueCountFromSensorType(sensorType);

    for (uint8_t i = 0; i < valueCount; ++i) {
      values += formatUserVarNoCheck(event, i);
      values += ';';
    }
  } else {
    switch (sensorType)
    {
      case Sensor_VType::SENSOR_TYPE_TEMP_HUM:      // temp + hum + hum_stat, used for DHT11
      case Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO: // temp + hum + hum_stat + bar + bar_fore, used for BME280
        // http://www.domoticz.com/wiki/Domoticz_API/JSON_URL%27s#Temperature.2Fhumidity
        values = strformat(
          F("%s;%s;%d;"),
          formatUserVarNoCheck(event, 0).c_str(), // TEMP = Temperature
          formatUserVarNoCheck(event, 1).c_str(), // HUM = Humidity
          humStatDomoticz(event, 1));             // HUM_STAT = Humidity status

        if (sensorType == Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO) {
          values += formatUserVarNoCheck(event, 2); // BAR = Barometric pressure
          values += F(";0;");                       // BAR_FOR = Barometer forecast
        }
        break;
      case Sensor_VType::SENSOR_TYPE_TEMP_BARO:       // temp + hum + hum_stat + bar + bar_fore, used for BMP085
      case Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO: // temp + bar + bar_fore, used for BMP280
      {
        // http://www.domoticz.com/wiki/Domoticz_API/JSON_URL%27s#Temperature.2Fbarometer
        const int baroIndex = sensorType == Sensor_VType::SENSOR_TYPE_TEMP_BARO ? 1 : 2;
        values = strformat(
          F("%s;%s;0;0;"),
          formatUserVarNoCheck(event, 0).c_str(),   // TEMP = Temperature
          formatUserVarNoCheck(event, baroIndex).c_str());  // BAR = Barometric pressure
                                                    // BAR_FOR = Barometer forecast
                                                    // ALTITUDE= Not used at the moment, can be 0
        break;
      }
      case Sensor_VType::SENSOR_TYPE_WIND:

        // WindDir in degrees; WindDir as text; Wind speed average ; Wind speed gust; 0
        // http://www.domoticz.com/wiki/Domoticz_API/JSON_URL%27s#Wind
        values = strformat(
          F("%s;%s;%d;%d;0;0;"),
          formatUserVarNoCheck(event, 0).c_str(),                   // WB = Wind bearing (0-359)
          String(getBearing(UserVar[event->BaseVarIndex])).c_str(), // WD = Wind direction (S, SW, NNW, etc.)
          static_cast<int>(UserVar[event->BaseVarIndex + 1] * 10),  // WS = 10 * Wind speed [m/s]
          static_cast<int>(UserVar[event->BaseVarIndex + 2] * 10)); // WG = 10 * Gust [m/s]
        break;
      case Sensor_VType::SENSOR_TYPE_SWITCH:
      case Sensor_VType::SENSOR_TYPE_DIMMER:

        // Too specific for HTTP/MQTT
        break;
      case Sensor_VType::SENSOR_TYPE_STRING:
        values = event->String2;
        break;
      default:
      {
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log = F("Domoticz Controller: Not yet implemented sensor type: ");
          log += static_cast<uint8_t>(event->sensorType);
          log += F(" idx: ");
          log += event->idx;
          addLogMove(LOG_LEVEL_ERROR, log);
        }
        # endif // ifndef BUILD_NO_DEBUG
        break;
      }
    }
  }

  // Now strip trailing semi colon.
  int index_last_char = values.length() - 1;

  if ((index_last_char > 0) && (values.charAt(index_last_char) == ';')) {
    values.setCharAt(index_last_char, ' ');
  }
  values.trim();
  {
    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F(" Domoticz: Sensortype: ");
      log += static_cast<uint8_t>(event->sensorType);
      log += F(" idx: ");
      log += event->idx;
      log += F(" values: ");
      log += values;
      addLogMove(LOG_LEVEL_INFO, log);
    }
    # endif // ifndef BUILD_NO_DEBUG
  }
  return values;
}

# ifdef USES_C002
#  include <ArduinoJson.h>

bool deserializeDomoticzJson(const String& json,
                             unsigned int& idx, float& nvalue, long& nvaluealt,
                             String& svalue1, String& switchtype) {
  uint16_t jsonlength = 512;

  DynamicJsonDocument root(jsonlength);

  deserializeJson(root, json);

  if (root.isNull()) {
    return false;
  }

  // Use long here as intermediate object type to prevent ArduinoJSON from adding a new template variant to the code.
  const long idx_long = root[F("idx")];

  idx       = idx_long;
  nvalue    = root[F("nvalue")];
  nvaluealt = root[F("nvalue")];

  // const char* name = root["name"]; // Not used
  // const char* svalue = root["svalue"]; // Not used
  const char *svalue1_c = root[F("svalue1")];

  if (svalue1_c != nullptr) {
    svalue1 = svalue1_c;
  }

  // const char* svalue2 = root["svalue2"]; // Not used
  // const char* svalue3 = root["svalue3"]; // Not used
  const char *switchtype_c = root[F("switchType")]; // Expect "On/Off" or "dimmer"

  // FIXME TD-er: Is this compare even useful?
  // nvalue is already assigned the same value as nvaluealt and not changed since.
  if (essentiallyZero(nvalue)) {
    nvalue = nvaluealt;
  }

  if (switchtype_c == nullptr) {
    switchtype = '?';
  } else {
    switchtype = switchtype_c;
  }
  return true;
}

String serializeDomoticzJson(struct EventStruct *event)
{
  String json;
  {
    json += '{';
    json += to_json_object_value(F("idx"), static_cast<int>(event->idx));
    json += ',';
    json += to_json_object_value(F("RSSI"), mapRSSItoDomoticz());
    #  if FEATURE_ADC_VCC
    json += ',';
    json += to_json_object_value(F("Battery"), mapVccToDomoticz());
    #  endif // if FEATURE_ADC_VCC

    const Sensor_VType sensorType = event->getSensorType();

    if (sensorType == Sensor_VType::SENSOR_TYPE_SWITCH ||
        sensorType == Sensor_VType::SENSOR_TYPE_DIMMER) {
      json += ',';
      json += to_json_object_value(F("command"), F("switchlight"));
      json += ',';

      const bool value_zero = essentiallyZero(UserVar[event->BaseVarIndex]);
      if (sensorType == Sensor_VType::SENSOR_TYPE_DIMMER && !value_zero)
      {
        json += to_json_object_value(F("Set%20Level"), toString(UserVar[event->BaseVarIndex], 2));
      } else {
        json += to_json_object_value(F("switchcmd"), value_zero ? F("Off") : F("On"));
      }
    } else {
      json += ',';
      json += to_json_object_value(F("nvalue"), 0);
      json += ',';
      json += to_json_object_value(F("svalue"), formatDomoticzSensorType(event), true);
    }
    json += '}';
  }

  return json;
}

# endif // ifdef USES_C002

#endif  // if FEATURE_DOMOTICZ