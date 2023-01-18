#ifndef PLUGINSTRUCTS_P082_DATA_STRUCT_H
#define PLUGINSTRUCTS_P082_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P082

# include <TinyGPS++.h>
# include <ESPeasySerial.h>

# ifndef BUILD_NO_DEBUG
# define P082_SEND_GPS_TO_LOG
//# define P082_USE_U_BLOX_SPECIFIC // TD-er: Disabled for now, as it is not working reliable/predictable
#endif

# define P082_TIMESTAMP_AGE       1000
# define P082_DEFAULT_FIX_TIMEOUT 2500 // TTL of fix status in ms since last update


enum class P082_query : uint8_t {
  P082_QUERY_LONG        = 0,
  P082_QUERY_LAT         = 1,
  P082_QUERY_ALT         = 2,
  P082_QUERY_SPD         = 3,
  P082_QUERY_SATVIS      = 4,
  P082_QUERY_SATUSE      = 5,
  P082_QUERY_HDOP        = 6,
  P082_QUERY_FIXQ        = 7,
  P082_QUERY_DB_MAX      = 8,
  P082_QUERY_CHKSUM_FAIL = 9,
  P082_QUERY_DISTANCE    = 10,
  P082_QUERY_DIST_REF    = 11,
  P082_NR_OUTPUT_OPTIONS
};

const __FlashStringHelper * Plugin_082_valuename(P082_query value_nr, bool displayString);

P082_query Plugin_082_from_valuename(const String& valuename);


enum class P082_PowerMode : uint8_t {
  Max_Performance = 0,
  Power_Save = 1,
  Eco = 2
};

const __FlashStringHelper* toString(P082_PowerMode mode);


enum class P082_DynamicModel : uint8_t {
  Portable    = 0,
  Stationary  = 2,
  Pedestrian  = 3,
  Automotive  = 4,
  Sea         = 5,
  Airborne_1g = 6, // airborne with <1g acceleration
  Airborne_2g = 7, // airborne with <2g acceleration
  Airborne_4g = 8, // airborne with <4g acceleration
  Wrist       = 9, // Only recommended for wrist-worn applications. Receiver will filter out armmotion (just available for protocol version > 17).
  Bike        = 10  // Used for applications with equivalent dynamics to those of a motor bike. Lowvertical acceleration assumed. (supported in protocol versions 19.2)
};

const __FlashStringHelper* toString(P082_DynamicModel model);

struct P082_data_struct : public PluginTaskData_base {

  // Enum is being stored, so don't change int values
  

  P082_data_struct();

  virtual ~P082_data_struct();

//  void reset();

  bool init(ESPEasySerialPort port,
            const int16_t     serial_rx,
            const int16_t     serial_tx);

  bool isInitialized() const;

  bool loop();

  bool hasFix(unsigned int maxAge_msec);

  bool storeCurPos(unsigned int maxAge_msec);

  // Return the distance in meters compared to last stored position.
  // @retval  -1 when no fix.
  double distanceSinceLast(unsigned int maxAge_msec);

  // Return the GPS time stamp, which is in UTC.
  // @param age is the time in msec since the last update of the time +
  // additional centiseconds given by the GPS.
  bool getDateTime(struct tm& dateTime,
                   uint32_t & age,
                   bool     & updated,
                   bool     & pps_sync);

  // Send command to GPS to put it in PMREQ backup mode (UBLOX only)
  // @retval true when successful in sending command
  bool powerDown();

  // Send some characters to GPS to wake up
  bool wakeUp();
#ifdef P082_USE_U_BLOX_SPECIFIC
  bool setPowerMode(P082_PowerMode mode);

  bool setDynamicModel(P082_DynamicModel model);
#endif

# if FEATURE_PLUGIN_STATS
  bool webformLoad_show_stats(struct EventStruct *event, uint8_t var_index, P082_query query_type);
# endif // if FEATURE_PLUGIN_STATS

private:
#ifdef P082_USE_U_BLOX_SPECIFIC
  // Compute checksum
  // Caller should offset the data pointer to the correct start where the CRC should start.
  // @param size  The length over which the CRC should be computed
  // @param CK_A, CK_B The 2 checksum bytes.
  static void computeUbloxChecksum(const uint8_t* data, size_t size, uint8_t & CK_A, uint8_t & CK_B);

  // Set checksum.
  // First 2 bytes of the array are skipped
  static void setUbloxChecksum(uint8_t* data, size_t size);
#endif

  bool writeToGPS(const uint8_t* data, size_t size);
public:

  TinyGPSPlus   *gps        = nullptr;
  ESPeasySerial *easySerial = nullptr;

  double _last_lat = 0.0;
  double _last_lng = 0.0;
  double _ref_lat  = 0.0;
  double _ref_lng  = 0.0;
  double _distance = 0.0;


  unsigned long _pps_time            = 0;
  unsigned long _last_measurement    = 0;
  uint32_t      _last_time           = 0;
  uint32_t      _last_date           = 0;
  uint32_t      _last_setSystemTime  = 0;
  uint32_t      _start_sentence      = 0;
  uint32_t      _start_prev_sentence = 0;
  uint32_t      _start_sequence      = 0;
# ifdef P082_SEND_GPS_TO_LOG
  String _lastSentence;
  String _currentSentence;
# endif // ifdef P082_SEND_GPS_TO_LOG

  float _cache[static_cast<uint8_t>(P082_query::P082_NR_OUTPUT_OPTIONS)];
};

#endif // ifdef USES_P082
#endif // ifndef PLUGINSTRUCTS_P082_DATA_STRUCT_H
