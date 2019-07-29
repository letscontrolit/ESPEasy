#ifdef USES_P087

// #######################################################################################################
// #################### Plugin 087 Serial Proxy ##########################################################
// #######################################################################################################
//
// Interact with a device connected to serial
// Allows to redirect data to a controller
//

#include <ESPeasySerial.h>

#define PLUGIN_087
#define PLUGIN_ID_087          87
#define PLUGIN_NAME_087       "Communication - Serial Proxy [TESTING]"


#define P087_TIMEOUT        PCONFIG(0)
#define P087_TIMEOUT_LABEL  PCONFIG_LABEL(0)
#define P087_BAUDRATE       PCONFIG(1)
#define P087_BAUDRATE_LABEL PCONFIG_LABEL(1)

#define P087_QUERY_VALUE        0 // Temp placement holder until we know what selectors are needed.
#define P087_NR_OUTPUT_OPTIONS  1

#define P087_NR_OUTPUT_VALUES   VARS_PER_TASK
#define P087_QUERY1_CONFIG_POS  3

#define P087_DEFAULT_TIMEOUT 2500

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

  bool init(const int16_t serial_rx, const int16_t serial_tx) {
    if ((serial_rx < 0) || (serial_tx < 0)) {
      return false;
    }
    reset();
    P087_easySerial = new ESPeasySerial(serial_rx, serial_tx);
    P087_easySerial->begin(9600);
    return isInitialized();
  }

  bool isInitialized() const {
    return P087_easySerial != nullptr;
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
            fullSentenceReceived = true;
            break;
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
      sentence_full = sentence_part;
      sentence_part = "";
      newData       = true;
      ++sentences_received;
    }
    return fullSentenceReceived;
  }

  String getFullSentence() {
    newData = false;
    return sentence_full;
  }

  bool hasNewData() const {
    return newData;
  }

  uint32_t getSentencesReceived() const {
    return sentences_received;
  }

private:

  bool max_length_reached() const {
    if (max_length == 0) { return false; }
    return sentence_part.length() >= max_length;
  }

  ESPeasySerial *P087_easySerial = nullptr;
  String         sentence_part;
  String         sentence_full;
  int16_t        max_length = 128;
  uint32_t sentences_received = 0;
  bool           newData    = false;
};


boolean Plugin_087(byte function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_087;
      Device[deviceCount].Type               = DEVICE_TYPE_TRIPLE;
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

    case PLUGIN_SET_DEFAULTS:
    {
      P087_TIMEOUT = P087_DEFAULT_TIMEOUT;

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

      addFormNumericBox(F("Fix Timeout"), P087_TIMEOUT_LABEL, P087_TIMEOUT, 100, 10000);
      addUnit(F("ms"));

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

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      serialHelper_webformSave(event);
      P087_TIMEOUT = getFormItemInt(P087_TIMEOUT_LABEL);

      // Save output selector parameters.
      for (byte i = 0; i < P087_NR_OUTPUT_VALUES; ++i) {
        const byte pconfigIndex = i + P087_QUERY1_CONFIG_POS;
        const byte choice       = PCONFIG(pconfigIndex);
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, Plugin_087_valuename(choice, false));
      }

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

      if (P087_data->init(serial_rx, serial_tx)) {
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

    case PLUGIN_TEN_PER_SECOND: {
      if (Settings.TaskDeviceEnabled[event->TaskIndex]) {
        P087_data_struct *P087_data =
          static_cast<P087_data_struct *>(getPluginTaskData(event->TaskIndex));

        if ((nullptr != P087_data) && P087_data->loop()) {
          schedule_task_device_timer(event->TaskIndex, millis() + 10);
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

      if ((nullptr != P087_data) && P087_data->isInitialized()) {
        // Check for internal state (waiting for reply or reply ready)
        // Output received data.
        if (P087_data->hasNewData()) {
          String log;
          log = F("Proxy: ");
          log += P087_data->getFullSentence();
          addLog(LOG_LEVEL_INFO, log);
          UserVar[event->BaseVarIndex] = P087_data->getSentencesReceived();
          success = true;
        }
      }
      break;
    }
  }
  return success;
}

String Plugin_087_valuename(byte value_nr, bool displayString) {
  switch (value_nr) {
    case P087_QUERY_VALUE: return displayString ? F("Value")          : F("v");
  }
  return "";
}

#endif // USES_P087
