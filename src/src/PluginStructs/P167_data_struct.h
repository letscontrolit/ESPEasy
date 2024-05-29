#ifndef PLUGINSTRUCTS_P167_DATA_STRUCT_H
#define PLUGINSTRUCTS_P167_DATA_STRUCT_H


//////////////////////////////////////////////////////////////////////////////////////////////////
// P167 device class for IKEA Vindstyrka SEN54 temperature , humidity and air quality sensors
// See datasheet https://sensirion.com/media/documents/6791EFA0/62A1F68F/Sensirion_Datasheet_Environmental_Node_SEN5x.pdf
// and info about extra request https://sensirion.com/media/documents/2B6FC1F3/6409E74A/PS_AN_Read_RHT_VOC_and_NOx_RAW_signals_D1.pdf
// Based upon code from Rob Tillaart, Viktor Balint, https://github.com/RobTillaart/SHT2x
// Rewritten and adapted for ESPeasy by andibaciu and tonhuisman
// changelog in _P167_Vindstyrka.ino
//////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../_Plugin_Helper.h"
#include "../ESPEasyCore/ESPEasyGPIO.h"
#ifdef USES_P167

# ifdef LIMIT_BUILD_SIZE
#  define PLUGIN_167_DEBUG  false
# else // ifdef LIMIT_BUILD_SIZE
#  define PLUGIN_167_DEBUG  true // set to true for extra log info in the debug
# endif // ifdef LIMIT_BUILD_SIZE


// ------------------------------------------------------------------------------
# define VIND_ERR_NO_ERROR                                    0 // no error
# define VIND_ERR_CRC_ERROR                                   1 // crc error
# define VIND_ERR_WRONG_BYTES                                 2 // bytes b0,b1 or b2 wrong
# define VIND_ERR_NOT_ENOUGHT_BYTES                           3 // not enough bytes from sdm
# define VIND_ERR_TIMEOUT                                     4 // timeout
// ------------------------------------------------------------------------------

// Make accessing specific parameters more readable in the code
# define P167_ENABLE_LOG            PCONFIG(0)
# define P167_ENABLE_LOG_LABEL      F("enlg")
# define P167_MODEL                 PCONFIG(1)
# define P167_MODEL_LABEL           F("mdl")
# define P167_MON_SCL_PIN           PCONFIG(2)
# define P167_QUERY1                PCONFIG(3)
# define P167_QUERY2                PCONFIG(4)
# define P167_QUERY3                PCONFIG(5)
# define P167_QUERY4                PCONFIG(6)


# define P167_I2C_ADDRESS_DFLT      0x69
# define P167_MON_SCL_PIN_DFLT      13
# define P167_MODEL_DFLT            P167_MODEL_VINDSTYRKA // Vindstyrka or SEN54 or SEN55
# define P167_QUERY1_DFLT           0                     // Temperature (C)
# define P167_QUERY2_DFLT           1                     // Humidity (%)
# define P167_QUERY3_DFLT           5                     // PM2.5 (ug/m3)
# define P167_QUERY4_DFLT           2                     // tVOC (index)


# define P167_NR_OUTPUT_OPTIONS     9
# define P167_QUERY1_CONFIG_POS     3
# define P167_SENSOR_TYPE_INDEX     (P167_QUERY1_CONFIG_POS + VARS_PER_TASK)
# define P167_NR_OUTPUT_VALUES      getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P167_SENSOR_TYPE_INDEX)))
# define P167_MAX_ATTEMPT           3 // Number of tentative before declaring NAN value
# define P167_VALUE_COUNT           9 // Number of available values


//////////////////////////////////////////////////////////////////////////////////////////////////
// Access to the Vindstyrka device is mainly by sequencing a Final State Machine
enum class P167_state : uint8_t {
  Uninitialized = 0,          // Initial state, unknown status of sensor device
  Wait_for_reset,             // Reset being performed
  Read_firm_version,          // Reading firmware version
  Read_prod_name,             // Reading production
  Read_serial_no,             // Reading serial number
  Write_user_reg,             // Write the user register
  Initialized,                // Initialization completed
  Ready,                      // Aquisition request is pending, ready to measure
  Wait_for_start_meas,        // Start measurement started
  Wait_for_read_flag,         // Read meas flag started
  Wait_for_read_meas,         // Read meas started
  Wait_for_read_raw_meas,     // RAW Read meas started
  Wait_for_read_raw_MYS_meas, // RAW Read meas MYSTERY started
  Wait_for_read_status,       // Read status
  cmdSTARTmeas,               // send command START meas to leave SEN5x ready flag for Vindstyrka
  IDLE,                       // Sensor device in IDLE mode
  New_Values_Available,       // Acqusition finished, new data available
  Error                       // Sensor device cannot be accessed or in error
};


enum class P167_statusinfo : uint8_t {
  sensor_speed = 0,
  sensor_autoclean,
  sensor_gas,
  sensor_rht,
  sensor_laser,
  sensor_fan
};

enum class P167_model : uint8_t {
  Vindstyrka = 0u,
  SEN54      = 1u,
  SEN55      = 2u,
};


# define P167_MODEL_VINDSTYRKA  static_cast<int>(P167_model::Vindstyrka)
# define P167_MODEL_SEN54       static_cast<int>(P167_model::SEN54)
# define P167_MODEL_SEN55       static_cast<int>(P167_model::SEN55)

const __FlashStringHelper* toString(P167_model model);

const __FlashStringHelper* P167_getQueryString(uint8_t query);
const __FlashStringHelper* P167_getQueryValueString(uint8_t query);


//////////////////////////////////////////////////////////////////////////////////////////////////
// ESPeasy standard PluginTaskData structure for this plugin
struct P167_data_struct : public PluginTaskData_base {
public:

  P167_data_struct();

  ~P167_data_struct();

  void IRAM_ATTR        checkPin_interrupt(void);
  static void IRAM_ATTR Plugin_167_interrupt(P167_data_struct *self);

  /////////////////////////////////////////////////////////
  // This method runs the FSM step by step on each call
  // Returns true when a stable state is reached
  bool update();
  bool monitorSCL();

  /////////////////////////////////////////////////////////
  // (re)configure the device properties
  // This will result in resetting and reloading the device
  bool setupDevice(uint8_t i2caddr);
  bool setupModel(P167_model model);
  bool setupMonPin(int16_t monpin);
  void disableInterrupt_monpin(void);

  void setLogging(bool logStatus);

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

  bool getStatusInfo(P167_statusinfo param);

  /////////////////////////////////////////////////////////
  //  Electronic Identification Code
  //  Sensirion_Humidity_SHT2x_Electronic_Identification_Code_V1.1.pdf
  //  Electronic ID bytes
  bool getEID(String & eid_productname,
              String & eid_serialnumber,
              uint8_t& firmware) const;

  /////////////////////////////////////////////////////////
  // Temperature, humidity, DewPoint, PMxpy retrieval
  // Note: values are fetched from memory and reflect latest succesful read cycle
  float    getRequestedValue(uint8_t request) const;


  uint16_t getErrCode(bool _clear   = false); // return last errorcode (optional clear this value, default false)
  uint16_t getErrCount(bool _clear  = false); // return total errors count (optional clear this value, default false)
  uint16_t getSuccCount(bool _clear = false); // return total success count (optional clear this value, default false)
  void     clearErrCode();                    // clear last errorcode
  void     clearErrCount();                   // clear total errors count
  void     clearSuccCount();                  // clear total success count
  void     startCleaning();                   // Start a fan cleaning session.

private:

  union devicestatus
  {
    uint32_t val;
    struct
    {
      uint32_t dummy4    : 4;  // bit 0..3
      uint32_t fan       : 1;  // bit 4
      uint32_t laser     : 1;  // bit 5
      uint32_t rht       : 1;  // bit 6
      uint32_t gas       : 1;  // bit 7
      uint32_t dummy3    : 11; // bit 8..18
      uint32_t autoclean : 1;  // bit 19
      uint32_t dummy2    : 1;  // bit 20
      uint32_t speed     : 1;  // bit 21
      uint32_t dummy1    : 10; // bit 22..31
    };
  };

  devicestatus _devicestatus;
  P167_state   _state;

  bool writeCmd(uint16_t cmd);
  bool writeCmd(uint16_t cmd,
                uint8_t  value);
  bool writeCmd(uint16_t cmd,
                uint8_t  length,
                uint8_t *buffer);
  bool readBytes(uint8_t  n,
                 uint8_t *val,
                 uint8_t  maxDuration);

  bool readMeasValue();
  bool readMeasRawValue();
  bool readMeasRawMYSValue();
  bool readDataRdyFlag();
  bool readDeviceStatus();
  bool calculateValue();

  bool getProductName();
  bool getSerialNumber();
  bool getFirmwareVersion();


  float         _Humidity            = 0.0f;                   // Humidity as fetched from the device [bits]
  float         _HumidityX           = 0.0f;                   // Humidity as calculated
  float         _Temperature         = 0.0f;                   // Temperature as fetched from the device [bits]
  float         _TemperatureX        = 0.0f;                   // Temperature as calculated
  float         _DewPoint            = 0.0f;                   // DewPoint as calculated
  float         _rawHumidity         = 0.0f;                   // Humidity as fetched from the device without compensation[bits]
  float         _rawTemperature      = 0.0f;                   // Temperature as fetched from the device without compensation[bits]
  float         _mysHumidity         = 0.0f;                   // Humidity as fetched from the device without compensation[bits]
  float         _mysTemperature      = 0.0f;                   // Temperature as fetched from the device without compensation[bits]
  float         _tVOC                = 0.0f;                   // tVOC as fetched from the device[bits]
  float         _NOx                 = 0.0f;                   // NOx as fetched from the device[bits]
  float         _rawtVOC             = 0.0f;                   // tVOC as fetched from the device without compensation[bits]
  float         _rawNOx              = 0.0f;                   // NOx as fetched from the device without compensation[bits]
  float         _mysOffset           = 0.0f;                   // Temperature Offset fetched from the device[bits]
  float         _PM1p0               = 0.0f;                   // PM1.0 as fetched from the device[bits]
  float         _PM2p5               = 0.0f;                   // PM2.5 as fetched from the device[bits]
  float         _PM4p0               = 0.0f;                   // PM4.0 as fetched from the device[bits]
  float         _PM10p0              = 0.0f;                   // PM10.0 as fetched from the device[bits]
  P167_model    _model               = P167_model::Vindstyrka; // Selected sensor model
  uint8_t       _i2caddr             = 0;                      // Programmed I2C address
  uint8_t       _monpin              = 0;                      // Pin to monitor I2C SCL to find when VindStyrka finish i2c communication
  unsigned long _last_action_started = 0;                      // Timestamp for last action that takes processing time
  uint16_t      _errCount            = 0;                      // Number of errors since last successful access
  String        _eid_productname;                              // Electronic Device ID - Product Name, read at initialization
  String        _eid_serialnumber;                             // Electronic Device ID - Serial Number, read at initialization
  uint8_t       _firmware       = 0;                           // Firmware version numer, read at initialization
  uint8_t       _userreg        = 0;                           // TODO debugging only
  uint16_t      _readingerrcode = VIND_ERR_NO_ERROR;           // 4 = timeout; 3 = not enough bytes; 2 = number of bytes OK but bytes b0,b1
                                                               // or b2 wrong, 1 = crc error
  uint16_t      _readingerrcount                   = 0;        // total errors couter
  uint32_t      _readingsuccesscount               = 0;        // total success couter
  uint8_t       stepMonitoring                     = 0;        // step for Monitorin SCL pin algorithm
  bool          _errmeas                           = false;
  bool          _errmeasraw                        = false;
  bool          _errmeasrawmys                     = false;
  bool          _errdevicestatus                   = false;
  bool          startMonitoringFlag                = false; // flag to START/STOP Monitoring algorithm
  bool          statusMonitoring                   = false; // flag for status return from Monitoring algorithm
  bool          _enableLogging                     = false; // flag for enabling some technical logging
  unsigned long lastSCLLowTransitionMonitoringTime = 0;     // last time when SCL i2c pin rising

  volatile uint32_t monpinValue              = 0;
  volatile uint32_t monpinValuelast          = 0;
  volatile uint8_t  monpinChanged            = 0;
  volatile uint64_t monpinLastTransitionTime = 0;
};
#endif // USES_P167
