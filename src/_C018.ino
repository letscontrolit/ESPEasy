#ifdef USES_C018

// #######################################################################################################
// ########################### Controller Plugin 018: LoRa TTN - RN2483/RN2903 ###########################
// #######################################################################################################

#define CPLUGIN_018
#define CPLUGIN_ID_018         18
#define CPLUGIN_NAME_018       "LoRa TTN - RN2483/RN2903 [TESTING]"

#define C018_BAUDRATE           9600
#define C018_BAUDRATE_LABEL     "baudrate"


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

  bool init(const int8_t serial_rx, const int8_t serial_tx, unsigned long baudrate) {
    cacheHWEUI       = "";
    cacheSysVer      = "";
    autobaud_success = false;

    if ((serial_rx < 0) && (serial_tx < 0)) {
      return false;
    }
    reset();
    C018_easySerial = new ESPeasySerial(serial_rx, serial_tx);

    if (C018_easySerial != nullptr) {
      C018_easySerial->begin(baudrate);
      myLora           = new rn2xx3(*C018_easySerial);
      autobaud_success = myLora->autobaud();
      cacheHWEUI       = myLora->hweui();
      cacheSysVer      = myLora->sysver();
    }
    return isInitialized();
  }

  bool isInitialized() const {
    return (C018_easySerial != nullptr) && (myLora != nullptr) && autobaud_success;
  }

  bool hasJoined() const {
    if (!isInitialized()) { return false; }
    return myLora->hasJoined();
  }

  bool txUncnfBytes(const byte *data, uint8_t size) {
    if (!hasJoined()) { return false; }
    return myLora->txBytes(data, size) != TX_FAIL;
  }

  bool txUncnf(const String& data) {
    if (!hasJoined()) { return false; }
    return myLora->tx(data) != TX_FAIL;
  }

  bool setFrequencyPlan(FREQ_PLAN plan) {
    if (!isInitialized()) { return false; }
    return myLora->setFrequencyPlan(plan);
  }

  bool setSF(uint8_t sf) {
    if (!isInitialized()) { return false; }
    return myLora->setSF(sf);
  }

  bool initOTAA(const String& AppEUI = "", const String& AppKey = "", const String& DevEUI = "") {
    if (!isInitialized()) { return false; }
    bool success = myLora->initOTAA(AppEUI, AppKey, DevEUI);
    updateCacheOnInit(success);
    return success;
  }

  bool initABP(const String& addr, const String& AppSKey, const String& NwkSKey) {
    if (!isInitialized()) { return false; }
    bool success = myLora->initABP(addr, AppSKey, NwkSKey);
    updateCacheOnInit(success);
    return success;
  }

  String sendRawCommand(const String& command) {
    if (!isInitialized()) { return ""; }
    return myLora->sendRawCommand(command);
  }

  int getVbat() {
    if (!isInitialized()) { return -1; }
    return myLora->getVbat();
  }

  String getLastErrorInvalidParam() {
    if (!isInitialized()) { return ""; }
    return myLora->getLastErrorInvalidParam();
  }

  // Cached data, only changing occasionally.

  String getDevaddr() {
    return cacheDevAddr;
  }

  String hweui() {
    return cacheHWEUI;
  }

  String sysver() {
    return cacheSysVer;
  }

private:

  void C018_logError(const String& command) {
    String error = myLora->getLastErrorInvalidParam();

    if (error.length() > 0) {
      String log = F("RN2384: ");
      log += command;
      log += ": ";
      log += myLora->getLastErrorInvalidParam();
      addLog(LOG_LEVEL_INFO, log);
    }
  }

  void updateCacheOnInit(bool success) {
    cacheDevAddr = "";

    if (!success) {
      C018_logError(F("Join"));
    }

    if (!success || !isInitialized()) { return; }
    cacheDevAddr = myLora->sendRawCommand(F("mac get devaddr"));
  }

  ESPeasySerial *C018_easySerial = nullptr;
  rn2xx3        *myLora          = nullptr;
  String         cacheDevAddr;
  String         cacheHWEUI;
  String         cacheSysVer;
  bool           autobaud_success = false;
} C018_data;


#define C018_DEVICE_EUI_LEN          17
#define C018_DEVICE_ADDR_LEN             33
#define C018_NETWORK_SESSION_KEY_LEN          33
#define C018_APP_SESSION_KEY_LEN            33
#define C018_USE_OTAA                 0
#define C018_USE_ABP                  1
struct C018_ConfigStruct
{
  C018_ConfigStruct() {
    reset();
  }

  void validate() {
    ZERO_TERMINATE(DeviceEUI);
    ZERO_TERMINATE(DeviceAddr);
    ZERO_TERMINATE(NetworkSessionKey);
    ZERO_TERMINATE(AppSessionKey);

    if ((baudrate < 2400) || (baudrate > 115200)) {
      reset();
    }
  }

  void reset() {
    ZERO_FILL(DeviceEUI);
    ZERO_FILL(DeviceAddr);
    ZERO_FILL(NetworkSessionKey);
    ZERO_FILL(AppSessionKey);
    baudrate      = 9600;
    rxpin         = 12;
    txpin         = 14;
    resetpin      = -1;
    sf            = 7;
    frequencyplan = TTN_EU;
    joinmethod    = C018_USE_OTAA;
  }

  char          DeviceEUI[C018_DEVICE_EUI_LEN]                  = { 0 };
  char          DeviceAddr[C018_DEVICE_ADDR_LEN]                = { 0 };
  char          NetworkSessionKey[C018_NETWORK_SESSION_KEY_LEN] = { 0 };
  char          AppSessionKey[C018_APP_SESSION_KEY_LEN]         = { 0 };
  unsigned long baudrate                                        = 9600;
  int8_t        rxpin                                           = 12;
  int8_t        txpin                                           = 14;
  int8_t        resetpin                                        = -1;
  uint8_t       sf                                              = 7;
  uint8_t       frequencyplan                                   = TTN_EU;
  uint8_t       joinmethod                                      = C018_USE_OTAA;
};


bool CPlugin_018(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
    {
      Protocol[++protocolCount].Number     = CPLUGIN_ID_018;
      Protocol[protocolCount].usesMQTT     = false;
      Protocol[protocolCount].usesAccount  = true;
      Protocol[protocolCount].usesPassword = true;
      Protocol[protocolCount].defaultPort  = 1;
      Protocol[protocolCount].usesID       = true;
      Protocol[protocolCount].usesHost     = false;
      break;
    }

    case CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_018);
      break;
    }

    case CPLUGIN_WEBFORM_SHOW_HOST_CONFIG:
    {
      string  = F("Dev addr: ");
      string += C018_data.getDevaddr();
      break;
    }

    case CPLUGIN_INIT:
    {
      MakeControllerSettings(ControllerSettings);
      LoadControllerSettings(event->ControllerIndex, ControllerSettings);
      C018_DelayHandler.configureControllerSettings(ControllerSettings);

      C018_ConfigStruct customConfig;
      LoadCustomControllerSettings(event->ControllerIndex, (byte *)&customConfig, sizeof(customConfig));
      customConfig.validate();

      C018_data.init(customConfig.rxpin, customConfig.txpin, customConfig.baudrate);

      String AppEUI = SecuritySettings.ControllerUser[event->ControllerIndex];
      String AppKey = SecuritySettings.ControllerPassword[event->ControllerIndex];

      C018_data.setFrequencyPlan(static_cast<FREQ_PLAN>(customConfig.frequencyplan));

      if (customConfig.joinmethod == C018_USE_OTAA) {
        C018_data.initOTAA(AppEUI, AppKey, customConfig.DeviceEUI);
      }
      else {
        C018_data.initABP(customConfig.DeviceAddr, customConfig.AppSessionKey, customConfig.NetworkSessionKey);
      }
      C018_data.setSF(customConfig.sf);

      C018_data.txUncnf("ESPeasy (TTN)");
      break;
    }

    case CPLUGIN_WEBFORM_LOAD:
    {
      C018_ConfigStruct customConfig;

      LoadCustomControllerSettings(event->ControllerIndex, (byte *)&customConfig, sizeof(customConfig));
      customConfig.validate();

      addFormTextBox(F("OTAA: Device EUI"), F("deveui"), customConfig.DeviceEUI, C018_DEVICE_EUI_LEN - 1);
      addFormNote(F("Leave empty to use HW DevEUI"));

      addFormTextBox(F("ABP: Device Addr"),         F("devaddr"), customConfig.DeviceAddr,        C018_DEVICE_ADDR_LEN - 1);
      addFormTextBox(F("ABP: Network Session Key"), F("nskey"),   customConfig.NetworkSessionKey, C018_NETWORK_SESSION_KEY_LEN - 1);
      addFormTextBox(F("ABP: App Session Key"),     F("appskey"), customConfig.AppSessionKey,     C018_APP_SESSION_KEY_LEN - 1);

      {
        byte   choice     = customConfig.joinmethod;
        String options[2] = { F("OTAA"),  F("ABP") };
        int    values[2]  = { C018_USE_OTAA, C018_USE_ABP };
        addFormSelector(F("Activation Method"), F("joinmethod"), 2, options, values, NULL, choice, false);
      }

      addTableSeparator(F("Connection Configuration"), 2, 3);
      {
        byte   choice     = customConfig.frequencyplan;
        String options[4] = { F("SINGLE_CHANNEL_EU"), F("TTN_EU"), F("TTN_US"), F("DEFAULT_EU") };
        int    values[4]  = { SINGLE_CHANNEL_EU, TTN_EU, TTN_US, DEFAULT_EU };

        addFormSelector(F("Frequency Plan"), F("frequencyplan"), 4, options, values, NULL, choice, false);
      }
      addFormNumericBox(F("Spread Factor"), F("sf"), customConfig.sf, 7, 12);


      addTableSeparator(F("Serial Port Configuration"), 2, 3);

      // Optional reset pin RN2xx3
      addRowLabel(formatGpioName_output_optional(F("Reset")));
      addPinSelect(false, F("taskdevicepin3"), customConfig.resetpin);

      // Show serial port selection
      addFormPinSelect(formatGpioName_RX(false), F("taskdevicepin1"), customConfig.rxpin);
      addFormPinSelect(formatGpioName_TX(false), F("taskdevicepin2"), customConfig.txpin);
      serialHelper_webformLoad(customConfig.rxpin, customConfig.txpin, true);

      addFormNumericBox(F("Baudrate"), F(C018_BAUDRATE_LABEL), customConfig.baudrate, 2400, 115200);
      addUnit(F("baud"));


      addTableSeparator(F("Device Status"), 2, 3);

      // Some information on detected device
      addRowLabel(F("Hardware DevEUI"));
      addHtml(String(C018_data.hweui()));
      addRowLabel(F("Version Number"));
      addHtml(String(C018_data.sysver()));

      addRowLabel(F("Voltage"));
      addHtml(String(static_cast<float>(C018_data.getVbat()) / 1000.0));

      addRowLabel(F("Dev Addr"));
      addHtml(C018_data.getDevaddr());

      addRowLabel(F("Uplink Frame Counter"));
      addHtml(C018_data.sendRawCommand(F("mac get upctr")));

      addRowLabel(F("Last Command Error"));
      addHtml(C018_data.getLastErrorInvalidParam());


      break;
    }
    case CPLUGIN_WEBFORM_SAVE:
    {
      C018_ConfigStruct customConfig;
      customConfig.reset();
      String deveui  = WebServer.arg(F("deveui"));
      String devaddr = WebServer.arg(F("devaddr"));
      String nskey   = WebServer.arg(F("nskey"));
      String appskey = WebServer.arg(F("appskey"));

      strlcpy(customConfig.DeviceEUI,         deveui.c_str(),  sizeof(customConfig.DeviceEUI));
      strlcpy(customConfig.DeviceAddr,        devaddr.c_str(), sizeof(customConfig.DeviceAddr));
      strlcpy(customConfig.NetworkSessionKey, nskey.c_str(),   sizeof(customConfig.NetworkSessionKey));
      strlcpy(customConfig.AppSessionKey,     appskey.c_str(), sizeof(customConfig.AppSessionKey));
      customConfig.baudrate      = getFormItemInt(F(C018_BAUDRATE_LABEL), customConfig.baudrate);
      customConfig.rxpin         = getFormItemInt(F("taskdevicepin1"), customConfig.rxpin);
      customConfig.txpin         = getFormItemInt(F("taskdevicepin2"), customConfig.txpin);
      customConfig.resetpin      = getFormItemInt(F("taskdevicepin3"), customConfig.resetpin);
      customConfig.sf            = getFormItemInt(F("sf"), customConfig.sf);
      customConfig.frequencyplan = getFormItemInt(F("frequencyplan"), customConfig.frequencyplan);
      customConfig.joinmethod    = getFormItemInt(F("joinmethod"), customConfig.joinmethod);
      serialHelper_webformSave(customConfig.rxpin, customConfig.txpin);
      SaveCustomControllerSettings(event->ControllerIndex, (byte *)&customConfig, sizeof(customConfig));
      break;
    }

    case CPLUGIN_GET_PROTOCOL_DISPLAY_NAME:
    {
      success = true;

      switch (event->idx) {
        case CONTROLLER_USER:
          string = F("OTAA: AppEUI");
          break;
        case CONTROLLER_PASS:
          string = F("OTAA: AppKey");
          break;
        case CONTROLLER_TIMEOUT:
          string = F("Gateway Timeout");
          break;
        case CONTROLLER_PORT:
          string = F("Port");
        default:
          success = false;
          break;
      }
      break;
    }

    case CPLUGIN_PROTOCOL_SEND:
    {
      byte valueCount = getValueCountFromSensorType(event->sensorType);
      success = C018_DelayHandler.addToQueue(C018_queue_element(event, valueCount));
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
  byte *buffer  = new byte[64];
  byte  length  = element.encode(buffer, 64);
  bool  success = C018_data.txUncnfBytes(buffer, length);

  delete[] buffer;
  return success;
}

#endif // ifdef USES_C018
