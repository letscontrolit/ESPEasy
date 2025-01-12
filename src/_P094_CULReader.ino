#include "_Plugin_Helper.h"
#ifdef USES_P094

// #######################################################################################################
// #################### Plugin 094 Brick4U CUL Reader ####################################################
// #######################################################################################################
//
// Interact with Brick4U CUL receiver
// Allows to control the mode of the CUL receiver
//

# include "src/ESPEasyCore/ESPEasyNetwork.h"

# include "src/Helpers/ESPEasy_Storage.h"
# include "src/Helpers/StringConverter.h"
# include "src/PluginStructs/P094_data_struct.h"

# include <Regexp.h>

# define PLUGIN_094
# define PLUGIN_ID_094           94
# define PLUGIN_NAME_094         "Communication - CUL Reader"
# define PLUGIN_VALUENAME1_094   "v"


bool Plugin_094_match_all(taskIndex_t   taskIndex,
                          const String& received,
                          const String& source,
                          bool          fromCUL);

void Plugin_094_setFlags(struct EventStruct *event);

// Plugin settings:
// Validate:
// - [0..9]
// - "+", "-", "."
// - [A..Z]
// - [a..z]
// - ASCII 32 - 217
// Sentence start:  char
// Sentence end:  CR/CRLF/LF/char
// Max length sentence: 1k max
// Interpret as:
// - Float
// - int
// - String
// Init string (incl parsing CRLF like characters)
// Timeout between sentences.


boolean Plugin_094(uint8_t function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      auto& dev = Device[++deviceCount];
      dev.Number             = PLUGIN_ID_094;
      dev.Type               = DEVICE_TYPE_SERIAL;
      dev.VType              = Sensor_VType::SENSOR_TYPE_STRING;
      dev.OutputDataType     = Output_Data_type_t::Default;
      dev.ValueCount         = 1;
      dev.SendDataOption     = true;
      dev.TimerOption        = true;
      dev.DuplicateDetection = true;

      // FIXME TD-er: Not sure if access to any existing task data is needed when saving
      dev.ExitTaskBeforeSave = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_094);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_094));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES: {
      serialHelper_getGpioNames(event, false, true); // TX optional
      break;
    }

    case PLUGIN_WEBFORM_SHOW_VALUES:
    {
      P094_data_struct *P094_data =
        static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P094_data) && P094_data->isInitialized()) {
        uint32_t success, error, length_last;
        P094_data->getSentencesReceived(success, error, length_last);
        uint8_t varNr = VARS_PER_TASK;
        pluginWebformShowValue(event->TaskIndex, varNr++, F("Success"),     String(success));
        pluginWebformShowValue(event->TaskIndex, varNr++, F("Error"),       String(error));
        pluginWebformShowValue(event->TaskIndex, varNr++, F("Length Last"), String(length_last), true);

        // success = true;
      }
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P094_BAUDRATE              = P094_DEFAULT_BAUDRATE;
      P094_DEBUG_SENTENCE_LENGTH = 0;

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
    {
      addFormNumericBox(F("Baudrate"), P094_BAUDRATE_LABEL, P094_BAUDRATE, 2400, 115200);
      addUnit(F("baud"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormCheckBox(F("Append system time"), F("systime"), P094_GET_APPEND_RECEIVE_SYSTIME);

# if P094_DEBUG_OPTIONS
      addFormSubHeader(F("Debug Options"));
      addFormNumericBox(F("(debug) Generated length"), P094_DEBUG_SENTENCE_LABEL, P094_DEBUG_SENTENCE_LENGTH, 0, 1024);
      addFormCheckBox(F("(debug) Generate CUL data"), F("debug_data"), P094_GET_GENERATE_DEBUG_CUL_DATA);
# endif // if P094_DEBUG_OPTIONS

      addFormSubHeader(F("Filtering"));
      addFormCheckBox(F("Mute Messages"), F("mute"), P094_GET_MUTE_MESSAGES);
      P094_html_show_matchForms(event);
      addFormCheckBox(F("Enable Interval Filter"), F("interval_filter"), P094_GET_INTERVAL_FILTER);

      addFormSubHeader(F("Statistics"));
      addFormCheckBox(F("Collect W-MBus Stats"), F("collect_stats"), P094_GET_COLLECT_STATS);
      addFormNote(F("Collect reception statistics of W-MBus devices received by the CUL reader"));


      P094_html_show_stats(event);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      P094_BAUDRATE              = getFormItemInt(P094_BAUDRATE_LABEL);
      P094_DEBUG_SENTENCE_LENGTH = getFormItemInt(P094_DEBUG_SENTENCE_LABEL);

      P094_DISABLE_WINDOW_TIME_MS = getFormItemInt(F("disableTime"));
      P094_NR_FILTERS             = getFormItemInt(F("nrfilters"));


      P094_SET_APPEND_RECEIVE_SYSTIME(isFormItemChecked(F("systime")));
# if P094_DEBUG_OPTIONS
      P094_SET_GENERATE_DEBUG_CUL_DATA(isFormItemChecked(F("debug_data")));
# endif // if P094_DEBUG_OPTIONS
      P094_SET_INTERVAL_FILTER(isFormItemChecked(F("interval_filter")));
      P094_SET_MUTE_MESSAGES(isFormItemChecked(F("mute")));
      P094_SET_COLLECT_STATS(isFormItemChecked(F("collect_stats")));


      P094_data_struct *P094_data =
        static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));


      const bool localAllocated = nullptr == P094_data;

      if (localAllocated) {
        P094_data = new (std::nothrow) P094_data_struct();
      }

      if ((nullptr != P094_data)) {
        P094_data->WebformSaveFilters(event, P094_NR_FILTERS);
        success = true;

        if (localAllocated) {
          delete P094_data;
        }
      }

      break;
    }

    case PLUGIN_INIT: {
      const int16_t serial_rx      = CONFIG_PIN1;
      const int16_t serial_tx      = CONFIG_PIN2;
      const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P094_data_struct());
      P094_data_struct *P094_data =
        static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P094_data) {
        return success;
      }

      if (P094_data->init(
            port,
            serial_rx,
            serial_tx,
            P094_BAUDRATE)) {
        P094_data->setFlags(
          P094_DISABLE_WINDOW_TIME_MS,
          P094_GET_INTERVAL_FILTER,
          P094_GET_MUTE_MESSAGES,
          P094_GET_COLLECT_STATS);
        P094_data->loadFilters(event, P094_NR_FILTERS);
# if P094_DEBUG_OPTIONS
        P094_data->setGenerate_DebugCulData(P094_GET_GENERATE_DEBUG_CUL_DATA);
# endif // if P094_DEBUG_OPTIONS
        success = true;

        serialHelper_log_GpioDescription(port, serial_rx, serial_tx);
      } else {
        clearPluginTaskData(event->TaskIndex);
      }
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P094_data_struct *P094_data =
        static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P094_data) {
        P094_data->interval_filter_purgeExpired();

        if (P094_data->dump_next_stats(event->String2)) {
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLogMove(LOG_LEVEL_INFO, concat(F("CUL Reader: "), event->String2));
          }

          // Do not create events for dumping stats.
          const bool sendEvents = false;
          sendData(event, sendEvents);
        }
      }
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND: {
      if (Settings.TaskDeviceEnabled[event->TaskIndex]) {
        P094_data_struct *P094_data =
          static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

        if ((nullptr != P094_data) && P094_data->loop()) {
          // Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
          delay(0); // Processing a full sentence may take a while, run some
                    // background tasks.
          P094_data->getSentence(event->String2, P094_GET_APPEND_RECEIVE_SYSTIME);

          if (event->String2.length() > 0) {
            const bool   fromCUL = true;
            const String source  = NetworkGetHostname();

            if (Plugin_094_match_all(event->TaskIndex, event->String2, source, fromCUL)) {
              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                String log;

                if (log.reserve(128)) {
                  log = F("CUL Reader: Sending: ");
                  const size_t messageLength = event->String2.length();

                  if (messageLength < 100) {
                    log += event->String2;
                  } else {
                    // Split string so we get start and end
                    log += event->String2.substring(0, 40);
                    log += F("...");
                    log += event->String2.substring(messageLength - 40);
                  }
                  addLogMove(LOG_LEVEL_INFO, log);
                }
              }

              // Filter length options:
              // - 22 char, for hash-value then we filter the exact meter including serial and meter type, (that will also prevent very quit
              // sending meters, which normaly is a fault)
              // - 38 char, The exact message, because we have 2 uint8_t from the value payload
              // sendData_checkDuplicates(event, event->String2.substring(0, 22));
              sendData(event);
            }
          }
        }
        success = true;
      }
      break;
    }

    case PLUGIN_READ: {
      if (P094_DEBUG_SENTENCE_LENGTH > 0) {
        P094_data_struct *P094_data =
          static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

        if ((nullptr != P094_data)) {
          # if P094_DEBUG_OPTIONS
          const uint32_t debug_count = P094_data->getDebugCounter();
          event->String2.reserve(P094_DEBUG_SENTENCE_LENGTH);
          event->String2 += String(debug_count);
          event->String2 += '_';
          const char c = '0' + debug_count % 10;

          for (long i = event->String2.length(); i < P094_DEBUG_SENTENCE_LENGTH; ++i) {
            event->String2 += c;
          }
          # endif // if P094_DEBUG_OPTIONS

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLogMove(LOG_LEVEL_INFO, strformat(F("CUL Reader: Sending: %s..."),
                                                 event->String2.substring(0, 20).c_str()));
          }

          //          sendData_checkDuplicates(event, event->String2.substring(0, 22));
          sendData(event);
        }
      }
      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      const String command = parseString(string, 1);

      P094_data_struct *P094_data =
        static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P094_data) {
        if (equals(command, F("getfiltermd5"))) {
          // to output a MD5 of the currently active filters.
          string  = P094_data->getFiltersMD5();
          success = true;
        } else if (equals(command, F("getfilterenabled"))) {
          string  = P094_GET_INTERVAL_FILTER;
          success = true;
        } else if (equals(command, F("getmuteenabled"))) {
          string  = P094_GET_MUTE_MESSAGES;
          success = true;
        }
      }
      break;
    }


    case PLUGIN_WRITE: {
      const String cmd    = parseString(string, 1);
      const String subcmd = parseString(string, 2);

      if (cmd.startsWith(F("culreader"))) {
        P094_data_struct *P094_data =
          static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (equals(subcmd, F("enablefilter"))) {
          // culreader,enablefilter
          P094_SET_INTERVAL_FILTER(1);
          success = true;
        } else if (equals(subcmd, F("disablefilter"))) {
          // culreader,disablefilter
          P094_SET_INTERVAL_FILTER(0);
          success = true;
        } else if (equals(subcmd, F("mute"))) {
          // culreader,mute
          P094_SET_MUTE_MESSAGES(1);
          success = true;
        } else if (equals(subcmd, F("unmute"))) {
          // culreader,unmute
          P094_SET_MUTE_MESSAGES(0);
          success = true;
        } else if ((nullptr != P094_data)) {
          if (equals(cmd, F("culreader_write")) ||
              equals(subcmd, F("write"))) {
            // culreader,write,<text>
            String param1 = parseStringKeepCase(string, 2);
            parseSystemVariables(param1, false);
            P094_data->sendString(param1);
            addLogMove(LOG_LEVEL_INFO, param1);
            success = true;
          } else if (equals(subcmd, F("dumpstats"))) {
            // culreader,dumpstats
            P094_data->prepare_dump_stats();
            success = true;
          } else if (equals(subcmd, F("clearfilters"))) {
            // culreader,clearfilters
            P094_data->clearFilters();
            success = true;
          } else if (equals(subcmd, F("savefilters"))) {
            // culreader,savefilters
            P094_data->saveFilters(event);
            SaveSettings();
            success = true;
          } else if (equals(subcmd, F("addfilter"))) {
            // culreader,addfilter,<filter>
            // Examples for a filter definition
            //   EBZ.02.12345678;all
            //   *.02.*;15m
            //   TCH.44.*;once
            //   *.*.*;5m
            success = true;
            P094_data->addFilter(event, parseString(string, 3));
          } else if (equals(subcmd, F("setfilters"))) {
            // culreader,setfilters,<filter1>|...|<filterN>
            // Examples for a filter definition
            // culreader,setfilters,EBZ.02.12345678;all|*.02.*;15m|TCH.44.*;once|*.*.*;5m
            success = true;
            P094_data->clearFilters();
            P094_NR_FILTERS = 0;
            const String argument = parseString(string, 3);

            if (!argument.isEmpty()) {
              int argNr = 1;

              while (argNr > 0) {
                const String filter = parseString(argument, argNr, '|');

                if (!filter.isEmpty())
                {
                  P094_data->addFilter(event, filter);
                  ++argNr;
                } else {
                  argNr = 0;
                }
              }
            }
            P094_data->saveFilters(event);
            SaveSettings();
          }
        }

        if (success) {
          Plugin_094_setFlags(event);
        }
      }


      break;
    }
# ifdef USES_ESPEASY_NOW
    case PLUGIN_FILTEROUT_CONTROLLER_DATA:
    {
      // event->String1 => topic;
      // event->String2 => payload;
      if (Settings.TaskDeviceEnabled[event->TaskIndex]) {
        const bool fromCUL = false;

        if (!Plugin_094_match_all(event->TaskIndex, event->String2, event->String1, fromCUL)) {
          // Inverse as we check for filtering 'out' the messages.
          success = true;
        }
      }

      break;
    }
# endif // ifdef USES_ESPEASY_NOW
  }
  return success;
}

bool Plugin_094_match_all(taskIndex_t taskIndex, const String& received, const String& source, bool fromCUL)
{
  P094_data_struct *P094_data =
    static_cast<P094_data_struct *>(getPluginTaskData(taskIndex));

  if ((nullptr == P094_data)) {
    return false;
  }

  if (P094_data->disableFilterWindowActive()) {
    addLog(LOG_LEVEL_INFO, F("CUL Reader: Disable Filter Window active"));
    return true;
  }

  mBusPacket_t packet;
  bool res = P094_data->parsePacket(received, packet);

  # ifdef ESP8266

  if (res && fromCUL) {
  # endif // ifdef ESP8266

  // Only collect stats from the actual CUL receiver, not when processing forwarded packets.
  // On ESP8266: only collect stats on the filtered nodes or else we will likely run out of memory
  P094_data->collect_stats_add(packet, source);
  # ifdef ESP8266
}

  # endif // ifdef ESP8266

  return res;
}

void Plugin_094_setFlags(struct EventStruct *event)
{
  P094_data_struct *P094_data =
    static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr != P094_data) {
    P094_data->setFlags(
      P094_DISABLE_WINDOW_TIME_MS,
      P094_GET_INTERVAL_FILTER,
      P094_GET_MUTE_MESSAGES,
      P094_GET_COLLECT_STATS);
  }
}

void P094_html_show_matchForms(struct EventStruct *event) {
  addFormNumericBox(F("Filter Off Window after send"),
                    F("disableTime"),
                    P094_DISABLE_WINDOW_TIME_MS,
                    0,
                    60000);
  addUnit(F("msec"));
  addFormNote(F("0 = Do not turn off filter after sending to the connected device."));

  addFormNumericBox(
    F("Nr Filters"),
    F("nrfilters"),
    P094_NR_FILTERS,
    0,
    P094_MAX_NR_FILTERS);


  P094_data_struct *P094_data =
    static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

  const bool localAllocated = nullptr == P094_data;

  if (localAllocated) {
    P094_data = new (std::nothrow) P094_data_struct();

    if (nullptr != P094_data) {
      P094_data->loadFilters(event, P094_NR_FILTERS);
    }
  }

  if ((nullptr != P094_data)) {
    P094_data->WebformLoadFilters(P094_NR_FILTERS);

    if (localAllocated) {
      delete P094_data;
    }
  }
}

void P094_html_show_stats(struct EventStruct *event) {
  P094_data_struct *P094_data =
    static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P094_data) || !P094_data->isInitialized()) {
    return;
  }

  P094_data->html_show_interval_filter_stats();

  P094_data->html_show_mBus_stats();

  {
    addRowLabel(F("Current Sentence"));
    addHtml(P094_data->peekSentence());
  }

  {
    addRowLabel(F("Sentences (pass/fail)"));
    uint32_t success, error, length_last;
    P094_data->getSentencesReceived(success, error, length_last);
    addHtmlInt(success);
    addHtml('/');
    addHtmlInt(error);
    addRowLabel(F("Length Last Sentence"));
    addHtmlInt(length_last);
  }
}

#endif // USES_P094
