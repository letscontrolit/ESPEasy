#ifdef USES_P082
//#######################################################################################################
//#################### Plugin 082 GPS
//###################################################################
//#######################################################################################################
//
// Read a GPS module connected via (Software)Serial
// Based on the library TinyGPS++
// http://arduiniana.org/libraries/tinygpsplus/
//
//

#include <ESPeasySerial.h>
#include <TinyGPS++.h>

#define PLUGIN_082
#define PLUGIN_ID_082          82
#define PLUGIN_NAME_082       "Position - GPS [TESTING]"
#define PLUGIN_VALUENAME1_082 "Longitude"
#define PLUGIN_VALUENAME2_082 "Latitude"
#define PLUGIN_VALUENAME3_082 "Altitude"
#define PLUGIN_VALUENAME4_082 "Speed"

#define P028_LONGITUDE         UserVar[event->BaseVarIndex + 0]
#define P028_LATITUDE          UserVar[event->BaseVarIndex + 1]
#define P028_ALTITUDE          UserVar[event->BaseVarIndex + 2]
#define P028_SPEED             UserVar[event->BaseVarIndex + 3]

#define P028_DEFAULT_FIX_TIMEOUT 2500

struct P082_data_struct {
  P082_data_struct() {}

  ~P082_data_struct() { reset(); }

  void reset() {
    if (gps != nullptr) {
      delete gps;
    }
    if (easySerial != nullptr) {
      delete easySerial;
    }
  }

  bool init(const int16_t serial_rx, const int16_t serial_tx) {
    if (serial_rx < 0 || serial_tx < 0)
      return false;
    reset();
    gps = new TinyGPSPlus();
    easySerial = new ESPeasySerial(serial_rx, serial_tx);
    easySerial->begin(9600);
    return isInitialized();
  }

  bool isInitialized() const {
    return gps != nullptr && easySerial != nullptr;
  }

  bool loop() {
    if (!isInitialized())
      return false;
    bool fullSentenceReceived = false;
    if (easySerial != nullptr) {
      while (easySerial->available() > 0) {
        if (gps->encode(easySerial->read())) {
          fullSentenceReceived = true;
        }
      }
    }
    return fullSentenceReceived;
  }

  bool hasFix(unsigned int maxAge_msec) {
    if (!isInitialized())
      return false;
    return (gps->location.isValid() && gps->location.age() < maxAge_msec);
  }

  bool storeCurPos(unsigned int maxAge_msec) {
    if (!hasFix(maxAge_msec))
      return false;
    last_lat = gps->location.lat();
    last_lng = gps->location.lng();
  }

  TinyGPSPlus *gps = nullptr;
  ESPeasySerial *easySerial = nullptr;

  double last_lat = 0.0;
  double last_lng = 0.0;
} P082_data;

boolean Plugin_082(byte function, struct EventStruct *event, String &string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number = PLUGIN_ID_082;
      Device[deviceCount].Type = DEVICE_TYPE_DUAL;
      Device[deviceCount].VType = SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports = 0;
      Device[deviceCount].PullUpOption = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption = true;
      Device[deviceCount].ValueCount = 4;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption = true;
      Device[deviceCount].GlobalSyncOption = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_082);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_082));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_082));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_082));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_082));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES: {
      event->String1 = formatGpioName_RX(false);
      event->String2 = formatGpioName_TX(false);
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
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
      //
      // Show some statistics on the load page.
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      success = true;
      break;
    }

    case PLUGIN_INIT: {
      const int16_t serial_rx = Settings.TaskDevicePin1[event->TaskIndex];
      const int16_t serial_tx = Settings.TaskDevicePin2[event->TaskIndex];
      if (P082_data.init(serial_rx, serial_tx)) {
        success = true;
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("GPS  : Init OK  ESP GPIO-pin RX:");
          log += serial_rx;
          log += F(" TX:");
          log += serial_tx;
          addLog(LOG_LEVEL_INFO, log);
        }
      }
      break;
    }

    case PLUGIN_EXIT: {
      P082_data.reset();
      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND: {
      if (P082_data.loop()) {
        schedule_task_device_timer(event->TaskIndex, millis() + 10);
      }
      success = true;
      break;
    }

    case PLUGIN_READ: {
      if (P082_data.isInitialized()) {
        static bool activeFix = P082_data.hasFix(P028_DEFAULT_FIX_TIMEOUT);
        const bool curFixStatus = P082_data.hasFix(P028_DEFAULT_FIX_TIMEOUT);
        if (activeFix != curFixStatus) {
          // Fix status changed, send events.
          String event = curFixStatus ? F("GPS#GotFix") : F("GPS#LostFix");
          rulesProcessing(event);
          activeFix = curFixStatus;
        }

        if (P082_data.hasFix(P028_DEFAULT_FIX_TIMEOUT)) {
          if (P082_data.gps->location.isUpdated()) {
            P028_LONGITUDE = P082_data.gps->location.lng();
            P028_LATITUDE = P082_data.gps->location.lat();
            success = true;
            addLog(LOG_LEVEL_INFO, F("GPS: Position update."));
          }
          if (P082_data.gps->altitude.isUpdated()) {
            // ToDo make unit selectable
            P028_ALTITUDE = P082_data.gps->altitude.meters();
            success = true;
            addLog(LOG_LEVEL_INFO, F("GPS: Altitude update."));
          }
          if (P082_data.gps->speed.isUpdated()) {
            // ToDo make unit selectable
            P028_SPEED = P082_data.gps->speed.mps();
            addLog(LOG_LEVEL_INFO, F("GPS: Speed update."));
            success = true;
          }
        }
        P082_logStats(event);
      }
      break;
    }
  }
  return success;
}

void P082_logStats(struct EventStruct *event) {
  if (!P082_data.isInitialized())
    return;
  if (!loglevelActiveFor(LOG_LEVEL_INFO))
    return;

  String log;
  log.reserve(96);
  log = F("GPS:");
  log += F(" Fix: ");
  log += String(P082_data.hasFix(P028_DEFAULT_FIX_TIMEOUT));
  log += F(" Long: ");
  log += P028_LONGITUDE;
  log += F(" Lat: ");
  log += P028_LATITUDE;
  log += F(" Alt: ");
  log += P028_ALTITUDE;
  log += F(" Spd: ");
  log += P028_SPEED;

  log += F(" Chksum(pass/fail): ");
  log += P082_data.gps->passedChecksum();
  log += '/';
  log += P082_data.gps->failedChecksum();

  addLog(LOG_LEVEL_INFO, log);
}

#endif // USES_P082
