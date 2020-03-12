#include "P087_data_struct.h"


#ifdef USES_P087


P087_data_struct::P087_data_struct() :  P087_easySerial(nullptr) {}

P087_data_struct::~P087_data_struct() {
  reset();
}

void P087_data_struct::reset() {
  if (P087_easySerial != nullptr) {
    delete P087_easySerial;
    P087_easySerial = nullptr;
  }
}

bool P087_data_struct::init(const int16_t serial_rx, const int16_t serial_tx, unsigned long baudrate) {
  if ((serial_rx < 0) && (serial_tx < 0)) {
    return false;
  }
  reset();
  P087_easySerial = new ESPeasySerial(serial_rx, serial_tx);

  if (isInitialized()) {
    P087_easySerial->begin(baudrate);
    return true;
  }
  return false;
}

bool P087_data_struct::isInitialized() const {
  return P087_easySerial != nullptr;
}

void P087_data_struct::sendString(const String& data) {
  if (isInitialized()) {
    if (data.length() > 0) {
      setDisableFilterWindowTimer();
      P087_easySerial->write(data.c_str());

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

  if (P087_easySerial != nullptr) {
    int available = P087_easySerial->available();

    while (available > 0 && !fullSentenceReceived) {
      // Look for end marker
      char c = P087_easySerial->read();
      --available;

      if (available == 0) {
        available = P087_easySerial->available();
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
    length_last_received = sentence_part.length();
  }
  return fullSentenceReceived;
}

void P087_data_struct::getSentence(String& string) {
  string        = sentence_part;
  sentence_part = "";
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
  }
  return false;
}

String P087_data_struct::getFilter(uint8_t lineNr, uint8_t& capture, P087_Filter_Comp& comparator) const
{
  uint8_t varNr = lineNr * 3 + P087_FIRST_FILTER_POS;

  if (varNr >= P87_Nlines) { return ""; }

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
  char cap[10]; // must be large enough to hold captures

  Serial.print("Matched: ");
  Serial.write((byte *)match, length);
  Serial.println();

  for (byte i = 0; i < ms.level; i++)
  {
    Serial.print("Capture ");
    Serial.print(i, DEC);
    Serial.print(" = ");
    ms.GetCapture(cap, i);
    Serial.println(cap);
  } // end of for each capture
}

bool P087_data_struct::matchRegexp(String& received) const {
  size_t strlength = received.length();

  if (strlength == 0) {
    return false;
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
    unsigned long count = ms.GlobalMatch(_lines[P087_REGEX_POS].c_str(), match_callback);
    match_result = count != 0;
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

bool P087_data_struct::max_length_reached() const {
  if (max_length == 0) { return false; }
  return sentence_part.length() >= max_length;
}

#endif // USES_P087
