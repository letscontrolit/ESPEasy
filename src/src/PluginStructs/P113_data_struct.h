#ifndef PLUGINSTRUCTS_P113_DATA_STRUCT_H
#define PLUGINSTRUCTS_P113_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P113

# define P113_DEBUG        // Enable debugging output (INFO loglevel)
# ifndef BUILD_NO_DEBUG
#  define P113_DEBUG_DEBUG // Enable extended debugging output (DEBUG loglevel)
# endif // ifndef BUILD_NO_DEBUG

# if defined(LIMIT_BUILD_SIZE) || defined(BUILD_NO_DEBUG)
#  ifdef P113_DEBUG_DEBUG
#   undef P113_DEBUG_DEBUG
#  endif // ifdef P113_DEBUG_DEBUG
# endif // if defined(LIMIT_BUILD_SIZE) || defined(BUILD_NO_DEBUG)

# include <SparkFun_VL53L1X.h>

# define P113_I2C_ADDRESS   PCONFIG(0)
# define P113_TIMING        PCONFIG(1)
# define P113_RANGE         PCONFIG(2)
# define P113_SEND_ALWAYS   PCONFIG(3)
# define P113_DELTA         PCONFIG(4)

struct P113_data_struct : public PluginTaskData_base {
public:

  P113_data_struct(uint8_t i2c_addr,
                   int     timing,
                   bool    range);
  P113_data_struct() = delete;
  virtual ~P113_data_struct();

  bool     begin();
  bool     startRead();
  bool     readAvailable();
  uint16_t readDistance();
  uint16_t readAmbient();
  bool     isReadSuccessful();

private:

  SFEVL53L1X *sensor = nullptr;

  const uint8_t i2cAddress;
  bool          initState = false;
  const int     timing;
  const bool    range;
  bool          success    = false;
  bool          readActive = false;
  uint16_t      distance   = 0u;
};
#endif // ifdef USES_P113
#endif // ifndef PLUGINSTRUCTS_P113_DATA_STRUCT_H
