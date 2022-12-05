#include "../PluginStructs/P087_data_struct.h"

#ifdef USES_P087


// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter
#include <ESPeasySerial.h>
#include <Regexp.h>


#include <vector>


P087_data_struct::~P087_data_struct() {
  if (easySerial != nullptr) {
    delete easySerial;
    easySerial = nullptr;
  }
}

void P087_data_struct::reset() {
  if (easySerial != nullptr) {
    delete easySerial;
    easySerial = nullptr;
  }
}

bool P087_data_struct::init(ESPEasySerialPort port, const int16_t serial_rx, const int16_t serial_tx, unsigned long baudrate, uint8_t config) {
  if ((serial_rx < 0) && (serial_tx < 0)) {
    return false;
  }
  reset();
  easySerial = new (std::nothrow) ESPeasySerial(port, serial_rx, serial_tx);

  tx_gpio_serial = serial_tx;

  if (isInitialized()) {
    # if defined(ESP8266)
    easySerial->begin(baudrate, (SerialConfig)config);
    # elif defined(ESP32)
    easySerial->begin(baudrate, config);
    # endif // if defined(ESP8266)
    return true;
  }
  return false;
}

void P087_data_struct::post_init() {
  # ifndef BUILD_NO_DEBUG
  String log = F("P087_data_init:");
  #endif
  hex_head_match   = 0;
  hex_head_match_a = false;
  hex_head_match_b = false;
  sentence_part    = String();
  hex_head_empty   = _lines[P087_HEX_HEADER_POS].isEmpty();
  hex_data_length  = _lines[P087_HEX_DATA_LEN_POS].toInt();
  # ifndef BUILD_NO_DEBUG
  log += F(" HEX Data Length: ");
  log += String(hex_data_length);
  #endif
  if (hex_data_length > 0) {
    hex_data_length  = _lines[P087_HEX_DATA_LEN_POS].toInt() * 2;
    max_length       = 2048;
  }
  if (hex_data_length < 0) {
    hex_data_add_length  = _lines[P087_HEX_DATA_LEN_ADD_POS].toInt();
    # ifndef BUILD_NO_DEBUG
    log += F(" HEX Data Adding Length: ");
    log += String(hex_data_add_length);
    #endif
    hex_data_add_length  = _lines[P087_HEX_DATA_LEN_ADD_POS].toInt() * 2;
    max_length       = 2048;
  }
  if (hex_data_length != 0 && !hex_head_empty) {
    String buf = _lines[P087_HEX_HEADER_POS];
    # ifndef BUILD_NO_DEBUG
    log += F(" HEX Header: ");
    log += buf;
    #endif
    uint16_t len = buf.length();
    head_sentence_length = (len / 2);
    # ifndef BUILD_NO_DEBUG
    log += F(" HEX Header length: ");
    log += String(head_sentence_length);
    #endif
    for(int i = 0; i < len / 2 && !hex_head_empty; i++) {
      uint8_t uc = buf[i * 2], dc = buf[i * 2 + 1];
      if(uc >= 48 && uc <= 57) {
        uc -= 48;
      } else if(uc >= 65 && uc <= 70) {
        uc -= 55;
      } else if(uc >= 97 && uc <= 102) {
        uc -= 87;
      } else {
        hex_head_empty = true;
      }

      if(dc >= 48 && dc <= 57) {
        dc -= 48;
      } else if(dc >= 65 && dc <= 70) {
        dc -= 55;
      } else if(dc >= 97 && dc <= 102) {
        dc -= 87;
      } else {
        hex_head_empty = true;
      }
      if(!hex_head_empty) {
        head_sentence[i] = (uc << 4 | dc);
      }
    }
  }
  # ifndef BUILD_NO_DEBUG
  addLogMove(LOG_LEVEL_DEBUG, log);
  #endif
  for (uint8_t i = 0; i < P87_MAX_CAPTURE_INDEX; ++i) {
    capture_index_used[i] = false;
  }
  regex_empty = _lines[P087_REGEX_POS].isEmpty();
  # ifndef BUILD_NO_DEBUG
  log = F("P087_post_init:");
  #endif

  for (uint8_t i = 0; i < P087_NR_FILTERS; ++i) {
    // Create some quick lookup table to see if we have a filter for the specific index
    capture_index_must_not_match[i] = _lines[i * 3 + P087_FIRST_FILTER_POS + 1].toInt() == P087_Filter_Comp::NotEqual;
    int index = _lines[i * 3 + P087_FIRST_FILTER_POS].toInt();

    // Index is negative when not used.
    if ((index >= 0) && (index < P87_MAX_CAPTURE_INDEX) && (_lines[i * 3 + P087_FIRST_FILTER_POS + 2].length() > 0)) {
      # ifndef BUILD_NO_DEBUG
      log                      += ' ';
      log                      += String(i);
      log                      += ':';
      log                      += String(index);
      #endif
      capture_index[i]          = index;
      capture_index_used[index] = true;
    }
  }
  # ifndef BUILD_NO_DEBUG
  addLogMove(LOG_LEVEL_DEBUG, log);
  #endif
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
        addLogMove(LOG_LEVEL_INFO, log);
      }
    }
  }
}

void P087_data_struct::sendStringhex(const String& data) {
  if (isInitialized()) {
    if (data.length() > 0) {
      setDisableFilterWindowTimer();
      unsigned int len = data.length();
      for(unsigned int i=0; i < len / 2; i++){
        uint8_t uc = data[i * 2], dc = data[i * 2 + 1];
        if(uc >= 48 && uc <= 57) {
          uc -= 48;
        } else if(uc >= 65 && uc <= 70) {
          uc -= 55;
        } else if(uc >= 97 && uc <= 102) {
          uc -= 87;
        } else {
          continue;
        }

        if(dc >= 48 && dc <= 57) {
          dc -= 48;
        } else if(dc >= 65 && dc <= 70) {
          dc -= 55;
        } else if(dc >= 97 && dc <= 102) {
          dc -= 87;
        } else {
          continue;
        }
        easySerial->write(uc << 4 | dc);
      }

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("Proxy: SendingHEX: ");
        log += data;
        addLogMove(LOG_LEVEL_INFO, log);
      }
    }
  }
}

bool P087_data_struct::loop() {
  if (!isInitialized()) {
    return false;
  }
  bool fullSentenceReceived = false;
  bool valid                = false;
  sentence_part_length      = 0;
  char temp_c[3];

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
// - 00 01 02 03 04 05 06 07
// - 08 09 0A 0B 0C 0D 0E 0F
// - 10 11 12 13 14 FE FF
      if (hex_data_length != 0 && !hex_head_empty) {
        if (!hex_head_empty) {
          valid = false;
          if (!hex_head_match_a && !hex_head_match_b && head_sentence[hex_head_match] == c) {
            ++hex_head_match;
            if (hex_head_match >= head_sentence_length) {
              hex_head_match_a = true; // First match head, the next char start reading
              hex_head_match = 0;
              if (max_length_reached()) { fullSentenceReceived = true; }
              continue;
            }
          }
          if (hex_head_match >= head_sentence_length) {
              hex_head_match = 0;
          }
          if (hex_head_match_a || hex_head_match_b) { // match head
            sprintf(temp_c, "%02x", c);
            sentence_part += String(temp_c);
            sentence_part_length = sentence_part.length();
            if (hex_data_read_length == 0 && hex_data_length < 0 && sentence_part_length / 2 == hex_data_length * -1) {
              hex_data_read_length = c * 2 + hex_data_add_length;
            }
            if ((hex_data_length < 0 && hex_data_read_length > 0 && sentence_part_length >= hex_data_read_length) || (hex_data_length > 0 && sentence_part_length >= hex_data_length)) { // full hex data ending read
              valid = true;
              if (hex_head_match_b) {
                hex_head_match_b = false;
              }
              if (hex_head_match_a) {
                hex_head_match_a = false;
              }
            }
            if (head_sentence[hex_head_match] == c) {
              ++hex_head_match;
              if (hex_head_match >= head_sentence_length) {
                hex_head_match_b = true; // second match head, send sentence, the next char start reading
                hex_head_match = 0;
                valid = true;
              }
            }
          }
        } else if (hex_head_empty) {
          sprintf(temp_c, "%02x", c);
          sentence_part += String(temp_c);
          sentence_part_length = sentence_part.length();
          if (hex_data_read_length == 0 && hex_data_length < 0 && sentence_part_length / 2 == hex_data_length * -1) {
            hex_data_read_length = c * 2 + hex_data_add_length;
          }
          if ((hex_data_length < 0 && hex_data_read_length > 0 && sentence_part_length >= hex_data_read_length) || (hex_data_length > 0 && sentence_part_length >= hex_data_length)) { // full hex data ending read
            valid = true;
          }
        }
        if (valid) { // send sentence
          fullSentenceReceived = true;
          last_sentence = sentence_part;
          sentence_part = String();
          hex_data_read_length = 0;
        }
      } else {
// - ASCII 32 - 217
      switch (c) {
        case 13:
        {
          sentence_part_length = sentence_part.length();
          valid                = sentence_part_length > 0;

          for (size_t i = 0; i < sentence_part_length && valid; ++i) {
            if ((sentence_part[i] > 127) || (sentence_part[i] < 32)) {
              sentence_part = String();
              ++sentences_received_error;
              valid = false;
            }
          }

          if (valid) {
            fullSentenceReceived = true;
            last_sentence = sentence_part;
            sentence_part = String();
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
      }

      if (max_length_reached()) { fullSentenceReceived = true; }
    }
  }

  if (fullSentenceReceived) {
    ++sentences_received;
    length_last_received = last_sentence.length();
    hex_data_read_length = 0;
  }
  return fullSentenceReceived;
}

bool P087_data_struct::getSentence(String& string) {
  string        = last_sentence;
  if (string.isEmpty()) {
    return false;
  }
  last_sentence = String();
  return true;
}

void P087_data_struct::getSentencePart(String& string) const {
  if (hex_data_length == 0 || hex_head_empty) {
    string      = sentence_part;
  }
}

void P087_data_struct::getSentencesReceived(uint32_t& succes, uint32_t& error, uint32_t& length_last) const {
  succes      = sentences_received;
  error       = sentences_received_error;
  length_last = length_last_received;
}

void P087_data_struct::setMaxLength(uint16_t maxlenght) {
  max_length = maxlenght;
}

void P087_data_struct::setLine(uint8_t varNr, const String& line) {
  if (varNr < P87_Nlines) {
    _lines[varNr] = line;
  }
}

int16_t P087_data_struct::getTXGpioSerial() const {
  return tx_gpio_serial;
}

String P087_data_struct::getHEXHeader() const {
  return _lines[P087_HEX_HEADER_POS];
}

int16_t P087_data_struct::getHEXDataLength() const {
  return _lines[P087_HEX_DATA_LEN_POS].toInt();
}

uint16_t P087_data_struct::getHEXDataAddLength() const {
  return _lines[P087_HEX_DATA_LEN_ADD_POS].toInt();
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

  if ((varNr + 3) > P087_NR_FILTERS_N) { return ""; }

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
  for (uint8_t i = 0; i < ms.level; i++)
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


  uint16_t regexp_match_length = getRegExpMatchLength();

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
                addLogMove(LOG_LEVEL_INFO, log);
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
            addLogMove(LOG_LEVEL_INFO, log);
          }
        }
      }
    }
    capture_vector.clear();
  } else {
    char result = ms.Match(_lines[P087_REGEX_POS].c_str());

    if (result == REGEXP_MATCHED) {
      #ifndef BUILD_NO_DEBUG
      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("Match at: ");
        log += ms.MatchStart;
        log += F(" Match Length: ");
        log += ms.MatchLength;
        addLogMove(LOG_LEVEL_DEBUG, log);
      }
      #endif
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
