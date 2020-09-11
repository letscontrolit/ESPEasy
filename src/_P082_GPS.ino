#ifdef USES_P082

// #######################################################################################################
// #################### Plugin 082 GPS ###################################################################
// #######################################################################################################
//
// Read a GPS module connected via (Software)Serial
// Based on the library TinyGPS++
// http://arduiniana.org/libraries/tinygpsplus/
//
//

#include <ESPeasySerial.h>
#include <TinyGPS++.h>
#include "_Plugin_Helper.h"
#include "ESPEasy_packed_raw_data.h"

#include "src/Globals/ESPEasy_time.h"
#include "src/Helpers/ESPEasy_time_calc.h"

#define PLUGIN_082
#define PLUGIN_ID_082          82
#define PLUGIN_NAME_082       "Position - GPS [TESTING]"
#define PLUGIN_VALUENAME1_082 "Longitude"
#define PLUGIN_VALUENAME2_082 "Latitude"
#define PLUGIN_VALUENAME3_082 "Altitude"
#define PLUGIN_VALUENAME4_082 "Speed"


#define P082_TIMEOUT        PCONFIG(0)
#define P082_TIMEOUT_LABEL  PCONFIG_LABEL(0)
#define P082_BAUDRATE       PCONFIG(1)
#define P082_BAUDRATE_LABEL PCONFIG_LABEL(1)
#define P082_DISTANCE       PCONFIG(2)
#define P082_DISTANCE_LABEL PCONFIG_LABEL(2)
#define P082_QUERY1         PCONFIG(3)
#define P082_QUERY2         PCONFIG(4)
#define P082_QUERY3         PCONFIG(5)
#define P082_QUERY4         PCONFIG(6)

#define P082_NR_OUTPUT_VALUES   VARS_PER_TASK
#define P082_QUERY1_CONFIG_POS  3


#define P082_QUERY_LONG        0
#define P082_QUERY_LAT         1
#define P082_QUERY_ALT         2
#define P082_QUERY_SPD         3
#define P082_QUERY_SATVIS      4
#define P082_QUERY_SATUSE      5
#define P082_QUERY_HDOP        6
#define P082_QUERY_FIXQ        7
#define P082_QUERY_DB_MAX      8
#define P082_QUERY_CHKSUM_FAIL 9
#define P082_NR_OUTPUT_OPTIONS 10

#define P082_TIMESTAMP_AGE       1500
#define P082_DEFAULT_FIX_TIMEOUT 2500 // TTL of fix status in ms since last update
#define P082_DISTANCE_DFLT       0    // Disable update per distance travelled.
#define P082_QUERY1_DFLT         P082_QUERY_LONG
#define P082_QUERY2_DFLT         P082_QUERY_LAT
#define P082_QUERY3_DFLT         P082_QUERY_ALT
#define P082_QUERY4_DFLT         P082_QUERY_SPD


#define P082_SEND_GPS_TO_LOG

struct P082_data_struct : public PluginTaskData_base {
  P082_data_struct() : gps(nullptr), P082_easySerial(nullptr) {}

  ~P082_data_struct() {
    reset();
  }

  void reset() {
    if (gps != nullptr) {
      delete gps;
      gps = nullptr;
    }

    if (P082_easySerial != nullptr) {
      delete P082_easySerial;
      P082_easySerial = nullptr;
    }
  }

  bool init(const int16_t serial_rx, const int16_t serial_tx) {
    if (serial_rx < 0) {
      return false;
    }
    reset();
    gps             = new (std::nothrow) TinyGPSPlus();
    P082_easySerial = new (std::nothrow) ESPeasySerial(serial_rx, serial_tx);
    if (P082_easySerial != nullptr) {
      P082_easySerial->begin(9600);
    }
    return isInitialized();
  }

  bool isInitialized() const {
    return gps != nullptr && P082_easySerial != nullptr;
  }

  bool loop() {
    if (!isInitialized()) {
      return false;
    }
    bool completeSentence = false;

    if (P082_easySerial != nullptr) {
      int available = P082_easySerial->available();
      unsigned long startLoop = millis();
      while (available > 0 && timePassedSince(startLoop) < 10) {
        --available;
        char c = P082_easySerial->read();
#ifdef P082_SEND_GPS_TO_LOG
        currentSentence += c;
#endif // ifdef P082_SEND_GPS_TO_LOG

        if (gps->encode(c)) {
          // Full sentence received
#ifdef P082_SEND_GPS_TO_LOG
          lastSentence    = currentSentence;
          currentSentence = "";
#endif // ifdef P082_SEND_GPS_TO_LOG
          completeSentence = true;
        }
      }
    }
    return completeSentence;
  }

  bool hasFix(unsigned int maxAge_msec) {
    if (!isInitialized()) {
      return false;
    }
    return gps->location.isValid() && gps->location.age() < maxAge_msec;
  }

  bool storeCurPos(unsigned int maxAge_msec) {
    if (!hasFix(maxAge_msec)) {
      return false;
    }
    last_lat = gps->location.lat();
    last_lng = gps->location.lng();
    return true;
  }

  // Return the distance in meters compared to last stored position.
  // @retval  -1 when no fix.
  double distanceSinceLast(unsigned int maxAge_msec) {
    if (!hasFix(maxAge_msec)) {
      return -1.0;
    }
    if ((last_lat < 0.0001 && last_lat > -0.0001) || (last_lng < 0.0001 && last_lng > -0.0001)) {
      return -1.0;
    }
    return gps->distanceBetween(last_lat, last_lng, gps->location.lat(), gps->location.lng());
  }

  // Return the GPS time stamp, which is in UTC.
  // @param age is the time in msec since the last update of the time +
  // additional centiseconds given by the GPS.
  bool getDateTime(struct tm& dateTime, uint32_t& age, bool& pps_sync) {
    if (!isInitialized()) {
      return false;
    }

    if (pps_time != 0) {
      age      = timePassedSince(pps_time);
      pps_time = 0;
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

  TinyGPSPlus   *gps             = nullptr;
  ESPeasySerial *P082_easySerial = nullptr;

  double last_lat = 0.0;
  double last_lng = 0.0;

  unsigned long pps_time         = 0;
  unsigned long last_measurement = 0;
#ifdef P082_SEND_GPS_TO_LOG
  String lastSentence;
  String currentSentence;
#endif // ifdef P082_SEND_GPS_TO_LOG
  
  float cache[P082_NR_OUTPUT_OPTIONS] = {0};
};

// Must use volatile declared variable (which will end up in iRAM)
volatile unsigned long P082_pps_time = 0;
void    Plugin_082_interrupt() ICACHE_RAM_ATTR;

boolean Plugin_082(byte function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_082;
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL_PLUS1;
      Device[deviceCount].VType              = SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_082);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      for (byte i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P082_NR_OUTPUT_VALUES) {
          const byte pconfigIndex = i + P082_QUERY1_CONFIG_POS;
          byte choice             = PCONFIG(pconfigIndex);
          safe_strncpy(
            ExtraTaskSettings.TaskDeviceValueNames[i],
            Plugin_082_valuename(choice, false),
            sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));

          switch (choice) {
            case P082_QUERY_LONG:
            case P082_QUERY_LAT:
              ExtraTaskSettings.TaskDeviceValueDecimals[i] = 6;
              break;
            default:
              ExtraTaskSettings.TaskDeviceValueDecimals[i] = 2;
              break;
          }
        } else {
          ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
        }
      }
      break;
    }

    case PLUGIN_WEBFORM_SHOW_VALUES:
    {
      P082_data_struct *P082_data =
        static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P082_data) && P082_data->isInitialized()) {
        byte varNr = VARS_PER_TASK;
        addHtml(pluginWebformShowValue(event->TaskIndex, varNr++, F("Fix"),     String(P082_data->hasFix(P082_TIMEOUT) ? 1 : 0)));
        addHtml(pluginWebformShowValue(event->TaskIndex, varNr++, F("Tracked"),
                                       String(P082_data->gps->satellitesStats.nrSatsTracked())));
        addHtml(pluginWebformShowValue(event->TaskIndex, varNr++, F("Best SNR"), String(P082_data->gps->satellitesStats.getBestSNR()), true));

        // success = true;
      }
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES: {
      serialHelper_getGpioNames(event, false, true); // TX optional
      event->String3 = formatGpioName_input_optional(F("PPS"));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P082_TIMEOUT  = P082_DEFAULT_FIX_TIMEOUT;
      P082_DISTANCE = P082_DISTANCE_DFLT;
      P082_QUERY1   = P082_QUERY1_DFLT;
      P082_QUERY2   = P082_QUERY2_DFLT;
      P082_QUERY3   = P082_QUERY3_DFLT;
      P082_QUERY4   = P082_QUERY4_DFLT;

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      serialHelper_webformLoad(event);

      /*
         P082_data_struct *P082_data =
            static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));
         if (nullptr != P082_data && P082_data->isInitialized()) {
            String detectedString = F("Detected: ");
            detectedString += String(P082_data->P082_easySerial->baudRate());
            addUnit(detectedString);
       */

      addFormNumericBox(F("Fix Timeout"), P082_TIMEOUT_LABEL, P082_TIMEOUT, 100, 10000);
      addUnit(F("ms"));

      P082_html_show_stats(event);

      // Settings to add:
      // Speed unit
      // Altitude unit
      // Set system time
      // Timeout in msec to consider still active fix.
      // Update interval: seconds, distance travelled
      // Position filtering
      // Speed filtering
      //
      // What to do with:
      // nr satellites
      // HDOP
      // fixQuality, fixMode
      // statistics (chars processed, failed checksum)

      {
        // In a separate scope to free memory of String array as soon as possible
        sensorTypeHelper_webformLoad_header();
        String options[P082_NR_OUTPUT_OPTIONS];

        for (int i = 0; i < P082_NR_OUTPUT_OPTIONS; ++i) {
          options[i] = Plugin_082_valuename(i, true);
        }

        for (byte i = 0; i < P082_NR_OUTPUT_VALUES; ++i) {
          const byte pconfigIndex = i + P082_QUERY1_CONFIG_POS;
          sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, P082_NR_OUTPUT_OPTIONS, options);
        }
      }

      addFormNumericBox(F("Distance Update Interval"), P082_DISTANCE_LABEL, P082_DISTANCE, 0, 10000);
      addUnit(F("m"));
      addFormNote(F("0 = disable update based on distance travelled"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      serialHelper_webformSave(event);
      P082_TIMEOUT  = getFormItemInt(P082_TIMEOUT_LABEL);
      P082_DISTANCE = getFormItemInt(P082_DISTANCE_LABEL);

      // Save output selector parameters.
      for (byte i = 0; i < P082_NR_OUTPUT_VALUES; ++i) {
        const byte pconfigIndex = i + P082_QUERY1_CONFIG_POS;
        const byte choice       = PCONFIG(pconfigIndex);
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, Plugin_082_valuename(choice, false));
      }

      success = true;
      break;
    }

    case PLUGIN_INIT: {
      if (P082_TIMEOUT < 100) {
        P082_TIMEOUT = P082_DEFAULT_FIX_TIMEOUT;
      }
      const int16_t serial_rx = CONFIG_PIN1;
      const int16_t serial_tx = CONFIG_PIN2;
      const int16_t pps_pin   = CONFIG_PIN3;
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P082_data_struct());
      P082_data_struct *P082_data =
        static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P082_data) {
        return success;
      }

      if (P082_data->init(serial_rx, serial_tx)) {
        success = true;
        serialHelper_log_GpioDescription(serial_rx, serial_tx);

        if (pps_pin != -1) {
          //          pinMode(pps_pin, INPUT_PULLUP);
          attachInterrupt(pps_pin, Plugin_082_interrupt, RISING);
        }
      } else {
        clearPluginTaskData(event->TaskIndex);
      }
      break;
    }

    case PLUGIN_EXIT: {
      clearPluginTaskData(event->TaskIndex);
      const int16_t pps_pin = CONFIG_PIN3;

      if (pps_pin != -1) {
        detachInterrupt(pps_pin);
      }
      success = true;
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND: {
      P082_data_struct *P082_data =
        static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P082_data) && P082_data->loop()) {
#ifdef P082_SEND_GPS_TO_LOG
        addLog(LOG_LEVEL_DEBUG, P082_data->lastSentence);
#endif // ifdef P082_SEND_GPS_TO_LOG
        Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
        delay(0); // Processing a full sentence may take a while, run some
                  // background tasks.
      }
      success = true;
      break;
    }

    case PLUGIN_READ: {
      P082_data_struct *P082_data =
        static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P082_data) && P082_data->isInitialized()) {
        static bool activeFix    = P082_data->hasFix(P082_TIMEOUT);
        const bool  curFixStatus = P082_data->hasFix(P082_TIMEOUT);

        if (activeFix != curFixStatus) {
          // Fix status changed, send events.
          String event = curFixStatus ? F("GPS#GotFix") : F("GPS#LostFix");
          eventQueue.add(event);
          activeFix = curFixStatus;
        }
        double distance = 0.0;

        if (curFixStatus) {
          if (P082_data->gps->location.isUpdated()) {
            P082_setOutputValue(event, P082_QUERY_LONG, P082_data->gps->location.lng());
            P082_setOutputValue(event, P082_QUERY_LAT,  P082_data->gps->location.lat());

            if (P082_DISTANCE > 0) {
              distance = P082_data->distanceSinceLast(P082_TIMEOUT);
            }
            success = true;
            addLog(LOG_LEVEL_DEBUG, F("GPS: Position update."));
          }

          if (P082_data->gps->altitude.isUpdated()) {
            // ToDo make unit selectable
            P082_setOutputValue(event, P082_QUERY_ALT, P082_data->gps->altitude.meters());
            success = true;
            addLog(LOG_LEVEL_DEBUG, F("GPS: Altitude update."));
          }

          if (P082_data->gps->speed.isUpdated()) {
            // ToDo make unit selectable
            P082_setOutputValue(event, P082_QUERY_SPD, P082_data->gps->speed.mps());
            addLog(LOG_LEVEL_DEBUG, F("GPS: Speed update."));
            success = true;
          }
        }
        P082_setOutputValue(event, P082_QUERY_SATVIS,      P082_data->gps->satellitesStats.nrSatsVisible());
        P082_setOutputValue(event, P082_QUERY_SATUSE,      P082_data->gps->satellitesStats.nrSatsTracked());
        P082_setOutputValue(event, P082_QUERY_HDOP,        P082_data->gps->hdop.value() / 100.0);
        P082_setOutputValue(event, P082_QUERY_FIXQ,        P082_data->gps->location.Quality());
        P082_setOutputValue(event, P082_QUERY_DB_MAX,      P082_data->gps->satellitesStats.getBestSNR());
        P082_setOutputValue(event, P082_QUERY_CHKSUM_FAIL, P082_data->gps->failedChecksum());

        if (curFixStatus) {
          P082_setSystemTime(event);
        }
        P082_logStats(event);

        if (success) {
          bool distance_passed = false;
          bool interval_passed = false;

          if (P082_DISTANCE > 0) {
            // Check travelled distance.
            if (distance > static_cast<double>(P082_DISTANCE) || distance < 0.0) {
              if (P082_data->storeCurPos(P082_TIMEOUT)) {
                distance_passed = true;

                // Add sanity check for distance travelled
                if (distance > static_cast<double>(P082_DISTANCE)) {
                  String eventString = F("GPS#travelled=");
                  eventString += distance;
                  eventQueue.add(eventString);

                  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                    String log = F("GPS: Distance trigger : ");
                    log += distance;
                    log += F(" m");
                    addLog(LOG_LEVEL_INFO, log);
                  }
                }
              }
            }
          }

          if (P082_data->last_measurement == 0) {
            interval_passed = true;
          } else if (timeOutReached(P082_data->last_measurement + (Settings.TaskDeviceTimer[event->TaskIndex] * 1000))) {
            interval_passed = true;
          }
          success = (distance_passed || interval_passed);

          if (success) {
            P082_data->last_measurement = millis();
          }
        }
      }
      break;
    }
#ifdef USES_PACKED_RAW_DATA
    case PLUGIN_GET_PACKED_RAW_DATA:
    {
      P082_data_struct *P082_data =
        static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P082_data) && P082_data->isInitialized()) {
        // Matching JS code:
        // return decode(bytes, [header, latLng, latLng, altitude, uint16_1e2, hdop, uint8, uint8],
        //      ['header', 'latitude', 'longitude', 'altitude', 'speed', 'hdop', 'max_snr', 'sat_tracked']);
        // altitude type: return +(int16(bytes) / 4 - 1000).toFixed(1);
        string += LoRa_addFloat(P082_data->cache[P082_QUERY_LAT], PackedData_latLng);
        string += LoRa_addFloat(P082_data->cache[P082_QUERY_LONG], PackedData_latLng);
        string += LoRa_addFloat(P082_data->cache[P082_QUERY_ALT], PackedData_altitude);
        string += LoRa_addFloat(P082_data->cache[P082_QUERY_SPD], PackedData_uint16_1e2);
        string += LoRa_addFloat(P082_data->cache[P082_QUERY_HDOP], PackedData_hdop);
        string += LoRa_addFloat(P082_data->cache[P082_QUERY_DB_MAX], PackedData_uint8);
        string += LoRa_addFloat(P082_data->cache[P082_QUERY_SATUSE], PackedData_uint8);
        event->Par1 = 7; // valuecount 7 
        
        success = true;
      }
      break;
    }
#endif // USES_PACKED_RAW_DATA

  }
  return success;
}

void P082_setOutputValue(struct EventStruct *event, byte outputType, float value) {
  P082_data_struct *P082_data =
    static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P082_data) || !P082_data->isInitialized()) {
    return;
  }
  if (outputType < P082_NR_OUTPUT_OPTIONS)
    P082_data->cache[outputType] = value;

  for (byte i = 0; i < P082_NR_OUTPUT_VALUES; ++i) {
    const byte pconfigIndex = i + P082_QUERY1_CONFIG_POS;

    if (PCONFIG(pconfigIndex) == outputType) {
      UserVar[event->BaseVarIndex + i] = value;
    }
  }
}

void P082_logStats(struct EventStruct *event) {
  if (!loglevelActiveFor(LOG_LEVEL_DEBUG)) { return; }
  P082_data_struct *P082_data =
    static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P082_data) || !P082_data->isInitialized()) {
    return;
  }
  String log;
  log.reserve(128);
  log  = F("GPS:");
  log += F(" Fix: ");
  log += String(P082_data->hasFix(P082_TIMEOUT));
  log += F(" #sat: ");
  log += P082_data->gps->satellites.value();
  log += F(" #SNR: ");
  log += P082_data->gps->satellitesStats.getBestSNR();
  log += F(" HDOP: ");
  log += P082_data->gps->hdop.value() / 100.0;
  log += F(" Chksum(pass/fail): ");
  log += P082_data->gps->passedChecksum();
  log += '/';
  log += P082_data->gps->failedChecksum();
  log += F(" invalid: ");
  log += P082_data->gps->invalidData();
  addLog(LOG_LEVEL_DEBUG, log);
}

void P082_html_show_satStats(struct EventStruct *event, bool tracked, bool onlyGPS) {
  P082_data_struct *P082_data =
    static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P082_data) || !P082_data->isInitialized()) {
    return;
  }

  bool first = true;

  for (byte i = 0; i < _GPS_MAX_ARRAY_LENGTH; ++i) {
    uint8_t id  = P082_data->gps->satellitesStats.id[i];
    uint8_t snr = P082_data->gps->satellitesStats.snr[i];

    if (id > 0) {
      if (((id <= 32) == onlyGPS) && ((snr > 0) == tracked)) {
        if (first) {
          first = false;
          String label;
          label.reserve(32);

          if (onlyGPS) {
            label = "GPS";
          } else {
            label = F("Other");
          }
          label += F(" sat. ");

          if (tracked) {
            label += F("tracked - id(SNR)");
          } else {
            label += F("in view - id");
          }
          addRowLabel(label);
        } else {
          addHtml(", ");
        }
        addHtml(String(id));

        if (tracked) {
          addHtml(  " (");
          addHtml(String(snr));
          addHtml(   ")");
        }
      }
    }
  }

  if (!first) {
    // Something was added, so add the unit here
    if (tracked) {
      html_I(F(" - SNR in dBHz"));
    }
  }
}

void P082_html_show_stats(struct EventStruct *event) {
  P082_data_struct *P082_data =
    static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P082_data) || !P082_data->isInitialized()) {
    return;
  }
  addRowLabel(F("Fix"));
  addHtml(String(P082_data->hasFix(P082_TIMEOUT)));

  addRowLabel(F("Fix Quality"));

  switch (P082_data->gps->location.Quality()) {
    case 0: addHtml(F("Invalid")); break;
    case 1: addHtml(F("GPS")); break;
    case 2: addHtml(F("DGPS")); break;
    case 3: addHtml(F("PPS")); break;
    case 4: addHtml(F("RTK")); break;
    case 5: addHtml(F("FloatRTK")); break;
    case 6: addHtml(F("Estimated")); break;
    case 7: addHtml(F("Manual")); break;
    case 8: addHtml(F("Simulated")); break;
    default:
      addHtml(F("Unknown"));
      break;
  }

  addRowLabel(F("Satellites tracked"));
  addHtml(String(P082_data->gps->satellitesStats.nrSatsTracked()));

  addRowLabel(F("Satellites visible"));
  addHtml(String(P082_data->gps->satellitesStats.nrSatsVisible()));

  addRowLabel(F("Best SNR"));
  addHtml(String(P082_data->gps->satellitesStats.getBestSNR()));
  addHtml(     F(" dBHz"));

  // Satellites tracked or in view.
  P082_html_show_satStats(event, true,  true);
  P082_html_show_satStats(event, false, true);
  P082_html_show_satStats(event, true,  false);
  P082_html_show_satStats(event, false, false);

  addRowLabel(F("HDOP"));
  addHtml(String(P082_data->gps->hdop.value() / 100.0));

  addRowLabel(F("UTC Time"));
  struct tm dateTime;
  uint32_t  age;
  bool pps_sync;

  if (P082_data->getDateTime(dateTime, age, pps_sync)) {
    dateTime = node_time.addSeconds(dateTime, (age / 1000), false);
    addHtml(ESPEasy_time::getDateTimeString(dateTime));
  } else {
    addHtml(F("-"));
  }

  addRowLabel(F("Checksum (pass/fail/invalid)"));
  String chksumStats;
  chksumStats  = P082_data->gps->passedChecksum();
  chksumStats += '/';
  chksumStats += P082_data->gps->failedChecksum();
  chksumStats += '/';
  chksumStats += P082_data->gps->invalidData();
  addHtml(chksumStats);
}

void P082_setSystemTime(struct EventStruct *event) {
  P082_data_struct *P082_data =
    static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P082_data) || !P082_data->isInitialized()) {
    return;
  }

  // Set the externalTimesource 10 seconds earlier to make sure no call is made
  // to NTP (if set)
  if (node_time.nextSyncTime > (node_time.sysTime + 10)) {
    return;
  }

  struct tm dateTime;
  uint32_t  age;
  bool pps_sync;
  P082_data->pps_time = P082_pps_time; // Must copy the interrupt gathered time first.

  if (P082_data->getDateTime(dateTime, age, pps_sync)) {
    // Use floating point precision to use the time since last update from GPS
    // and the given offset in centisecond.
    double time = makeTime(dateTime);
    time += static_cast<double>(age) / 1000.0;
    node_time.setExternalTimeSource(time, GPS_time_source);
    node_time.initTime();
  }
  P082_pps_time = 0;
}

void Plugin_082_interrupt() {
  P082_pps_time = millis();
}

String Plugin_082_valuename(byte value_nr, bool displayString) {
  switch (value_nr) {
    case P082_QUERY_LONG: return displayString ? F("Longitude")          : F("long");
    case P082_QUERY_LAT: return displayString ? F("Latitude")           : F("lat");
    case P082_QUERY_ALT: return displayString ? F("Altitude")           : F("alt");
    case P082_QUERY_SPD: return displayString ? F("Speed (m/s)")        : F("spd");
    case P082_QUERY_SATVIS: return displayString ? F("Satellites Visible") : F("sat_vis");
    case P082_QUERY_SATUSE: return displayString ? F("Satellites Tracked") : F("sat_tr");
    case P082_QUERY_HDOP: return displayString ? F("HDOP")               : F("hdop");
    case P082_QUERY_FIXQ: return displayString ? F("Fix Quality")        : F("fix_qual");
    case P082_QUERY_DB_MAX: return displayString ? F("Max SNR in dBHz")    : F("snr_max");
  }
  return "";
}

#endif // USES_P082
