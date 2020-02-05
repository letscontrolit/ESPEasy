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
#include "src/Globals/CPlugins.h"

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
    cacheHWEUI       = "";
    cacheSysVer      = "";
    autobaud_success = false;
    if (resetPin != -1) {
      pinMode(resetPin, OUTPUT); 
      digitalWrite(resetPin, LOW);
      delay(500);
      digitalWrite(resetPin, HIGH);
      delay(500);
    }
  }

  bool init(const int8_t serial_rx, const int8_t serial_tx, unsigned long baudrate, bool joinIsOTAA, taskIndex_t sampleSet_Initiator, int8_t reset_pin) {
    if ((serial_rx < 0) || (serial_tx < 0)) {
      // Both pins are needed, or else no serial possible
      return false;
    }
    sampleSetInitiator = sampleSet_Initiator;

    if (isInitialized()) {
      // Check to see if serial parameters have changed.
      bool notChanged = true;
      notChanged &= C018_easySerial->getRxPin() == serial_rx;
      notChanged &= C018_easySerial->getTxPin() == serial_tx;
      notChanged &= C018_easySerial->getBaudRate() == baudrate;

      if (notChanged) { return true; }
    }
    resetPin = reset_pin;

    reset();
    C018_easySerial = new ESPeasySerial(serial_rx, serial_tx);

    if (C018_easySerial != nullptr) {
      C018_easySerial->begin(baudrate);
      myLora           = new rn2xx3(*C018_easySerial);
      // wakeUP_RN2483 and set data rate
      // Delay must be longer than specified in the datasheet for firmware 1.0.3
      // See: https://www.thethingsnetwork.org/forum/t/rn2483a-problems-no-serial-communication/7866/36?u=td-er
      pinMode(serial_tx, OUTPUT); 
      digitalWrite(serial_tx, LOW); // Send a break to the RN2483
      delay(26);
      digitalWrite(serial_tx, HIGH);
      autobaud_success = myLora->autobaud();
      myLora->setLastUsedJoinMode(joinIsOTAA);
    }
    return isInitialized();
  }

  bool isInitialized() const {
    return (C018_easySerial != nullptr) && (myLora != nullptr) /* && autobaud_success */;
  }

  bool hasJoined() const {
    if (!isInitialized()) { return false; }
    return myLora->hasJoined();
  }

  bool useOTAA() const {
    if (!isInitialized()) { return true; }
    return myLora->useOTAA();
  }

  bool txUncnfBytes(const byte *data, uint8_t size, uint8_t port) {
    if (!hasJoined()) { return false; }
    return myLora->txBytes(data, size, port) != TX_FAIL;
  }

  bool txHexBytes(const String& data, uint8_t port) {
    if (!hasJoined()) { return false; }
    return myLora->txHexBytes(data, port) != TX_FAIL;
  }

  bool txUncnf(const String& data, uint8_t port) {
    if (!hasJoined()) { return false; }
    return myLora->tx(data, port) != TX_FAIL;
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
    updateCacheOnInit();
    return success;
  }

  bool initABP(const String& addr, const String& AppSKey, const String& NwkSKey) {
    if (!isInitialized()) { return false; }
    bool success = myLora->initABP(addr, AppSKey, NwkSKey);
    updateCacheOnInit();
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

  String peekLastErrorInvalidParam() {
    if (!isInitialized()) { return ""; }
    return myLora->peekLastErrorInvalidParam();
  }

  String getLastErrorInvalidParam() {
    if (!isInitialized()) { return ""; }
    return myLora->getLastErrorInvalidParam();
  }

  String getDataRate() {
    if (!isInitialized()) { return ""; }
    return myLora->getDataRate();
  }

  int getRSSI() {
    if (!isInitialized()) { return 0; }
    return myLora->getRSSI();
  }

  uint32_t getRawStatus() {
    if (!isInitialized()) { return 0; }
    return myLora->Status.getRawStatus();
  }

  bool getFrameCounters(uint32_t& dnctr, uint32_t& upctr) {
    if (!isInitialized()) { return false; }
    return myLora->getFrameCounters(dnctr, upctr);
  }

  bool setFrameCounters(uint32_t dnctr, uint32_t upctr) {
    if (!isInitialized()) { return false; }
    return myLora->setFrameCounters(dnctr, upctr);
  }

  // Cached data, only changing occasionally.

  String getDevaddr() {
    if (cacheDevAddr.length() == 0)
    {
      updateCacheOnInit();
    }
    return cacheDevAddr;
  }

  String hweui() {
    if (cacheHWEUI.length() == 0) {
      if (isInitialized()) {
        cacheHWEUI = myLora->hweui();
      }
    }
    return cacheHWEUI;
  }

  String sysver() {
    if (cacheSysVer.length() == 0) {
      if (isInitialized()) {
        cacheSysVer = myLora->sysver();
      }
    }
    return cacheSysVer;
  }

  uint8_t getSampleSetCount() const { return sampleSetCounter; }

  uint8_t getSampleSetCount(taskIndex_t taskIndex) {
    if (sampleSetInitiator == taskIndex)
    {
      ++sampleSetCounter;
    }
    return sampleSetCounter;
  }


private:

  void C018_logError(const String& command) {
    String error = myLora->peekLastErrorInvalidParam();

    if (error.length() > 0) {
      String log = F("RN2384: ");
      log += command;
      log += ": ";
      log += error;
      addLog(LOG_LEVEL_INFO, log);
    }
  }

  void updateCacheOnInit() {
    cacheDevAddr = "";

    if (!isInitialized()) {
      return;
    }

    if (myLora->Status.Joined)
    {
      cacheDevAddr = myLora->sendRawCommand(F("mac get devaddr"));

      if (cacheDevAddr == F("00000000")) {
        cacheDevAddr = "";
      }
    }
  }

  ESPeasySerial *C018_easySerial = nullptr;
  rn2xx3        *myLora          = nullptr;
  String         cacheDevAddr;
  String         cacheHWEUI;
  String         cacheSysVer;
  uint8_t        sampleSetCounter = 0;
  taskIndex_t    sampleSetInitiator = INVALID_TASK_INDEX;
  int8_t         resetPin = -1;
  bool           autobaud_success = false;
} C018_data;


#define C018_DEVICE_EUI_LEN          17
#define C018_DEVICE_ADDR_LEN         33
#define C018_NETWORK_SESSION_KEY_LEN 33
#define C018_APP_SESSION_KEY_LEN     33
#define C018_USE_OTAA                0
#define C018_USE_ABP                 1
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
      Protocol[++protocolCount].Number       = CPLUGIN_ID_018;
      Protocol[protocolCount].usesMQTT       = false;
      Protocol[protocolCount].usesAccount    = true;
      Protocol[protocolCount].usesPassword   = true;
      Protocol[protocolCount].defaultPort    = 1;
      Protocol[protocolCount].usesID         = true;
      Protocol[protocolCount].usesHost       = false;
      Protocol[protocolCount].usesSampleSets = true;
      break;
    }

    case CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_018);
      break;
    }

    case CPLUGIN_WEBFORM_SHOW_HOST_CONFIG:
    {
      if (C018_data.isInitialized()) {
        string  = F("Dev addr: ");
        string += C018_data.getDevaddr();
        string += C018_data.useOTAA() ? F(" (OTAA)") : F(" (ABP)");
      } else {
        string = F("-");
      }
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

      C018_data.init(customConfig.rxpin, customConfig.txpin, customConfig.baudrate, 
                     customConfig.joinmethod == C018_USE_OTAA,
                     ControllerSettings.SampleSetInitiator, customConfig.resetpin);

      C018_data.setFrequencyPlan(static_cast<FREQ_PLAN>(customConfig.frequencyplan));

      if (customConfig.joinmethod == C018_USE_OTAA) {
        String AppEUI = SecuritySettings.ControllerUser[event->ControllerIndex];
        String AppKey = SecuritySettings.ControllerPassword[event->ControllerIndex];
        C018_data.initOTAA(AppEUI, AppKey, customConfig.DeviceEUI);
      }
      else {
        C018_data.initABP(customConfig.DeviceAddr, customConfig.AppSessionKey, customConfig.NetworkSessionKey);
      }
      C018_data.setSF(customConfig.sf);

      C018_data.txUncnf("ESPeasy (TTN)", ControllerSettings.Port);
      break;
    }

    case CPLUGIN_WEBFORM_LOAD:
    {
      C018_ConfigStruct customConfig;

      LoadCustomControllerSettings(event->ControllerIndex, (byte *)&customConfig, sizeof(customConfig));
      customConfig.validate();

      {
        // Script to toggle visibility of OTAA/ABP field, based on the activation method selector.
        protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(event->ControllerIndex);
        html_add_script(false);
        addHtml(F("function joinChanged(elem){ var styleOTAA = elem.value == 0 ? '' : 'none'; var styleABP = elem.value == 1 ? '' : 'none';"));
        addHtml(c018_add_joinChanged_script_element_line(getControllerParameterInternalName(ProtocolIndex, CONTROLLER_USER), true));
        addHtml(c018_add_joinChanged_script_element_line(getControllerParameterInternalName(ProtocolIndex, CONTROLLER_PASS), true));
        addHtml(c018_add_joinChanged_script_element_line(F("deveui"), true));
        addHtml(c018_add_joinChanged_script_element_line(F("deveui_note"), true));

        addHtml(c018_add_joinChanged_script_element_line(F("devaddr"), false));
        addHtml(c018_add_joinChanged_script_element_line(F("nskey"), false));
        addHtml(c018_add_joinChanged_script_element_line(F("appskey"), false));
        addHtml("}");
        html_add_script_end();
      }

      {
        addFormTextBox(F("Device EUI"), F("deveui"), customConfig.DeviceEUI, C018_DEVICE_EUI_LEN - 1);
        String deveui_note = F("Leave empty to use HW DevEUI: ");
        deveui_note += C018_data.hweui();
        addFormNote(deveui_note, F("deveui_note"));
      }

      addFormTextBox(F("Device Addr"),         F("devaddr"), customConfig.DeviceAddr,        C018_DEVICE_ADDR_LEN - 1);
      addFormTextBox(F("Network Session Key"), F("nskey"),   customConfig.NetworkSessionKey, C018_NETWORK_SESSION_KEY_LEN - 1);
      addFormTextBox(F("App Session Key"),     F("appskey"), customConfig.AppSessionKey,     C018_APP_SESSION_KEY_LEN - 1);

      {
        byte   choice     = customConfig.joinmethod;
        String options[2] = { F("OTAA"),  F("ABP") };
        int    values[2]  = { C018_USE_OTAA, C018_USE_ABP };
        addFormSelector_script(F("Activation Method"), F("joinmethod"), 2,
                               options, values, NULL, choice,
                               F("joinChanged(this)")); // Script to toggle OTAA/ABP fields visibility when changing selection.
      }
      html_add_script(F("document.getElementById('joinmethod').onchange();"), false);

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
      addFormPinSelect(formatGpioName_output_optional(F("Reset")), F("taskdevicepin3"), customConfig.resetpin);

      // Show serial port selection
      addFormPinSelect(formatGpioName_RX(false),                   F("taskdevicepin1"), customConfig.rxpin);
      addFormPinSelect(formatGpioName_TX(false),                   F("taskdevicepin2"), customConfig.txpin);
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
      addHtml(String(static_cast<float>(C018_data.getVbat()) / 1000.0, 3));

      addRowLabel(F("Dev Addr"));
      addHtml(C018_data.getDevaddr());

      uint32_t dnctr, upctr;

      if (C018_data.getFrameCounters(dnctr, upctr)) {
        addRowLabel(F("Frame Counters (down/up)"));
        String values = String(dnctr);
        values += '/';
        values += upctr;
        addHtml(values);
      }

      addRowLabel(F("Last Command Error"));
      addHtml(C018_data.getLastErrorInvalidParam());

      addRowLabel(F("Sample Set Counter"));
      addHtml(String(C018_data.getSampleSetCount()));

      addRowLabel(F("Status"));
      addHtml(String(C018_data.getRawStatus()));


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
          string = F("AppEUI");
          break;
        case CONTROLLER_PASS:
          string = F("AppKey");
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
      success = C018_DelayHandler.addToQueue(
        C018_queue_element(event, C018_data.getSampleSetCount(event->TaskIndex)));
      scheduleNextDelayQueue(TIMER_C018_DELAY_QUEUE, C018_DelayHandler.getNextScheduleTime());

      break;
    }

    case CPLUGIN_PROTOCOL_RECV:
    {


      // FIXME TD-er: WHen should this be scheduled?
      //schedule_controller_event_timer(event->ProtocolIndex, CPLUGIN_PROTOCOL_RECV, event);
      break;
    }

    case CPLUGIN_TEN_PER_SECOND:
    {
      // FIXME TD-er: Handle reading error state or return values.
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

bool do_process_c018_delay_queue(int controller_number, const C018_queue_element& element, ControllerSettingsStruct& ControllerSettings);

bool do_process_c018_delay_queue(int controller_number, const C018_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
  bool  success = C018_data.txHexBytes(element.packed, ControllerSettings.Port);
  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("C018 : Sent: ");
    log += element.packed;
    log += F(" length: ");
    log += String(element.packed.length());
    if (success) {
      log += F(" (success)");
    }
    addLog(LOG_LEVEL_DEBUG, log);
  }
  return success;
}

String c018_add_joinChanged_script_element_line(const String& id, bool forOTAA) {
  String result = F("document.getElementById('tr_");

  result += id;
  result += F("').style.display = style");
  result += forOTAA ? F("OTAA") : F("ABP");
  result += ';';
  return result;
}

#endif // ifdef USES_C018
