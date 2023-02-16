#include "_Plugin_Helper.h"

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

# include <ESPeasySerial.h>
# include <TinyGPS++.h>

# include "src/DataStructs/ESPEasy_packed_raw_data.h"
# include "src/Globals/ESPEasy_time.h"
# include "src/Helpers/ESPEasy_time_calc.h"

# include "src/PluginStructs/P082_data_struct.h"

# define PLUGIN_082
# define PLUGIN_ID_082          82
# define PLUGIN_NAME_082       "Position - GPS"
# define PLUGIN_VALUENAME1_082 "Longitude"
# define PLUGIN_VALUENAME2_082 "Latitude"
# define PLUGIN_VALUENAME3_082 "Altitude"
# define PLUGIN_VALUENAME4_082 "Speed"


# define P082_TIMEOUT        PCONFIG(0)
# define P082_TIMEOUT_LABEL  PCONFIG_LABEL(0)
# define P082_BAUDRATE       PCONFIG(1)
# define P082_BAUDRATE_LABEL PCONFIG_LABEL(1)
# define P082_DISTANCE       PCONFIG(2)
# define P082_DISTANCE_LABEL PCONFIG_LABEL(2)

# define P082_QUERY1_CONFIG_POS  3
# define P082_QUERY1         PCONFIG(3) // P082_QUERY1_CONFIG_POS
# define P082_QUERY2         PCONFIG(4) // P082_QUERY1_CONFIG_POS + 1
# define P082_QUERY3         PCONFIG(5) // P082_QUERY1_CONFIG_POS + 2
# define P082_QUERY4         PCONFIG(6) // P082_QUERY1_CONFIG_POS + 3

# define P082_LONG_REF       PCONFIG_FLOAT(0)
# define P082_LAT_REF        PCONFIG_FLOAT(1)
# ifdef P082_USE_U_BLOX_SPECIFIC
#  define P082_POWER_MODE     PCONFIG(7)
#  define P082_DYNAMIC_MODEL  PCONFIG_LONG(0)
# endif // P082_USE_U_BLOX_SPECIFIC

# define P082_NR_OUTPUT_VALUES   VARS_PER_TASK


# define P082_DISTANCE_DFLT       0 // Disable update per distance travelled.
# define P082_QUERY1_DFLT         P082_query::P082_QUERY_LONG
# define P082_QUERY2_DFLT         P082_query::P082_QUERY_LAT
# define P082_QUERY3_DFLT         P082_query::P082_QUERY_ALT
# define P082_QUERY4_DFLT         P082_query::P082_QUERY_SPD

// Must use volatile declared variable (which will end up in iRAM)
volatile unsigned long P082_pps_time = 0;
void    Plugin_082_interrupt() IRAM_ATTR;

boolean Plugin_082(uint8_t function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_082;
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL_PLUS1;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].OutputDataType     = Output_Data_type_t::Simple;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_082);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P082_NR_OUTPUT_VALUES) {
          const uint8_t pconfigIndex = i + P082_QUERY1_CONFIG_POS;
          P082_query    choice       = static_cast<P082_query>(PCONFIG(pconfigIndex));
          safe_strncpy(
            ExtraTaskSettings.TaskDeviceValueNames[i],
            Plugin_082_valuename(choice, false),
            sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));

          switch (choice) {
            case P082_query::P082_QUERY_LONG:
            case P082_query::P082_QUERY_LAT:
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
        uint8_t varNr = VARS_PER_TASK;
        pluginWebformShowValue(event->TaskIndex, varNr++, F("Fix"), String(P082_data->hasFix(P082_TIMEOUT) ? 1 : 0));
        pluginWebformShowValue(event->TaskIndex, varNr++, F("Tracked"),
                               String(P082_data->gps->satellitesStats.nrSatsTracked()));
        pluginWebformShowValue(event->TaskIndex, varNr++, F("Best SNR"), String(P082_data->gps->satellitesStats.getBestSNR()), true);

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
      P082_QUERY1   = static_cast<uint8_t>(P082_QUERY1_DFLT);
      P082_QUERY2   = static_cast<uint8_t>(P082_QUERY2_DFLT);
      P082_QUERY3   = static_cast<uint8_t>(P082_QUERY3_DFLT);
      P082_QUERY4   = static_cast<uint8_t>(P082_QUERY4_DFLT);

      success = true;
      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P082_data_struct *P082_data =
        static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P082_data) && P082_data->isInitialized()) {
        const P082_query query = Plugin_082_from_valuename(string);

        if (query != P082_query::P082_NR_OUTPUT_OPTIONS) {
          const float value = P082_data->_cache[static_cast<uint8_t>(query)];
          int nrDecimals    = 2;

          if ((query == P082_query::P082_QUERY_LONG) || (query == P082_query::P082_QUERY_LAT)) {
            nrDecimals = 6;
          } else if ((query == P082_query::P082_QUERY_SATVIS) ||
                     (query == P082_query::P082_QUERY_SATUSE) ||
                     (query == P082_query::P082_QUERY_FIXQ) ||
                     (query == P082_query::P082_QUERY_CHKSUM_FAIL)) {
            nrDecimals = 0;
          }

          string  = toString(value, nrDecimals);
          success = true;
        }
      }
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      const __FlashStringHelper *options[static_cast<uint8_t>(P082_query::P082_NR_OUTPUT_OPTIONS)];

      for (uint8_t i = 0; i < static_cast<uint8_t>(P082_query::P082_NR_OUTPUT_OPTIONS); ++i) {
        options[i] = Plugin_082_valuename(static_cast<P082_query>(i), true);
      }

      for (uint8_t i = 0; i < P082_NR_OUTPUT_VALUES; ++i) {
        const uint8_t pconfigIndex = i + P082_QUERY1_CONFIG_POS;
        sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, static_cast<int>(P082_query::P082_NR_OUTPUT_OPTIONS), options);
      }
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      /*
         P082_data_struct *P082_data =
            static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));
         if (nullptr != P082_data && P082_data->isInitialized()) {
            String detectedString = F("Detected: ");
            detectedString += String(P082_data->easySerial->baudRate());
            addUnit(detectedString);
       */

      addFormNumericBox(F("Fix Timeout"), P082_TIMEOUT_LABEL, P082_TIMEOUT, 100, 10000);
      addUnit(F("ms"));

# ifdef P082_USE_U_BLOX_SPECIFIC

      addFormSubHeader(F("U-Blox specific"));

      {
        const __FlashStringHelper *options[3] = {
          toString(P082_PowerMode::Max_Performance),
          toString(P082_PowerMode::Power_Save),
          toString(P082_PowerMode::Eco)
        };
        const int indices[3] = {
          static_cast<int>(P082_PowerMode::Max_Performance),
          static_cast<int>(P082_PowerMode::Power_Save),
          static_cast<int>(P082_PowerMode::Eco)
        };
        addFormSelector(F("Power Mode"), F("pwrmode"), 3, options, indices, P082_POWER_MODE);
      }

      {
        const __FlashStringHelper *options[10] = {
          toString(P082_DynamicModel::Portable),
          toString(P082_DynamicModel::Stationary),
          toString(P082_DynamicModel::Pedestrian),
          toString(P082_DynamicModel::Automotive),
          toString(P082_DynamicModel::Sea),
          toString(P082_DynamicModel::Airborne_1g),
          toString(P082_DynamicModel::Airborne_2g),
          toString(P082_DynamicModel::Airborne_4g),
          toString(P082_DynamicModel::Wrist),
          toString(P082_DynamicModel::Bike)
        };
        const int indices[10] = {
          static_cast<int>(P082_DynamicModel::Portable),
          static_cast<int>(P082_DynamicModel::Stationary),
          static_cast<int>(P082_DynamicModel::Pedestrian),
          static_cast<int>(P082_DynamicModel::Automotive),
          static_cast<int>(P082_DynamicModel::Sea),
          static_cast<int>(P082_DynamicModel::Airborne_1g),
          static_cast<int>(P082_DynamicModel::Airborne_2g),
          static_cast<int>(P082_DynamicModel::Airborne_4g),
          static_cast<int>(P082_DynamicModel::Wrist),
          static_cast<int>(P082_DynamicModel::Bike)
        };
        addFormSelector(F("Dynamic Platform Model"), F("dynmodel"), 10, options, indices, P082_DYNAMIC_MODEL);
      }
# endif // P082_USE_U_BLOX_SPECIFIC

      addFormSubHeader(F("Current Sensor Data"));

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
        addFormSubHeader(F("Reference Point"));

        addFormFloatNumberBox(F("Latitude"),  F("lat_ref"), P082_LAT_REF,  -90.0f,  90.0f);
        addFormFloatNumberBox(F("Longitude"), F("lng_ref"), P082_LONG_REF, -180.0f, 180.0f);
      }

      addFormNumericBox(F("Distance Update Interval"), P082_DISTANCE_LABEL, P082_DISTANCE, 0, 10000);
      addUnit('m');
      addFormNote(F("0 = disable update based on distance travelled"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      # ifdef P082_USE_U_BLOX_SPECIFIC
      P082_POWER_MODE    = getFormItemInt(F("pwrmode"));
      P082_DYNAMIC_MODEL = getFormItemInt(F("dynmodel"));
      # endif // P082_USE_U_BLOX_SPECIFIC
      P082_TIMEOUT  = getFormItemInt(P082_TIMEOUT_LABEL);
      P082_DISTANCE = getFormItemInt(P082_DISTANCE_LABEL);

      P082_LONG_REF = getFormItemFloat(F("lng_ref"));
      P082_LAT_REF  = getFormItemFloat(F("lat_ref"));

      // Save output selector parameters.
      for (int i = 0; i < P082_NR_OUTPUT_VALUES; ++i) {
        const uint8_t pconfigIndex = i + P082_QUERY1_CONFIG_POS;
        const P082_query choice    = static_cast<P082_query>(PCONFIG(pconfigIndex));
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, Plugin_082_valuename(choice, false));
      }

      success = true;
      break;
    }

# if FEATURE_PLUGIN_STATS
    case PLUGIN_WEBFORM_LOAD_SHOW_STATS:
    {
      P082_data_struct *P082_data =
        static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P082_data) {
        for (uint8_t i = 0; i < P082_NR_OUTPUT_VALUES; ++i) {
          const uint8_t pconfigIndex = i + P082_QUERY1_CONFIG_POS;

          if (P082_data->webformLoad_show_stats(event, i, static_cast<P082_query>(PCONFIG(pconfigIndex)))) {
            success = true; // Something added
          }
        }
      }
      break;
    }
# endif // if FEATURE_PLUGIN_STATS

    case PLUGIN_INIT: {
      if (P082_TIMEOUT < 100) {
        P082_TIMEOUT = P082_DEFAULT_FIX_TIMEOUT;
      }
      const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);
      const int16_t serial_rx      = CONFIG_PIN1;
      const int16_t serial_tx      = CONFIG_PIN2;
      const int16_t pps_pin        = CONFIG_PIN3;
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P082_data_struct());
      P082_data_struct *P082_data =
        static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P082_data) {
        return success;
      }

      if (P082_data->init(port, serial_rx, serial_tx)) {
        success = true;
        serialHelper_log_GpioDescription(port, serial_rx, serial_tx);

        if (validGpio(pps_pin)) {
          //          pinMode(pps_pin, INPUT_PULLUP);
          attachInterrupt(pps_pin, Plugin_082_interrupt, RISING);
        }
        # ifdef P082_USE_U_BLOX_SPECIFIC
        P082_data->setPowerMode(static_cast<P082_PowerMode>(P082_POWER_MODE));
        P082_data->setDynamicModel(static_cast<P082_DynamicModel>(P082_DYNAMIC_MODEL));
        # endif // P082_USE_U_BLOX_SPECIFIC
      } else {
        clearPluginTaskData(event->TaskIndex);
      }
      break;
    }

    case PLUGIN_EXIT: {
      P082_data_struct *P082_data =
        static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P082_data) {
        P082_data->powerDown();
      }

      const int16_t pps_pin = CONFIG_PIN3;

      if (validGpio(pps_pin)) {
        detachInterrupt(pps_pin);
      }
      success = true;
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND: {
      P082_data_struct *P082_data =
        static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P082_data) && P082_data->loop()) {
        P082_setSystemTime(event);
# ifdef P082_SEND_GPS_TO_LOG

        if (P082_data->_lastSentence.substring(0, 10).indexOf(F("TXT")) != -1) {
          addLog(LOG_LEVEL_INFO, P082_data->_lastSentence);
        } else {
          #  ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, P082_data->_lastSentence);
          #  endif // ifndef BUILD_NO_DEBUG
        }
# endif            // ifdef P082_SEND_GPS_TO_LOG
        Scheduler.schedule_task_device_timer(event->TaskIndex, millis());
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
          if (Settings.UseRules) {
            eventQueue.add(curFixStatus ? F("GPS#GotFix") : F("GPS#LostFix"));
          }
          activeFix = curFixStatus;
        }
        double distance = 0.0;

        if (curFixStatus) {
          if (P082_data->gps->location.isUpdated()) {
            const float lng = P082_data->gps->location.lng();
            const float lat = P082_data->gps->location.lat();
            P082_setOutputValue(event, static_cast<uint8_t>(P082_query::P082_QUERY_LONG),     lng);
            P082_setOutputValue(event, static_cast<uint8_t>(P082_query::P082_QUERY_LAT),      lat);

            P082_setOutputValue(event, static_cast<uint8_t>(P082_query::P082_QUERY_DISTANCE), P082_data->_distance);
            const float dist_ref = P082_data->gps->distanceBetween(P082_LAT_REF, P082_LONG_REF,  lat, lng);
            P082_setOutputValue(event, static_cast<uint8_t>(P082_query::P082_QUERY_DIST_REF), dist_ref);


            if (P082_DISTANCE > 0) {
              distance = P082_data->distanceSinceLast(P082_TIMEOUT);
            }
            success = true;
            # ifndef BUILD_NO_DEBUG
            addLog(LOG_LEVEL_DEBUG, F("GPS: Position update."));
            # endif // ifndef BUILD_NO_DEBUG
          }

          if (P082_data->gps->altitude.isUpdated()) {
            // ToDo make unit selectable
            P082_setOutputValue(event, static_cast<uint8_t>(P082_query::P082_QUERY_ALT), P082_data->gps->altitude.meters());
            success = true;
            # ifndef BUILD_NO_DEBUG
            addLog(LOG_LEVEL_DEBUG, F("GPS: Altitude update."));
            # endif // ifndef BUILD_NO_DEBUG
          }

          if (P082_data->gps->speed.isUpdated()) {
            // ToDo make unit selectable
            P082_setOutputValue(event, static_cast<uint8_t>(P082_query::P082_QUERY_SPD), P082_data->gps->speed.mps());
            # ifndef BUILD_NO_DEBUG
            addLog(LOG_LEVEL_DEBUG, F("GPS: Speed update."));
            # endif // ifndef BUILD_NO_DEBUG
            success = true;
          }
        }
        P082_setOutputValue(event, static_cast<uint8_t>(P082_query::P082_QUERY_SATVIS),      P082_data->gps->satellitesStats.nrSatsVisible());
        P082_setOutputValue(event, static_cast<uint8_t>(P082_query::P082_QUERY_SATUSE),      P082_data->gps->satellitesStats.nrSatsTracked());
        P082_setOutputValue(event, static_cast<uint8_t>(P082_query::P082_QUERY_HDOP),        P082_data->gps->hdop.value() / 100.0f);
        P082_setOutputValue(event, static_cast<uint8_t>(P082_query::P082_QUERY_FIXQ),        P082_data->gps->location.Quality());
        P082_setOutputValue(event, static_cast<uint8_t>(P082_query::P082_QUERY_DB_MAX),      P082_data->gps->satellitesStats.getBestSNR());
        P082_setOutputValue(event, static_cast<uint8_t>(P082_query::P082_QUERY_CHKSUM_FAIL), P082_data->gps->failedChecksum());

        P082_logStats(event);

        if (success) {
          bool distance_passed = false;
          bool interval_passed = false;

          if (P082_DISTANCE > 0) {
            // Check travelled distance.
            if ((distance > static_cast<double>(P082_DISTANCE)) || (distance < 0.0)) {
              if (P082_data->storeCurPos(P082_TIMEOUT)) {
                distance_passed = true;

                // Add sanity check for distance travelled
                if (distance > static_cast<double>(P082_DISTANCE)) {
                  if (Settings.UseRules) {
                    String eventString = F("GPS#travelled=");
                    eventString += distance;
                    eventQueue.addMove(std::move(eventString));
                  }

                  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                    String log = F("GPS: Distance trigger : ");
                    log += distance;
                    log += F(" m");
                    addLogMove(LOG_LEVEL_INFO, log);
                  }
                }
              }
            }
          }

          if (P082_data->_last_measurement == 0) {
            interval_passed = true;
          } else if (timeOutReached(P082_data->_last_measurement + (Settings.TaskDeviceTimer[event->TaskIndex] * 1000))) {
            interval_passed = true;
          }
          success = (distance_passed || interval_passed);

          if (success) {
            P082_data->_last_measurement = millis();
          }
        }
      }
      break;
    }
    case PLUGIN_WRITE:
    {
      P082_data_struct *P082_data =
        static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P082_data) && P082_data->isInitialized()) {
        const String command    = parseString(string, 1);
        const String subcommand = parseString(string, 2);

        if (equals(command, F("gps"))) {
          if (equals(subcommand, F("wake"))) {
            success = P082_data->wakeUp();
          } else if (equals(subcommand, F("sleep"))) {
            success = P082_data->powerDown();
          }
# ifdef P082_USE_U_BLOX_SPECIFIC
          else if (equals(subcommand, F("maxperf"))) {
            success = P082_data->setPowerMode(P082_PowerMode::Max_Performance);
          } else if (equals(subcommand, F("powersave"))) {
            success = P082_data->setPowerMode(P082_PowerMode::Power_Save);
          } else if (equals(subcommand, F("eco"))) {
            success = P082_data->setPowerMode(P082_PowerMode::Eco);
          }
# endif // P082_USE_U_BLOX_SPECIFIC
        }
      }

      break;
    }
# if FEATURE_PACKED_RAW_DATA
    case PLUGIN_GET_PACKED_RAW_DATA:
    {
      P082_data_struct *P082_data =
        static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P082_data) && P082_data->isInitialized()) {
        // Matching JS code:
        // return decode(bytes, [header, latLng, latLng, altitude, uint16_1e2, hdop, uint8, uint8, uint24, uint24_1e1],
        //      ['header', 'latitude', 'longitude', 'altitude', 'speed', 'hdop', 'max_snr', 'sat_tracked', 'distance_total',
        // 'distance_ref']);
        // altitude type: return +(int16(bytes) / 4 - 1000).toFixed(1);
        string += LoRa_addFloat(P082_data->_cache[static_cast<uint8_t>(P082_query::P082_QUERY_LAT)], PackedData_latLng);
        string += LoRa_addFloat(P082_data->_cache[static_cast<uint8_t>(P082_query::P082_QUERY_LONG)], PackedData_latLng);
        string += LoRa_addFloat(P082_data->_cache[static_cast<uint8_t>(P082_query::P082_QUERY_ALT)], PackedData_altitude);
        string += LoRa_addFloat(P082_data->_cache[static_cast<uint8_t>(P082_query::P082_QUERY_SPD)], PackedData_uint16_1e2);
        string += LoRa_addFloat(P082_data->_cache[static_cast<uint8_t>(P082_query::P082_QUERY_HDOP)], PackedData_hdop);
        string += LoRa_addFloat(P082_data->_cache[static_cast<uint8_t>(P082_query::P082_QUERY_DB_MAX)], PackedData_uint8);
        string += LoRa_addFloat(P082_data->_cache[static_cast<uint8_t>(P082_query::P082_QUERY_SATUSE)], PackedData_uint8);
        string += LoRa_addFloat(P082_data->_cache[static_cast<uint8_t>(P082_query::P082_QUERY_DISTANCE)] / 1000, PackedData_uint24_1e2); //
                                                                                                                                         // Max
                                                                                                                                         // 167772.16
                                                                                                                                         // km
        event->Par1 = 8;

        if (P082_referencePointSet(event)) {
          string += LoRa_addFloat(P082_data->_cache[static_cast<uint8_t>(P082_query::P082_QUERY_DIST_REF)], PackedData_uint24_1e1); // Max
                                                                                                                                    // 1677.7216
                                                                                                                                    // km
          event->Par1 = 9;
        }

        success = true;
      }
      break;
    }
# endif // if FEATURE_PACKED_RAW_DATA
  }
  return success;
}

bool P082_referencePointSet(struct EventStruct *event) {
  return !((P082_LONG_REF < 0.1f) && (P082_LONG_REF > -0.1f)
           && (P082_LAT_REF < 0.1f) && (P082_LAT_REF > -0.1f));
}

void P082_setOutputValue(struct EventStruct *event, uint8_t outputType, float value) {
  P082_data_struct *P082_data =
    static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P082_data) || !P082_data->isInitialized()) {
    return;
  }

  if (outputType < static_cast<uint8_t>(P082_query::P082_NR_OUTPUT_OPTIONS)) {
    P082_data->_cache[outputType] = value;
  }

  for (uint8_t i = 0; i < P082_NR_OUTPUT_VALUES; ++i) {
    const uint8_t pconfigIndex = i + P082_QUERY1_CONFIG_POS;

    if (PCONFIG(pconfigIndex) == outputType) {
      UserVar[event->BaseVarIndex + i] = value;
    }
  }
}

void P082_logStats(struct EventStruct *event) {
  # ifndef BUILD_NO_DEBUG

  if (!loglevelActiveFor(LOG_LEVEL_DEBUG)) { return; }
  P082_data_struct *P082_data =
    static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P082_data) || !P082_data->isInitialized()) {
    return;
  }
  String log;

  if (log.reserve(128)) {
    log  = F("GPS:");
    log += F(" Fix: ");
    log += P082_data->hasFix(P082_TIMEOUT) ? 1 : 0;
    log += F(" #sat: ");
    log += P082_data->gps->satellites.value();
    log += F(" #SNR: ");
    log += P082_data->gps->satellitesStats.getBestSNR();
    log += F(" HDOP: ");
    log += P082_data->gps->hdop.value() / 100.0f;
    log += F(" Chksum(pass/fail): ");
    log += P082_data->gps->passedChecksum();
    log += '/';
    log += P082_data->gps->failedChecksum();
    log += F(" invalid: ");
    log += P082_data->gps->invalidData();
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
  # endif // ifndef BUILD_NO_DEBUG
}

void P082_html_show_satStats(struct EventStruct *event, bool tracked, bool onlyGPS) {
  P082_data_struct *P082_data =
    static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P082_data) || !P082_data->isInitialized()) {
    return;
  }

  bool first = true;

  for (uint8_t i = 0; i < _GPS_MAX_ARRAY_LENGTH; ++i) {
    uint8_t id  = P082_data->gps->satellitesStats.id[i];
    uint8_t snr = P082_data->gps->satellitesStats.snr[i];

    if (id > 0) {
      if (((id <= 32) == onlyGPS) && ((snr > 0) == tracked)) {
        if (first) {
          first = false;
          String label;
          label.reserve(32);

          if (onlyGPS) {
            label = F("GPS");
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
          addHtml(',', ' ');
        }
        addHtmlInt(id);

        if (tracked) {
          addHtml(' ', '(');
          addHtmlInt(snr);
          addHtml(')');
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
  addEnabled(P082_data->hasFix(P082_TIMEOUT));

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
  addHtmlInt(P082_data->gps->satellitesStats.nrSatsTracked());

  addRowLabel(F("Satellites visible"));
  addHtmlInt(P082_data->gps->satellitesStats.nrSatsVisible());

  addRowLabel(F("Best SNR"));
  addHtmlInt(P082_data->gps->satellitesStats.getBestSNR());
  addHtml(F(" dBHz"));

  // Satellites tracked or in view.
  P082_html_show_satStats(event, true,  true);
  P082_html_show_satStats(event, false, true);
  P082_html_show_satStats(event, true,  false);
  P082_html_show_satStats(event, false, false);

  addRowLabel(F("HDOP"));
  addHtmlFloat(P082_data->gps->hdop.value() / 100.0f);

  addRowLabel(F("UTC Time"));
  struct tm dateTime;
  uint32_t  age;
  bool updated;
  bool pps_sync;

  if (P082_data->getDateTime(dateTime, age, updated, pps_sync)) {
    dateTime = node_time.addSeconds(dateTime, (age / 1000), false);
    addHtml(formatDateTimeString(dateTime));
  } else {
    addHtml('-');
  }

  addRowLabel(F("Distance Travelled"));
  addHtmlInt(static_cast<int>(P082_data->_cache[static_cast<uint8_t>(P082_query::P082_QUERY_DISTANCE)]));
  addUnit('m');

  if (P082_referencePointSet(event)) {
    addRowLabel(F("Distance from Ref. Point"));
    addHtmlInt(static_cast<int>(P082_data->_cache[static_cast<uint8_t>(P082_query::P082_QUERY_DIST_REF)]));
    addUnit('m');
  }

  addRowLabel(F("Checksum (pass/fail/invalid)"));
  {
    String chksumStats;

    chksumStats  = P082_data->gps->passedChecksum();
    chksumStats += '/';
    chksumStats += P082_data->gps->failedChecksum();
    chksumStats += '/';
    chksumStats += P082_data->gps->invalidData();
    addHtml(chksumStats);
  }
}

void P082_setSystemTime(struct EventStruct *event) {
  P082_data_struct *P082_data =
    static_cast<P082_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P082_data) || !P082_data->isInitialized()) {
    return;
  }

  if ((timeSource_t::GPS_time_source == node_time.timeSource) &&
      (P082_data->_last_setSystemTime != 0) &&
      (timePassedSince(P082_data->_last_setSystemTime) < EXT_TIME_SOURCE_MIN_UPDATE_INTERVAL_MSEC))
  {
    // Only update the system time every hour from the same time source.
    return;
  }

  struct tm dateTime;
  uint32_t  age;
  bool updated;
  bool pps_sync;

  P082_data->_pps_time = P082_pps_time; // Must copy the interrupt gathered time first.

  if (P082_data->getDateTime(dateTime, age, updated, pps_sync)) {
    if (updated) {
      // Use floating point precision to use the time since last update from GPS
      // and the given offset in centisecond.
      double time = makeTime(dateTime);
      time += static_cast<double>(age) / 1000.0;
      node_time.setExternalTimeSource(time, timeSource_t::GPS_time_source);
      P082_data->_last_setSystemTime = millis();
    }
  }
  P082_pps_time = 0;
}

void Plugin_082_interrupt() {
  P082_pps_time = millis();
}

#endif // USES_P082
