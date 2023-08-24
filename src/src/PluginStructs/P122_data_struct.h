//////////////////////////////////////////////////////////////////////////////////////////////////
// P122 device class for SHT2x temperature & humidity sensors
// See datasheet https://www.sensirion.com/products/catalog/SHT21/
// Based upon code from Rob Tillaart, Viktor Balint, https://github.com/RobTillaart/SHT2x
// Rewritten and adapted for ESPeasy by Flashmark
// 2023-04-01 Initial version by Flashmark
//////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../_Plugin_Helper.h"
#ifdef USES_P122

# ifdef LIMIT_BUILD_SIZE
#  define PLUGIN_122_DEBUG  false
# else // ifdef LIMIT_BUILD_SIZE
#  define PLUGIN_122_DEBUG  false // set to true for extra log info in the debug
# endif // ifdef LIMIT_BUILD_SIZE

// SHT2x device properties
# define P122_I2C_ADDRESS_AD0_0    0x40 // I2C address for the sensor (SHT2x address when AD0 is pulled low)
# define P122_I2C_ADDRESS_AD0_1    0x41 // I2C address for the sensor when AD0 is pulled high

// Resolution settings
# define P122_RESOLUTION_14T_12RH 0x00  // 12 bits RH / 14 bits Temp
# define P122_RESOLUTION_12T_08RH 0x01  //  8 bits RH / 12 bits Temp
# define P122_RESOLUTION_13T_10RH 0x02  // 10 bits RH / 13 bits Temp
# define P122_RESOLUTION_11T_11RH 0x03  // 11 bits RH / 11 bits Temp

//////////////////////////////////////////////////////////////////////////////////////////////////
// Access to the SHT2x device is mainly by sequencing a Finate State Machine
enum class P122_state {
  Uninitialized = 0,            // Initial state, unknown status of sensor device
  Wait_for_reset,               // Reset being performed
  Read_eid,                     // Reading electronic ID
  Write_user_reg,               // Write the user register
  Initialized,                  // Initialization completed
  Ready,                        // Aquisition request is pending, ready to measure
  Wait_for_temperature_samples, // Temperature measurement started
  Wait_for_humidity_samples,    // Humidity measurement started
  New_Values_Available,         // Acqusition finished, new data available
  Error                         // Sensor device cannot be accessed or in error
};

//////////////////////////////////////////////////////////////////////////////////////////////////
// ESPeasy standard PluginTaskData structure for this plugin
struct P122_data_struct : public PluginTaskData_base {
public:

  P122_data_struct();
  virtual ~P122_data_struct() = default;

  /////////////////////////////////////////////////////////
  // This method runs the FSM step by step on each call
  // Returns true when a stable state is reached
  bool update();

  /////////////////////////////////////////////////////////
  // (re)configure the device properties
  // This will result in resetting and reloading the device
  bool setupDevice(uint8_t i2caddr,
                   uint8_t resolution);

  /////////////////////////////////////////////////////////
  //  check sensor is reachable over I2C
  bool isConnected() const;

  /////////////////////////////////////////////////////////
  bool newValues() const;

  /////////////////////////////////////////////////////////
  bool inError() const;

  /////////////////////////////////////////////////////////
  // Reset the FSM to initial state
  bool reset();

  /////////////////////////////////////////////////////////
  // Trigger a measurement cycle
  // Only perform the measurements with big interval to prevent the sensor from warming up.
  bool startMeasurements();

  /////////////////////////////////////////////////////////
  //  Electronic Identification Code
  //  Sensirion_Humidity_SHT2x_Electronic_Identification_Code_V1.1.pdf
  //  Electronic ID bytes
  bool    getEID(uint32_t& eida,
                 uint32_t& eidb,
                 uint8_t & firmware) const;
  uint8_t getUserReg() const;

  /////////////////////////////////////////////////////////
  // Temperature and humidity retrieval
  // Note: values are fetched from memory and reflect latest succesful read cycle
  float    getTemperature() const;
  float    getHumidity() const;
  uint16_t getRawTemperature() const;
  uint16_t getRawHumidity() const;

  /////////////////////////////////////////////////////////
  //  RESOLUTION
  //
  //  table 8 SHT20 datasheet
  //  table 7 shows different timing per resolution
  //  RES     HUM       TEMP
  //   0      12 bit    14 bit
  //   1      08 bit    12 bit
  //   2      10 bit    13 bit
  //   3      11 bit    11 bit
  //
  bool    setResolution(uint8_t res = 0);
  uint8_t getResolution() const;

  /////////////////////////////////////////////////////////
  // Get the battery status from teh device
  // Note: this triggers an actual read cycle and might interfere with
  bool batteryOK();

protected:

  P122_state _state = P122_state::Uninitialized;
  uint8_t crc8(const uint8_t *data,
               uint8_t        len);
  bool    writeCmd(uint8_t cmd);
  bool    writeCmd(uint8_t cmd,
                   uint8_t value);
  bool    readBytes(uint8_t  n,
                    uint8_t *val,
                    uint8_t  maxDuration);

  unsigned long getTempDuration();
  unsigned long getHumDuration();
  bool          requestTemperature();
  bool          requestHumidity();
  bool          readValue(uint16_t& value);

  uint32_t      getEIDA();
  uint32_t      getEIDB();
  uint8_t       getFirmwareVersion();
  bool          writeUserReg();

  uint16_t      _rawHumidity         = 0; // Humidity as fetched from the device [bits]
  uint16_t      _rawTemperature      = 0; // Temperature as fetched from the device [bits]
  uint8_t       _resolution          = 0; // Programmed resolution
  uint8_t       _i2caddr             = 0; // Programmed I2C address
  unsigned long _last_action_started = 0; // Timestamp for last action that takes processing time
  uint16_t      _errCount            = 0; // Number of errors since last successful access
  uint32_t      _eida                = 0; // Electronic Device ID part EIDA, read at initialization
  uint32_t      _eidb                = 0; // Electronic Device ID part EIDB, read at initialization
  uint8_t       _firmware            = 0; // Firmware version numer, read at initialization
  uint8_t       _userreg             = 0; // TODO debugging only
};
#endif // USES_P122
