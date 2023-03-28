#ifndef PLUGINSTRUCTS_P025_DATA_STRUCT_H
#define PLUGINSTRUCTS_P025_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P025

struct P025_data_struct : public PluginTaskData_base {
public:

  P025_data_struct(uint8_t i2c_addr,
                   uint8_t pga,
                   uint8_t mux);
  P025_data_struct()          = delete;
  virtual ~P025_data_struct() = default;

  bool read(int16_t& value) const;

private:

  bool readConversionRegister025(int16_t& value) const;
  bool waitReady025(unsigned long timeout_ms) const;

  uint16_t _configRegisterValue{};

  const uint8_t _i2cAddress;
};

#endif // ifdef USES_P025
#endif // ifndef PLUGINSTRUCTS_P025_DATA_STRUCT_H
