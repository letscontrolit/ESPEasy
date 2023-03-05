#ifndef PLUGINSTRUCTS_P053_DATA_STRUCT_H
#define PLUGINSTRUCTS_P053_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P053

// # define P053_LOW_LEVEL_DEBUG

# include "../Helpers/LongTermTimer.h"

# include <ESPeasySerial.h>

// Can be unset for memory-tight  builds to remove support for the PMSx003ST and PMS2003/PMS3003 sensor models
// Difference in build size is roughly 4k
# define PLUGIN_053_ENABLE_EXTRA_SENSORS

# if !defined(PLUGIN_BUILD_CUSTOM) && defined(ESP8266_1M) && defined(PLUGIN_053_ENABLE_EXTRA_SENSORS) // Turn off for 1M OTA builds
#  undef PLUGIN_053_ENABLE_EXTRA_SENSORS
# endif // if defined(ESP8266_1M) && defined(PLUGIN_BUILD_MINIMAL_OTA) && defined(PLUGIN_053_ENABLE_EXTRA_SENSORS)


// Do not change values, as they are being stored
enum class PMSx003_type {
  PMS1003_5003_7003 = 0, // PMSx003 = PMS1003 / PMS5003 / PMS7003
  # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  PMS2003_3003 = 1,      // PMS2003/PMS3003
  PMS5003_S    = 2,      // PMS5003S // Not supported yet
  PMS5003_T    = 3,      // PMS5003T // Not supported yet
  PMS5003_ST   = 4       // PMS5003ST
  # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
};

const __FlashStringHelper* toString(PMSx003_type sensorType);

// Selection data type to output as task values
// Do not change values, as they are being stored
enum class PMSx003_output_selection {
  Particles_ug_m3                          = 0, // Particles pm1.0/pm2.5/pm10
  PM2_5_TempHum_Formaldehyde               = 1, // pm2.5/Temp/Hum/HCHO
  ParticlesCount_100ml_cnt1_0_cnt2_5_cnt10 = 2, // cnt1.0/cnt2.5/cnt5.0/cnt10
  ParticlesCount_100ml_cnt0_3__cnt_2_5     = 3  // cnt0.3/cnt0.5/cnt1.0/cnt2.5
};

const __FlashStringHelper* toString(PMSx003_output_selection selection);

// Selection of data type to send as events, which are not selected as task value output.
// Do not change values, as they are being stored
enum class PMSx003_event_datatype {
  Event_None                      = 0, // Events: None
  Event_PMxx_TempHum_Formaldehyde = 1, // PMxx/temp/humi/hcho
  Event_All                       = 2, // also Particle count
  Event_All_count_bins            = 3
};

const __FlashStringHelper* toString(PMSx003_event_datatype selection);


# define PLUGIN_053_SENSOR_MODEL_SELECTOR PCONFIG(0)
# define PLUGIN_053_OUTPUT_SELECTOR       PCONFIG(1)
# define PLUGIN_053_EVENT_OUT_SELECTOR    PCONFIG(2)

# define PLUGIN_053_RST_PIN               CONFIG_PIN3
# define PLUGIN_053_PWR_PIN               PCONFIG(3)

# define PLUGIN_053_SEC_IGNORE_AFTER_WAKE PCONFIG(5)

# define PLUGIN_053_DATA_PROCESSING_FLAGS PCONFIG(4)

// Bits for data processing flags
# define PLUGIN_053_SPLIT_CNT_BINS_BIT    0
# define PLUGIN_053_OVERSAMPLING_BIT      1


// Helper defines to make code a bit more readable.
# define GET_PLUGIN_053_SENSOR_MODEL_SELECTOR static_cast<PMSx003_type>(PLUGIN_053_SENSOR_MODEL_SELECTOR)
# define GET_PLUGIN_053_OUTPUT_SELECTOR       static_cast<PMSx003_output_selection>(PLUGIN_053_OUTPUT_SELECTOR)
# define GET_PLUGIN_053_EVENT_OUT_SELECTOR    static_cast<PMSx003_event_datatype>(PLUGIN_053_EVENT_OUT_SELECTOR)

# define PMSx003_SIG1 0x42
# define PMSx003_SIG2 0x4d

// Packet sizes
# define PMS1003_5003_7003_SIZE 32
# define PMS5003_S_SIZE         32
# define PMS5003_T_SIZE         32
# define PMS5003_ST_SIZE        40
# define PMS2003_3003_SIZE      24

// Use the largest possible packet size as buffer size
# define PMSx003_PACKET_BUFFER_SIZE  PMS5003_ST_SIZE


// Active mode transport protocol description
// "factory" relates to "CF=1" in the datasheet. (CF: Calibration Factory)
// Datasheet Note: CF=1 should be used in the factory environment
# define PMS_PM1_0_ug_m3_factory   0
# define PMS_PM2_5_ug_m3_factory   1
# define PMS_PM10_0_ug_m3_factory  2
# define PMS_PM1_0_ug_m3_normal    3
# define PMS_PM2_5_ug_m3_normal    4
# define PMS_PM10_0_ug_m3_normal   5
# define PMS_cnt0_3_100ml          6
# define PMS_cnt0_5_100ml          7
# define PMS_cnt1_0_100ml          8
# define PMS_cnt2_5_100ml          9
# define PMS_cnt5_0_100ml          10
# define PMS_cnt10_0_100ml         11

# define PMS_Formaldehyde_mg_m3    12
# define PMS_Temp_C                13
# define PMS_T_Temp_C              10
# define PMS_Hum_pct               14
# define PMS_T_Hum_pct             11
# define PMS_Reserved              15
# define PMS_FW_rev_error          16
# define PMS_RECEIVE_BUFFER_SIZE   ((PMSx003_PACKET_BUFFER_SIZE / 2) - 3)


struct P053_data_struct : public PluginTaskData_base {
public:

  P053_data_struct(
    taskIndex_t             TaskIndex,
    int8_t                  rxPin,
    int8_t                  txPin,
    const ESPEasySerialPort port,
    int8_t                  resetPin,
    int8_t                  pwrPin,
    PMSx003_type            sensortype,
    uint32_t                delay_read_after_wakeup_ms
    # ifdef                 PLUGIN_053_ENABLE_EXTRA_SENSORS
    ,
    bool                    oversample
    ,
    bool                    splitCntBins
    # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
    );

  P053_data_struct() = delete;

  virtual ~P053_data_struct();

  bool init();

  bool initialized() const;

private:

  void    PacketRead16(uint16_t& value,
                       uint16_t *checksum);

  void    SerialFlush();

  uint8_t packetSize() const;

public:

  bool packetAvailable();

private:

  # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  void sendEvent(taskIndex_t TaskIndex,
                 uint8_t       index);

  bool hasFormaldehyde() const;
  bool hasTempHum() const;
  # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

public:

  bool processData(struct EventStruct *event);

  // Check if there are values read.
  // If so, also send out the events and clear the averaging buffer for new samples.
  bool checkAndClearValuesReceived(struct EventStruct *event);

  bool resetSensor();

  bool wakeSensor();
  bool sleepSensor();

  void setActiveReadingMode();
  void setPassiveReadingMode();

private:

  void requestData();

  # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

  float getValue(uint8_t index);

  bool  getValue(uint8_t index,
                 float & value);

  # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  void clearReceivedData();

public:

  void clearPacket();

  static const __FlashStringHelper* getEventString(uint8_t index);

  static void                       setTaskValueNames(ExtraTaskSettingsStruct& settings,
                                                      const uint8_t            indices[],
                                                      uint8_t                  nrElements,
                                                      bool                     oversample);

  static unsigned char getNrDecimals(uint8_t index,
                                     bool    oversample);

private:


  ESPeasySerial     *_easySerial = nullptr;
  uint8_t            _packet[PMSx003_PACKET_BUFFER_SIZE] = { 0 };
  uint8_t            _packetPos = 0;
  const taskIndex_t  _taskIndex  = INVALID_TASK_INDEX;
  const int8_t                  _rxPin = -1;
  const int8_t                  _txPin = -1;
  const ESPEasySerialPort _port = ESPEasySerialPort::not_set;
  const PMSx003_type _sensortype;
  # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  const bool _oversample                    = false;
  const bool _splitCntBins                  = false;
  float      _data[PMS_RECEIVE_BUFFER_SIZE] = { 0 };
  uint32_t   _value_mask                    = 0; // Keeping track of values already sent.
  # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  LongTermTimer  _last_wakeup_moment;
  const uint32_t _delay_read_after_wakeup_ms = 0;
  uint32_t       _values_received            = 0;
  uint16_t       _last_checksum              = 0; // To detect duplicate messages
  const int8_t   _resetPin                   = -1;
  const int8_t   _pwrPin                     = -1;

  bool           _activeReadingModeEnabled   = true;
};


#endif // ifdef USES_P053
#endif // ifndef PLUGINSTRUCTS_P053_DATA_STRUCT_H
