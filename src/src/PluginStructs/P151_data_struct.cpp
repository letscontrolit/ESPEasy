#include "../PluginStructs/P151_data_struct.h"

#ifdef USES_P151


P151_data_struct::~P151_data_struct() {}

bool P151_data_struct::plugin_read(struct EventStruct *event) const {
  union Honeywell_struct {
    struct {
      uint32_t status      : 2;
      uint32_t bridgeData  : 14;
      uint32_t temperature : 11;
      uint32_t dummy       : 5;
    };
    struct {
      uint8_t bytes[4];
    };
    uint32_t raw;
  } conv;

  I2Cdata_bytes data(4);

  if (!I2C_read_bytes(P151_I2C_ADDR, data)) {
    return false;
  }

  for (size_t i = 0; i < 4; ++i) {
    conv.bytes[i] = data[i];
  }

  if (conv.status != 0) {
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
