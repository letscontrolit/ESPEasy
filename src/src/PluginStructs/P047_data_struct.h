#ifndef PLUGINSTRUCTS_P047_DATA_STRUCT_H
#define PLUGINSTRUCTS_P047_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P047

# ifndef P047_FEATURE_ADAFRUIT
#  ifdef LIMIT_BUILD_SIZE
#   define P047_FEATURE_ADAFRUIT 0
#  else // ifdef LIMIT_BUILD_SIZE
#   define P047_FEATURE_ADAFRUIT 1
#  endif // ifdef LIMIT_BUILD_SIZE
# endif // ifndef P047_FEATURE_ADAFRUIT

// Default I2C Address of the sensor
# define P047_CATNIP_DEFAULT_ADDR     0x20
# define P047_BEFLE_DEFAULT_ADDR      0x55
# define P047_ADAFRUIT_DEFAULT_ADDR   0x36

// Soil Moisture Sensor Register Addresses
// Catnip electronics / miceuz:
# define P047_CATNIP_GET_CAPACITANCE      0x00   // (r)     2 bytes
# define P047_CATNIP_SET_ADDRESS          0x01   //	(w)     1 uint8_t
# define P047_CATNIP_GET_ADDRESS          0x02   // (r)     1 uint8_t
# define P047_CATNIP_MEASURE_LIGHT        0x03   //	(w)     n/a
# define P047_CATNIP_GET_LIGHT            0x04   //	(r)     2 bytes
# define P047_CATNIP_GET_TEMPERATURE      0x05   //	(r)     2 bytes
# define P047_CATNIP_RESET                0x06   //	(w)     n/a
# define P047_CATNIP_GET_VERSION          0x07   //	(r)     1 bytes
# define P047_CATNIP_SLEEP                0x08   // (w)     n/a
# define P047_CATNIP_GET_BUSY             0x09   // (r)	    1 bytes

// BeFlE: (unsupported features set to 0xFF)
# define P047_BEFLE_GET_CAPACITANCE       0x76   // (r)     2 bytes
# define P047_BEFLE_SET_ADDRESS           0x41   //	(w)     1 uint8_t
# define P047_BEFLE_GET_ADDRESS           0xFF   // (r)     n/a
# define P047_BEFLE_MEASURE_LIGHT         0xFF   //	(w)     n/a
# define P047_BEFLE_GET_LIGHT             0xFF   //	(r)     n/a
# define P047_BEFLE_GET_TEMPERATURE       0x74   //	(r)     2 bytes
# define P047_BEFLE_RESET                 0xFF   //	(w)     n/a
# define P047_BEFLE_GET_VERSION           0xFF   //	(r)     n/a
# define P047_BEFLE_SLEEP                 0xFF   // (w)     n/a
# define P047_BEFLE_GET_BUSY              0xFF   // (r)	    n/a

// Adafruit I2C Capacitive Moisture Sensor
# define P047_ADAFRUIT_GET_CAPACITANCE      0x10 // (r)     2 bytes
# define P047_ADAFRUIT_SET_ADDRESS          0xFF //	(w)     n/a
# define P047_ADAFRUIT_GET_ADDRESS          0xFF // (r)     n/a
# define P047_ADAFRUIT_MEASURE_LIGHT        0xFF //	(w)     n/a
# define P047_ADAFRUIT_GET_LIGHT            0xFF //	(r)     n/a
# define P047_ADAFRUIT_GET_TEMPERATURE      0x04 //	(r)     4 bytes
# define P047_ADAFRUIT_RESET                0xFF //	(w)     n/a
# define P047_ADAFRUIT_GET_VERSION          0x02 //	(r)     4 bytes
# define P047_ADAFRUIT_SLEEP                0xFF // (w)     n/a
# define P047_ADAFRUIT_GET_BUSY             0xFF // (r)	    n/a
# define P047_ADAFRUIT_STATUS_BASE          0x00 // Adafruit SeeSaw commands
# define P047_ADAFRUIT_TOUCH_BASE           0x0F

# define P047_I2C_ADDR       PCONFIG(0)
# define P047_SENSOR_SLEEP   PCONFIG(1)
# define P047_CHECK_VERSION  PCONFIG(2)
# define P047_NEW_ADDR       PCONFIG(3)
# define P047_CHANGE_ADDR    PCONFIG(4)
# define P047_MODEL          PCONFIG(5)

// Use Staged reading
enum class P047_ReadMode : uint8_t {
  NotReading = 0,
  ReadStarted,
};

// Supported sensor models, setting is stored, so don't change values
enum class P047_SensorModels : uint8_t {
  CatnipMiceuz = 0,
  BeFlE        = 1,
  # if P047_FEATURE_ADAFRUIT
  Adafruit = 2,
  # endif // if P047_FEATURE_ADAFRUIT
};

// Shortcuts
# define P047_MODEL_CATNIP    P047_SensorModels::CatnipMiceuz
# define P047_MODEL_BEFLE     P047_SensorModels::BeFlE
# if P047_FEATURE_ADAFRUIT
#  define P047_MODEL_ADAFRUIT P047_SensorModels::Adafruit
# endif // if P047_FEATURE_ADAFRUIT

const __FlashStringHelper* toString(P047_SensorModels sensor);

struct P047_data_struct : public PluginTaskData_base {
public:

  P047_data_struct(uint8_t address,
                   uint8_t model);
  P047_data_struct()          = delete;
  virtual ~P047_data_struct() = default;

  bool plugin_read(struct EventStruct *event);

private:

  float        readTemperature();
  float        readLight();
  unsigned int readMoisture();
  uint32_t     getVersion();
  bool         changeAddress(uint8_t new_i2cAddr);
  bool         checkAddress(uint8_t new_i2cAddr);
  bool         resetSensor();
  void         setToSleep();
  void         startMeasure();

  uint8_t           _address       = 0;
  P047_SensorModels _model         = P047_MODEL_CATNIP;
  P047_ReadMode     _readMode      = P047_ReadMode::NotReading;
  uint32_t          _sensorVersion = 0;
};

#endif // ifdef USES_P047
#endif // ifndef PLUGINSTRUCTS_P047_DATA_STRUCT_H
