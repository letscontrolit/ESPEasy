#include "_Plugin_Helper.h"
#ifdef USES_P044_ORG

// #################################### Plugin 044: P1WifiGateway ########################################
//
//  based on P020 Ser2Net, extended by Ronald Leenes romix/-at-/macuser.nl
//
//  designed for combo
//    Wemos D1 mini (see http://wemos.cc) and
//    P1 wifi gateway shield (see http://www.esp8266thingies.nl for print design and kits)
//    See also http://domoticx.com/p1-poort-slimme-meter-hardware/
// #######################################################################################################

/** Changelog:
 * 2022-10-08 tonhuisman: Disable plugin-code and merge all functionality into P020 as it was originally a modified copy of that plugin
 *                        *** This code is deprecated ***
 * 2022-10-01 tonhuisman: Add Led configuration options (Enabled, Pin, Inverted), changed device configuration
 * 2022-10-01 tonhuisman: Format source using Uncrustify
 */

# include "src/Helpers/_Plugin_Helper_serial.h"
# include "src/PluginStructs/P044_data_struct.h"
# include <ESPeasySerial.h>

# define PLUGIN_044
# define PLUGIN_ID_044         44
# define PLUGIN_NAME_044       "Communication - P1 Wifi Gateway"


boolean Plugin_044(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number    = PLUGIN_ID_044;
      Device[deviceCount].Type        = DEVICE_TYPE_CUSTOM2;
      Device[deviceCount].Custom      = true;
      Device[deviceCount].TimerOption = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_044);
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P044_LED_PIN = P044_STATUS_LED; // Former default
      break;
    }

    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      string  = concat(F("RST: "), formatGpioLabel(P044_RESET_TARGET_PIN, false));
      string += event->String1;
      string += concat(F("LED: "), formatGpioLabel((P044_LED_ENABLED & 0x7f) == 0 ? P044_LED_PIN : -1, false));

      if ((P044_LED_INVERTED == 1) && ((P044_LED_ENABLED & 0x7f) == 0)) {
        string += F(" (inv)");
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      # ifdef USES_P020
      String msg;
      msg.reserve(132);
      msg += F("This plugin is deprecated and will be removed in a future release. Please use P020 - ");
      msg += getPluginNameFromPluginID(20);
      addFormNote(msg);
      # endif // ifdef USES_P020
      LoadTaskSettings(event->TaskIndex);

      { // Serial settings
        addFormSubHeader(F("Serial"));

        uint8_t serialConfChoice = serialHelper_convertOldSerialConfig(P044_SERIAL_CONFIG);
        serialHelper_serialconfig_webformLoad(event, serialConfChoice);

        addFormNumericBox(F("Baud Rate"), F("pbaud"), P044_BAUDRATE, 0, 115200);
      }

      { // Device settings
        addFormSubHeader(F("Device"));

        addFormNumericBox(F("TCP Port"), F("pport"), P044_WIFI_SERVER_PORT, 0, 65535);
        # ifndef LIMIT_BUILD_SIZE
        addUnit(F("0..65535"));
        # endif // ifndef LIMIT_BUILD_SIZE

        // FIXME TD-er: Why isn't this using the normal pin selection functions?
        addFormPinSelect(PinSelectPurpose::Generic, F("Reset target after boot"), F("taskdevicepin1"), P044_RESET_TARGET_PIN);

        addFormNumericBox(F("RX Receive Timeout (mSec)"), F("prxwait"), P044_RX_WAIT, 0);
      }

      { // Led settings
        addFormSubHeader(F("Led"));

        addFormCheckBox(F("Led enabled"), F("pled"), (P044_LED_ENABLED & 0x7f) == 0);
        addFormPinSelect(PinSelectPurpose::Generic, F("Led pin"), F("taskdevicepin2"), P044_LED_PIN);
        addFormCheckBox(F("Led inverted"), F("pledinv"), P044_LED_INVERTED == 1);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      LoadTaskSettings(event->TaskIndex);
      P044_WIFI_SERVER_PORT = getFormItemInt(F("pport"));
      P044_BAUDRATE         = getFormItemInt(F("pbaud"));
      P044_RX_WAIT          = getFormItemInt(F("prxwait"));
      P044_LED_ENABLED      = 0x80 + (isFormItemChecked(F("pled")) ? 0 : 1); // Invert + set 8th bit to confirm new settings have been saved
      P044_LED_INVERTED     = isFormItemChecked(F("pledinv")) ? 1 : 0;
      P044_SERIAL_CONFIG    = serialHelper_serialconfig_webformSave();

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (((P044_LED_ENABLED & 0x7f) == 1) && (P044_LED_PIN != -1)) {
        pinMode(P044_LED_PIN, OUTPUT);
        digitalWrite(P044_LED_PIN, P044_LED_INVERTED == 1 ? 1 : 0);
      }

      LoadTaskSettings(event->TaskIndex);

      if ((P044_WIFI_SERVER_PORT == 0) || (P044_BAUDRATE == 0)) {
        clearPluginTaskData(event->TaskIndex);
        break;
      }

      // try to reuse to keep webserver running
      P044_Task *task = static_cast<P044_Task *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != task) && task->isInit()) {
        // It was already created and initialzed
        // So don't recreate to keep the webserver running.
      } else {
        initPluginTaskData(event->TaskIndex, new (std::nothrow) P044_Task(event));
        task = static_cast<P044_Task *>(getPluginTaskData(event->TaskIndex));
      }

      if (nullptr == task) {
        break;
      }

      int rxPin;
      int txPin;

      // FIXME TD-er: Must use proper pin settings and standard ESPEasySerial wrapper
      ESPeasySerialType::getSerialTypePins(ESPEasySerialPort::serial0, rxPin, txPin);
      uint8_t serialconfig = serialHelper_convertOldSerialConfig(P044_SERIAL_CONFIG);
      task->serialBegin(ESPEasySerialPort::not_set, rxPin, txPin, P044_BAUDRATE, serialconfig);
      task->startServer(P044_WIFI_SERVER_PORT);

      if (!task->isInit()) {
        clearPluginTaskData(event->TaskIndex);
        break;
      }

      if (validGpio(P044_RESET_TARGET_PIN)) {
        pinMode(P044_RESET_TARGET_PIN, OUTPUT);
        digitalWrite(P044_RESET_TARGET_PIN, LOW);
        delay(500);
        digitalWrite(P044_RESET_TARGET_PIN, HIGH);
        pinMode(P044_RESET_TARGET_PIN, INPUT_PULLUP);
      }

      task->blinkLED();

      if (P044_BAUDRATE == 115200) {
        # ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("P1   : DSMR version 5 meter, CRC on"));
        # endif // ifndef BUILD_NO_DEBUG
        task->CRCcheck = true;
      } else {
        # ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("P1   : DSMR version 4 meter, CRC off"));
        # endif // ifndef BUILD_NO_DEBUG
        task->CRCcheck = false;
      }

      success = true;
      break;
    }

    case PLUGIN_EXIT:
      {
        P044_Task *task = static_cast<P044_Task *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != task) {
          task->stopServer();
          task->serialEnd();
        }

        success = true;
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
    {
      P044_Task *task = static_cast<P044_Task *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != task) {
        task->checkServer();
        success = true;
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P044_Task *task = static_cast<P044_Task *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != task) {
        if (task->hasClientConnected()) {
          task->discardClientIn();
        }
        task->checkBlinkLED();
        success = true;
      }
      break;
    }

    case PLUGIN_SERIAL_IN:
    {
      P044_Task *task = static_cast<P044_Task *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != task) {
        if (task->hasClientConnected()) {
          task->handleSerialIn(event);
        } else {
          task->discardSerialIn();
        }
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P044
