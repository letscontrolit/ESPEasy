#include "_Plugin_Helper.h"
#ifdef USES_P020

// #######################################################################################################
// #################################### Plugin 020: Ser2Net ##############################################
// #######################################################################################################


# include "src/Helpers/_Plugin_Helper_serial.h"
# include "src/PluginStructs/P020_data_struct.h"
# include <ESPeasySerial.h>


# define PLUGIN_020
# define PLUGIN_ID_020         20
# define PLUGIN_NAME_020       "Communication - Serial Server"
# define PLUGIN_VALUENAME1_020 "Ser2Net"


# define P020_SERVER_PORT          ExtraTaskSettings.TaskDevicePluginConfigLong[0]
# define P020_BAUDRATE             ExtraTaskSettings.TaskDevicePluginConfigLong[1]

// #define P020_BAUDRATE             ExtraTaskSettings.TaskDevicePluginConfigLong[1]
# define P020_RX_WAIT              PCONFIG(4)
# define P020_SERIAL_CONFIG        PCONFIG(1)
# define P020_SERIAL_PROCESSING    PCONFIG(5)
# define P020_RESET_TARGET_PIN     CONFIG_PIN1


# define P020_QUERY_VALUE        0 // Temp placement holder until we know what selectors are needed.
# define P020_NR_OUTPUT_OPTIONS  1

# define P020_NR_OUTPUT_VALUES   1
# define P020_QUERY1_CONFIG_POS  3

# define P020_DEFAULT_SERVER_PORT 1234
# define P020_DEFAULT_BAUDRATE   115200


boolean Plugin_020(byte function, struct EventStruct *event, String& string)
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
      P020_BAUDRATE    = P020_DEFAULT_BAUDRATE;
      P020_SERVER_PORT = P020_DEFAULT_SERVER_PORT;
      success          = true;
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


    case PLUGIN_WEBFORM_LOAD:
    { 
      addFormNumericBox(F("TCP Port"),  F("p020_port"), P020_SERVER_PORT, 0);
      addFormNumericBox(F("Baud Rate"), F("p020_baud"), P020_BAUDRATE,    0);
      byte serialConfChoice = serialHelper_convertOldSerialConfig(P020_SERIAL_CONFIG);
      serialHelper_serialconfig_webformLoad(event, serialConfChoice);
      {
        byte   choice = P020_SERIAL_PROCESSING;
        String options[3];
        options[0] = F("None");
        options[1] = F("Generic");
        options[2] = F("RFLink");
        addFormSelector(F("Event processing"), F("p020_events"), 3, options, NULL, choice);
      }
      addFormNumericBox(F("RX Receive Timeout (mSec)"), F("p020_rxwait"), P020_RX_WAIT, 0);
      success = true;
      break; 
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P020_SERVER_PORT       = getFormItemInt(F("p020_port"));
      P020_BAUDRATE          = getFormItemInt(F("p020_baud"));
      P020_SERIAL_CONFIG     = serialHelper_serialconfig_webformSave();
      P020_SERIAL_PROCESSING = getFormItemInt(F("p020_events"));
      P020_RX_WAIT           = getFormItemInt(F("p020_rxwait"));
      success                = true;
      break;
    }

    case PLUGIN_INIT:
    {
      LoadTaskSettings(event->TaskIndex);

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
        initPluginTaskData(event->TaskIndex, new (std::nothrow) P020_Task(event->TaskIndex));
        task = static_cast<P020_Task *>(getPluginTaskData(event->TaskIndex));
      }

      if (nullptr == task) {
        break;
      }

      // int rxPin =-1;
      // int txPin =-1;
      int rxPin                    = CONFIG_PIN1;
      int txPin                    = CONFIG_PIN2;
      const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);

      // const ESPEasySerialPort port= ESPEasySerialPort::serial0;
      if ((rxPin < 0) && (rxPin < 0)) {
        ESPeasySerialType::getSerialTypePins(port, rxPin, txPin);
        CONFIG_PIN1 = rxPin;
        CONFIG_PIN2 = txPin;
      }
      addLog(LOG_LEVEL_INFO,
             String(F("Ser2net: TaskIndex=")) + event->TaskIndex + String(F(" port=")) + CONFIG_PORT + String(F(" rxPin=")) + rxPin +
             String(F(" txPin=")) + txPin + String(F(" BAUDRATE=")) + P020_BAUDRATE + String(F(" SERVER_PORT=")) + P020_SERVER_PORT +
             String(F(" SERIAL_PROCESSING=")) + P020_SERIAL_PROCESSING);

      // serial0 on esp32 is Ser2net: port=2 rxPin=3 txPin=1; serial1 on esp32 is Ser2net: port=4 rxPin=13 txPin=15; Serial2 on esp32 is
      // Ser2net: port=4 rxPin=16 txPin=17
      byte serialconfig = serialHelper_convertOldSerialConfig(P020_SERIAL_CONFIG);
      task->serialBegin(port, rxPin, txPin, P020_BAUDRATE, serialconfig);
      task->startServer(P020_SERVER_PORT);

      if (!task->isInit()) {
        clearPluginTaskData(event->TaskIndex);
        break;
      }

      if (P020_RESET_TARGET_PIN != -1) {
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

      if (task->hasClientConnected()) {
        task->handleClientIn(event);
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

      if (task->hasClientConnected()) { 
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
      
      if (command == F("serialsend")) {
        const char *tmpBuf = string.substring(11).c_str();
        task->ser2netSerial->write(tmpBuf);
        task->ser2netSerial->flush();
        success = true;
      }

      if ((command == F("ser2netclientsend")) && (task->hasClientConnected())) {
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
