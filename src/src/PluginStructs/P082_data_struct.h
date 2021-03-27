#ifndef PLUGINSTRUCTS_P082_DATA_STRUCT_H
#define PLUGINSTRUCTS_P082_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P082

# include <TinyGPS++.h>
# include <ESPeasySerial.h>

#ifndef LIMIT_BUILD_SIZE
# define P082_SEND_GPS_TO_LOG
#endif

# define P082_TIMESTAMP_AGE       1500
# define P082_DEFAULT_FIX_TIMEOUT 2500 // TTL of fix status in ms since last update


enum class P082_query : byte {
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

String Plugin_082_valuename(P082_query value_nr, bool displayString);



struct P082_data_struct : public PluginTaskData_base {
  P082_data_struct();

  ~P082_data_struct();

  void reset();

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
                   bool     & pps_sync);

  TinyGPSPlus   *gps        = nullptr;
  ESPeasySerial *easySerial = nullptr;

  double _last_lat = 0.0;
  double _last_lng = 0.0;
  double _ref_lat  = 0.0;
  double _ref_lng  = 0.0;
  double _distance = 0.0;



  unsigned long _pps_time         = 0;
  unsigned long _last_measurement = 0;
# ifdef P082_SEND_GPS_TO_LOG
  String _lastSentence;
  String _currentSentence;
# endif // ifdef P082_SEND_GPS_TO_LOG

  float _cache[static_cast<byte>(P082_query::P082_NR_OUTPUT_OPTIONS)] = { 0 };
};

#endif // ifdef USES_P082
#endif // ifndef PLUGINSTRUCTS_P082_DATA_STRUCT_H
