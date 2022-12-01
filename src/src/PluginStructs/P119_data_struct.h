#ifndef PLUGINSTRUCTS_P119_DATA_STRUCT_H
#define PLUGINSTRUCTS_P119_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P119

// # define PLUGIN_119_DEBUG  // set to true for extra log info in the debug

# include <vector>
# include "ITG3205.h"

struct P119_data_struct : public PluginTaskData_base {
public:

  P119_data_struct(uint8_t i2c_addr,
                   bool    rawData,
                   uint8_t aSize);

  P119_data_struct() = delete;

  virtual ~P119_data_struct();

  bool read_sensor();

  bool read_data(int& X,
                 int& Y,
                 int& Z);

  bool initialized() {
    return nullptr != itg3205;
  }

private:

  ITG3205 *itg3205 = nullptr;

  bool init_sensor();

  uint8_t _i2cAddress;
  bool    _rawData;
  uint8_t _aSize;

  std::vector<int>_XA;
  std::vector<int>_YA;
  std::vector<int>_ZA;
  uint8_t         _aUsed = 0;
  uint8_t         _aMax  = 0;
};

#endif // ifdef USES_P119
#endif // ifndef PLUGINSTRUCTS_P119_DATA_STRUCT_H
