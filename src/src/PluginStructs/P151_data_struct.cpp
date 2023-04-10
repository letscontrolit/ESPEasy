#include "../PluginStructs/P151_data_struct.h"

#ifdef USES_P151


P151_data_struct::~P151_data_struct() {}

bool P151_data_struct::plugin_read(struct EventStruct *event) const {
  union Honeywell_struct {
    struct {
      uint32_t dummy       : 5;
      uint32_t temperature : 11;
      uint32_t bridgeData  : 14;
      uint32_t status      : 2;
    };
    struct {
      uint8_t bytes[4];
    };
    uint32_t raw;
  } conv;

  I2Cdata_bytes data(4);

  if (!I2C_read_bytes(P151_I2C_ADDR, data)) {
    addLog(LOG_LEVEL_ERROR, concat(F("P151 : cannot read I2C, address "), formatToHex(P151_I2C_ADDR)));
    return false;
  }

//  String log = F("P151 : RAW data: ");
  for (size_t i = 0; i < 4; ++i) {
    conv.bytes[3-i] = data[i];
//    log += formatToHex(data[i]);
//    log += ' ';
  }
/*
  log += formatToHex(conv.raw);
  log += concat(F(" st:"), conv.status);
  log += concat(F(" br:"), conv.bridgeData);
  log += concat(F(" temp:"), conv.temperature);
  log += concat(F(" dummy:"), conv.dummy);

  addLog(LOG_LEVEL_INFO, log);
*/

  if (conv.status != 0) {
//    addLog(LOG_LEVEL_ERROR, F("P151 : conv.status != 0"));
    return false;
  }
  float pressure =
    (conv.bridgeData - P151_OUTPUT_MIN) * (P151_PRESSURE_MAX - P151_PRESSURE_MIN);

  if (P151_OUTPUT_MAX != P151_OUTPUT_MIN) {
    pressure /= (P151_OUTPUT_MAX - P151_OUTPUT_MIN);
  }
  pressure += P151_PRESSURE_MIN;

  const float temperature = ((conv.temperature * 200.0f) / 2047.0f) - 50.0f;

  UserVar[event->BaseVarIndex]     = pressure;
  UserVar[event->BaseVarIndex + 1] = temperature;

  return true;
}

bool P151_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  return true;
}

bool P151_data_struct::plugin_fifty_per_second(struct EventStruct *event) {
  return true;
}

#endif // ifdef USES_P151
