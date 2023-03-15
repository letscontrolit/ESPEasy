#include "../PluginStructs/P082_data_struct.h"

#ifdef USES_P082


// Needed also here for PlatformIO's library finder as the .h file
// is in a directory which is excluded in the src_filter
# include <TinyGPS++.h>
# include <ESPeasySerial.h>


const __FlashStringHelper * Plugin_082_valuename(P082_query value_nr, bool displayString) {
  switch (value_nr) {
    case P082_query::P082_QUERY_LONG:        return displayString ? F("Longitude")          : F("long");
    case P082_query::P082_QUERY_LAT:         return displayString ? F("Latitude")           : F("lat");
    case P082_query::P082_QUERY_ALT:         return displayString ? F("Altitude")           : F("alt");
    case P082_query::P082_QUERY_SPD:         return displayString ? F("Speed (m/s)")        : F("spd");
    case P082_query::P082_QUERY_SATVIS:      return displayString ? F("Satellites Visible") : F("sat_vis");
    case P082_query::P082_QUERY_SATUSE:      return displayString ? F("Satellites Tracked") : F("sat_tr");
    case P082_query::P082_QUERY_HDOP:        return displayString ? F("HDOP")               : F("hdop");
    case P082_query::P082_QUERY_FIXQ:        return displayString ? F("Fix Quality")        : F("fix_qual");
    case P082_query::P082_QUERY_DB_MAX:      return displayString ? F("Max SNR in dBHz")    : F("snr_max");
    case P082_query::P082_QUERY_CHKSUM_FAIL: return displayString ? F("Checksum Fail")      : F("chksum_fail");
    case P082_query::P082_QUERY_DISTANCE:    return displayString ? F("Distance (ODO)")     : F("dist");
    case P082_query::P082_QUERY_DIST_REF:    return displayString ? F("Distance from Reference Point") : F("dist_ref");
    case P082_query::P082_NR_OUTPUT_OPTIONS: break;
  }
  return F("");
}

P082_query Plugin_082_from_valuename(const String& valuename)
{
  for (uint8_t query = 0; query < static_cast<uint8_t>(P082_query::P082_NR_OUTPUT_OPTIONS); ++query) {
    if (valuename.equalsIgnoreCase(Plugin_082_valuename(static_cast<P082_query>(query), false))) {
      return static_cast<P082_query>(query);
    }
  }
  return P082_query::P082_NR_OUTPUT_OPTIONS;
}

const __FlashStringHelper* toString(P082_PowerMode mode) {
  switch (mode) {
    case P082_PowerMode::Max_Performance: return F("Max Performance");
    case P082_PowerMode::Power_Save:      return F("Power Save");
    case P082_PowerMode::Eco:             return F("ECO");
  }
  return F("");
}

const __FlashStringHelper* toString(P082_DynamicModel model) {
  switch (model) {
    case P082_DynamicModel::Portable:    return F("Portable");
    case P082_DynamicModel::Stationary:  return F("Stationary");
    case P082_DynamicModel::Pedestrian:  return F("Pedestrian");
    case P082_DynamicModel::Automotive:  return F("Automotive");
    case P082_DynamicModel::Sea:         return F("Sea");
    case P082_DynamicModel::Airborne_1g: return F("Airborne_1g");
    case P082_DynamicModel::Airborne_2g: return F("Airborne_2g");
    case P082_DynamicModel::Airborne_4g: return F("Airborne_4g");
    case P082_DynamicModel::Wrist:       return F("Wrist");
    case P082_DynamicModel::Bike:        return F("Bike");
  }
  return F("");
}

P082_data_struct::P082_data_struct() : gps(nullptr), easySerial(nullptr) {
  for (size_t i = 0; i < static_cast<uint8_t>(P082_query::P082_NR_OUTPUT_OPTIONS); ++i) {
    _cache[i] = 0.0f;
  }
}

P082_data_struct::~P082_data_struct() {
  if (gps != nullptr) {
    delete gps;
    gps = nullptr;
  }

  if (easySerial != nullptr) {
    delete easySerial;
    easySerial = nullptr;
  }
}

/*
void P082_data_struct::reset() {
  if (gps != nullptr) {
    delete gps;
    gps = nullptr;
  }

  if (easySerial != nullptr) {
    delete easySerial;
    easySerial = nullptr;
  }
}
*/

bool P082_data_struct::init(ESPEasySerialPort port, const int16_t serial_rx, const int16_t serial_tx) {
  if (serial_rx < 0) {
    return false;
  }
  if (gps != nullptr) {
    delete gps;
    gps = nullptr;
  }

  if (easySerial != nullptr) {
    delete easySerial;
    easySerial = nullptr;
  }

  gps        = new (std::nothrow) TinyGPSPlus();
  easySerial = new (std::nothrow) ESPeasySerial(port, serial_rx, serial_tx);

  if (easySerial != nullptr) {
    easySerial->begin(9600);
    wakeUp();
  }
  return isInitialized();
}

bool P082_data_struct::isInitialized() const {
  return gps != nullptr && easySerial != nullptr;
}

bool P082_data_struct::loop() {
  if (!isInitialized()) {
    return false;
  }
  bool completeSentence = false;

  if (easySerial != nullptr) {
    int available           = easySerial->available();
    unsigned long startLoop = millis();

    while (available > 0 && timePassedSince(startLoop) < 10) {
      --available;
      int c = easySerial->read();
      if (c >= 0) {
# ifdef P082_SEND_GPS_TO_LOG
        if (_currentSentence.length() <= 80) {
          // No need to capture more than 80 bytes as a NMEA message is never that long.
          if (c != 0) {
            _currentSentence += static_cast<char>(c);
          }
        }
# endif // ifdef P082_SEND_GPS_TO_LOG

        if (c == 0x85) {
          // Found possible start of u-blox message
          unsigned long timeout = millis() + 200;
          unsigned int bytesRead = 0;
          bool done = false;
          bool ack_nak_read = false;
          while (!timeOutReached(timeout) && !done)
          {
            if (available == 0) {
              available = easySerial->available();
            } else {
              const int c = easySerial->read();
              if (c >= 0) {
                switch (bytesRead) {
                  case 0:
                    if (c != 0x62) {
                      done = true;
                    }
                    ++bytesRead;
                    break;
                  case 1:
                    if (c != 0x05) {
                      done = true;
                    }
                    ++bytesRead;
                    break;
                  case 2:
                    if (c == 0x01) {
                      ack_nak_read = true;
                      addLog(LOG_LEVEL_INFO, F("GPS  : ACK-ACK"));
                    } else if (c == 0x00) {
                      ack_nak_read = true;
                      addLog(LOG_LEVEL_ERROR, F("GPS  : ACK-NAK"));
                    }
                    done = true;
                    break;
                  default:
                    done = true;
                    break;
                }
              }
            }
          }
          if (!done) {
            addLog(LOG_LEVEL_ERROR, F("GPS  : Ack/Nack timeout"));
          } else if (!ack_nak_read) {
            addLog(LOG_LEVEL_ERROR, F("GPS  : Unexpected reply"));
          }
        }

        if (gps->encode(c)) {
          // Full sentence received
# ifdef P082_SEND_GPS_TO_LOG
          _lastSentence    = _currentSentence;
          _currentSentence = String();
# endif // ifdef P082_SEND_GPS_TO_LOG
          completeSentence = true;
        } else {
          if (available == 0) {
            available = easySerial->available();
          }
          if (c == '$') {
            _start_prev_sentence = _start_sentence;
            _start_sentence = millis();
            const unsigned long baudrate = easySerial->getBaudRate();
            if (baudrate != 0) {
              // Subtract the time (msec) taken to send the nr of bytes present in the serial buffer
              // Assume 10 bits per byte. (8N1)
              _start_sentence -= (available * 10000) / baudrate;
            }
            const int32_t max_sentence_duration = (160 * 10000) / baudrate;
            if (timeDiff(_start_prev_sentence, _start_sentence) > max_sentence_duration) {
              _start_sequence = _start_sentence;
              // Debug accuracy of computing the time stability
//              addLog(LOG_LEVEL_INFO, concat(F("GPS  : Start Sequence: "), _start_sequence));
            }
          }
        }
      }
    }
  }
  return completeSentence;
}

bool P082_data_struct::hasFix(unsigned int maxAge_msec) {
  if (!isInitialized()) {
    return false;
  }
  return gps->location.isValid() && gps->location.age() < maxAge_msec;
}

bool P082_data_struct::storeCurPos(unsigned int maxAge_msec) {
  if (!hasFix(maxAge_msec)) {
    return false;
  }

  _distance += distanceSinceLast(maxAge_msec);
  _last_lat  = gps->location.lat();
  _last_lng  = gps->location.lng();
  return true;
}

// Return the distance in meters compared to last stored position.
// @retval  -1 when no fix.
double P082_data_struct::distanceSinceLast(unsigned int maxAge_msec) {
  if (!hasFix(maxAge_msec)) {
    return -1.0;
  }

  if (((_last_lat < 0.0001) && (_last_lat > -0.0001)) || ((_last_lng < 0.0001) && (_last_lng > -0.0001))) {
    return -1.0;
  }
  return gps->distanceBetween(_last_lat, _last_lng, gps->location.lat(), gps->location.lng());
}

// Return the GPS time stamp, which is in UTC.
// @param age is the time in msec since the last update of the time +
// additional centiseconds given by the GPS.
bool P082_data_struct::getDateTime(
  struct tm& dateTime,
  uint32_t & age,
  bool     & updated,
  bool     & pps_sync) {
  updated = false;

  if (!isInitialized()) {
    return false;
  }

  if (!gps->time.isUpdated() || !gps->date.isUpdated()) {
    return false;
  }

  if (_pps_time != 0) {
    age       = timePassedSince(_pps_time);
    _pps_time = 0;
    pps_sync  = true;

    if ((age > P082_TIMESTAMP_AGE) || (gps->time.age() > age)) {
      return false;
    }
  } else {
    age      = gps->time.age();
    pps_sync = false;
  }

  if (age > P082_TIMESTAMP_AGE) {
    return false;
  }

  if (!gps->time.isUpdated() || !gps->date.isUpdated()) {
    return false;
  }

  if (gps->date.age() > P082_TIMESTAMP_AGE) {
    return false;
  }

  if (!gps->time.isValid()) {
    gps->time.value(); // Clear the 'updated' state
    return false;
  }
  if (!gps->date.isValid()) {
    gps->date.value(); // Clear the 'updated' state
    return false;
  }
  dateTime.tm_year = gps->date.year() - 1900;
  dateTime.tm_mon  = gps->date.month() - 1; // GPS month starts at 1, tm_mon at 0
  dateTime.tm_mday = gps->date.day();

  dateTime.tm_hour = gps->time.hour();
  dateTime.tm_min  = gps->time.minute();
  dateTime.tm_sec  = gps->time.second();

  const uint32_t reported_time = gps->time.value();
  const uint32_t reported_date = gps->date.value();
  updated = reported_time != _last_time;

  _last_time = reported_time;
  _last_date = reported_date;
  // FIXME TD-er: Must the offset in centisecond be added when pps_sync active?
  if (!pps_sync) {
    // Don't use the "commit" time when the sentence was read, but use the timestamp when the first sentence of a NMEA sequence was received.
    const long time_since_start_seq = timePassedSince(_start_sequence);
    if (time_since_start_seq < P082_TIMESTAMP_AGE) {
      age = time_since_start_seq;
    }
    age += (gps->time.centisecond() * 10);
  }

  return true;
}

bool P082_data_struct::powerDown() {
  const uint8_t UBLOX_GPSStandby[] = {0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x4D, 0x3B}; 
  return writeToGPS(UBLOX_GPSStandby, sizeof(UBLOX_GPSStandby));
}

bool P082_data_struct::wakeUp() {
  if (isInitialized()) {
    if (easySerial->isTxEnabled()) {
      easySerial->println();   // Send some character to wake it up.
    }
  }
  return false;
}

#ifdef P082_USE_U_BLOX_SPECIFIC
bool P082_data_struct::setPowerMode(P082_PowerMode mode) {
  switch (mode) {
    case P082_PowerMode::Max_Performance: 
    {
      const uint8_t UBLOX_command[] = {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x00, 0x21, 0x91}; 
      return writeToGPS(UBLOX_command, sizeof(UBLOX_command));
    }
    case P082_PowerMode::Power_Save:      
    {
      const uint8_t UBLOX_command[] = {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x01, 0x22, 0x92}; 
      return writeToGPS(UBLOX_command, sizeof(UBLOX_command));
    }
    case P082_PowerMode::Eco:             
    {
      const uint8_t UBLOX_command[] = {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x04, 0x25, 0x95}; 
      return writeToGPS(UBLOX_command, sizeof(UBLOX_command));
    }
  }
  return false;
}

bool P082_data_struct::setDynamicModel(P082_DynamicModel model) {

  const uint8_t dynModel = static_cast<uint8_t>(model);
  if (dynModel == 1 || dynModel > 10) {
    return false;
  }

  uint8_t UBLOX_command[] = {
    0xB5, 0x62, // header
    0x06, // class
    0x24, // ID, UBX-CFG-NAV5
    0x24, 0x00, // length
    0x01, 0x00, // mask
    dynModel, // dynModel
    0x03, // fixMode auto 2D/3D
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };
  setUbloxChecksum(UBLOX_command, sizeof(UBLOX_command));
  return writeToGPS(UBLOX_command, sizeof(UBLOX_command));
}
#endif

#ifdef P082_USE_U_BLOX_SPECIFIC
void P082_data_struct::computeUbloxChecksum(const uint8_t* data, size_t size, uint8_t & CK_A, uint8_t & CK_B) {
  CK_A = 0;
  CK_B = 0;
  for (size_t i = 0; i < size; ++i) {
    CK_A = CK_A + data[i];
    CK_B = CK_B + CK_A;
  }
}

void P082_data_struct::setUbloxChecksum(uint8_t* data, size_t size) {
  uint8_t CK_A;
  uint8_t CK_B;
  computeUbloxChecksum(data + 2, size - 4, CK_A, CK_B);
  data[size - 2] = CK_A;
  data[size - 1] = CK_B;
}
#endif

bool P082_data_struct::writeToGPS(const uint8_t* data, size_t size) {
  if (isInitialized()) {
    if (easySerial->isTxEnabled()) {
      if (size != easySerial->write(data, size)) {
        addLog(LOG_LEVEL_ERROR, F("GPS  : Written less bytes than expected"));
        return false;
      }
      return true;
    }
  }
  addLog(LOG_LEVEL_ERROR, F("GPS  : Cannot send to GPS"));
  return false;
}

# if FEATURE_PLUGIN_STATS
bool P082_data_struct::webformLoad_show_stats(struct EventStruct *event, uint8_t var_index, P082_query query_type)
{
  bool somethingAdded = false;

  const PluginStats *stats = getPluginStats(var_index);


  if (stats != nullptr) {
    if (stats->webformLoad_show_avg(event)) {
      somethingAdded = true;
    }

    bool   show_custom = false;
    double dist_p2p    = 0.0;
    double dist_stddev = 0.0;

    if (gps != nullptr) {
      switch (query_type) {
        case P082_query::P082_QUERY_LAT:
          show_custom = true;
          // Compute distance between min and max peak
          dist_p2p            = gps->distanceBetween(
            stats->getPeakLow(),  _last_lng,
            stats->getPeakHigh(), _last_lng);
          dist_stddev         = gps->distanceBetween(
            _last_lat,                            _last_lng,
            _last_lat + stats->getSampleStdDev(), _last_lng);
          break;
        case P082_query::P082_QUERY_LONG:
          show_custom = true;
          // Compute distance between min and max peak
          dist_p2p            = gps->distanceBetween(
            _last_lat, stats->getPeakLow(),
            _last_lat, stats->getPeakHigh());
          // Compute distance for std.dev
          dist_stddev         = gps->distanceBetween(
            _last_lat, _last_lng,
            _last_lat, _last_lng + stats->getSampleStdDev());
          break;
        default:
          break;
      }
    }

    // Only show standard deviation in meters, which is more useful than std. dev in degrees.
    if (somethingAdded) {
      if (show_custom) {
        stats->webformLoad_show_val(
          event,
          F(" std. dev"),
          dist_stddev,
          F("m"));
      } else {
        stats->webformLoad_show_stdev(event);
      }
    }

    if (stats->webformLoad_show_peaks(event, !show_custom)) {
      somethingAdded = true;

      if (show_custom) {
        stats->webformLoad_show_val(
          event,
          F(" Peak-to-peak coordinates"),
          stats->getPeakHigh() - stats->getPeakLow(),
          F("deg"));
        stats->webformLoad_show_val(
          event,
          F(" Peak-to-peak distance"),
          dist_p2p,
          F("m"));
      }
    }

    if (somethingAdded) {
      addFormSeparator(4);
    }
  }
  return somethingAdded;
}

# endif // if FEATURE_PLUGIN_STATS


#endif // ifdef USES_P082
