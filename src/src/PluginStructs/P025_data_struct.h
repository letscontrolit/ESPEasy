#ifndef PLUGINSTRUCTS_P025_DATA_STRUCT_H
#define PLUGINSTRUCTS_P025_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P025

struct P025_data_struct : public PluginTaskData_base {
public:

  P025_data_struct(uint8_t i2c_addr,
                   uint8_t _pga,
                   uint8_t _mux);
  P025_data_struct()          = delete;
  virtual ~P025_data_struct() = default;

  int16_t read() const;

private:

  uint16_t readConversionRegister025() const;
  bool     isReady025() const;

  uint8_t pga; // Gain
  uint8_t mux; // Input multiplexer
  uint8_t i2cAddress;
};

#endif // ifdef USES_P025
#endif // ifndef PLUGINSTRUCTS_P025_DATA_STRUCT_H
