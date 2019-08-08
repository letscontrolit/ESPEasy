#ifdef USES_C018
//#######################################################################################################
//########################### Controller Plugin 018: TTN - RN2483/RN2903 ################################
//#######################################################################################################

#define CPLUGIN_018
#define CPLUGIN_ID_018         18
#define CPLUGIN_NAME_018       "TTN - RN2483/RN2903"


#include <rn2xx3.h>
#include <ESPeasySerial.h>

struct C018_data_struct {
  C018_data_struct() : C018_easySerial(nullptr), myLora(nullptr) {}

  ~C018_data_struct() {
    reset();
  }

  void reset() {
    if (myLora != nullptr) {
      delete myLora;
      myLora = nullptr;
    }
    if (C018_easySerial != nullptr) {
      delete C018_easySerial;
      C018_easySerial = nullptr;
    }
  }

  bool init(const int16_t serial_rx, const int16_t serial_tx, unsigned long baudrate) {
    if ((serial_rx < 0) && (serial_tx < 0)) {
      return false;
    }
    reset();
    C018_easySerial = new ESPeasySerial(serial_rx, serial_tx);

    if (C018_easySerial != nullptr) {
      C018_easySerial->begin(baudrate);
      myLora = new rn2xx3(*C018_easySerial);
      myLora->autobaud();
    }
    return isInitialized();
  }

  bool isInitialized() const {
    return (C018_easySerial != nullptr) && (myLora != nullptr);
  }

  bool txUncnfBytes(const byte* data, uint8_t size) {
    if (!isInitialized()) return false;
    return myLora->txBytes(data, size) != TX_FAIL;
  }

  bool txUncnf(const String& data) {
    if (!isInitialized()) return false;
    return myLora->tx(data) != TX_FAIL;
  }

  bool initOTAA(String AppEUI="", String AppKey="", String DevEUI="") {
    if (!isInitialized()) return false;
    return myLora->initOTAA(AppEUI, AppKey, DevEUI);
  }

  private:
    ESPeasySerial* C018_easySerial = nullptr;
    rn2xx3* myLora = nullptr;
} C018_data;

bool CPlugin_018(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_018;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 80;
        Protocol[protocolCount].usesID = true;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_018);
        break;
      }

    case CPLUGIN_INIT:
      {
        MakeControllerSettings(ControllerSettings);
        LoadControllerSettings(event->ControllerIndex, ControllerSettings);
        C018_DelayHandler.configureControllerSettings(ControllerSettings);

        
        C018_data.init(12, 14, 9600);

        String AppEUI = SecuritySettings.ControllerUser[event->ControllerIndex];
        String AppKey = SecuritySettings.ControllerPassword[event->ControllerIndex];

        C018_data.initOTAA(AppEUI, AppKey);

        C018_data.txUncnf("ESPeasy (TTN)");


        break;
      }

    case CPLUGIN_WEBFORM_LOAD:
    {
      /*
        myeasySerial.begin(9600);
        myLora.autobaud();

        addRowLabel(F("OTAA DevEUI"));
        addHtml(String(myLora.hweui()));
        addRowLabel(F("Version Number"));
        addHtml(String(myLora.sysver()));

        addRowLabel(F("Voltage"));
        addHtml(String(static_cast<float>(myLora.getVbat()) / 1000.0));

        addRowLabel(F("Dev Addr"));
        addHtml(myLora.sendRawCommand(F("mac get devaddr")));

        addRowLabel(F("Uplink Frame Counter"));
        addHtml(myLora.sendRawCommand(F("mac get upctr")));

        addRowLabel(F("Last Command Error"));
        addHtml(myLora.getLastErrorInvalidParam());

        */

    }
    break;
    case CPLUGIN_GET_PROTOCOL_DISPLAY_NAME:
      {
        success = true;
        switch (event->idx) {
          case CONTROLLER_USER:
            string = F("AppEUI");
            break;
          case CONTROLLER_PASS:
            string = F("AppKey");
            break;
          default:
            success = false;
            break;
        }
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        success = C018_DelayHandler.addToQueue(C018_queue_element(event, VARS_PER_TASK));
        scheduleNextDelayQueue(TIMER_C018_DELAY_QUEUE, C018_DelayHandler.getNextScheduleTime());

        break;
      }

    case CPLUGIN_FLUSH:
      {
        process_c018_delay_queue();
        delay(0);
        break;
      }

  }
  return success;
}

bool do_process_c018_delay_queue(int controller_number, const C018_queue_element& element, ControllerSettingsStruct& ControllerSettings) {

  return true;
}
#endif
