#ifndef PLUGINSTRUCTS_P173_DATA_STRUCT_H
#define PLUGINSTRUCTS_P173_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P173

# define P173_I2C_ADDRESS                 0x70

# define P173_CONFIG_LOW_POWER            PCONFIG(0)
# define P173_TEMPERATURE_OFFSET          PCONFIG_FLOAT(0)

# define P173_SHTC3_READ_DEVICEID         0xEFC8
# define P173_SHTC3_RESET                 0x805D
# define P173_SHTC3_WAKEUP                0x3517
# define P173_SHTC3_SLEEP                 0xB098
# define P173_SHTC3_READ_RH_T             0x7866
# define P173_SHTC3_READ_RH_T_LP          0x609C

// FIXME Not sure if we should support Clock stretching mode, commands kept for documentation
// # define P173_SHTC3_READ_RH_T_CLKSTR      0x7CA2
// # define P173_SHTC3_READ_RH_T_CLKSTR_LP   0x6458

# define P173_MAX_ERRORCOUNT          5

struct P173_data_struct : public PluginTaskData_base {
public:

  P173_data_struct(float tempOffset,
                   bool  lowPower);

  P173_data_struct() = delete;
  virtual ~P173_data_struct() {}

  bool init();

  bool plugin_read(struct EventStruct *event);
  bool isInitialized() const {
    return initialized;
  }

private:

  void softwareReset();
  void wakeup();
  void sleep();
  bool checkDeviceID();
  void writeCommand(uint16_t command);
  bool readValue(uint16_t command,
                 uint8_t  readnum,
                 uint8_t *readArray);

  uint8_t _address    = P173_I2C_ADDRESS;
  float   _tempOffset = 0.0f;
  bool    _lowPower   = false;

  float temperature = 0.0f;
  float humidity    = 0.0f;
  int   errorCount  = 0;
  bool  initialized = false;
};

#endif // ifdef USES_P173
#endif // ifndef PLUGINSTRUCTS_P173_DATA_STRUCT_H
