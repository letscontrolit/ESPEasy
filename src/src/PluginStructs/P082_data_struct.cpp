#include "../PluginStructs/P082_data_struct.h"


// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter
# include <TinyGPS++.h>
# include <ESPeasySerial.h>

#ifdef USES_P082

String Plugin_082_valuename(P082_query value_nr, bool displayString) {
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
  return "";
}


P082_data_struct::P082_data_struct() : gps(nullptr), easySerial(nullptr) {}

P082_data_struct::~P082_data_struct() {
  reset();
}

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

bool P082_data_struct::init(ESPEasySerialPort port, const int16_t serial_rx, const int16_t serial_tx) {
  if (serial_rx < 0) {
    return false;
  }
  reset();
  gps             = new (std::nothrow) TinyGPSPlus();
  easySerial = new (std::nothrow) ESPeasySerial(port, serial_rx, serial_tx);

  if (easySerial != nullptr) {
    easySerial->begin(9600);
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
      char c = easySerial->read();
# ifdef P082_SEND_GPS_TO_LOG
      _currentSentence += c;
# endif // ifdef P082_SEND_GPS_TO_LOG

      if (gps->encode(c)) {
        // Full sentence received
# ifdef P082_SEND_GPS_TO_LOG
        _lastSentence    = _currentSentence;
        _currentSentence = "";
# endif // ifdef P082_SEND_GPS_TO_LOG
        completeSentence = true;
      } else {
        if (available == 0) {
          available = easySerial->available();
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
  _last_lat = gps->location.lat();
  _last_lng = gps->location.lng();
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
bool P082_data_struct::getDateTime(struct tm& dateTime, uint32_t& age, bool& pps_sync) {
  if (!isInitialized()) {
    return false;
  }

  if (_pps_time != 0) {
    age      = timePassedSince(_pps_time);
    _pps_time = 0;
    pps_sync = true;

    if ((age > 1000) || (gps->time.age() > age)) {
      return false;
    }
  } else {
    age      = gps->time.age();
    pps_sync = false;
  }

  if (age > P082_TIMESTAMP_AGE) {
    return false;
  }

  if (gps->date.age() > P082_TIMESTAMP_AGE) {
    return false;
  }

  if (!gps->date.isValid() || !gps->time.isValid()) {
    return false;
  }
  dateTime.tm_year = gps->date.year() - 1970;
  dateTime.tm_mon  = gps->date.month();
  dateTime.tm_mday = gps->date.day();

  dateTime.tm_hour = gps->time.hour();
  dateTime.tm_min  = gps->time.minute();
  dateTime.tm_sec  = gps->time.second();

  // FIXME TD-er: Must the offset in centisecond be added when pps_sync active?
  if (!pps_sync) {
    age += (gps->time.centisecond() * 10);
  }
  return true;
}

#endif // ifdef USES_P082
