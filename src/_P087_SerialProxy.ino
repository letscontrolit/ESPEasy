#include "_Plugin_Helper.h"
#ifdef USES_P087

// #######################################################################################################
// #################### Plugin 087 Serial Proxy ##########################################################
// #######################################################################################################
//
// Interact with a device connected to serial
// Allows to redirect data to a controller
//

/**
 * Changelog:
 * 2023-03-25 tonhuisman: Change serialproxy_writemix to handle 0x00 also, by implementing parseHexTextData()
 * 2023-03-22 tonhuisman: Add command serialproxy_writemix to handle mixed hex characters and text to send
 *                        using parseHexTextString()
 *                        Format source using Uncrustify
 * 2022-07-08 tonhuisman: Allow baudrate lowest value to 300 (from 2400)
 *                        Don't trim off pre/post white-space from string to send
 * 2022-07-07 tonhuisman: Add selection for serial protocol configuration (databits, parity, nr. of stopbits)
 * 2022-07 First recorded changelog
 **/


# include "src/PluginStructs/P087_data_struct.h"

# include <Regexp.h>

# define PLUGIN_087
# define PLUGIN_ID_087           87
# define PLUGIN_NAME_087         "Communication - Serial Proxy"


# define P087_BAUDRATE           PCONFIG_LONG(0)
# define P087_BAUDRATE_LABEL     PCONFIG_LABEL(0)
# define P087_SERIAL_CONFIG      PCONFIG_LONG(1)

# define P087_QUERY_VALUE        0 // Temp placement holder until we know what selectors are needed.
# define P087_NR_OUTPUT_OPTIONS  1

# define P087_NR_OUTPUT_VALUES   1
# define P087_QUERY1_CONFIG_POS  3

# define P087_DEFAULT_BAUDRATE   38400


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


boolean Plugin_087(uint8_t function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_087;
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_STRING;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = false;

      // FIXME TD-er: Not sure if access to any existing task data is needed when saving
      Device[deviceCount].ExitTaskBeforeSave = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_087);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P087_NR_OUTPUT_VALUES) {
          const uint8_t pconfigIndex = i + P087_QUERY1_CONFIG_POS;
          uint8_t choice             = PCONFIG(pconfigIndex);
          safe_strncpy(
            ExtraTaskSettings.TaskDeviceValueNames[i],
            Plugin_087_valuename(choice, false),
            sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
        } else {
          ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
        }
      }
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES: {
      serialHelper_getGpioNames(event, false, true); // TX optional
      break;
    }

    case PLUGIN_WEBFORM_SHOW_VALUES:
    {
      P087_data_struct *P087_data =
        static_cast<P087_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P087_data) && P087_data->isInitialized()) {
        uint32_t success, error, length_last;
        P087_data->getSentencesReceived(success, error, length_last);
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
      P087_BAUDRATE = P087_DEFAULT_BAUDRATE;

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
      addFormNumericBox(F("Baudrate"), P087_BAUDRATE_LABEL, P087_BAUDRATE, 300, 115200);
      addUnit(F("baud"));
      uint8_t serialConfChoice = serialHelper_convertOldSerialConfig(P087_SERIAL_CONFIG);
      serialHelper_serialconfig_webformLoad(event, serialConfChoice);
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      addFormSubHeader(F("Filtering"));
      P087_html_show_matchForms(event);

      addFormSubHeader(F("Statistics"));
      P087_html_show_stats(event);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      P087_BAUDRATE      = getFormItemInt(P087_BAUDRATE_LABEL);
      P087_SERIAL_CONFIG = serialHelper_serialconfig_webformSave();

      P087_data_struct *P087_data =
        static_cast<P087_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P087_data) {
        for (uint8_t varNr = 0; varNr < P87_Nlines; varNr++)
        {
          P087_data->setLine(varNr, webArg(getPluginCustomArgName(varNr)));
        }

        addHtmlError(SaveCustomTaskSettings(event->TaskIndex, P087_data->_lines, P87_Nlines, 0));
        success = true;
      }

      break;
    }

    case PLUGIN_INIT: {
      const int16_t serial_rx      = CONFIG_PIN1;
      const int16_t serial_tx      = CONFIG_PIN2;
      const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P087_data_struct());
      P087_data_struct *P087_data =
        static_cast<P087_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P087_data) {
        return success;
      }

      if (P087_data->init(port, serial_rx, serial_tx, P087_BAUDRATE, static_cast<uint8_t>(P087_SERIAL_CONFIG))) {
        LoadCustomTaskSettings(event->TaskIndex, P087_data->_lines, P87_Nlines, 0);
        P087_data->post_init();
        success = true;
        serialHelper_log_GpioDescription(port, serial_rx, serial_tx);
      } else {
        clearPluginTaskData(event->TaskIndex);
      }
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND: {
      if (Settings.TaskDeviceEnabled[event->TaskIndex]) {
        P087_data_struct *P087_data =
          static_cast<P087_data_struct *>(getPluginTaskData(event->TaskIndex));

        if ((nullptr != P087_data) && P087_data->loop()) {
          Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
          delay(0); // Processing a full sentence may take a while, run some
                    // background tasks.
        }
        success = true;
      }
      break;
    }

    case PLUGIN_READ: {
      P087_data_struct *P087_data =
        static_cast<P087_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P087_data) && P087_data->getSentence(event->String2)) {
        if (Plugin_087_match_all(event->TaskIndex, event->String2)) {
          //          sendData(event);
# ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, event->String2);
# endif // ifndef BUILD_NO_DEBUG
          success = true;
        }
      }

      if ((nullptr != P087_data)) {}
      break;
    }

    case PLUGIN_WRITE: {
      P087_data_struct *P087_data =
        static_cast<P087_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P087_data)) {
        String cmd = parseString(string, 1);

        if (equals(cmd, F("serialproxy_write"))) {
          String param1 = parseStringKeepCase(string, 2, ',', false); // Don't trim off white-space
          parseSystemVariables(param1, false);                        // FIXME tonhuisman: Doesn't seem to be needed?
          P087_data->sendString(param1);
          addLogMove(LOG_LEVEL_INFO, param1);                         // FIXME tonhuisman: Should we always want to write to the log?
          success = true;
        } else
        if (equals(cmd, F("serialproxy_writemix"))) {
          std::vector<uint8_t> param1 = parseHexTextData(string);
          P087_data->sendData(&param1[0], param1.size());
          success = true;
        }
      }

      break;
    }
  }
  return success;
}

bool Plugin_087_match_all(taskIndex_t taskIndex, String& received)
{
  P087_data_struct *P087_data =
    static_cast<P087_data_struct *>(getPluginTaskData(taskIndex));

  if ((nullptr == P087_data)) {
    return false;
  }


  if (P087_data->disableFilterWindowActive()) {
    addLog(LOG_LEVEL_INFO, F("Serial Proxy: Disable Filter Window active"));
    return true;
  }

  bool res = P087_data->matchRegexp(received);

  if (P087_data->invertMatch()) {
    addLog(LOG_LEVEL_INFO, F("Serial Proxy: invert filter"));
    return !res;
  }
  return res;
}

String Plugin_087_valuename(uint8_t value_nr, bool displayString) {
  switch (value_nr) {
    case P087_QUERY_VALUE: return displayString ? F("Value")          : F("v");
  }
  return "";
}

void P087_html_show_matchForms(struct EventStruct *event) {
  P087_data_struct *P087_data =
    static_cast<P087_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr != P087_data)) {
    addFormTextBox(F("RegEx"), getPluginCustomArgName(P087_REGEX_POS), P087_data->getRegEx(), P87_Nchars);
    addFormNote(F("Captures are specified using round brackets."));

    addFormNumericBox(F("Nr Chars use in regex"), getPluginCustomArgName(P087_NR_CHAR_USE_POS), P087_data->getRegExpMatchLength(), 0, 1024);
    addFormNote(F("0 = Use all of the received string."));

    addFormNumericBox(F("Filter Off Window after send"),
                      getPluginCustomArgName(P087_FILTER_OFF_WINDOW_POS),
                      P087_data->getFilterOffWindowTime(),
                      0,
                      60000);
    addUnit(F("msec"));
    addFormNote(F("0 = Do not turn off filter after sending to the connected device."));

    {
      const __FlashStringHelper *options[P087_Match_Type_NR_ELEMENTS];
      int optionValues[P087_Match_Type_NR_ELEMENTS];

      for (int i = 0; i < P087_Match_Type_NR_ELEMENTS; ++i) {
        P087_Match_Type matchType = static_cast<P087_Match_Type>(i);
        options[i]      = P087_data_struct::MatchType_toString(matchType);
        optionValues[i] = matchType;
      }
      P087_Match_Type choice = P087_data->getMatchType();
      addFormSelector(F("Match Type"),
                      getPluginCustomArgName(P087_MATCH_TYPE_POS),
                      P087_Match_Type_NR_ELEMENTS,
                      options,
                      optionValues,
                      choice,
                      false);
      addFormNote(F("Capture filter can only be used on Global Match"));
    }


    uint8_t lineNr              = 0;
    uint8_t capture             = 0;
    P087_Filter_Comp comparator = P087_Filter_Comp::Equal;
    String filter;

    for (uint8_t varNr = P087_FIRST_FILTER_POS; varNr < P87_Nlines; ++varNr)
    {
      String id = getPluginCustomArgName(varNr);

      switch (varNr % 3) {
        case 0:
        {
          // Label + first parameter
          filter = P087_data->getFilter(lineNr, capture, comparator);
          ++lineNr;
          String label;
          label  = F("Capture Filter ");
          label += String(lineNr);
          addRowLabel_tr_id(label, id);

          addNumericBox(id, capture, -1, P87_MAX_CAPTURE_INDEX);
          break;
        }
        case 1:
        {
          // Comparator
          const __FlashStringHelper *options[2];
          options[P087_Filter_Comp::Equal]    = F("==");
          options[P087_Filter_Comp::NotEqual] = F("!=");
          int optionValues[2] = { P087_Filter_Comp::Equal, P087_Filter_Comp::NotEqual };
          addSelector(id, 2, options, optionValues, nullptr, static_cast<int>(comparator), false, true, F(""));
          break;
        }
        case 2:
        {
          // Compare with
          addTextBox(id, filter, 32, false, false, EMPTY_STRING, F(""));
          break;
        }
      }
    }
  }
}

void P087_html_show_stats(struct EventStruct *event) {
  P087_data_struct *P087_data =
    static_cast<P087_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P087_data) || !P087_data->isInitialized()) {
    return;
  }
  {
    addRowLabel(F("Current Sentence"));
    String sentencePart;
    P087_data->getSentence(sentencePart);
    addHtml(sentencePart);
  }

  {
    addRowLabel(F("Sentences (pass/fail)"));
    String   chksumStats;
    uint32_t success, error, length_last;
    P087_data->getSentencesReceived(success, error, length_last);
    chksumStats  = success;
    chksumStats += '/';
    chksumStats += error;
    addHtml(chksumStats);
    addRowLabel(F("Length Last Sentence"));
    addHtmlInt(length_last);
  }
}

#endif // USES_P087
