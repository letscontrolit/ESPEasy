#include "../Controller_struct/C018_data_struct.h"

#ifdef USES_C018

C018_data_struct::C018_data_struct() :
  C018_easySerial(nullptr),
  myLora(nullptr) {}

C018_data_struct::~C018_data_struct() {
  reset();
}

void C018_data_struct::reset() {
  if (myLora != nullptr) {
    delete myLora;
    myLora = nullptr;
  }

  if (C018_easySerial != nullptr) {
    delete C018_easySerial;
    C018_easySerial = nullptr;
  }
  cacheDevAddr     = String();
  cacheHWEUI       = String();
  cacheSysVer      = String();
  autobaud_success = false;
}

bool C018_data_struct::init(const uint8_t port, const int8_t serial_rx, const int8_t serial_tx, unsigned long baudrate,
                            bool joinIsOTAA, taskIndex_t sampleSet_Initiator, int8_t reset_pin) {
  if ((serial_rx < 0) || (serial_tx < 0)) {
    // Both pins are needed, or else no serial possible
    return false;
  }

  // FIXME TD-er: Prevent unneeded OTAA joins.
  // See: https://www.thethingsnetwork.org/forum/t/how-often-should-a-node-do-an-otaa-join-and-is-otaa-better-than-abp/11192/47?u=td-er


  sampleSetInitiator = sampleSet_Initiator;

  if (isInitialized()) {
    // Check to see if serial parameters have changed.
    bool notChanged = true;
    notChanged &= C018_easySerial->getRxPin() == serial_rx;
    notChanged &= C018_easySerial->getTxPin() == serial_tx;
    notChanged &= C018_easySerial->getBaudRate() == baudrate;
    notChanged &= myLora->useOTAA() == joinIsOTAA;

    if (notChanged) { return true; }
  }
  reset();
  _resetPin = reset_pin;
  _baudrate = baudrate;

  // FIXME TD-er: Make force SW serial a proper setting.
  if (C018_easySerial != nullptr) {
    delete C018_easySerial;
  }

  C018_easySerial = new (std::nothrow) ESPeasySerial(static_cast<ESPEasySerialPort>(port), serial_rx, serial_tx, false, 64);

  if (C018_easySerial != nullptr) {
    if (myLora != nullptr) {
      delete myLora;
    }
    myLora = new (std::nothrow) rn2xx3(*C018_easySerial);

    if (myLora == nullptr) {
      delete C018_easySerial;
      C018_easySerial = nullptr;
    } else {
      myLora->setAsyncMode(true);
      myLora->setLastUsedJoinMode(joinIsOTAA);
      triggerAutobaud();
    }
  }
  return isInitialized();
}

bool C018_data_struct::isInitialized() const {
  if ((C018_easySerial != nullptr) && (myLora != nullptr)) {
    if (autobaud_success) {
      return true;
    }
  }
  return false;
}

bool C018_data_struct::hasJoined() const {
  if (!isInitialized()) { return false; }
  return myLora->hasJoined();
}

bool C018_data_struct::useOTAA() const {
  if (!isInitialized()) { return true; }
  bool res = myLora->useOTAA();

  C018_logError(F("useOTA()"));
  return res;
}

bool C018_data_struct::command_finished() const {
  return myLora->command_finished();
}

bool C018_data_struct::txUncnfBytes(const uint8_t *data, uint8_t size, uint8_t port) {
  bool res = myLora->txBytes(data, size, port) != RN2xx3_datatypes::TX_return_type::TX_FAIL;

  C018_logError(F("txUncnfBytes()"));
  return res;
}

bool C018_data_struct::txHexBytes(const String& data, uint8_t port) {
  bool res = myLora->txHexBytes(data, port) != RN2xx3_datatypes::TX_return_type::TX_FAIL;

  C018_logError(F("txHexBytes()"));
  return res;
}

bool C018_data_struct::txUncnf(const String& data, uint8_t port) {
  bool res = myLora->tx(data, port) != RN2xx3_datatypes::TX_return_type::TX_FAIL;

  C018_logError(F("txUncnf()"));
  return res;
}

bool C018_data_struct::setTTNstack(RN2xx3_datatypes::TTN_stack_version version) {
  if (!isInitialized()) { return false; }
  bool res = myLora->setTTNstack(version);

  C018_logError(F("setTTNstack"));
  return res;
}

bool C018_data_struct::setFrequencyPlan(RN2xx3_datatypes::Freq_plan plan, uint32_t rx2_freq) {
  if (!isInitialized()) { return false; }
  bool res = myLora->setFrequencyPlan(plan, rx2_freq);

  C018_logError(F("setFrequencyPlan()"));
  return res;
}

bool C018_data_struct::setSF(uint8_t sf) {
  if (!isInitialized()) { return false; }
  bool res = myLora->setSF(sf);

  C018_logError(F("setSF()"));
  return res;
}

bool C018_data_struct::setAdaptiveDataRate(bool enabled) {
  if (!isInitialized()) { return false; }
  bool res = myLora->setAdaptiveDataRate(enabled);

  C018_logError(F("setAdaptiveDataRate()"));
  return res;
}

bool C018_data_struct::initOTAA(const String& AppEUI, const String& AppKey, const String& DevEUI) {
  if (myLora == nullptr) { return false; }
  bool success = myLora->initOTAA(AppEUI, AppKey, DevEUI);

  cacheDevAddr = String();

  C018_logError(F("initOTAA()"));
  updateCacheOnInit();
  return success;
}

bool C018_data_struct::initABP(const String& addr, const String& AppSKey, const String& NwkSKey) {
  if (myLora == nullptr) { return false; }
  bool success = myLora->initABP(addr, AppSKey, NwkSKey);

  cacheDevAddr = addr;

  C018_logError(F("initABP()"));
  updateCacheOnInit();
  return success;
}

String C018_data_struct::sendRawCommand(const String& command) {
  if (!isInitialized()) { return EMPTY_STRING; }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("sendRawCommand: ");
    log += command;
    addLogMove(LOG_LEVEL_INFO, log);
  }
  String res = myLora->sendRawCommand(command);

  C018_logError(F("sendRawCommand()"));
  return res;
}

int C018_data_struct::getVbat() {
  if (!isInitialized()) { return -1; }
  return myLora->getVbat();
}

String C018_data_struct::peekLastError() {
  if (!isInitialized()) { return EMPTY_STRING; }
  return myLora->peekLastError();
}

String C018_data_struct::getLastError() {
  if (!isInitialized()) { return EMPTY_STRING; }
  return myLora->getLastError();
}

String C018_data_struct::getDataRate() {
  if (!isInitialized()) { return EMPTY_STRING; }
  String res = myLora->getDataRate();

  C018_logError(F("getDataRate()"));
  return res;
}

int C018_data_struct::getRSSI() {
  if (!isInitialized()) { return 0; }
  return myLora->getRSSI();
}

uint32_t C018_data_struct::getRawStatus() {
  if (!isInitialized()) { return 0; }
  return myLora->getStatus().getRawStatus();
}

RN2xx3_status C018_data_struct::getStatus() const {
  if (!isInitialized()) { return RN2xx3_status(); }
  return myLora->getStatus();
}

bool C018_data_struct::getFrameCounters(uint32_t& dnctr, uint32_t& upctr) {
  if (!isInitialized()) { return false; }
  bool res = myLora->getFrameCounters(dnctr, upctr);

  C018_logError(F("getFrameCounters()"));
  return res;
}

bool C018_data_struct::setFrameCounters(uint32_t dnctr, uint32_t upctr) {
  if (!isInitialized()) { return false; }
  bool res = myLora->setFrameCounters(dnctr, upctr);

  C018_logError(F("setFrameCounters()"));
  return res;
}

// Cached data, only changing occasionally.

String C018_data_struct::getDevaddr() {
  if (cacheDevAddr.isEmpty())
  {
    updateCacheOnInit();
  }
  return cacheDevAddr;
}

String C018_data_struct::hweui() {
  if (cacheHWEUI.isEmpty()) {
    if (isInitialized()) {
      cacheHWEUI = myLora->hweui();
    }
  }
  return cacheHWEUI;
}

String C018_data_struct::sysver() {
  if (cacheSysVer.isEmpty()) {
    if (isInitialized()) {
      cacheSysVer = myLora->sysver();
    }
  }
  return cacheSysVer;
}

uint8_t C018_data_struct::getSampleSetCount() const {
  return sampleSetCounter;
}

uint8_t C018_data_struct::getSampleSetCount(taskIndex_t taskIndex) {
  if (sampleSetInitiator == taskIndex)
  {
    ++sampleSetCounter;
  }
  return sampleSetCounter;
}

float C018_data_struct::getLoRaAirTime(uint8_t pl) const {
  if (isInitialized()) {
    return myLora->getLoRaAirTime(pl + 13); // We have a LoRaWAN header of 13 bytes.
  }
  return -1.0;
}

void C018_data_struct::async_loop() {
  if (isInitialized()) {
    rn2xx3_handler::RN_state state = myLora->async_loop();

    if (rn2xx3_handler::RN_state::must_perform_init == state) {
      if (myLora->get_busy_count() > 10) {
        if (validGpio(_resetPin)) {
          pinMode(_resetPin, OUTPUT);
          digitalWrite(_resetPin, LOW);
          delay(50);
          digitalWrite(_resetPin, HIGH);
          delay(200);
        }
        autobaud_success = false;

        //          triggerAutobaud();
      }
    }
  }
}

void C018_data_struct::C018_logError(const __FlashStringHelper *command) const {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String error = myLora->peekLastError();

    //    String error = myLora->getLastError();

    if (error.length() > 0) {
      String log = F("RN2483: ");
      log += command;
      log += F(": ");
      log += error;
      addLogMove(LOG_LEVEL_INFO, log);
    }
  }
}

void C018_data_struct::updateCacheOnInit() {
  if (isInitialized()) {
    if (cacheDevAddr.isEmpty() && myLora->getStatus().Joined)
    {
      cacheDevAddr = myLora->sendRawCommand(F("mac get devaddr"));

      if (cacheDevAddr == F("00000000")) {
        cacheDevAddr = String();
      }
    }
  }
}

void C018_data_struct::triggerAutobaud() {
  if ((C018_easySerial == nullptr) || (myLora == nullptr)) {
    return;
  }
  int retries = 2;

  while (retries > 0 && !autobaud_success) {
    if (retries == 1) {
      if (validGpio(_resetPin)) {
        pinMode(_resetPin, OUTPUT);
        digitalWrite(_resetPin, LOW);
        delay(50);
        digitalWrite(_resetPin, HIGH);
        delay(200);
      }
    }

    // wakeUP_RN2483 and set data rate
    // Delay must be longer than specified in the datasheet for firmware 1.0.3
    // See: https://www.thethingsnetwork.org/forum/t/rn2483a-problems-no-serial-communication/7866/36?u=td-er

    // First set the baud rate low enough to even trigger autobaud when 9600 baud is active
    C018_easySerial->begin(600);
    C018_easySerial->write(static_cast<uint8_t>(0x00));

    // Set to desired baud rate.
    C018_easySerial->begin(_baudrate);
    C018_easySerial->write(static_cast<uint8_t>(0x55));
    C018_easySerial->println();
    delay(100);

    String response = myLora->sysver();

    // we could use sendRawCommand(F("sys get ver")); here
    //      C018_easySerial->println(F("sys get ver"));
    //      String response = C018_easySerial->readStringUntil('\n');
    autobaud_success = response.length() > 10;

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("C018 AutoBaud: ");
      log += response;
      log += F(" status: ");
      log += myLora->sendRawCommand(F("mac get status"));
      addLogMove(LOG_LEVEL_INFO, log);
      C018_logError(F("autobaud check"));
    }
    --retries;
  }
}

  #endif // ifdef USES_C018
