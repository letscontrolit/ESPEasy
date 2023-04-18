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


# define P094_REGEX_POS             0
# define P094_NR_CHAR_USE_POS       1
# define P094_FILTER_OFF_WINDOW_POS 2
# define P094_MATCH_TYPE_POS        3

# define P094_FIRST_FILTER_POS   10

# define P094_ITEMS_PER_FILTER   4
# define P094_AND_FILTER_BLOCK   3
# define P094_NR_FILTERS         (7 * P094_AND_FILTER_BLOCK)
# define P94_Nlines              (P094_FIRST_FILTER_POS + (P094_ITEMS_PER_FILTER * (P094_NR_FILTERS)))
# define P94_Nchars              128
# define P94_MAX_CAPTURE_INDEX   32




struct P094_data_struct : public PluginTaskData_base {
public:

  P094_data_struct();

  virtual ~P094_data_struct();

  void reset();

  bool init(ESPEasySerialPort port,
            const int16_t     serial_rx,
            const int16_t     serial_tx,
            unsigned long     baudrate,
            unsigned long     filterOffWindowTime_ms,
            bool              intervalFilterEnabled,
            bool              collectStats);

  void          loadFilters(struct EventStruct *event,
                            uint8_t             nrFilters);

  String        saveFilters(struct EventStruct *event) const;

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


  // Made public so we don't have to copy the values when loading/saving.
  std::vector<P094_filter>_filters;

  static size_t P094_Get_filter_base_index(size_t filterLine);

# if P094_DEBUG_OPTIONS

  // Get (and increment) debug counter
  uint32_t getDebugCounter();

  void     setGenerate_DebugCulData(bool value) {
    debug_generate_CUL_data = value;
  }

# endif // if P094_DEBUG_OPTIONS

  bool interval_filter_add(const mBusPacket_t& packet);
  void interval_filter_purgeExpired();

  bool collect_stats_add(const mBusPacket_t& packet);
  void prepare_dump_stats();
  bool dump_next_stats(String& str);

private:

  bool max_length_reached() const;

  ESPeasySerial *easySerial = nullptr;
  String         sentence_part;
  uint16_t       max_length = 550;
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
  bool interval_filter_enabled = false;
  bool collect_stats           = false;

  bool firstStatsIndexActive = false;

  CUL_interval_filter interval_filter;

  // Alternating stats, one being flushed, the other used to collect new stats
  CUL_Stats mBus_stats[2];
};


#endif // USES_P094

#endif // PLUGINSTRUCTS_P094_DATA_STRUCT_H
