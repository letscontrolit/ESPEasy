//////////////////////////////////////////////////////////////////////////////////////////////////
// P167 device class for IKEA Vindstyrka SEN54 temperature , humidity and air quality sensors 
// See datasheet https://sensirion.com/media/documents/6791EFA0/62A1F68F/Sensirion_Datasheet_Environmental_Node_SEN5x.pdf
// and info about extra request https://sensirion.com/media/documents/2B6FC1F3/6409E74A/PS_AN_Read_RHT_VOC_and_NOx_RAW_signals_D1.pdf
// Based upon code from Rob Tillaart, Viktor Balint, https://github.com/RobTillaart/SHT2x
// Rewritten and adapted for ESPeasy by andibaciu
// 2023-06-20 Initial version by andibaciu
//////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../_Plugin_Helper.h"
#include "../ESPEasyCore/ESPEasyGPIO.h"
#ifdef USES_P167

#ifdef LIMIT_BUILD_SIZE
#define PLUGIN_167_DEBUG  false
#else
#define PLUGIN_167_DEBUG  false        // set to true for extra log info in the debug
#endif

// Vindstyrka device properties
//#define P167_I2C_ADDRESS_DFLT      0x69


//------------------------------------------------------------------------------
#define VIND_ERR_NO_ERROR                                    0                   // no error
#define VIND_ERR_CRC_ERROR                                   1                   // crc error
#define VIND_ERR_WRONG_BYTES                                 2                   // bytes b0,b1 or b2 wrong
#define VIND_ERR_NOT_ENOUGHT_BYTES                           3                   // not enough bytes from sdm
#define VIND_ERR_TIMEOUT                                     4                   // timeout
//------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////////////
// Access to the Vindstyrka device is mainly by sequencing a Finate State Machine
enum class P167_state {
  Uninitialized = 0,            // Initial state, unknown status of sensor device
  Wait_for_reset,               // Reset being performed
  Read_firm_version,            // Reading firmware version
  Read_prod_name,               // Reading production
  Read_serial_no,               // Reading serial number
  Write_user_reg,               // Write the user register
  Initialized,                  // Initialization completed
  Ready,                        // Aquisition request is pending, ready to measure
  Wait_for_start_meas,          // Start measurement started
  Wait_for_read_flag,           // Read meas flag started
  Wait_for_read_meas,           // Read meas started
  Wait_for_read_raw_meas,       // RAW Read meas started
  Wait_for_read_raw_MYS_meas,   // RAW Read meas MYSTERY started
  Wait_for_read_status,         // Read status
  cmdSTARTmeas,                 // send command START meas to leave SEN5x ready flag for Vindstyrka
  IDLE,                         // Sensor device in IDLE mode
  New_Values_Available,         // Acqusition finished, new data available
  Error                         // Sensor device cannot be accessed or in error
};


enum param_statusinfo
{
  sensor_speed = 0,
  sensor_autoclean,
  sensor_gas,
  sensor_rht,
  sensor_laser,
  sensor_fan
};



//////////////////////////////////////////////////////////////////////////////////////////////////
// ESPeasy standard PluginTaskData structure for this plugin
//struct P167_data_struct : public PluginTaskData_base
class P167_data_struct 
{

public:
  P167_data_struct();
  //virtual ~P167_data_struct();
  ~P167_data_struct();

  void            checkPin_interrupt(void);

  /////////////////////////////////////////////////////////
  // This method runs the FSM step by step on each call
  // Returns true when a stable state is reached
  bool            update();
  bool            monitorSCL();

  /////////////////////////////////////////////////////////
  // (re)configure the device properties
  // This will result in resetting and reloading the device
  bool            setupDevice(uint8_t i2caddr);
  bool            setupModel(uint8_t model);
  bool            setupMonPin(uint8_t monpin);
  void            enableInterrupt_monpin(void);
  void            disableInterrupt_monpin(void);
  
  /////////////////////////////////////////////////////////
  //  check sensor is reachable over I2C
  bool            isConnected() const;

  /////////////////////////////////////////////////////////
  bool            newValues() const;

  /////////////////////////////////////////////////////////
  bool            inError() const;

  /////////////////////////////////////////////////////////
  // Reset the FSM to initial state
  bool            reset();

  /////////////////////////////////////////////////////////
  // Trigger a measurement cycle
  // Only perform the measurements with big interval to prevent the sensor from warming up.
  bool            startMeasurements();
  
  bool            getStatusInfo(param_statusinfo param);
  /////////////////////////////////////////////////////////
  //  Electronic Identification Code
  //  Sensirion_Humidity_SHT2x_Electronic_Identification_Code_V1.1.pdf
  //  Electronic ID bytes
  bool            getEID(String &eid_productname, String &eid_serialnumber, uint8_t &firmware) const;

  /////////////////////////////////////////////////////////
  // Temperature, humidity, DewPoint, PMxpy retrieval
  // Note: values are fetched from memory and reflect latest succesful read cycle
  float           getRequestedValue(uint8_t request) const;


  uint16_t        getErrCode(bool _clear = false);                                   //return last errorcode (optional clear this value, default false)
  uint16_t        getErrCount(bool _clear = false);                                  //return total errors count (optional clear this value, default false)
  uint16_t        getSuccCount(bool _clear = false);                                 //return total success count (optional clear this value, default false)
  void            clearErrCode();                                                    //clear last errorcode
  void            clearErrCount();                                                   //clear total errors count
  void            clearSuccCount();                                                  //clear total success count

//protected:
private:

  union devicestatus
  {
    uint32_t val;
    struct
    {
      uint16_t dummy1:10;
      bool speed;
      bool dummy2;
      bool autoclean;
      uint16_t dummy3:11;
      bool gas;
      bool rht;
      bool laser;
      bool fan;
      uint16_t dummy4:4;
    };
  };

  devicestatus    _devicestatus;
  P167_state      _state;
  
  uint8_t         crc8(const uint8_t *data, uint8_t len);
  bool            writeCmd(uint16_t cmd);
  bool            writeCmd(uint16_t cmd, uint8_t value);
  bool            writeCmd(uint16_t cmd, uint8_t length, uint8_t *buffer);
  bool            readBytes(uint8_t n, uint8_t *val, uint8_t maxDuration);

  bool            readMeasValue();
  bool            readMeasRawValue();
  bool            readMeasRawMYSValue();
  bool            readDataRdyFlag();
  bool            readDeviceStatus();
  bool            calculateValue();

  bool            getProductName();
  bool            getSerialNumber();
  bool            getFirmwareVersion();


  float           _Humidity;                              // Humidity as fetched from the device [bits]
  float           _HumidityX;                             // Humidity as calculated
  float           _Temperature;                           // Temperature as fetched from the device [bits]
  float           _TemperatureX;                          // Temperature as calculated
  float           _DewPoint;                              // DewPoint as calculated
  float           _rawHumidity;                           // Humidity as fetched from the device without compensation[bits]
  float           _rawTemperature;                        // Temperature as fetched from the device without compensation[bits]
  float           _mysHumidity;                           // Humidity as fetched from the device without compensation[bits]
  float           _mysTemperature;                        // Temperature as fetched from the device without compensation[bits]
  float           _tVOC;                                  // tVOC as fetched from the device[bits]
  float           _NOx;                                   // NOx as fetched from the device[bits]
  float           _rawtVOC;                               // tVOC as fetched from the device without compensation[bits]
  float           _rawNOx;                                // NOx as fetched from the device without compensation[bits]
  float           _mysOffset;                             // Temperature Offset fetched from the device[bits]
  float           _PM1p0;                                 // PM1.0 as fetched from the device[bits]
  float           _PM2p5;                                 // PM2.5 as fetched from the device[bits]
  float           _PM4p0;                                 // PM4.0 as fetched from the device[bits]
  float           _PM10p0;                                // PM10.0 as fetched from the device[bits]
  uint8_t         _model;                                 // Selected sensor model
  uint8_t         _i2caddr;                               // Programmed I2C address
  uint8_t         _monpin;                                // Pin to monitor I2C SCL to find when VindStyrka finish i2c communication
  unsigned long   _last_action_started;                   // Timestamp for last action that takes processing time
  uint16_t        _errCount;                              // Number of errors since last successful access
  String          _eid_productname;                       // Electronic Device ID - Product Name, read at initialization
  String          _eid_serialnumber;                      // Electronic Device ID - Serial Number, read at initialization
  uint8_t         _firmware;                              // Firmware version numer, read at initialization
  uint8_t         _userreg;                               // TODO debugging only
  uint16_t        _readingerrcode = VIND_ERR_NO_ERROR;    // 4 = timeout; 3 = not enough bytes; 2 = number of bytes OK but bytes b0,b1 or b2 wrong, 1 = crc error
  uint16_t        _readingerrcount = 0;                   // total errors couter
  uint32_t        _readingsuccesscount = 0;               // total success couter
  bool            _errmeas;
  bool            _errmeasraw;
  bool            _errmeasrawmys;
  bool            _errdevicestatus;
  uint8_t         stepMonitoring;                         // step for Monitorin SCL pin algorithm
  bool            startMonitoringFlag;                    // flag to START/STOP Monitoring algorithm
  bool            statusMonitoring;                       // flag for status return from Monitoring algorithm
  unsigned long   lastSCLLowTransitionMonitoringTime;     // last time when SCL i2c pin rising

  volatile uint32_t           monpinValue         = 0;
  volatile uint32_t           monpinValuelast     = 0;
  volatile uint8_t            monpinChanged       = 0;
  volatile uint64_t      monpinLastTransitionTime = 0;
  
};
#endif // USES_P167