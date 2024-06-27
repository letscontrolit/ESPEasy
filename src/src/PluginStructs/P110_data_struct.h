#ifndef PLUGINSTRUCTS_P110_DATA_STRUCT_H
#define PLUGINSTRUCTS_P110_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P110

// # define P110_INFO_LOG  // Enable debugging output (INFO loglevel)
# define P110_DEBUG_LOG // Enable extended debugging output (DEBUG loglevel)

# if defined(LIMIT_BUILD_SIZE) || defined(BUILD_NO_DEBUG)
#  ifdef P110_DEBUG_LOG
#   undef P110_DEBUG_LOG
#  endif // ifdef P110_DEBUG_LOG
# endif  // if defined(LIMIT_BUILD_SIZE) || defined(BUILD_NO_DEBUG)

# include <VL53L0X.h>

# define P110_I2C_ADDRESS PCONFIG(0)
# define P110_TIMING      PCONFIG(1)
# define P110_RANGE       PCONFIG(2)
# define P110_SEND_ALWAYS PCONFIG(3)
# define P110_DELTA       PCONFIG(4)

# define P110_DISTANCE_UNINITIALIZED  -1
# define P110_DISTANCE_READ_TIMEOUT   -2
# define P110_DISTANCE_READ_ERROR     -3
# define P110_DISTANCE_OUT_OF_RANGE   -4
# define P110_DISTANCE_WAITING        -5


enum class P110_initPhases : uint8_t {
  Undefined       = 0xFF,
  InitDelay       = 0x00,
  Ready           = 0x01,
  WaitMeasurement = 0x02
};

struct P110_data_struct : public PluginTaskData_base {
public:

  P110_data_struct(uint8_t i2c_addr,
                   int     timing,
                   bool    range);
  P110_data_struct()          = delete;
  virtual ~P110_data_struct() = default;

  bool    begin(uint32_t interval_ms);
  int16_t readDistance();

  // Return last reading and clear the cached _distance value
  // This way we know if there was a new successful reading since last call of getDistance()
  int16_t getDistance();
  bool    isReadSuccessful() const;
  bool    plugin_fifty_per_second();

private:

  VL53L0X sensor;

  const uint8_t _i2cAddress;
  const int     _timing;
  const bool    _range;

  int16_t         _distance   = P110_DISTANCE_UNINITIALIZED;
  int32_t         _timeToWait = 0;
  P110_initPhases _initPhase  = P110_initPhases::Undefined;
};
#endif // ifdef USES_P110
#endif // ifndef PLUGINSTRUCTS_P110_DATA_STRUCT_H
