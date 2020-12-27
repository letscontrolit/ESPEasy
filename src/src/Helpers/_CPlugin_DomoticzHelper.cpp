#include "_CPlugin_DomoticzHelper.h"

#ifdef USES_DOMOTICZ

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../DataTypes/TaskIndex.h"

#include "../ESPEasyCore/ESPEasy_Log.h"

#include "../Globals/ExtraTaskSettings.h"

#include "../Helpers/Convert.h"
#include "../Helpers/StringConverter.h"

#include "../../ESPEasy-Globals.h"



// HUM_STAT can be one of:

// 0=Normal
// 1=Comfortable
// 2=Dry
// 3=Wet
String humStatDomoticz(struct EventStruct *event, byte rel_index) {
  userVarIndex_t userVarIndex = event->BaseVarIndex + rel_index;
  if (validTaskVarIndex(rel_index) && validUserVarIndex(userVarIndex)) {
    const int hum = UserVar[userVarIndex];

    if (hum < 30) { return formatUserVarDomoticz(2); }

    if (hum < 40) { return formatUserVarDomoticz(0); }

    if (hum < 59) { return formatUserVarDomoticz(1); }
  }
  return formatUserVarDomoticz(3);
}

int mapRSSItoDomoticz() {
  long rssi = WiFi.RSSI();

  if (-50 < rssi) { return 10; }

  if (rssi <= -98) { return 0;  }
  rssi = rssi + 97; // Range 0..47 => 1..9
  return (rssi / 5) + 1;
}

int mapVccToDomoticz() {
  #if FEATURE_ADC_VCC

  // Voltage range from 2.6V .. 3.6V => 0..100%
  if (vcc < 2.6) { return 0; }
  return (vcc - 2.6) * 100;
  #else // if FEATURE_ADC_VCC
  return 255;
  #endif // if FEATURE_ADC_VCC
}

// Format including trailing semi colon
String formatUserVarDomoticz(struct EventStruct *event, byte rel_index) {
  String text = formatUserVarNoCheck(event, rel_index);

  text += ';';
  return text;
}

String formatUserVarDomoticz(int value) {
  String text;

  text += value;
  text.trim();
  text += ';';
  return text;
}

String formatDomoticzSensorType(struct EventStruct *event) {
  String values;

  switch (event->getSensorType())
  {
    case Sensor_VType::SENSOR_TYPE_SINGLE: // single value sensor, used for Dallas, BH1750, etc
      values = formatUserVarDomoticz(event, 0);
      break;
    case Sensor_VType::SENSOR_TYPE_LONG:   // single LONG value, stored in two floats (rfid tags)
      values = (unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16);
      break;
    case Sensor_VType::SENSOR_TYPE_DUAL:   // any sensor that uses two simple values
      values  = formatUserVarDomoticz(event, 0);
      values += formatUserVarDomoticz(event, 1);
      break;
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM:

      // temp + hum + hum_stat, used for DHT11
      // http://www.domoticz.com/wiki/Domoticz_API/JSON_URL%27s#Temperature.2Fhumidity
      values  = formatUserVarDomoticz(event, 0); // TEMP = Temperature
      values += formatUserVarDomoticz(event, 1); // HUM = Humidity
      values += humStatDomoticz(event, 1);       // HUM_STAT = Humidity status
      break;
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO:

      // temp + hum + hum_stat + bar + bar_fore, used for BME280
      // http://www.domoticz.com/wiki/Domoticz_API/JSON_URL%27s#Temperature.2Fhumidity.2Fbarometer
      values  = formatUserVarDomoticz(event, 0); // TEMP = Temperature
      values += formatUserVarDomoticz(event, 1); // HUM = Humidity
      values += humStatDomoticz(event, 1);       // HUM_STAT = Humidity status
      values += formatUserVarDomoticz(event, 2); // BAR = Barometric pressure
      values += formatUserVarDomoticz(0);        // BAR_FOR = Barometer forecast
      break;
    case Sensor_VType::SENSOR_TYPE_TEMP_BARO:

      // temp + hum + hum_stat + bar + bar_fore, used for BMP085
      // http://www.domoticz.com/wiki/Domoticz_API/JSON_URL%27s#Temperature.2Fbarometer
      values  = formatUserVarDomoticz(event, 0); // TEMP = Temperature
      values += formatUserVarDomoticz(event, 1); // BAR = Barometric pressure
      values += formatUserVarDomoticz(0);        // BAR_FOR = Barometer forecast
      values += formatUserVarDomoticz(0);        // ALTITUDE= Not used at the moment, can be 0
      break;
    case Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO:

      // temp + bar + bar_fore, used for BMP280
      // http://www.domoticz.com/wiki/Domoticz_API/JSON_URL%27s#Temperature.2Fbarometer
      values  = formatUserVarDomoticz(event, 0); // TEMP = Temperature
      values += formatUserVarDomoticz(event, 2); // BAR = Barometric pressure
      values += formatUserVarDomoticz(0);        // BAR_FOR = Barometer forecast
      values += formatUserVarDomoticz(0);        // ALTITUDE= Not used at the moment, can be 0
      break;
    case Sensor_VType::SENSOR_TYPE_TRIPLE:
      values  = formatUserVarDomoticz(event, 0);
      values += formatUserVarDomoticz(event, 1);
      values += formatUserVarDomoticz(event, 2);
      break;
    case Sensor_VType::SENSOR_TYPE_QUAD:
      values  = formatUserVarDomoticz(event, 0);
      values += formatUserVarDomoticz(event, 1);
      values += formatUserVarDomoticz(event, 2);
      values += formatUserVarDomoticz(event, 3);
      break;
    case Sensor_VType::SENSOR_TYPE_WIND:

      // WindDir in degrees; WindDir as text; Wind speed average ; Wind speed gust; 0
      // http://www.domoticz.com/wiki/Domoticz_API/JSON_URL%27s#Wind
      values  = formatUserVarDomoticz(event, 0);          // WB = Wind bearing (0-359)
      values += getBearing(UserVar[event->BaseVarIndex]); // WD = Wind direction (S, SW, NNW, etc.)
      values += ";";                                      // Needed after getBearing
      // Domoticz expects the wind speed in (m/s * 10)
      values += toString((UserVar[event->BaseVarIndex + 1] * 10), ExtraTaskSettings.TaskDeviceValueDecimals[1]);
      values += ";";                                      // WS = 10 * Wind speed [m/s]
      values += toString((UserVar[event->BaseVarIndex + 2] * 10), ExtraTaskSettings.TaskDeviceValueDecimals[2]);
      values += ";";                                      // WG = 10 * Gust [m/s]
      values += formatUserVarDomoticz(0);                 // Temperature
      values += formatUserVarDomoticz(0);                 // Temperature Windchill
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
      #ifndef BUILD_NO_DEBUG
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String log = F("Domoticz Controller: Not yet implemented sensor type: ");
        log += static_cast<byte>(event->sensorType);
        log += F(" idx: ");
        log += event->idx;
        addLog(LOG_LEVEL_ERROR, log);
      }
      #endif
      break;
    }
  }

  // Now strip trailing semi colon.
  int index_last_char = values.length() - 1;

  if ((index_last_char > 0) && (values.charAt(index_last_char) == ';')) {
    values.setCharAt(index_last_char, ' ');
  }
  values.trim();
  {
    #ifndef BUILD_NO_DEBUG
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F(" Domoticz: Sensortype: ");
      log += static_cast<byte>(event->sensorType);
      log += F(" idx: ");
      log += event->idx;
      log += F(" values: ");
      log += values;
      addLog(LOG_LEVEL_INFO, log);
    }
    #endif
  }
  return values;
}
#endif // USES_DOMOTICZ
