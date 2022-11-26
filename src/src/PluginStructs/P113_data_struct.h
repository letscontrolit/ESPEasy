#ifndef PLUGINSTRUCTS_P113_DATA_STRUCT_H
#define PLUGINSTRUCTS_P113_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P113

# define P113_DEBUG       // Enable debugging output (INFO loglevel)
# ifndef BUILD_NO_DEBUG
# define P113_DEBUG_DEBUG // Enable extended debugging output (DEBUG loglevel)
#endif

# ifdef LIMIT_BUILD_SIZE
  #  ifdef P113_DEBUG_DEBUG
    #   undef P113_DEBUG_DEBUG
  #  endif // ifdef P113_DEBUG_DEBUG
# endif    // ifdef LIMIT_BUILD_SIZE

# include <Wire.h>
# include <SparkFun_VL53L1X.h>

struct P113_data_struct : public PluginTaskData_base {
public:

  P113_data_struct(uint8_t i2c_addr,
                   int     timing,
                   bool    range);
  P113_data_struct() = delete;
  virtual ~P113_data_struct() = default;

  bool     begin();
  bool     startRead();
  bool     readAvailable();
  uint16_t readDistance();
  uint16_t readAmbient();
  bool     isReadSuccessful();

private:

  SFEVL53L1X sensor;

  const uint8_t  i2cAddress;
  bool     initState = false;
  const int      timing;
  const bool     range;
  bool     success    = false;
  bool     readActive = false;
  uint16_t distance = 0u;
};
#endif // ifdef USES_P113
#endif // ifndef PLUGINSTRUCTS_P113_DATA_STRUCT_H
