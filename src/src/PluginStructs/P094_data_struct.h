#ifndef PLUGINSTRUCTS_P094_DATA_STRUCT_H
#define PLUGINSTRUCTS_P094_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P094

# include "../Helpers/CUL_interval_filter.h"
# include "../Helpers/CUL_stats.h"

# include "../PluginStructs/P094_Filter.h"

# include <ESPeasySerial.h>
# include <Regexp.h>

# ifndef P094_DEBUG_OPTIONS
#  define P094_DEBUG_OPTIONS 0
# endif // ifndef P094_DEBUG_OPTIONS


# define P094_BAUDRATE           PCONFIG_LONG(0)
# define P094_BAUDRATE_LABEL     PCONFIG_LABEL(0)

# define P094_DEBUG_SENTENCE_LENGTH  PCONFIG_LONG(1)
# define P094_DEBUG_SENTENCE_LABEL   PCONFIG_LABEL(1)

# define P094_DISABLE_WINDOW_TIME_MS  PCONFIG_LONG(2)

# define P094_GET_APPEND_RECEIVE_SYSTIME    bitRead(PCONFIG(0), 0)
# define P094_SET_APPEND_RECEIVE_SYSTIME(X) bitWrite(PCONFIG(0), 0, X)

# if P094_DEBUG_OPTIONS
#  define P094_GET_GENERATE_DEBUG_CUL_DATA    bitRead(PCONFIG(0), 1)
#  define P094_SET_GENERATE_DEBUG_CUL_DATA(X) bitWrite(PCONFIG(0), 1, X)
# endif // if P094_DEBUG_OPTIONS

# define P094_GET_INTERVAL_FILTER    bitRead(PCONFIG(0), 2)
# define P094_SET_INTERVAL_FILTER(X) bitWrite(PCONFIG(0), 2, X)

# define P094_GET_COLLECT_STATS    bitRead(PCONFIG(0), 3)
# define P094_SET_COLLECT_STATS(X) bitWrite(PCONFIG(0), 3, X)

# define P094_GET_MUTE_MESSAGES    bitRead(PCONFIG(0), 4)
# define P094_SET_MUTE_MESSAGES(X) bitWrite(PCONFIG(0), 4, X)

# define P094_NR_FILTERS           PCONFIG(1)

# ifdef ESP8266
#  define P094_MAX_NR_FILTERS      25
# endif // ifdef ESP8266
# ifdef ESP32
#  define P094_MAX_NR_FILTERS      100
# endif // ifdef ESP32


# ifdef ESP8266
#  define P094_MAX_MSG_LENGTH      550
# endif // ifdef ESP8266
# ifdef ESP32
#  define P094_MAX_MSG_LENGTH      1024
# endif // ifdef ESP32


# define P094_DEFAULT_BAUDRATE   38400


struct P094_data_struct : public PluginTaskData_base {
public:

  P094_data_struct();

  virtual ~P094_data_struct();

  void reset();

  bool init(ESPEasySerialPort port,
            const int16_t     serial_rx,
            const int16_t     serial_tx,
            unsigned long     baudrate);

  void setFlags(unsigned long filterOffWindowTime_ms,
                bool          intervalFilterEnabled,
                bool          mute,
                bool          collectStats);


  void          loadFilters(struct EventStruct *event,
                            uint8_t             nrFilters);

  String        saveFilters(struct EventStruct *event) const;


  void          clearFilters();

  bool          addFilter(struct EventStruct *event, const String& filter);

  String        getFiltersMD5() const;

  void          WebformLoadFilters(uint8_t nrFilters) const;

  void          WebformSaveFilters(struct EventStruct *event,
                                   uint8_t             nrFilters);

  bool          isInitialized() const;

  void          sendString(const String& data);

  bool          loop();

  const String& peekSentence() const;

  void          getSentence(String& string,
                            bool    appendSysTime);

  void          getSentencesReceived(uint32_t& succes,
                                     uint32_t& error,
                                     uint32_t& length_last) const;

  void     setMaxLength(uint16_t maxlenght);

  void     setLine(uint8_t       varNr,
                   const String& line);

  uint32_t getFilterOffWindowTime() const;

  bool     filterUsed(uint8_t lineNr) const;

  void     setDisableFilterWindowTimer();

  bool     disableFilterWindowActive() const;

  bool     parsePacket(const String& received,
                       mBusPacket_t& packet);


# if P094_DEBUG_OPTIONS

  // Get (and increment) debug counter
  uint32_t getDebugCounter();

  void     setGenerate_DebugCulData(bool value) {
    debug_generate_CUL_data = value;
  }

# endif // if P094_DEBUG_OPTIONS

  void interval_filter_purgeExpired();

  void html_show_interval_filter_stats() const;


  bool collect_stats_add(const mBusPacket_t& packet, const String& source);
  void prepare_dump_stats();
  bool dump_next_stats(String& str);

  void html_show_mBus_stats() const;

private:

  bool max_length_reached() const;

  bool isDuplicate(const P094_filter& other) const;

  std::vector<P094_filter>_filters;

  ESPeasySerial *easySerial = nullptr;
  String         sentence_part;
  uint16_t       max_length = P094_MAX_MSG_LENGTH;
  uint16_t       nrFilters{};
  unsigned long  filterOffWindowTime      = 0;
  uint32_t       sentences_received       = 0;
  uint32_t       sentences_received_error = 0;
  bool           current_sentence_errored = false;
  uint32_t       length_last_received     = 0;
  unsigned long  disable_filter_window    = 0;

  # if P094_DEBUG_OPTIONS
  uint32_t debug_counter           = 0;
  bool     debug_generate_CUL_data = false;
  # endif // if P094_DEBUG_OPTIONS
  bool collect_stats = false;
  bool mute_messages = false;

  bool firstStatsIndexActive = false;

  CUL_interval_filter interval_filter;

  // Alternating stats, one being flushed, the other used to collect new stats
  CUL_Stats mBus_stats[2];
};


#endif // USES_P094

#endif // PLUGINSTRUCTS_P094_DATA_STRUCT_H
