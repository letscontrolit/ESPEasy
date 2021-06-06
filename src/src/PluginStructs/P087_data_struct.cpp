#include "../PluginStructs/P087_data_struct.h"


// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter
#include <ESPeasySerial.h>
#include <Regexp.h>


#ifdef USES_P087


P087_data_struct::P087_data_struct() :  easySerial(nullptr) {}

P087_data_struct::~P087_data_struct() {
  reset();
}

void P087_data_struct::reset() {
  if (easySerial != nullptr) {
    delete easySerial;
    easySerial = nullptr;
  }
}

bool P087_data_struct::init(ESPEasySerialPort port, const int16_t serial_rx, const int16_t serial_tx, unsigned long baudrate) {
  if ((serial_rx < 0) && (serial_tx < 0)) {
    return false;
  }
  reset();
  easySerial = new (std::nothrow) ESPeasySerial(port, serial_rx, serial_tx);

  if (isInitialized()) {
    easySerial->begin(baudrate);
    return true;
  }
  return false;
}

void P087_data_struct::post_init() {
  for (uint8_t i = 0; i < P87_MAX_CAPTURE_INDEX; ++i) {
    capture_index_used[i] = false;
  }
  regex_empty = _lines[P087_REGEX_POS].isEmpty();
  String log = F("P087_post_init:");

  for (uint8_t i = 0; i < P087_NR_FILTERS; ++i) {
    // Create some quick lookup table to see if we have a filter for the specific index
    capture_index_must_not_match[i] = _lines[i * 3 + P087_FIRST_FILTER_POS + 1].toInt() == P087_Filter_Comp::NotEqual;
    int index = _lines[i * 3 + P087_FIRST_FILTER_POS].toInt();

    // Index is negative when not used.
    if ((index >= 0) && (index < P87_MAX_CAPTURE_INDEX) && (_lines[i * 3 + P087_FIRST_FILTER_POS + 2].length() > 0)) {
      log                      += ' ';
      log                      += String(i);
      log                      += ':';
      log                      += String(index);
      capture_index[i]          = index;
      capture_index_used[index] = true;
    }
  }
  addLog(LOG_LEVEL_DEBUG, log);
}

bool P087_data_struct::isInitialized() const {
  return easySerial != nullptr;
}

void P087_data_struct::sendString(const String& data) {
  if (isInitialized()) {
    if (data.length() > 0) {
      setDisableFilterWindowTimer();
      easySerial->write(data.c_str());

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("Proxy: Sending: ");
        log += data;
        addLog(LOG_LEVEL_INFO, log);
      }
    }
  }
}

bool P087_data_struct::loop() {
  if (!isInitialized()) {
    return false;
  }
  bool fullSentenceReceived = false;

  if (easySerial != nullptr) {
    int available = easySerial->available();

    while (available > 0 && !fullSentenceReceived) {
      // Look for end marker
      char c = easySerial->read();
      --available;

      if (available == 0) {
        available = easySerial->available();
        delay(0);
      }

      switch (c) {
        case 13:
        {
          const size_t length = sentence_part.length();
          bool valid          = length > 0;

          for (size_t i = 0; i < length && valid; ++i) {
            if ((sentence_part[i] > 127) || (sentence_part[i] < 32)) {
              sentence_part = "";
              ++sentences_received_error;
              valid = false;
            }
          }

          if (valid) {
            fullSentenceReceived = true;
            last_sentence = sentence_part;
            sentence_part = "";
          }
          break;
        }
        case 10:

          // Ignore LF
          break;
        default:
          sentence_part += c;
          break;
      }

      if (max_length_reached()) { fullSentenceReceived = true; }
    }
  }

  if (fullSentenceReceived) {
    ++sentences_received;
    length_last_received = last_sentence.length();
  }
  return fullSentenceReceived;
}

bool P087_data_struct::getSentence(String& string) {
  string        = last_sentence;
  if (string.isEmpty()) {
    return false;
  }
  last_sentence = "";
  return true;
}

void P087_data_struct::getSentencesReceived(uint32_t& succes, uint32_t& error, uint32_t& length_last) const {
  succes      = sentences_received;
  error       = sentences_received_error;
  length_last = length_last_received;
}

void P087_data_struct::setMaxLength(uint16_t maxlenght) {
  max_length = maxlenght;
}

void P087_data_struct::setLine(byte varNr, const String& line) {
  if (varNr < P87_Nlines) {
    _lines[varNr] = line;
  }
}

String P087_data_struct::getRegEx() const {
  return _lines[P087_REGEX_POS];
}

uint16_t P087_data_struct::getRegExpMatchLength() const {
  return _lines[P087_NR_CHAR_USE_POS].toInt();
}

uint32_t P087_data_struct::getFilterOffWindowTime() const {
  return _lines[P087_FILTER_OFF_WINDOW_POS].toInt();
}

P087_Match_Type P087_data_struct::getMatchType() const {
  return static_cast<P087_Match_Type>(_lines[P087_MATCH_TYPE_POS].toInt());
}

bool P087_data_struct::invertMatch() const {
  switch (getMatchType()) {
    case Regular_Match:          // fallthrough
    case Global_Match:
      break;
    case Regular_Match_inverted: // fallthrough
    case Global_Match_inverted:
      return true;
    case Filter_Disabled:
      break;
  }
  return false;
}

bool P087_data_struct::globalMatch() const {
  switch (getMatchType()) {
    case Regular_Match: // fallthrough
    case Regular_Match_inverted:
      break;
    case Global_Match:  // fallthrough
    case Global_Match_inverted:
      return true;
    case Filter_Disabled:
      break;
  }
  return false;
}

String P087_data_struct::getFilter(uint8_t lineNr, uint8_t& capture, P087_Filter_Comp& comparator) const
{
  uint8_t varNr = lineNr * 3 + P087_FIRST_FILTER_POS;

  if ((varNr + 3) > P87_Nlines) { return ""; }

  capture    = _lines[varNr++].toInt();
  comparator = _lines[varNr++] == "1" ? P087_Filter_Comp::NotEqual : P087_Filter_Comp::Equal;
  return _lines[varNr];
}

void P087_data_struct::setDisableFilterWindowTimer() {
  if (getFilterOffWindowTime() == 0) {
    disable_filter_window = 0;
  }
  else {
    disable_filter_window = millis() + getFilterOffWindowTime();
  }
}

bool P087_data_struct::disableFilterWindowActive() const {
  if (disable_filter_window != 0) {
    if (!timeOutReached(disable_filter_window)) {
      // We're still in the window where filtering is disabled
      return true;
    }
  }
  return false;
}

typedef std::pair<uint8_t, String> capture_tuple;
static std::vector<capture_tuple> capture_vector;


// called for each match
void P087_data_struct::match_callback(const char *match, const unsigned int length, const MatchState& ms)
{
  for (byte i = 0; i < ms.level; i++)
  {
    capture_tuple tuple;
    tuple.first  = i;
    tuple.second = ms.GetCapture(i);
    capture_vector.push_back(tuple);
  } // end of for each capture
}

bool P087_data_struct::matchRegexp(String& received) const {
  size_t strlength = received.length();

  if (strlength == 0) {
    return false;
  }
  if (regex_empty || getMatchType() == Filter_Disabled) {
    return true;
  }


  uint32_t regexp_match_length = getRegExpMatchLength();

  if ((regexp_match_length > 0) && (strlength > regexp_match_length)) {
    strlength = regexp_match_length;
  }

  // We need to do a const_cast here, but this only is valid as long as we
  // don't call a replace function from regexp.
  MatchState ms(const_cast<char *>(received.c_str()), strlength);

  bool match_result = false;
  if (globalMatch()) {
    capture_vector.clear();
    ms.GlobalMatch(_lines[P087_REGEX_POS].c_str(), match_callback);
    const uint8_t vectorlength = capture_vector.size();

    for (uint8_t i = 0; i < vectorlength; ++i) {
      if ((capture_vector[i].first < P87_MAX_CAPTURE_INDEX) && capture_index_used[capture_vector[i].first]) {
        for (uint8_t n = 0; n < P087_NR_FILTERS; ++n) {
          unsigned int lines_index = n * 3 + P087_FIRST_FILTER_POS + 2;

          if ((capture_index[n] == capture_vector[i].first) && !(_lines[lines_index].isEmpty())) {
            String log;
            log.reserve(32);
            log  = F("P087: Index: ");
            log += capture_vector[i].first;
            log += F(" Found ");
            log += capture_vector[i].second;

            // Found a Capture Filter with this capture index.
            if (capture_vector[i].second == _lines[lines_index]) {
              log += F(" Matches");

              // Found a match. Now check if it is supposed to be one or not.
              if (capture_index_must_not_match[n]) {
                log += F(" (!=)");
                addLog(LOG_LEVEL_INFO, log);
                return false;
              } else {
                match_result = true;
                log         += F(" (==)");
              }
            } else {
              log += F(" No Match");

              if (capture_index_must_not_match[n]) {
                log += F(" (!=) ");
              } else {
                log += F(" (==) ");
              }
              log += _lines[lines_index];
            }
            addLog(LOG_LEVEL_INFO, log);
          }
        }
      }
    }
    capture_vector.clear();
  } else {
    char result = ms.Match(_lines[P087_REGEX_POS].c_str());

    if (result == REGEXP_MATCHED) {
      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("Match at: ");
        log += ms.MatchStart;
        log += F(" Match Length: ");
        log += ms.MatchLength;
        addLog(LOG_LEVEL_DEBUG, log);
      }
      match_result = true;
    }
  }
  return match_result;
}

const __FlashStringHelper * P087_data_struct::MatchType_toString(P087_Match_Type matchType) {
  switch (matchType)
  {
    case P087_Match_Type::Regular_Match:          return F("Regular Match");
    case P087_Match_Type::Regular_Match_inverted: return F("Regular Match inverted");
    case P087_Match_Type::Global_Match:           return F("Global Match");
    case P087_Match_Type::Global_Match_inverted:  return F("Global Match inverted");
    case P087_Match_Type::Filter_Disabled:        return F("Filter Disabled");
  }
  return F("");
}

bool P087_data_struct::max_length_reached() const {
  if (max_length == 0) { return false; }
  return sentence_part.length() >= max_length;
}

#endif // USES_P087
