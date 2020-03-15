#include "P094_data_struct.h"


#ifdef USES_P094


P094_data_struct::P094_data_struct() :  easySerial(nullptr) {}

P094_data_struct::~P094_data_struct() {
  reset();
}

void P094_data_struct::reset() {
  if (easySerial != nullptr) {
    delete easySerial;
    easySerial = nullptr;
  }
}

bool P094_data_struct::init(const int16_t serial_rx, const int16_t serial_tx, unsigned long baudrate) {
  if ((serial_rx < 0) && (serial_tx < 0)) {
    return false;
  }
  reset();
  easySerial = new ESPeasySerial(serial_rx, serial_tx);

  if (isInitialized()) {
    easySerial->begin(baudrate);
    return true;
  }
  return false;
}

void P094_data_struct::post_init() {
  for (uint8_t i = 0; i < P094_FILTER_VALUE_Type_NR_ELEMENTS; ++i) {
    valueType_used[i] = false;
  }

  for (uint8_t i = 0; i < P094_NR_FILTERS; ++i) {
    size_t lines_baseindex            = i * 3 + P094_FIRST_FILTER_POS;
    int    index                      = _lines[lines_baseindex].toInt();
    int    tmp_filter_comp            = _lines[lines_baseindex + 1].toInt();
    const bool filter_string_notempty = _lines[lines_baseindex + 2].length() > 0;
    const bool valid_index            = index >= 0 && index < P094_FILTER_VALUE_Type_NR_ELEMENTS;
    const bool valid_filter_comp      = tmp_filter_comp >= 0 && tmp_filter_comp < P094_FILTER_COMP_NR_ELEMENTS;

    valueType_index[i] = P094_not_used;

    if (valid_index && valid_filter_comp && filter_string_notempty) {
      valueType_used[index] = true;
      valueType_index[i]    = static_cast<P094_Filter_Value_Type>(index);
      filter_comp[i]        = static_cast<P094_Filter_Comp>(tmp_filter_comp);
    }
  }
}

bool P094_data_struct::isInitialized() const {
  return easySerial != nullptr;
}

void P094_data_struct::sendString(const String& data) {
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

bool P094_data_struct::loop() {
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

void P094_data_struct::getSentence(String& string) {
  string        = sentence_part;
  sentence_part = "";
}

void P094_data_struct::getSentencesReceived(uint32_t& succes, uint32_t& error, uint32_t& length_last) const {
  succes      = sentences_received;
  error       = sentences_received_error;
  length_last = length_last_received;
}

void P094_data_struct::setMaxLength(uint16_t maxlenght) {
  max_length = maxlenght;
}

void P094_data_struct::setLine(byte varNr, const String& line) {
  if (varNr < P94_Nlines) {
    _lines[varNr] = line;
  }
}

uint32_t P094_data_struct::getFilterOffWindowTime() const {
  return _lines[P094_FILTER_OFF_WINDOW_POS].toInt();
}

P094_Match_Type P094_data_struct::getMatchType() const {
  return static_cast<P094_Match_Type>(_lines[P094_MATCH_TYPE_POS].toInt());
}

bool P094_data_struct::invertMatch() const {
  switch (getMatchType()) {
    case P094_Regular_Match:          // fallthrough
    case P094_Global_Match:
      break;
    case P094_Regular_Match_inverted: // fallthrough
    case P094_Global_Match_inverted:
      return true;
    case P094_Filter_Disabled:
      break;
  }
  return false;
}

bool P094_data_struct::globalMatch() const {
  switch (getMatchType()) {
    case P094_Regular_Match: // fallthrough
    case P094_Regular_Match_inverted:
      break;
    case P094_Global_Match:  // fallthrough
    case P094_Global_Match_inverted:
      return true;
    case P094_Filter_Disabled:
      break;
  }
  return false;
}

String P094_data_struct::getFilter(uint8_t lineNr, P094_Filter_Value_Type& filterValueType, P094_Filter_Comp& comparator) const
{
  uint8_t varNr = lineNr * 3 + P094_FIRST_FILTER_POS;

  filterValueType = P094_Filter_Value_Type::P094_not_used;

  if (varNr >= P94_Nlines) { return ""; }

  filterValueType = valueType_index[lineNr];
  comparator      = filter_comp[lineNr];

  //  filterValueType = static_cast<P094_Filter_Value_Type>(_lines[varNr].toInt());
  //  comparator      = static_cast<P094_Filter_Comp>(_lines[varNr + 1].toInt());
  return _lines[varNr + 2];
}

void P094_data_struct::setDisableFilterWindowTimer() {
  if (getFilterOffWindowTime() == 0) {
    disable_filter_window = 0;
  }
  else {
    disable_filter_window = millis() + getFilterOffWindowTime();
  }
}

bool P094_data_struct::disableFilterWindowActive() const {
  if (disable_filter_window != 0) {
    if (!timeOutReached(disable_filter_window)) {
      // We're still in the window where filtering is disabled
      return true;
    }
  }
  return false;
}

bool P094_data_struct::parsePacket(String& received) const {
  size_t strlength = received.length();

  if (strlength == 0) {
    return false;
  }

  bool match_result = false;

  // FIXME TD-er: For now added '$' to test with GPS.
  if ((received[0] == 'b') || (received[0] == '$')) {
    // Received a data packet in CUL format.
    if (strlength < 21) {
      return false;
    }

    // Decoded packet

    unsigned long packet_header[P094_FILTER_VALUE_Type_NR_ELEMENTS];
    packet_header[P094_packet_length] = hexToUL(received, 1, 2);
    packet_header[P094_unknown1]      = hexToUL(received, 3, 2);
    packet_header[P094_manufacturer]  = hexToUL(received, 5, 4);
    packet_header[P094_serial_number] = hexToUL(received, 9, 8);
    packet_header[P094_unknown2]      = hexToUL(received, 17, 2);
    packet_header[P094_meter_type]    = hexToUL(received, 19, 2);

    // FIXME TD-er: Is this also correct?
    packet_header[P094_rssi] = hexToUL(received, strlength - 4, 4);

    // FIXME TD-er: Is this correct?
    // match_result = packet_length == (strlength - 21) / 2;

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      log.reserve(128);
      log  = F("CUL Reader: ");
      log += F(" length: ");
      log += packet_header[P094_packet_length];
      log += F(" (header: ");
      log += strlength - (packet_header[P094_packet_length] * 2);
      log += F(") manu: ");
      log += formatToHex_decimal(packet_header[P094_manufacturer]);
      log += F(" serial: ");
      log += formatToHex_decimal(packet_header[P094_serial_number]);
      log += F(" mType: ");
      log += formatToHex_decimal(packet_header[P094_meter_type]);
      log += F(" RSSI: ");
      log += formatToHex_decimal(packet_header[P094_rssi]);
      addLog(LOG_LEVEL_INFO, log);
    }

    // Do not check for "not used" (0)
    for (unsigned int i = 1; i < P094_FILTER_VALUE_Type_NR_ELEMENTS; ++i) {
      if (valueType_used[i]) {
        for (unsigned int f = 0; f < P094_NR_FILTERS; ++f) {
          if (valueType_index[f] == i) {
            // Have a matching filter

            P094_Filter_Value_Type filterValueType;
            P094_Filter_Comp comparator;
            unsigned long    value = hexToUL(getFilter(f, filterValueType, comparator));
            const bool match       = (value == packet_header[i]);

            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              String log;
              log.reserve(64);
              log  = F("CUL Reader: ");
              log += P094_FilterValueType_toString(valueType_index[f]);
              log += F(":  in:");
              log += packet_header[i];
              log += ' ';
              log += P094_FilterComp_toString(comparator);
              log += ' ';
              log += value;
              addLog(LOG_LEVEL_INFO, log);
            }

            switch (comparator) {
              case P094_Filter_Comp::P094_Equal_OR:

                if (match) { match_result = true; }
                break;
              case P094_Filter_Comp::P094_NotEqual_OR:

                if (!match) { match_result = true; }
                break;

              case P094_Filter_Comp::P094_Equal_MUST:

                if (!match) { return false; }
              case P094_Filter_Comp::P094_NotEqual_MUST:

                if (match) { return false; }
            }
          }
        }
      }
    }
  } else {
    // FIXME TD-er: Must test the result of the other possible answers.
    match_result = true;
  }


  return match_result;
}

String P094_data_struct::MatchType_toString(P094_Match_Type matchType) {
  switch (matchType)
  {
    case P094_Match_Type::P094_Regular_Match:          return F("Regular Match");
    case P094_Match_Type::P094_Regular_Match_inverted: return F("Regular Match inverted");
    case P094_Match_Type::P094_Global_Match:           return F("Global Match");
    case P094_Match_Type::P094_Global_Match_inverted:  return F("Global Match inverted");
    case P094_Match_Type::P094_Filter_Disabled:        return F("Filter Disabled");
  }
  return "";
}

String P094_data_struct::P094_FilterValueType_toString(P094_Filter_Value_Type valueType)
{
  switch (valueType) {
    case P094_Filter_Value_Type::P094_not_used:      return F("---");
    case P094_Filter_Value_Type::P094_packet_length: return F("Packet Length");
    case P094_Filter_Value_Type::P094_unknown1:      return F("unknown1");
    case P094_Filter_Value_Type::P094_manufacturer:  return F("Manufacturer");
    case P094_Filter_Value_Type::P094_serial_number: return F("Serial Number");
    case P094_Filter_Value_Type::P094_unknown2:      return F("unknown2");
    case P094_Filter_Value_Type::P094_meter_type:    return F("Meter Type");
    case P094_Filter_Value_Type::P094_rssi:          return F("RSSI");

      //    default: break;
  }
  return F("unknown");
}

String P094_data_struct::P094_FilterComp_toString(P094_Filter_Comp comparator)
{
  switch (comparator) {
    case P094_Filter_Comp::P094_Equal_OR:      return F("==");
    case P094_Filter_Comp::P094_NotEqual_OR:   return F("!=");
    case P094_Filter_Comp::P094_Equal_MUST:    return F("== (must)");
    case P094_Filter_Comp::P094_NotEqual_MUST: return F("!= (must)");
  }
  return "";
}

bool P094_data_struct::max_length_reached() const {
  if (max_length == 0) { return false; }
  return sentence_part.length() >= max_length;
}

#endif // USES_P094
