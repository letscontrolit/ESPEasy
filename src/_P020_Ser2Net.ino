#include "_Plugin_Helper.h"
#if defined(USES_P020) || defined(USES_P044)

// #######################################################################################################
// #################################### Plugin 020: Ser2Net ##############################################
// #################################### Plugin 044: P1WifiGateway ########################################
// #######################################################################################################

/************
 * Changelog:
 * 2022-12-12 tonhuisman: Add character conversion for the received serial data, act on Space and/or Newline
 * 2022-10-11 tonhuisman: Add option for including the message in P1 #data event
 * 2022-10-09 tonhuisman: Check P044 migration on PLUGIN_INIT too, still needs a manual save (from UI or by save command)
 * 2022-10-08 tonhuisman: Merged code from P044 into this plugin, and use a global flag to emulate P044 with P020
 *                        When USES_P044 is enabled, also USES_P020 will be enabled!
 *                        Add Led settings, similar to P044
 * 2022-05-28 tonhuisman: Add option to generate events for all lines of a multi-line message
 * 2022-05-26 tonhuisman: Add option to allow processing without webclient connected.
 * No older changelog available.
 ***************************************************************/

# include "src/Helpers/_Plugin_Helper_serial.h"
# include "src/PluginStructs/P020_data_struct.h"
# include <ESPeasySerial.h>


# define PLUGIN_020
# define PLUGIN_ID_020         20
# define PLUGIN_NAME_020       "Communication - Serial Server"
# define PLUGIN_VALUENAME1_020 "Ser2Net"

# define PLUGIN_ID_020_044     44
# define PLUGIN_NAME_020_044   "Communication - P1 Wifi Gateway"

bool P020_Emulate_P044 = false; // Global flag
# ifdef USES_P044

// Emulate P044 using P020 with a global flag
boolean Plugin_044(uint8_t function, struct EventStruct *event, String& string) {
  P020_Emulate_P044 = true;

  boolean result = Plugin_020(function, event, string);

  P020_Emulate_P044 = false;
  return result;
}

bool P020_ConvertP044Settings(struct EventStruct *event) {
  if (P020_Emulate_P044 && !P020_GET_P044_MODE_SAVED) {
    // Convert existing P044 settings to P020 settings
    P020_RX_WAIT = PCONFIG(0); // No conflict
    // P020_SERIAL_CONFIG    = PCONFIG(1); // No need to convert
    P020_RESET_TARGET_PIN = CONFIG_PIN1;

    // 'Conflicting' stuff, set defaults to: Serial0, RX=gpio-3 and TX=gpio-1
    CONFIG_PORT = static_cast<int>(ESPEasySerialPort::serial0);            // P044 Serial port
    CONFIG_PIN1 = 3;                                                       // P044 RX pin
    CONFIG_PIN2 = 1;                                                       // P044 TX pin

    // Former P044 defaults
    bitSet(P020_FLAGS, P020_FLAG_LED_ENABLED);                             // Led enabled...
    P020_LED_PIN           = P020_STATUS_LED;                              // ...and connected to GPIO-12
    P020_SERIAL_PROCESSING = static_cast<int>(P020_Events::P1WiFiGateway); // Enable P1 WiFi Gateway processing
    return true;
  }
  return false;
}

# endif // ifdef USES_P044

boolean Plugin_020(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      if (P020_Emulate_P044) {
        Device[++deviceCount].Number       = PLUGIN_ID_020_044;
        Device[deviceCount].SendDataOption = false;
      } else {
        Device[++deviceCount].Number       = PLUGIN_ID_020;
        Device[deviceCount].SendDataOption = true;
      }
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_STRING;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 0;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].GlobalSyncOption   = false;
      break;
    }
    case PLUGIN_GET_DEVICENAME:
    {
      string = P020_Emulate_P044 ? F(PLUGIN_NAME_020_044) : F(PLUGIN_NAME_020);
      break;
    }


    case PLUGIN_SET_DEFAULTS:
    {
      if (P020_Emulate_P044) {
        CONFIG_PORT            = static_cast<int>(ESPEasySerialPort::serial0); // P044 Serial port
        CONFIG_PIN1            = 3;                                            // P044 RX pin
        CONFIG_PIN2            = 1;                                            // P044 TX pin
        P020_BAUDRATE          = P020_DEFAULT_P044_BAUDRATE;
        P020_SERVER_PORT       = P020_DEFAULT_P044_SERVER_PORT;
        P020_RESET_TARGET_PIN  = P020_DEFAULT_RESET_TARGET_PIN;
        P020_SERIAL_PROCESSING = static_cast<int>(P020_Events::P1WiFiGateway); // Enable P1 WiFi Gateway processing (only)
        P020_LED_PIN           = P020_STATUS_LED;
        P020_RX_WAIT           = 0;
        P020_REPLACE_SPACE     = 0;                                            // Force empty
        P020_REPLACE_NEWLINE   = 0;
        bitSet(P020_FLAGS, P020_FLAG_LED_ENABLED);
        bitSet(P020_FLAGS, P020_FLAG_P044_MODE_SAVED);                         // Inital config, no conversion needed
      } else {
        P020_BAUDRATE         = P020_DEFAULT_BAUDRATE;
        P020_SERVER_PORT      = P020_DEFAULT_SERVER_PORT;
        P020_RESET_TARGET_PIN = P020_DEFAULT_RESET_TARGET_PIN;
        P020_RX_BUFFER        = P020_DEFAULT_RX_BUFFER;
        P020_LED_PIN          = -1;
      }
      success = true;
      break;
    }


    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      serialHelper_getGpioNames(event);
      break;
    }

    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      string  = F("RST: ");
      string += formatGpioLabel(P020_RESET_TARGET_PIN, false);
      string += event->String1;
      string += F("LED: ");
      string += formatGpioLabel(P020_GET_LED_ENABLED ? P020_LED_PIN : -1, false);

      if ((P020_GET_LED_INVERTED == 1) && (P020_GET_LED_ENABLED)) {
        string += F(" (inv)");
      }
      success = true;
      break;
    }

    # ifdef USES_P044
    case PLUGIN_WEBFORM_PRE_SERIAL_PARAMS:
    {
      // P044 Settings to convert?
      if (P020_Emulate_P044 && P020_ConvertP044Settings(event)) {
        addFormNote(F("Settings migrated from previous plugin version."));
      }
      break;
    }
    # endif // ifdef USES_P044

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("TCP Port"), F("pport"), P020_SERVER_PORT, 0);
      # ifndef LIMIT_BUILD_SIZE
      addUnit(F("0..65535"));
      # endif // ifndef LIMIT_BUILD_SIZE

      addFormNumericBox(F("Baud Rate"), F("pbaud"), P020_BAUDRATE, 0);
      uint8_t serialConfChoice = serialHelper_convertOldSerialConfig(P020_SERIAL_CONFIG);
      serialHelper_serialconfig_webformLoad(event, serialConfChoice);
      {
        if (!P020_Emulate_P044) {
          const __FlashStringHelper *options[] = {
            F("None"),
            F("Generic"),
            F("RFLink"),
            F("P1 WiFi Gateway")
          };
          const int optionValues[] = {
            static_cast<int>(P020_Events::None),
            static_cast<int>(P020_Events::Generic),
            static_cast<int>(P020_Events::RFLink),
            static_cast<int>(P020_Events::P1WiFiGateway),
          };
          addFormSelector(F("Event processing"), F("pevents"),
                          sizeof(optionValues) / sizeof(int), options, optionValues,
                          P020_SERIAL_PROCESSING);
        }
        addFormCheckBox(F("P1 #data event with message"), F("pp1event"), P020_GET_P1_EVENT_DATA);
        # ifndef LIMIT_BUILD_SIZE
        addFormNote(F("When enabled, passes the entire message in the event. <B>Warning:</B> can cause memory overflow issues!"));
        # endif // ifndef LIMIT_BUILD_SIZE

        if (!P020_Emulate_P044) { // Not appropriate for P1 WiFi Gateway
          addFormSeparatorCharInput(F("Replace spaces in event by"),   F("replspace"),
                                    P020_REPLACE_SPACE, F(P020_REPLACE_CHAR_SET), F(""));

          addFormSeparatorCharInput(F("Replace newlines in event by"), F("replcrlf"),
                                    P020_REPLACE_NEWLINE, F(P020_REPLACE_CHAR_SET), F(""));
        }

        addFormCheckBox(F("Process events without client"), F("pignoreclient"), P020_IGNORE_CLIENT_CONNECTED);
        # ifndef LIMIT_BUILD_SIZE
        addFormNote(F("When enabled, will process serial data without a network client connected."));
        # endif // ifndef LIMIT_BUILD_SIZE

        if (!P020_Emulate_P044) { // Not appropriate for P1 WiFi Gateway
          addFormCheckBox(F("Multiple lines processing"), F("pmultiline"), P020_HANDLE_MULTI_LINE);
        }
      }
      {
        addFormNumericBox(F("RX Receive Timeout (mSec)"), F("prxwait"), P020_RX_WAIT, 0, 200);
        addFormPinSelect(PinSelectPurpose::Generic, F("Reset target after init"), F("presetpin"), P020_RESET_TARGET_PIN);

        if (!P020_Emulate_P044) {
          addFormNumericBox(F("RX buffer size (bytes)"), F("prx_buffer"), P020_RX_BUFFER, 256, 1024);
          # ifndef LIMIT_BUILD_SIZE
          addFormNote(F("Standard RX buffer 256B; higher values could be unstable; energy meters could require 1024B"));
          # endif // ifndef LIMIT_BUILD_SIZE
        }
      }
      { // Led settings
        addFormSubHeader(F("Led"));

        addFormCheckBox(F("Led enabled"), F("pled"), P020_GET_LED_ENABLED);
        addFormPinSelect(PinSelectPurpose::Generic, F("Led pin"), F("pledpin"), P020_LED_PIN);
        addFormCheckBox(F("Led inverted"), F("pledinv"), P020_GET_LED_INVERTED == 1);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P020_SERVER_PORT      = getFormItemInt(F("pport"));
      P020_BAUDRATE         = getFormItemInt(F("pbaud"));
      P020_SERIAL_CONFIG    = serialHelper_serialconfig_webformSave();
      P020_RX_WAIT          = getFormItemInt(F("prxwait"));
      P020_RESET_TARGET_PIN = getFormItemInt(F("presetpin"));

      if (P020_Emulate_P044) {
        P020_SERIAL_PROCESSING = static_cast<int>(P020_Events::P1WiFiGateway); // Force P1 WiFi Gateway processing
      } else {
        P020_SERIAL_PROCESSING = getFormItemInt(F("pevents"));
        P020_RX_BUFFER         = getFormItemInt(F("prx_buffer"));
      }
      P020_LED_PIN = getFormItemInt(F("pledpin"));

      if (!P020_Emulate_P044) {
        P020_REPLACE_SPACE   = getFormItemInt(F("replspace"));
        P020_REPLACE_NEWLINE = getFormItemInt(F("replcrlf"));
      }

      uint32_t lSettings = 0u;
      bitWrite(lSettings, P020_FLAG_IGNORE_CLIENT, isFormItemChecked(F("pignoreclient")));
      bitWrite(lSettings, P020_FLAG_LED_ENABLED,   isFormItemChecked(F("pled")));
      bitWrite(lSettings, P020_FLAG_LED_INVERTED,  isFormItemChecked(F("pledinv")));
      bitWrite(lSettings, P020_FLAG_P1_EVENT_DATA, isFormItemChecked(F("pp1event")));

      if (P020_Emulate_P044) {
        bitSet(lSettings, P020_FLAG_P044_MODE_SAVED); // Set to P044 configuration done on every save
      } else {
        bitWrite(lSettings, P020_FLAG_MULTI_LINE, isFormItemChecked(F("pmultiline")));
      }

      P020_FLAGS = lSettings;

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      # ifdef USES_P044

      // P044 Settings to convert?
      if (P020_Emulate_P044 && P020_ConvertP044Settings(event)) {
        addLog(LOG_LEVEL_INFO, F("P1   : Automatic settings conversion, please save settings manually."));
        bitSet(P020_FLAGS, P020_FLAG_P044_MODE_SAVED); // Set to P044 configuration done on next save
      }
      # endif // ifdef USES_P044

      LoadTaskSettings(event->TaskIndex);

      if (P020_GET_LED_ENABLED && validGpio(P020_LED_PIN)) {
        pinMode(P020_LED_PIN, OUTPUT);
        digitalWrite(P020_LED_PIN, P020_GET_LED_INVERTED ? 1 : 0);
      }

      if ((P020_SERVER_PORT == 0) || (P020_BAUDRATE == 0)) {
        clearPluginTaskData(event->TaskIndex);
        break;
      }

      // try to reuse to keep webserver running
      P020_Task *task = static_cast<P020_Task *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != task) && task->isInit()) {
        // It was already created and initialzed
        // So don't recreate to keep the webserver running.
      } else {
        initPluginTaskData(event->TaskIndex, new (std::nothrow) P020_Task(event));
        task = static_cast<P020_Task *>(getPluginTaskData(event->TaskIndex));
      }

      if (nullptr == task) {
        break;
      }
      task->handleMultiLine = P020_HANDLE_MULTI_LINE && static_cast<P020_Events>(P020_SERIAL_PROCESSING) != P020_Events::P1WiFiGateway;

      int rxPin                    = CONFIG_PIN1;
      int txPin                    = CONFIG_PIN2;
      const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);

      if ((rxPin < 0) && (txPin < 0)) {
        ESPeasySerialType::getSerialTypePins(port, rxPin, txPin);
        CONFIG_PIN1 = rxPin;
        CONFIG_PIN2 = txPin;
      }

      # ifndef LIMIT_BUILD_SIZE

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log;
        log.reserve(100);
        log  = concat(F("Ser2net: TaskIndex="), static_cast<int>(event->TaskIndex));
        log += concat(F(" port="), static_cast<int>(CONFIG_PORT));
        log += concat(F(" rxPin="), static_cast<int>(rxPin));
        log += concat(F(" txPin="), static_cast<int>(txPin));
        log += concat(F(" BAUDRATE="), static_cast<int>(P020_BAUDRATE));
        log += concat(F(" SERVER_PORT="), static_cast<int>(P020_SERVER_PORT));
        log += concat(F(" SERIAL_PROCESSING="), static_cast<int>(P020_SERIAL_PROCESSING));
        addLogMove(LOG_LEVEL_INFO, log);
      }
      # endif // ifndef LIMIT_BUILD_SIZE

      // serial0 on esp32 is Ser2net: port=2 rxPin=3 txPin=1; serial1 on esp32 is Ser2net: port=4 rxPin=13 txPin=15; Serial2 on esp32 is
      // Ser2net: port=4 rxPin=16 txPin=17
      uint8_t serialconfig = serialHelper_convertOldSerialConfig(P020_SERIAL_CONFIG);
      task->serialBegin(port, rxPin, txPin, P020_BAUDRATE, serialconfig);
      task->startServer(P020_SERVER_PORT);

      if (!task->isInit()) {
        clearPluginTaskData(event->TaskIndex);
        break;
      }

      if (validGpio(P020_RESET_TARGET_PIN)) {
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          addLogMove(LOG_LEVEL_DEBUG, concat(F("Ser2net  : P020_RESET_TARGET_PIN : "), static_cast<int>(P020_RESET_TARGET_PIN)));
        }
        # endif // ifndef BUILD_NO_DEBUG
        pinMode(P020_RESET_TARGET_PIN, OUTPUT);
        digitalWrite(P020_RESET_TARGET_PIN, LOW);
        delay(500);
        digitalWrite(P020_RESET_TARGET_PIN, HIGH);
        pinMode(P020_RESET_TARGET_PIN, INPUT_PULLUP);
      }

      task->serial_processing = static_cast<P020_Events>(P020_SERIAL_PROCESSING);
      task->_P1EventData      = P020_GET_P1_EVENT_DATA;

      task->blinkLED();

      if (task->serial_processing == P020_Events::P1WiFiGateway) {
        task->_CRCcheck = P020_BAUDRATE == 115200;
        # ifndef BUILD_NO_DEBUG

        if (task->_CRCcheck) {
          addLog(LOG_LEVEL_DEBUG, F("P1   : DSMR version 5 meter, CRC on"));
        } else {
          addLog(LOG_LEVEL_DEBUG, F("P1   : DSMR version 4 meter, CRC off"));
        }
        # endif // ifndef BUILD_NO_DEBUG
      }

      success = true;
      break;
    }

    case PLUGIN_EXIT:
    {
      P020_Task *task = static_cast<P020_Task *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != task) {
        task->stopServer();
        task->serialEnd();
      }
      success = true;
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P020_Task *task = static_cast<P020_Task *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != task) {
        task->checkServer();
        success = true;
      }
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P020_Task *task = static_cast<P020_Task *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != task) {
        bool hasClient = task->hasClientConnected();

        if (P020_IGNORE_CLIENT_CONNECTED || hasClient) {
          if (hasClient) {
            task->handleClientIn(event);
          }
          task->handleSerialIn(event); // in case of second serial connected, PLUGIN_SERIAL_IN is not called anymore
        }
        task->checkBlinkLED();
        success = true;
      }
      break;
    }

    case PLUGIN_SERIAL_IN:
    {
      P020_Task *task = static_cast<P020_Task *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != task) {
        if (P020_IGNORE_CLIENT_CONNECTED || task->hasClientConnected()) {
          task->handleSerialIn(event);
        } else {
          task->discardSerialIn();
        }
        success = true;
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P020_Task *task = static_cast<P020_Task *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != task) {
        String command = parseString(string, 1);

        if (command.equals(F("serialsend"))) {
          task->ser2netSerial->write(string.substring(11).c_str());
          task->ser2netSerial->flush();
          success = true;
        }

        if ((command.equals(F("ser2netclientsend"))) && (task->hasClientConnected())) {
          task->ser2netClient.print(string.substring(18));
          task->ser2netClient.flush();
          success = true;
        }
      }
      break;
    }
  }
  return success;
}

#endif // if defined(USES_P020) || defined(USES_P044)
