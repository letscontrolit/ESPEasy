#include "_Plugin_Helper.h"
#ifdef USES_P044
//#################################### Plugin 044: P1WifiGateway ########################################
//
//  based on P020 Ser2Net, extended by Ronald Leenes romix/-at-/macuser.nl
//
//  designed for combo
//    Wemos D1 mini (see http://wemos.cc) and
//    P1 wifi gateway shield (see http://www.esp8266thingies.nl for print design and kits)
//    See also http://domoticx.com/p1-poort-slimme-meter-hardware/
//#######################################################################################################


#include "src/Helpers/_Plugin_Helper_serial.h"
#include "src/PluginStructs/P044_data_struct.h"
#include <ESPeasySerial.h>

#define PLUGIN_044
#define PLUGIN_ID_044         44
#define PLUGIN_NAME_044       "Communication - P1 Wifi Gateway"


#define P044_SET_WIFI_SERVER_PORT     ExtraTaskSettings.TaskDevicePluginConfigLong[0]
#define P044_SET_BAUDRATE             ExtraTaskSettings.TaskDevicePluginConfigLong[1]
#define P044_GET_WIFI_SERVER_PORT     Cache.getTaskDevicePluginConfigLong(event->TaskIndex, 0)
#define P044_GET_BAUDRATE             Cache.getTaskDevicePluginConfigLong(event->TaskIndex, 1)


#define P044_RX_WAIT              PCONFIG(0)
#define P044_SERIAL_CONFIG        PCONFIG(1)
#define P044_RESET_TARGET_PIN     CONFIG_PIN1
 


boolean Plugin_044(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_044;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].Custom = true;
        Device[deviceCount].TimerOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_044);
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
      	addFormNumericBox(F("TCP Port"), F("p044_port"), P044_GET_WIFI_SERVER_PORT, 0);
      	addFormNumericBox(F("Baud Rate"), F("p044_baud"), P044_GET_BAUDRATE, 0);

        uint8_t serialConfChoice = serialHelper_convertOldSerialConfig(P044_SERIAL_CONFIG);
        serialHelper_serialconfig_webformLoad(event, serialConfChoice);

        // FIXME TD-er: Why isn't this using the normal pin selection functions?
      	addFormPinSelect(PinSelectPurpose::Generic, F("Reset target after boot"), F("taskdevicepin1"), P044_RESET_TARGET_PIN);

      	addFormNumericBox(F("RX Receive Timeout (mSec)"), F("p044_rxwait"), P044_RX_WAIT, 0);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        P044_SET_WIFI_SERVER_PORT = getFormItemInt(F("p044_port"));
        P044_SET_BAUDRATE = getFormItemInt(F("p044_baud"));
        P044_RX_WAIT = getFormItemInt(F("p044_rxwait"));
        P044_SERIAL_CONFIG = serialHelper_serialconfig_webformSave();

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        pinMode(P044_STATUS_LED, OUTPUT);
        digitalWrite(P044_STATUS_LED, 0);

        if ((P044_GET_WIFI_SERVER_PORT == 0) || (P044_GET_BAUDRATE == 0)) {
          clearPluginTaskData(event->TaskIndex);
          break;
        }

        // try to reuse to keep webserver running
        P044_Task *task = static_cast<P044_Task *>(getPluginTaskData(event->TaskIndex));
        if (nullptr != task && task->isInit()) {
          // It was already created and initialzed
          // So don't recreate to keep the webserver running.
        } else {
          initPluginTaskData(event->TaskIndex, new (std::nothrow) P044_Task());
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
        task->serialBegin(ESPEasySerialPort::not_set,  rxPin, txPin, P044_GET_BAUDRATE, serialconfig);
        task->startServer(P044_GET_WIFI_SERVER_PORT);

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
        if (P044_GET_BAUDRATE == 115200) {
          #ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, F("P1   : DSMR version 4 meter, CRC on"));
          #endif
          task->CRCcheck = true;
        } else {
          #ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, F("P1   : DSMR version 4 meter, CRC off"));
          #endif
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
        if (nullptr == task) {
          break;
        }
        task->checkServer();
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        P044_Task *task = static_cast<P044_Task *>(getPluginTaskData(event->TaskIndex));
        if (nullptr == task) {
          break;
        }
        if (task->hasClientConnected()) {
          task->discardClientIn();
        }
        task->checkBlinkLED();
        success = true;
        break;
      }

    case PLUGIN_SERIAL_IN:
      {
        P044_Task *task = static_cast<P044_Task *>(getPluginTaskData(event->TaskIndex));
        if (nullptr == task) {
          break;
        }
        if (task->hasClientConnected()) {
          task->handleSerialIn(event);
        } else {
          task->discardSerialIn();
        }
        success = true;
        break;
      }

  }
  return success;
}
#endif // USES_P044
