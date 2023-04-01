#include "_Plugin_Helper.h"
#ifdef USES_P020

// #######################################################################################################
// #################################### Plugin 020: Ser2Net ##############################################
// #######################################################################################################

/************
 * Changelog:
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


# define P020_SET_SERVER_PORT      ExtraTaskSettings.TaskDevicePluginConfigLong[0]
# define P020_SET_BAUDRATE         ExtraTaskSettings.TaskDevicePluginConfigLong[1]

# define P020_GET_SERVER_PORT      Cache.getTaskDevicePluginConfigLong(event->TaskIndex, 0)
# define P020_GET_BAUDRATE         Cache.getTaskDevicePluginConfigLong(event->TaskIndex, 1)

// #define P020_SET_BAUDRATE             ExtraTaskSettings.TaskDevicePluginConfigLong[1]
# define P020_RX_WAIT              PCONFIG(4)
# define P020_SERIAL_CONFIG        PCONFIG(1)
# define P020_SERIAL_PROCESSING    PCONFIG(5)
# define P020_RESET_TARGET_PIN     PCONFIG(6)
# define P020_RX_BUFFER            PCONFIG(7)

# define P020_FLAGS                     PCONFIG_LONG(0)
# define P020_FLAG_IGNORE_CLIENT        0
# define P020_FLAG_MULTI_LINE           1
# define P020_IGNORE_CLIENT_CONNECTED   bitRead(P020_FLAGS, P020_FLAG_IGNORE_CLIENT)
# define P020_HANDLE_MULTI_LINE         bitRead(P020_FLAGS, P020_FLAG_MULTI_LINE)


# define P020_QUERY_VALUE        0 // Temp placement holder until we know what selectors are needed.
# define P020_NR_OUTPUT_OPTIONS  1
# define P020_QUERY1_CONFIG_POS  3

# define P020_DEFAULT_SERVER_PORT 1234
# define P020_DEFAULT_BAUDRATE   115200
# define P020_DEFAULT_RESET_TARGET_PIN -1
# define P020_DEFAULT_RX_BUFFER 256


boolean Plugin_020(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_020;
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_STRING;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 0;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].GlobalSyncOption   = false;
      break;
    }
    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_020);
      break;
    }


    case PLUGIN_SET_DEFAULTS:
    {
      P020_SET_BAUDRATE     = P020_DEFAULT_BAUDRATE;
      P020_SET_SERVER_PORT  = P020_DEFAULT_SERVER_PORT;
      P020_RESET_TARGET_PIN = P020_DEFAULT_RESET_TARGET_PIN;
      P020_RX_BUFFER        = P020_DEFAULT_RX_BUFFER;
      success               = true;
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
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("TCP Port"),  F("p020_port"), P020_GET_SERVER_PORT, 0);
      addFormNumericBox(F("Baud Rate"), F("p020_baud"), P020_GET_BAUDRATE,    0);
      uint8_t serialConfChoice = serialHelper_convertOldSerialConfig(P020_SERIAL_CONFIG);
      serialHelper_serialconfig_webformLoad(event, serialConfChoice);
      {
        const __FlashStringHelper *options[3] = {
          F("None"),
          F("Generic"),
          F("RFLink")
        };
        addFormSelector(F("Event processing"), F("p020_events"), 3, options, nullptr, P020_SERIAL_PROCESSING);
        addFormCheckBox(F("Process events without client"), F("p020_ignoreclient"), P020_IGNORE_CLIENT_CONNECTED);
        # ifndef LIMIT_BUILD_SIZE
        addFormNote(F("When enabled, will process serial data without a network client connected."));
        # endif // ifndef LIMIT_BUILD_SIZE
        addFormCheckBox(F("Multiple lines processing"), F("p020_multiline"), P020_HANDLE_MULTI_LINE);
      }
      addFormNumericBox(F("RX Receive Timeout (mSec)"), F("p020_rxwait"), P020_RX_WAIT, 0, 20);
      addFormPinSelect(PinSelectPurpose::Generic, F("Reset target after init"), F("p020_resetpin"), P020_RESET_TARGET_PIN);

      addFormNumericBox(F("RX buffer size (bytes)"), F("p020_rx_buffer"), P020_RX_BUFFER, 256, 1024);
      # ifndef LIMIT_BUILD_SIZE
      addFormNote(F("Standard RX buffer 256B; higher values could be unstable; energy meters could require 1024B"));
      # endif // ifndef LIMIT_BUILD_SIZE

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P020_SET_SERVER_PORT   = getFormItemInt(F("p020_port"));
      P020_SET_BAUDRATE      = getFormItemInt(F("p020_baud"));
      P020_SERIAL_CONFIG     = serialHelper_serialconfig_webformSave();
      P020_SERIAL_PROCESSING = getFormItemInt(F("p020_events"));
      P020_RX_WAIT           = getFormItemInt(F("p020_rxwait"));
      P020_RESET_TARGET_PIN  = getFormItemInt(F("p020_resetpin"));
      P020_RX_BUFFER         = getFormItemInt(F("p020_rx_buffer"));

      bitWrite(P020_FLAGS, P020_FLAG_IGNORE_CLIENT, isFormItemChecked(F("p020_ignoreclient")));
      bitWrite(P020_FLAGS, P020_FLAG_MULTI_LINE,    isFormItemChecked(F("p020_multiline")));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if ((P020_GET_SERVER_PORT == 0) || (P020_GET_BAUDRATE == 0)) {
        clearPluginTaskData(event->TaskIndex);
        break;
      }

      // try to reuse to keep webserver running
      P020_Task *task = static_cast<P020_Task *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != task) && task->isInit()) {
        // It was already created and initialzed
        // So don't recreate to keep the webserver running.
      } else {
        initPluginTaskData(event->TaskIndex, new (std::nothrow) P020_Task(event->TaskIndex));
        task = static_cast<P020_Task *>(getPluginTaskData(event->TaskIndex));
      }

      if (nullptr == task) {
        break;
      }
      task->handleMultiLine = P020_HANDLE_MULTI_LINE;

      // int rxPin =-1;
      // int txPin =-1;
      int rxPin                    = CONFIG_PIN1;
      int txPin                    = CONFIG_PIN2;
      const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);

      // const ESPEasySerialPort port= ESPEasySerialPort::serial0;
      if ((rxPin < 0) && (txPin < 0)) {
        ESPeasySerialType::getSerialTypePins(port, rxPin, txPin);
        CONFIG_PIN1 = rxPin;
        CONFIG_PIN2 = txPin;
      }

      # ifndef LIMIT_BUILD_SIZE

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("Ser2net: TaskIndex=");
        log += event->TaskIndex;
        log += F(" port=");
        log += CONFIG_PORT;
        log += F(" rxPin=");
        log += rxPin;
        log += F(" txPin=");
        log += txPin;
        log += F(" BAUDRATE=");
        log += P020_GET_BAUDRATE;
        log += F(" SERVER_PORT=");
        log += P020_GET_SERVER_PORT;
        log += F(" SERIAL_PROCESSING=");
        log += P020_SERIAL_PROCESSING;
        addLogMove(LOG_LEVEL_INFO, log);
      }
      # endif // ifndef LIMIT_BUILD_SIZE

      // serial0 on esp32 is Ser2net: port=2 rxPin=3 txPin=1; serial1 on esp32 is Ser2net: port=4 rxPin=13 txPin=15; Serial2 on esp32 is
      // Ser2net: port=4 rxPin=16 txPin=17
      uint8_t serialconfig = serialHelper_convertOldSerialConfig(P020_SERIAL_CONFIG);
      task->serialBegin(port, rxPin, txPin, P020_GET_BAUDRATE, serialconfig);
      task->startServer(P020_GET_SERVER_PORT);

      if (!task->isInit()) {
        clearPluginTaskData(event->TaskIndex);
        break;
      }

      if (validGpio(P020_RESET_TARGET_PIN)) {
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log;
          log.reserve(38);
          log += F("Ser2net  : P020_RESET_TARGET_PIN : ");
          log += P020_RESET_TARGET_PIN;
          addLogMove(LOG_LEVEL_DEBUG, log);
        }
        # endif // ifndef BUILD_NO_DEBUG
        pinMode(P020_RESET_TARGET_PIN, OUTPUT);
        digitalWrite(P020_RESET_TARGET_PIN, LOW);
        delay(500);
        digitalWrite(P020_RESET_TARGET_PIN, HIGH);
        pinMode(P020_RESET_TARGET_PIN, INPUT_PULLUP);
      }

      task->serial_processing = P020_SERIAL_PROCESSING;
      success                 = true;
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

      if (nullptr == task) {
        break;
      }
      task->checkServer();
      success = true;
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P020_Task *task = static_cast<P020_Task *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == task) {
        break;
      }

      bool hasClient = task->hasClientConnected();

      if (P020_IGNORE_CLIENT_CONNECTED || hasClient) {
        if (hasClient) {
          task->handleClientIn(event);
        }
        task->handleSerialIn(event); // in case of second serial connected, PLUGIN_SERIAL_IN is not called anymore
      }
      success = true;
      break;
    }

    case PLUGIN_SERIAL_IN:
    {
      P020_Task *task = static_cast<P020_Task *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == task) {
        break;
      }

      if (P020_IGNORE_CLIENT_CONNECTED || task->hasClientConnected()) {
        task->handleSerialIn(event);
      } else {
        task->discardSerialIn();
      }
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      String command  = parseString(string, 1);
      P020_Task *task = static_cast<P020_Task *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == task) {
        break;
      }

        if (equals(command, F("serialsend"))) {
          task->ser2netSerial->write(string.substring(11).c_str());
          task->ser2netSerial->flush();
          success = true;
        } else

        if (equals(command, F("serialsendmix"))) {
          std::vector<uint8_t> argument = parseHexTextData(string);
          task->ser2netSerial->write(&argument[0], argument.size());
          task->ser2netSerial->flush();
          success = true;
        } else

      if ((equals(command, F("ser2netclientsend"))) && (task->hasClientConnected())) {
        task->ser2netClient.print(string.substring(18));
        task->ser2netClient.flush();
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P020
