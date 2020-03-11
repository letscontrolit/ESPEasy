#ifdef USES_P087

// #######################################################################################################
// #################### Plugin 087 Serial Proxy ##########################################################
// #######################################################################################################
//
// Interact with a device connected to serial
// Allows to redirect data to a controller
//

#include <ESPeasySerial.h>
#include "_Plugin_Helper.h"
#include <Regexp.h>

#define PLUGIN_087
#define PLUGIN_ID_087           87
#define PLUGIN_NAME_087         "Communication - Serial Proxy [TESTING]"


#define P087_BAUDRATE           PCONFIG_LONG(0)
#define P087_BAUDRATE_LABEL     PCONFIG_LABEL(0)

#define P087_QUERY_VALUE        0 // Temp placement holder until we know what selectors are needed.
#define P087_NR_OUTPUT_OPTIONS  1

#define P087_NR_OUTPUT_VALUES   1
#define P087_QUERY1_CONFIG_POS  3

#define P087_DEFAULT_BAUDRATE   38400

#define P87_Nlines              (1 + 3 * 10)
#define P87_Nchars              128

#define P087_INITSTRING         0
#define P087_EXITSTRING         1

enum P087_Filter_Comp {
  Equal = 0,
  NotEqual = 1

};


struct P087_data_struct : public PluginTaskData_base {

  P087_data_struct() :  P087_easySerial(nullptr) {}

  ~P087_data_struct() {
    reset();
  }

  void reset() {
    if (P087_easySerial != nullptr) {
      delete P087_easySerial;
      P087_easySerial = nullptr;
    }
  }

  bool init(const int16_t serial_rx, const int16_t serial_tx, unsigned long baudrate) {
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

  bool isInitialized() const {
    return P087_easySerial != nullptr;
  }

  void sendString(const String& data) {
    if (isInitialized()) {
      if (data.length() > 0) {
        P087_easySerial->write(data.c_str());

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("Proxy: Sending: ");
          log += data;
          addLog(LOG_LEVEL_INFO, log);
        }
      }
    }
  }

  bool loop() {
    if (!isInitialized()) {
      return false;
    }
    bool fullSentenceReceived = false;

    if (P087_easySerial != nullptr) {
      while (P087_easySerial->available() > 0 && !fullSentenceReceived) {
        // Look for end marker
        char c = P087_easySerial->read();

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

  void getSentence(String& string) {
    string        = sentence_part;
    sentence_part = "";
  }

  void getSentencesReceived(uint32_t& succes, uint32_t& error, uint32_t& length_last) const {
    succes      = sentences_received;
    error       = sentences_received_error;
    length_last = length_last_received;
  }

  void setMaxLength(uint16_t maxlenght) {
    max_length = maxlenght;
  }

  void setRegExpMatchLength(uint16_t length) {
    regexp_match_length = length;
  }

  void loadLines(taskIndex_t taskIndex) {
    LoadCustomTaskSettings(taskIndex, _lines, P87_Nlines, 0);
  }

  String getRegEx() const {
    return _lines[0];
  }

  String getLine(uint8_t lineNr) const {
    if (lineNr < P87_Nlines) {
      return _lines[lineNr];
    }
    return "";
  }

  String getFilter(uint8_t lineNr, uint8_t& capture, P087_Filter_Comp& comparator) const
  {
    uint8_t varNr = lineNr * 3 + 1;
    if (varNr >= P87_Nlines) return "";

    capture = _lines[varNr++].toInt();
    comparator = _lines[varNr++] == "1" ? P087_Filter_Comp::NotEqual : P087_Filter_Comp::Equal;
    return _lines[varNr];
  }


  bool matchRegexp(String& received) const {
    size_t strlength = received.length();

    if (strlength == 0) {
      return false;
    }

    if ((regexp_match_length > 0) && (strlength > regexp_match_length)) {
      strlength = regexp_match_length;
    }

    // We need to do a const_cast here, but this only is valid as long as we
    // don't call a replace function from regexp.
    MatchState ms(const_cast<char *>(received.c_str()), strlength);
    char result = ms.Match(_lines[0].c_str());

    if (result == REGEXP_MATCHED) {
      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("Match at: ");
        log += ms.MatchStart;
        log += F(" Match Length: ");
        log += ms.MatchLength;
        addLog(LOG_LEVEL_DEBUG, log);
      }
      return true;
    }
    return false;
  }

private:

  bool max_length_reached() const {
    if (max_length == 0) { return false; }
    return sentence_part.length() >= max_length;
  }

  String _lines[P87_Nlines];

  ESPeasySerial *P087_easySerial = nullptr;
  String         sentence_part;
  uint16_t       max_length               = 550;
  uint32_t       sentences_received       = 0;
  uint32_t       sentences_received_error = 0;
  uint32_t       length_last_received     = 0;
  uint32_t       regexp_match_length      = 0;
};


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


boolean Plugin_087(byte function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_087;
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL;
      Device[deviceCount].VType              = SENSOR_TYPE_STRING;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_087);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      for (byte i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P087_NR_OUTPUT_VALUES) {
          const byte pconfigIndex = i + P087_QUERY1_CONFIG_POS;
          byte choice             = PCONFIG(pconfigIndex);
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
        byte varNr = VARS_PER_TASK;
        addHtml(pluginWebformShowValue(event->TaskIndex, varNr++, F("Success"),     String(success)));
        addHtml(pluginWebformShowValue(event->TaskIndex, varNr++, F("Error"),       String(error)));
        addHtml(pluginWebformShowValue(event->TaskIndex, varNr++, F("Length Last"), String(length_last), true));

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

    case PLUGIN_WEBFORM_LOAD: {
      serialHelper_webformLoad(event);

      /*
         P087_data_struct *P087_data =
            static_cast<P087_data_struct *>(getPluginTaskData(event->TaskIndex));
         if (nullptr != P087_data && P087_data->isInitialized()) {
            String detectedString = F("Detected: ");
            detectedString += String(P087_data->P087_easySerial->baudRate());
            addUnit(detectedString);
       */

      addFormNumericBox(F("Baudrate"), P087_BAUDRATE_LABEL, P087_BAUDRATE, 2400, 115200);
      addUnit(F("baud"));

      /*
         {
         // In a separate scope to free memory of String array as soon as possible
         sensorTypeHelper_webformLoad_header();
         String options[P087_NR_OUTPUT_OPTIONS];

         for (int i = 0; i < P087_NR_OUTPUT_OPTIONS; ++i) {
          options[i] = Plugin_087_valuename(i, true);
         }

         for (byte i = 0; i < P087_NR_OUTPUT_VALUES; ++i) {
          const byte pconfigIndex = i + P087_QUERY1_CONFIG_POS;
          sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, P087_NR_OUTPUT_OPTIONS, options);
         }
         }
       */

      addFormSubHeader(F("Filtering"));
      P087_html_show_matchForms(event);


      addFormSubHeader(F("Statistics"));
      P087_html_show_stats(event);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      serialHelper_webformSave(event);
      P087_BAUDRATE = getFormItemInt(P087_BAUDRATE_LABEL);

      String lines[P87_Nlines];

      for (byte varNr = 0; varNr < P87_Nlines; varNr++)
      {
        lines[varNr] = web_server.arg(getPluginCustomArgName(varNr));
      }
      addHtmlError(SaveCustomTaskSettings(event->TaskIndex, lines, P87_Nlines, 0));

      success = true;
      break;
    }

    case PLUGIN_INIT: {
      const int16_t serial_rx = CONFIG_PIN1;
      const int16_t serial_tx = CONFIG_PIN2;
      initPluginTaskData(event->TaskIndex, new P087_data_struct());
      P087_data_struct *P087_data =
        static_cast<P087_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P087_data) {
        return success;
      }

      if (P087_data->init(serial_rx, serial_tx, P087_BAUDRATE)) {
        P087_data->loadLines(event->TaskIndex);
        success = true;

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("Serial : Init OK  ESP GPIO-pin RX:");
          log += serial_rx;
          log += F(" TX:");
          log += serial_tx;
          addLog(LOG_LEVEL_DEBUG, log);
        }
      } else {
        clearPluginTaskData(event->TaskIndex);
      }
      break;
    }

    case PLUGIN_EXIT: {
      clearPluginTaskData(event->TaskIndex);
      success = true;
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND: {
      if (Settings.TaskDeviceEnabled[event->TaskIndex]) {
        P087_data_struct *P087_data =
          static_cast<P087_data_struct *>(getPluginTaskData(event->TaskIndex));

        if ((nullptr != P087_data) && P087_data->loop()) {
          // schedule_task_device_timer(event->TaskIndex, millis() + 10);
          delay(0); // Processing a full sentence may take a while, run some
                    // background tasks.
          P087_data->getSentence(event->String2);

          if (event->String2.length() > 0) {
            if (Plugin_087_match_all(event->TaskIndex, event->String2)) {
              sendData(event);
              addLog(LOG_LEVEL_INFO, event->String2);
            }
          }
        }
        success = true;
      }
      break;
    }

    case PLUGIN_READ: {
      P087_data_struct *P087_data =
        static_cast<P087_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P087_data)) {
        String initstr = P087_data->getLine(0);
        parseSystemVariables(initstr, false);
        P087_data->sendString(initstr);
      }
      break;
    }

    case PLUGIN_WRITE: {
      String cmd = parseString(string, 1);


      if (cmd.equalsIgnoreCase(F("serialproxy_write"))) {
        P087_data_struct *P087_data =
          static_cast<P087_data_struct *>(getPluginTaskData(event->TaskIndex));

        if ((nullptr != P087_data)) {
          String param1 = parseString(string, 2);
          parseSystemVariables(param1, false);
          P087_data->sendString(param1);
          addLog(LOG_LEVEL_INFO, param1);
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

  return P087_data->matchRegexp(received);
}

String Plugin_087_valuename(byte value_nr, bool displayString) {
  switch (value_nr) {
    case P087_QUERY_VALUE: return displayString ? F("Value")          : F("v");
  }
  return "";
}

void P087_html_show_matchForms(struct EventStruct *event) {
  P087_data_struct *P087_data =
    static_cast<P087_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr != P087_data)) {
    byte varNr = 0;
    addFormTextBox(F("RegEx"), getPluginCustomArgName(varNr), P087_data->getLine(varNr), P87_Nchars);
    addFormNote(F("Captures are specified using round brackets."));

    ++varNr;

    byte lineNr = 0;
    uint8_t capture = 0;
    P087_Filter_Comp comparator = P087_Filter_Comp::Equal;
    String filter;

    for (; varNr < P87_Nlines; varNr++)
    {
      String id = getPluginCustomArgName(varNr);

      switch ((varNr - 1) % 3) {
        case 0:
        {
          // Label + first parameter
          filter = P087_data->getFilter(lineNr, capture, comparator);
          ++lineNr;
          String label;
          label  = F("Capture Filter ");
          label += String(lineNr);
          addRowLabel_tr_id(label, id);

          addNumericBox(id, capture, 0, 99);
          break;
        }
        case 1:
        {
          // Comparator
          String options[2];
          options[P087_Filter_Comp::Equal] = F("==");
          options[P087_Filter_Comp::NotEqual] = F("!=");
          int optionValues[2] = { P087_Filter_Comp::Equal, P087_Filter_Comp::NotEqual };
          addSelector(id, 2, options, optionValues, NULL, comparator, false, "");
          break;
        }
        case 2:
        {
          // Compare with
          addTextBox(id, filter, 32, false, false, "", "");
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
    addHtml(String(length_last));
  }
}

#endif // USES_P087
