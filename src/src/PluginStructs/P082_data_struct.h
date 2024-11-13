#ifndef PLUGINSTRUCTS_P082_DATA_STRUCT_H
#define PLUGINSTRUCTS_P082_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P082

# include <TinyGPS++.h>
# include <ESPeasySerial.h>
# include "../Helpers/ModuloOversamplingHelper.h"

# ifndef BUILD_NO_DEBUG
#  define P082_SEND_GPS_TO_LOG

// # define P082_USE_U_BLOX_SPECIFIC // TD-er: Disabled for now, as it is not working reliable/predictable
# endif // ifndef BUILD_NO_DEBUG

# define P082_TIMESTAMP_AGE       1000
# define P082_DEFAULT_FIX_TIMEOUT 2500 // TTL of fix status in ms since last update


# define P082_TIMEOUT        PCONFIG(0)
# define P082_TIMEOUT_LABEL  PCONFIG_LABEL(0)
# define P082_BAUDRATE       PCONFIG(1)
# define P082_BAUDRATE_LABEL PCONFIG_LABEL(1)
# define P082_DISTANCE       PCONFIG(2)
# define P082_DISTANCE_LABEL PCONFIG_LABEL(2)

# define P082_QUERY1_CONFIG_POS  3
# define P082_QUERY1         PCONFIG(3) // P082_QUERY1_CONFIG_POS
# define P082_QUERY2         PCONFIG(4) // P082_QUERY1_CONFIG_POS + 1
# define P082_QUERY3         PCONFIG(5) // P082_QUERY1_CONFIG_POS + 2
# define P082_QUERY4         PCONFIG(6) // P082_QUERY1_CONFIG_POS + 3

# define P082_LONG_REF       PCONFIG_FLOAT(0)
# define P082_LAT_REF        PCONFIG_FLOAT(1)
# ifdef P082_USE_U_BLOX_SPECIFIC
#  define P082_POWER_MODE     PCONFIG(7)
#  define P082_DYNAMIC_MODEL  PCONFIG_LONG(0)
# endif // P082_USE_U_BLOX_SPECIFIC

# define P082_NR_OUTPUT_VALUES   VARS_PER_TASK


# define P082_DISTANCE_DFLT       0 // Disable update per distance travelled.
# define P082_QUERY1_DFLT         P082_query::P082_QUERY_LONG
# define P082_QUERY2_DFLT         P082_query::P082_QUERY_LAT
# define P082_QUERY3_DFLT         P082_query::P082_QUERY_ALT
# define P082_QUERY4_DFLT         P082_query::P082_QUERY_SPD


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
  P082_QUERY_COURSE      = 12,
  P082_NR_OUTPUT_OPTIONS
};

const __FlashStringHelper* Plugin_082_valuename(P082_query value_nr,
                                                bool       displayString);

P082_query                 Plugin_082_from_valuename(const String& valuename);


enum class P082_PowerMode : uint8_t {
  Max_Performance = 0,
  Power_Save      = 1,
  Eco             = 2
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
  Wrist       = 9, // Only recommended for wrist-worn applications. Receiver will filter out armmotion (just available for protocol version
                   // > 17).
  Bike = 10        // Used for applications with equivalent dynamics to those of a motor bike. Lowvertical acceleration assumed. (supported
                   // in protocol versions 19.2)
};

const __FlashStringHelper* toString(P082_DynamicModel model);


// Class to help determine the most likely fraction of a second
// when the burst of messages starts
// This fraction is relative to the system micros.
struct P082_software_pps {
  P082_software_pps();

  // Keep track of fraction of second relative to the system micros
  // when the start of the sentence ('$') was received.
  void addStartOfSentence(uint32_t bytesAvailableInSerialBuffer);

  // Commit the last recorded start of sentence time to the 
  // matching ModuloOversamplingHelper
  void setSentenceType(TinyGPSPlus::GPS_Sentence_type sentenceType, uint32_t bytesAvailableInSerialBuffer);

  void setBaudrate(uint32_t baudrate);

  // Search for the second fraction of the sentence
  // starting the burst of sentences, marking the start of a second.
  bool getPPS(uint32_t& second_frac_in_usec) const;

#ifndef BUILD_NO_DEBUG
  String getStats() const;
#endif

private:

  uint64_t bytesToUsec(uint32_t bytes) const;

  ModuloOversamplingHelper<uint32_t, uint32_t>_second_frac_in_usec[TinyGPSPlus::GPS_SENTENCE_OTHER]{};

  uint64_t _cur_start_sentence_usec = 0;

  int32_t _baudrate = 0;
};


struct P082_data_struct : public PluginTaskData_base {
  // Enum is being stored, so don't change int values


  P082_data_struct();

  virtual ~P082_data_struct();

  //  void reset();

  bool init(ESPEasySerialPort port,
            const int16_t     serial_rx,
            const int16_t     serial_tx,
            const int8_t      pps_pin);

  bool isInitialized() const {
    return gps != nullptr && easySerial != nullptr;
  }

  bool loop();

  bool hasFix(unsigned int maxAge_msec);

  bool storeCurPos(unsigned int maxAge_msec);

  // Return the distance in meters compared to last stored position.
  // @retval  -1 when no fix.
  ESPEASY_RULES_FLOAT_TYPE distanceSinceLast(unsigned int maxAge_msec);

private:

  // Return the GPS time stamp, which is in UTC.
  // @param age is the time in msec since the last update of the time +
  // additional centiseconds given by the GPS.
  bool getDateTime(struct tm& dateTime,
                   uint8_t  & centiseconds,
                   uint32_t & age,
                   bool     & updated);

public:

  bool getDateTime(struct tm& dateTime) const;

  // Try to fetch 5 timestamps in a row, filter out the peaks and use the average to set the
  bool tryUpdateSystemTime();

  // Send command to GPS to put it in PMREQ backup mode (UBLOX only)
  // @retval true when successful in sending command
  bool powerDown();

  // Send some characters to GPS to wake up
  bool wakeUp();
# ifdef P082_USE_U_BLOX_SPECIFIC
  bool setPowerMode(P082_PowerMode mode);

  bool setDynamicModel(P082_DynamicModel model);
# endif // ifdef P082_USE_U_BLOX_SPECIFIC

# if FEATURE_PLUGIN_STATS
  bool webformLoad_show_stats(struct EventStruct *event,
                              uint8_t             var_index,
                              P082_query          query_type) const;

#  if FEATURE_CHART_JS
  void webformLoad_show_position_scatterplot(struct EventStruct *event);
#  endif // if FEATURE_CHART_JS
# endif  // if FEATURE_PLUGIN_STATS

#ifndef BUILD_NO_DEBUG
  String getPPSStats() const;
#endif


private:

# ifdef P082_USE_U_BLOX_SPECIFIC

  // Compute checksum
  // Caller should offset the data pointer to the correct start where the CRC should start.
  // @param size  The length over which the CRC should be computed
  // @param CK_A, CK_B The 2 checksum bytes.
  static void computeUbloxChecksum(const uint8_t *data,
                                   size_t         size,
                                   uint8_t      & CK_A,
                                   uint8_t      & CK_B);

  // Set checksum.
  // First 2 bytes of the array are skipped
  static void setUbloxChecksum(uint8_t *data,
                               size_t   size);
# endif // ifdef P082_USE_U_BLOX_SPECIFIC

  bool        writeToGPS(const uint8_t *data,
                         size_t         size);

  static void pps_interrupt(P082_data_struct *self);

public:

  TinyGPSPlus   *gps        = nullptr;
  ESPeasySerial *easySerial = nullptr;

  ESPEASY_RULES_FLOAT_TYPE _last_lat{};
  ESPEASY_RULES_FLOAT_TYPE _last_lng{};
  ESPEASY_RULES_FLOAT_TYPE _ref_lat{};
  ESPEASY_RULES_FLOAT_TYPE _ref_lng{};
  ESPEASY_RULES_FLOAT_TYPE _distance{};


  unsigned long _last_measurement = 0;
  uint32_t      _last_time        = 0;
  uint32_t      _last_date        = 0;

  //  uint32_t      _start_sentence      = 0;
  //  uint32_t      _start_prev_sentence = 0;
  //  uint32_t      _start_sequence      = 0;
# ifdef P082_SEND_GPS_TO_LOG
  String _lastSentence;
  String _currentSentence;
# endif // ifdef P082_SEND_GPS_TO_LOG

  float _cache[static_cast<uint8_t>(P082_query::P082_NR_OUTPUT_OPTIONS)]{};

  OversamplingHelper<uint64_t, uint64_t>_oversampling_gps_time_offset_usec;

  P082_software_pps _softwarePPS;

  // When using PPS pin, we're only interested in the moment during a second when it triggers.
  // So we keep only track of the micros() % 1000000 so we have some offset from the system micros counter.
  // This will also be used to keep track of when the first sentence is received as the GPS will send those out in a burst at the start of a
  // new second.
  ESPEASY_VOLATILE(int64_t) _pps_time_micros = -1;

  int8_t _ppsPin = -1;
};

#endif // ifdef USES_P082
#endif // ifndef PLUGINSTRUCTS_P082_DATA_STRUCT_H
