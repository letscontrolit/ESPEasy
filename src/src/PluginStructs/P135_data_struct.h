#ifndef PLUGINSTRUCTS_P135_DATA_STRUCT_H
#define PLUGINSTRUCTS_P135_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P135

# include <SparkFun_SCD4x_Arduino_Library.h>

# define P135_SENSOR_TYPE         PCONFIG(0)
# define P135_SENSOR_ALTITUDE     PCONFIG(1)
# define P135_AUTO_CALIBRATION    PCONFIG(2)
# define P135_MEASURE_INTERVAL    PCONFIG(3)
# define P135_MEASURE_SINGLE_SHOT PCONFIG(4)
# define P135_TEMPERATURE_OFFSET  PCONFIG_FLOAT(0)

# define P135_NORMAL_MEASURE_TIME       5000  // 5 seconds regular measuring time
# define P135_LOW_POWER_MEASURE_TIME    30000 // 30 seconds low power measuring time
# define P135_SINGLE_SHOT_MEASURE_TIME  5000  // 5 seconds measuring time for a single shot
# define P135_EXTEND_MEASURE_TIME       1000  // 1 second measuring time extension if not yet ready
# define P135_STOP_MEASUREMENT_DELAY    500   // Delay after stopping or restarting the periodic measurements

# define P135_MAX_ERRORS                100   // After this count of consecutive errors the plugin stops measuring

// # ifndef LIMIT_BUILD_SIZE                  // Only activate if really needed
# ifndef P135_FEATURE_RESET_COMMANDS
#  define P135_FEATURE_RESET_COMMANDS  1 // Enable (~1700 bytes) 'selftest', 'factoryreset' and 'setfrc' subcommands
# endif // ifndef P135_FEATURE_RESET_COMMANDS
// # else // ifndef LIMIT_BUILD_SIZE
// #  ifdef P135_FEATURE_RESET_COMMANDS
// #   undef P135_FEATURE_RESET_COMMANDS
// #  endif // ifdef P135_FEATURE_RESET_COMMANDS
// #  define P135_FEATURE_RESET_COMMANDS   0 // Explicitly disable
// # endif // ifndef LIMIT_BUILD_SIZE

# if P135_FEATURE_RESET_COMMANDS
enum class SCD4x_Operations_e : uint8_t {
  None,
  RunFactoryReset,
  RunSelfTest,
  RunForcedRecalibration,
};
# endif // if P135_FEATURE_RESET_COMMANDS
struct P135_data_struct : public PluginTaskData_base {
public:

  P135_data_struct(taskIndex_t taskIndex,
                   uint8_t     sensorType,
                   uint16_t    altitude,
                   float       tempOffset,
                   bool        autoCalibrate,
                   bool        lowPowerMeasurement,
                   bool        useSingleShot);

  P135_data_struct() = delete;
  virtual ~P135_data_struct();

  bool init();

  bool plugin_read(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    String            & string);
  bool plugin_get_config_value(struct EventStruct *event,
                               String            & string);
  bool isInitialized() const {
    return initialized;
  }

private:

  SCD4x *scd4x = nullptr;

  bool startPeriodicMeasurements();

  uint8_t  _sensorType;
  uint16_t _altitude;
  float    _tempOffset;
  bool     _autoCalibrate;
  bool     _lowPowerMeasurement;
  bool     _useSingleShot;

  char serialNumber[13] = { 0 };

  bool     initialized       = false;
  bool     singleShotStarted = false;
  bool     firstRead         = true;
  uint16_t errorCount        = 0;
  # if P135_FEATURE_RESET_COMMANDS
  String             factoryResetCode;
  SCD4x_Operations_e operation = SCD4x_Operations_e::None;
  uint16_t           frcValue  = 0;
  # endif // if P135_FEATURE_RESET_COMMANDS
};

#endif // ifdef USES_P135
#endif // ifndef PLUGINSTRUCTS_P135_DATA_STRUCT_H
