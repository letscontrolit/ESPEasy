#ifndef PLUGINSTRUCTS_P087_DATA_STRUCT_H
#define PLUGINSTRUCTS_P087_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P087

#include <ESPeasySerial.h>

#include <Regexp.h>


# define P087_REGEX_POS          0
# define P087_NR_CHAR_USE_POS    1
# define P087_FILTER_OFF_WINDOW_POS 2
# define P087_MATCH_TYPE_POS 3

# define P087_FIRST_FILTER_POS   6

# define P087_NR_FILTERS         10
# define P87_Nlines              (P087_FIRST_FILTER_POS + 3 * (P087_NR_FILTERS))
# define P87_Nchars              128
# define P87_MAX_CAPTURE_INDEX   32


enum P087_Filter_Comp {
  Equal    = 0,
  NotEqual = 1
};

enum P087_Match_Type {
  Regular_Match          = 0,
  Regular_Match_inverted = 1,
  Global_Match           = 2,
  Global_Match_inverted  = 3,
  Filter_Disabled        = 4
};
# define P087_Match_Type_NR_ELEMENTS 5


struct P087_data_struct : public PluginTaskData_base {
public:

  P087_data_struct();

  ~P087_data_struct();

  void reset();

  bool init(ESPEasySerialPort port, 
            const int16_t serial_rx,
            const int16_t serial_tx,
            unsigned long baudrate);

  // Called after loading the config from the settings.
  // Will interpret some data and load caches.
  void post_init();

  bool isInitialized() const;

  void sendString(const String& data);

  bool loop();

  // Get the received sentence
  // @retval true when the string is not empty.
  bool getSentence(String& string);

  void getSentencesReceived(uint32_t& succes,
                            uint32_t& error,
                            uint32_t& length_last) const;

  void            setMaxLength(uint16_t maxlenght);

  void            setLine(byte          varNr,
                          const String& line);

  String          getRegEx() const;

  uint16_t        getRegExpMatchLength() const;

  uint32_t        getFilterOffWindowTime() const;

  P087_Match_Type getMatchType() const;

  bool            invertMatch() const;

  bool            globalMatch() const;

  String          getFilter(uint8_t           lineNr,
                            uint8_t         & capture,
                            P087_Filter_Comp& comparator) const;

  void        setDisableFilterWindowTimer();

  bool        disableFilterWindowActive() const;

  // called for each match when calling Global_Match
  static void match_callback(const char        *match,
                             const unsigned int length,
                             const MatchState & ms);

  bool          matchRegexp(String& received) const;

  static String MatchType_toString(P087_Match_Type matchType);


  // Made public so we don't have to copy the values when loading/saving.
  String _lines[P87_Nlines];

private:

  bool max_length_reached() const;

  ESPeasySerial *easySerial = nullptr;
  String         sentence_part;
  String         last_sentence;
  uint16_t       max_length               = 550;
  uint32_t       sentences_received       = 0;
  uint32_t       sentences_received_error = 0;
  uint32_t       length_last_received     = 0;
  unsigned long  disable_filter_window    = 0;

  uint8_t capture_index[P87_MAX_CAPTURE_INDEX] = { 0 };

  bool capture_index_used[P87_MAX_CAPTURE_INDEX];
  bool capture_index_must_not_match[P87_MAX_CAPTURE_INDEX];
  bool regex_empty = false;
};


#endif // USES_P087

#endif // PLUGINSTRUCTS_P087_DATA_STRUCT_H
