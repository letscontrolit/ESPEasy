#ifndef PLUGINSTRUCTS_P110_DATA_STRUCT_H
#define PLUGINSTRUCTS_P110_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P110

# define P110_INFO_LOG  // Enable debugging output (INFO loglevel)
# define P110_DEBUG_LOG // Enable extended debugging output (DEBUG loglevel)

# if defined(LIMIT_BUILD_SIZE) || defined(BUILD_NO_DEBUG)
  #  ifdef P110_DEBUG_LOG
  #    undef P110_DEBUG_LOG
  #  endif // ifdef P110_DEBUG_LOG
# endif // if defined(LIMIT_BUILD_SIZE) || defined(BUILD_NO_DEBUG)

# include <Wire.h>
# include <VL53L0X.h>

# define P110_I2C_ADDRESS PCONFIG(0)
# define P110_TIMING      PCONFIG(1)
# define P110_RANGE       PCONFIG(2)

enum class P110_initPhases : uint8_t {
  Ready     = 0x00,
  InitDelay = 0x01,
  Undefined = 0xFF
};

struct P110_data_struct : public PluginTaskData_base {
public:

  P110_data_struct(uint8_t i2c_addr,
                   int     timing,
                   bool    range);
  P110_data_struct() = delete;
  virtual ~P110_data_struct() = default;

  bool begin();
  long readDistance();
  bool isReadSuccessful();
  bool plugin_fifty_per_second();

private:

  VL53L0X sensor;

  uint8_t i2cAddress;
  int     timing;
  bool    range;

  int32_t         timeToWait = 0;
  P110_initPhases initPhase  = P110_initPhases::Undefined;
  bool            initState  = false;
  bool            success    = false;
};
#endif // ifdef USES_P110
#endif // ifndef PLUGINSTRUCTS_P110_DATA_STRUCT_H
