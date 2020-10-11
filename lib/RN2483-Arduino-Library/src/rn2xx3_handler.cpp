#include "rn2xx3_handler.h"

#include "rn2xx3_helper.h"
#include "rn2xx3_received_types.h"


rn2xx3_handler::rn2xx3_handler(Stream& serial) : _serial(serial)
{
  clearSerialBuffer();
}

String rn2xx3_handler::sendRawCommand(const String& command)
{
  unsigned long timer = millis();

  if (!prepare_raw_command(command)) {
    setLastError(F("sendRawCommand: Prepare fail"));
    return "";
  }

  if (wait_command_finished() == RN_state::timeout) {
    String log = F("sendRawCommand timeout: ");
    log += command;
    setLastError(log);
  }
  String ret = get_received_data();

  if (_extensive_debug) {
    String log = command;
    log += '(';
    log += String(millis() - timer);
    log += ')';
    log += F(" max length: ");
    log += _max_received_length;
    setLastError(log);    
  }

  ret.trim();
  return ret;
}

bool rn2xx3_handler::prepare_raw_command(const String& command)
{
  if (!command_finished()) {
    // Handling of another command has not finished.
    return false;
  }
  _sendData       = command;
  _processing_cmd = Active_cmd::other;
  _busy_count     = 0;
  _retry_count    = 0;
  set_state(RN_state::command_set_to_send);

  // Set state may set command_finished to true if no _sendData is set.
  return !command_finished();
}

bool rn2xx3_handler::prepare_tx_command(const String& command, const String& data, bool shouldEncode, uint8_t port) {
  int estimatedSize = command.length() + 4; // port + space

  estimatedSize += shouldEncode ? 2 * data.length() : data.length();
  String tmpCommand;
  tmpCommand.reserve(estimatedSize);
  tmpCommand = command;

  if (command.endsWith(F("cnf "))) {
    // No port was given in the command, so add the port.
    tmpCommand += String(port);
    tmpCommand += ' ';
  }

  if (shouldEncode)
  {
    tmpCommand += rn2xx3_helper::base16encode(data);
  }
  else
  {
    tmpCommand += data;
  }

  if (!prepare_raw_command(tmpCommand)) {
    return false;
  }
  _processing_cmd = Active_cmd::TX;
  return true;
}

bool rn2xx3_handler::prepare_join(bool useOTAA) {
  updateStatus();

  if (!prepare_raw_command(useOTAA ? F("mac join otaa") : F("mac join abp"))) {
    return false;
  }
  _processing_cmd = Active_cmd::join;
  Status.Joined   = false;
  return true;
}

rn2xx3_handler::RN_state rn2xx3_handler::async_loop()
{
  if (_state != RN_state::must_pause) {
    if (!command_finished() && time_out_reached()) {
      set_state(RN_state::timeout);
    }
  }


  switch (get_state()) {
    case RN_state::idle:

      // Noting to do.
      break;
    case RN_state::command_set_to_send:
    {
      ++_retry_count;

      // retransmit/retry a maximum of 10 times
      // N.B. this also applies when no_free_ch was received.
      if (_retry_count > 10) {
        set_state(RN_state::max_attempt_reached);
      } else {
        _receivedData = "";
        clearSerialBuffer();

        // Write the commmand
        _serial.print(get_send_data());
        _serial.println();

        set_state(RN_state::wait_for_reply);
      }
      break;
    }
    case RN_state::must_pause:
    {
      // Do not call writes for a while.
      if (time_out_reached()) {
        set_state(RN_state::command_set_to_send);
      }
      break;
    }
    case RN_state::wait_for_reply:
    case RN_state::wait_for_reply_rx2:
    {
      if (read_line()) {
        switch (_state) {
          case RN_state::wait_for_reply:
            set_state(RN_state::reply_received);
            break;
          case RN_state::wait_for_reply_rx2:
            set_state(RN_state::reply_received_rx2);
            break;
          default:

            // Only process data when in the wait for reply state
            break;
        }
      }

      if (_invalid_char_read) {
        set_state(RN_state::invalid_char_read);
      }
      break;
    }
    case RN_state::reply_received:
    case RN_state::reply_received_rx2:
    {
      handle_reply_received();
      break;
    }
    case RN_state::must_perform_init:
      break;

    case RN_state::timeout:
      sendWakeSequence();
      break;

    case RN_state::max_attempt_reached:
    case RN_state::error:
    case RN_state::duty_cycle_exceeded:
    case RN_state::invalid_char_read:

      break;

    case RN_state::tx_success:
    case RN_state::tx_success_with_rx:
    case RN_state::reply_received_finished:
    case RN_state::join_accepted:
      break;

      // Do not use default: here, so the compiler warns when a new state is not yet implemented here.
      //    default:
      //      break;
  }
  return get_state();
}

rn2xx3_handler::RN_state rn2xx3_handler::wait_command_finished(unsigned long timeout)
{
  // Still use a timeout to prevent endless loops, although the state machine should always obey the set timeouts.
  unsigned long start_timer = millis();

  while ((millis() - start_timer) < timeout) {
    async_loop();

    if (command_finished()) { return get_state(); }
    delay(10);
  }
  return get_state();
}

rn2xx3_handler::RN_state rn2xx3_handler::wait_command_accepted(unsigned long timeout)
{
  // Still use a timeout to prevent endless loops, although the state machine should always obey the set timeouts.
  unsigned long start_timer = millis();

  while ((millis() - start_timer) < timeout) {
    async_loop();

    if (command_finished() || (get_state() == RN_state::wait_for_reply_rx2)) {
      return get_state();
    }
    delay(10);
  }
  return get_state();
}

bool rn2xx3_handler::command_finished() const
{
  return _processing_cmd == Active_cmd::none;
}

bool rn2xx3_handler::init()
{
  if (!check_set_keys())
  {
    // FIXME TD-er: Do we need to set the state here to idle ???
    // or maybe introduce a new "not_started" ???
    setLastError(F("Not all keys are set"));
    return false;
  }

  bool mustInit =
    get_state() == RN_state::must_perform_init ||
    !Status.Joined;

  if (!mustInit) {
    // What should be returned? The joined state or whether there has been a join performed?
    return false;
  }

  if (!resetModule()) { return false; }

  // We set both sets of keys, as some reports on older firmware suggest the save
  // may not be successful after a factory reset if not all fields are set.

  // Set OTAA keys
  sendMacSet(F("deveui"),  _deveui);
  sendMacSet(F("appeui"),  _appeui);
  sendMacSet(F("appkey"),  _appkey);

  // Set ABP keys
  sendMacSet(F("nwkskey"), _nwkskey);
  sendMacSet(F("appskey"), _appskey);
  sendMacSet(F("devaddr"), _devaddr);

  // Set max. allowed power.
  // 868 MHz EU   : 1 -> 14 dBm
  // 900 MHz US/AU: 5 -> 20 dBm
  setTXoutputPower(_moduleType == RN2xx3_datatypes::Model::RN2903 ? 5 : 1);
  setSF(_sf);

  // TTN does not yet support Adaptive Data Rate.
  // Using it is also only necessary in limited situations.
  // Therefore disable it by default.
  setAdaptiveDataRate(false);

  // Switch off automatic replies, because this library can not
  // handle more than one mac_rx per tx. See RN2483 datasheet,
  // 2.4.8.14, page 27 and the scenario on page 19.
  setAutomaticReply(false);

  // Semtech and TTN both use a non default RX2 window freq and SF.
  // Maybe we should not specify this for other networks.
  // if (_moduleType == RN2xx3_datatypes::Model::RN2483)
  // {
  //   set2ndRecvWindow(3, 869525000);
  // }
  // Disabled for now because an OTAA join seems to work fine without.

  if (_asyncMode) {
    return prepare_join(_otaa);
  }
  return wait_command_accepted() ==  RN_state::join_accepted;
}

bool rn2xx3_handler::initOTAA(const String& AppEUI, const String& AppKey, const String& DevEUI)
{
  // If the Device EUI was given as a parameter, use it
  // otherwise use the Hardware EUI.
  if (rn2xx3_helper::isHexStr_of_length(DevEUI, 16))
  {
    _deveui = DevEUI;
  }
  else
  {
    String addr = sendRawCommand(F("sys get hweui"));

    if (rn2xx3_helper::isHexStr_of_length(addr, 16))
    {
      _deveui = addr;
    }
  }

  if (!rn2xx3_helper::isHexStr_of_length(AppEUI, 16) ||
      !rn2xx3_helper::isHexStr_of_length(AppKey, 32) ||
      !rn2xx3_helper::isHexStr_of_length(_deveui, 16))
  {
    // No valid config
    setLastError(F("InitOTAA: Not all keys are valid."));
    return false;
  }
  _appeui = AppEUI;
  _appkey = AppKey;
  _otaa   = true;
  return init();
}

bool rn2xx3_handler::initOTAA(uint8_t *AppEUI, uint8_t *AppKey, uint8_t *DevEUI)
{
  if ((AppEUI == nullptr) || (AppKey == nullptr)) {
    return false;
  }

  String app_eui;
  String dev_eui;
  String app_key;
  char   buff[3];

  for (uint8_t i = 0; i < 8; i++)
  {
    sprintf(buff, "%02X", AppEUI[i]);
    app_eui += String(buff);
  }

  if (DevEUI == nullptr)
  {
    dev_eui = "0";
  } else {
    for (uint8_t i = 0; i < 8; i++)
    {
      sprintf(buff, "%02X", DevEUI[i]);
      dev_eui += String(buff);
    }
  }

  for (uint8_t i = 0; i < 16; i++)
  {
    sprintf(buff, "%02X", AppKey[i]);
    app_key += String(buff);
  }

  return initOTAA(app_eui, app_key, dev_eui);
}

bool rn2xx3_handler::initABP(const String& devAddr, const String& AppSKey, const String& NwkSKey)
{
  _devaddr = devAddr;
  _appskey = AppSKey;
  _nwkskey = NwkSKey;
  _otaa    = false;
  return init();
}

RN2xx3_datatypes::TX_return_type rn2xx3_handler::txCommand(const String& command, const String& data, bool shouldEncode, uint8_t port)
{
  if (get_state() == RN_state::must_perform_init) {
    init();
  }

  if (!prepare_tx_command(command, data, shouldEncode, port)) {
    return RN2xx3_datatypes::TX_return_type::TX_FAIL;
  }

  if (_asyncMode) {
    // Unlikely the state will be other than an error or wait_for_reply_rx2
    switch (wait_command_accepted()) {
      case RN_state::wait_for_reply_rx2:
      case RN_state::tx_success:
      case RN_state::tx_success_with_rx:
        return RN2xx3_datatypes::TX_return_type::TX_SUCCESS;
        break;

      default:
        break;
    }
  } else {
    switch (wait_command_finished()) {
      case RN_state::tx_success:
        return RN2xx3_datatypes::TX_return_type::TX_SUCCESS;
      case RN_state::tx_success_with_rx:
        return RN2xx3_datatypes::TX_return_type::TX_WITH_RX;
        break;

      default:
        break;
    }
  }

  return RN2xx3_datatypes::TX_return_type::TX_FAIL;
}

bool rn2xx3_handler::setSF(uint8_t sf)
{
  if ((sf >= 7) && (sf <= 12))
  {
    int dr = -1;

    switch (_fp)
    {
      case RN2xx3_datatypes::Freq_plan::TTN_EU:
      case RN2xx3_datatypes::Freq_plan::SINGLE_CHANNEL_EU:
      case RN2xx3_datatypes::Freq_plan::DEFAULT_EU:

        //  case TTN_FP_EU868:
        //  case TTN_FP_IN865_867:
        //  case TTN_FP_AS920_923:
        //  case TTN_FP_AS923_925:
        //  case TTN_FP_KR920_923:
        dr = 12 - sf;
        break;
      case RN2xx3_datatypes::Freq_plan::TTN_US:

        // case TTN_FP_US915:
        // case TTN_FP_AU915:
        dr = 10 - sf;
        break;
      default:
        break;
    }

    if (dr >= 0)
    {
      _sf = sf;
      return setDR(dr);
    }
  }
  setLastError(F("error in setSF"));
  return false;
}

bool rn2xx3_handler::setDR(int dr)
{
  if ((dr >= 0) && (dr <= 7))
  {
    return sendMacSet(F("dr"), String(dr));
  }
  return false;
}

void rn2xx3_handler::setAsyncMode(bool enabled) {
  _asyncMode = enabled;
}

bool rn2xx3_handler::getAsyncMode() const {
  return _asyncMode;
}

bool rn2xx3_handler::useOTAA() const {
  return _otaa;
}

void rn2xx3_handler::setLastUsedJoinMode(bool isOTAA) {
  if (_otaa != isOTAA) {
    Status.Joined = false;
    _otaa         = isOTAA;
  }
}

bool rn2xx3_handler::setFrequencyPlan(RN2xx3_datatypes::Freq_plan fp)
{
  bool returnValue = false;

  switch (fp)
  {
    case RN2xx3_datatypes::Freq_plan::SINGLE_CHANNEL_EU:
    {
      if (_moduleType == RN2xx3_datatypes::Model::RN2483)
      {
        // mac set rx2 <dataRate> <frequency>
        // set2ndRecvWindow(5, 868100000); //use this for "strict" one channel gateways
        set2ndRecvWindow(3, 869525000); // use for "non-strict" one channel gateways
        setChannelDutyCycle(0, 99);     // 1% duty cycle for this channel
        setChannelDutyCycle(1, 65535);  // almost never use this channel
        setChannelDutyCycle(2, 65535);  // almost never use this channel

        for (uint8_t ch = 3; ch < 8; ch++)
        {
          setChannelEnabled(ch, false);
        }
        returnValue = true;
      }
      break;
    }

    case RN2xx3_datatypes::Freq_plan::TTN_EU:
    {
      if (_moduleType == RN2xx3_datatypes::Model::RN2483)
      {
        /*
         * The <dutyCycle> value that needs to be configured can be
         * obtained from the actual duty cycle X (in percentage)
         * using the following formula: <dutyCycle> = (100/X) – 1
         *
         *  10% -> 9
         *  1% -> 99
         *  0.33% -> 299
         *  8 channels, total of 1% duty cycle:
         *  0.125% per channel -> 799
         *
         * Most of the RN2xx3_datatypes::Freq_plan::TTN_EU frequency plan was copied from:
         * https://github.com/TheThingsNetwork/arduino-device-lib
         */

        uint32_t freq = 867100000;

        for (uint8_t ch = 0; ch < 8; ch++)
        {
          setChannelDutyCycle(ch, 799); // All channels

          if (ch == 1)
          {
            setChannelDataRateRange(ch, 0, 6);
          }
          else if (ch > 2)
          {
            setChannelDataRateRange(ch, 0, 5);
            setChannelFrequency(ch, freq);
            freq = freq + 200000;
          }
          setChannelEnabled(ch, true); // frequency, data rate and duty cycle must be set first.
        }

        // RX window 2
        set2ndRecvWindow(3, 869525000);

        returnValue = true;
      }

      break;
    }

    case RN2xx3_datatypes::Freq_plan::TTN_US:
    {
      /*
       * Most of the RN2xx3_datatypes::Freq_plan::TTN_US frequency plan was copied from:
       * https://github.com/TheThingsNetwork/arduino-device-lib
       */
      if (_moduleType == RN2xx3_datatypes::Model::RN2903)
      {
        for (int channel = 0; channel < 72; channel++)
        {
          bool enabled = (channel >= 8 && channel < 16);
          setChannelEnabled(channel, enabled);
        }
        returnValue = true;
      }
      break;
    }

    case RN2xx3_datatypes::Freq_plan::DEFAULT_EU:
    {
      if (_moduleType == RN2xx3_datatypes::Model::RN2483)
      {
        for (int channel = 0; channel < 8; channel++)
        {
          if (channel < 3) {
            // fix duty cycle - 1% = 0.33% per channel
            setChannelDutyCycle(channel, 799);
            setChannelEnabled(channel, true);
          } else {
            // disable non-default channels
            setChannelEnabled(channel, false);
          }
        }
        returnValue = true;
      }

      break;
    }
    default:
    {
      // set default channels 868.1, 868.3 and 868.5?
      returnValue = false; // well we didn't do anything, so yes, false
      break;
    }
  }

  return returnValue;
}

RN2xx3_datatypes::Model rn2xx3_handler::configureModuleType()
{
  RN2xx3_datatypes::Firmware firmware;

  _moduleType = RN2xx3_datatypes::parseVersion(sysver(), firmware);
  return _moduleType;
}

bool rn2xx3_handler::resetModule()
{
  // reset the module - this will clear all keys set previously
  String result;

  switch (configureModuleType())
  {
    case RN2xx3_datatypes::Model::RN2903:
      result = sendRawCommand(F("mac reset"));
      break;
    case RN2xx3_datatypes::Model::RN2483:
      result = sendRawCommand(F("mac reset 868"));
      break;
    default:

      // we shouldn't go forward with the init
      setLastError(F("error in reset"));
      return false;
  }

  // setLastError(F("success resetmodule"));
  return true;

  //  return RN2xx3_received_types::determineReceivedDataType(result) == ok;
}

const String& rn2xx3_handler::get_send_data() const {
  return _sendData;
}

const String& rn2xx3_handler::get_received_data() const {
  return _receivedData;
}

const String& rn2xx3_handler::get_received_data(unsigned long& duration) const {
  duration = millis() - _start_prep;
  return _receivedData;
}

const String& rn2xx3_handler::get_rx_message() const {
  return _rxMessenge;
}

String rn2xx3_handler::peekLastError() const
{
  return _lastError;
}

String rn2xx3_handler::getLastError()
{
  String res = _lastError;

  _lastError = "";
  return res;
}

void rn2xx3_handler::setLastError(const String& error)
{
  if (_extensive_debug) {
    _lastError += '\n';
    _lastError += String(millis());
    _lastError += F(" : ");
    _lastError += error;
  } else {
    _lastError = error;
  }
}

rn2xx3_handler::RN_state rn2xx3_handler::get_state() const {
  return _state;
}

uint8_t rn2xx3_handler::get_busy_count() const {
  return _busy_count;
}

String rn2xx3_handler::sysver() {
  String ver = sendRawCommand(F("sys get ver"));

  ver.trim();
  return ver;
}

bool rn2xx3_handler::getRxDelayValues(uint32_t& rxdelay1,
                                      uint32_t& rxdelay2)
{
  rxdelay1 = _rxdelay1;
  rxdelay2 = _rxdelay2;
  return _rxdelay1 != 0 && _rxdelay2 != 0;
}

void rn2xx3_handler::set_state(rn2xx3_handler::RN_state state) {
  const bool was_processing_cmd = _processing_cmd != Active_cmd::none;

  _state = state;

  switch (state) {
    case RN_state::wait_for_reply:
    case RN_state::wait_for_reply_rx2:
    {
      // We will wait for data, so make sure the receiving buffer is empty.
      _receivedData = "";

      if (state == RN_state::wait_for_reply_rx2)
      {
        // Enough time to wait for:
        // Transmit Time On Air + receive_delay2 + receiving RX2 packet.
        switch (_processing_cmd) {
          case Active_cmd::join:
            set_timeout(10000);            // Do take a bit more time for a join.
            break;
          case Active_cmd::TX:
            set_timeout(_rxdelay2 + 3000); // 55 bytes @EU868 data rate of SF12/125kHz = 2,957.31 milliseconds
            break;
          default:

            // Other commands do not use RX2
            break;
        }
      }
      break;
    }
    case RN_state::reply_received:
    case RN_state::reply_received_rx2:

      // Nothing to set here, as we will now inspect the received data and not communicate with the module.
      break;
    case RN_state::command_set_to_send:

      if (_sendData.length() == 0) {
        set_state(RN_state::idle);
      } else {
        _start_prep = millis();

        set_timeout(1500); // Roughly 1100 msec needed for mac save
                           // Almost all other commands reply in 20 - 100 msec.
      }

      break;
    case RN_state::must_pause:
      set_timeout(1000);
      break;

    case RN_state::invalid_char_read:

      if (_processing_cmd == Active_cmd::other) {
        // Must retry to run the command again.
        set_state(RN_state::command_set_to_send);
      } else {
        _processing_cmd = Active_cmd::none;
      }
      break;

    case RN_state::idle:

      // ToDo: Add support for sleep mode.
      // Clear the strings to free up some memory.
      _processing_cmd = Active_cmd::none;
      _sendData       = "";
      _receivedData   = "";
      _rxMessenge     = "";
      _lastError      = "";
      break;
    case RN_state::timeout:
    case RN_state::max_attempt_reached:
    case RN_state::error:
    case RN_state::must_perform_init:
    case RN_state::duty_cycle_exceeded:

      // We cannot continue from this error
      _processing_cmd = Active_cmd::none;
      break;
    case RN_state::tx_success:
    case RN_state::tx_success_with_rx:
    case RN_state::reply_received_finished:
      _processing_cmd = Active_cmd::none;
      break;

    case RN_state::join_accepted:
      Status.Joined = true;
      saveUpdatedStatus();
      _processing_cmd = Active_cmd::none;
      break;

      // Do not use default: here, so the compiler warns when a new state is not yet implemented here.
      //    default:
      //      break;
  }

  if (was_processing_cmd && (_processing_cmd == Active_cmd::none)) {
    _start             = 0;
    _invalid_char_read = false;
    _busy_count        = 0;
    _retry_count       = 0;
  }
}

bool rn2xx3_handler::read_line()
{
  int available = _serial.available();
  while (available > 0) {
    int c = _serial.read();
    --available;

    if (c >= 0) {
      const char character = static_cast<char>(c & 0xFF);

      if (!rn2xx3_helper::valid_char(character)) {
        _invalid_char_read = true;
        return false;
      }

      _receivedData += character;

      if (character == '\n') {
        if (_receivedData.length() > _max_received_length) {
          _max_received_length = _receivedData.length();
        }
        return true;
      }
      if (available == 0) {
        available = _serial.available();
      }
    }
  }
  return false;
}

void rn2xx3_handler::set_timeout(unsigned long timeout)
{
  _timeout = timeout;
  _start   = millis();
}

bool rn2xx3_handler::time_out_reached() const
{
  return (millis() - _start) >= _timeout;
}

void rn2xx3_handler::clearSerialBuffer()
{
  while (_serial.available()) {
    _serial.read();
  }
}

bool rn2xx3_handler::updateStatus()
{
  if (!Status.modelVersionSet()) {
    Status.setModelVersion(sysver());
  }

  const String status_str = sendRawCommand(F("mac get status"));

  // pre 1.0.1 firmware revisions only used 16 bits.
  // Newer firmware revisions use 32 bits.
  if (!(rn2xx3_helper::isHexStr_of_length(status_str, 4) ||
        rn2xx3_helper::isHexStr_of_length(status_str, 8))) {
    String error = F("mac get status  : No valid hex string \"");
    error += status_str;
    error += '\"';
    setLastError(error);
    return false;
  }
  uint32_t status_value = strtoul(status_str.c_str(), 0, 16);
  Status.decode(status_value);

  if ((_rxdelay1 == 0) || (_rxdelay2 == 0) || Status.SecondReceiveWindowParamUpdated)
  {
    readUIntMacGet(F("rxdelay1"), _rxdelay1);
    readUIntMacGet(F("rxdelay2"), _rxdelay2);
    Status.SecondReceiveWindowParamUpdated = false;
  }
  return true;
}

bool rn2xx3_handler::saveUpdatedStatus()
{
  // Only save to the eeprom when really needed.
  // No need to store the current config when there is no active connection.
  // Todo: Must keep track of last saved counters and decide to update when current counter differs more than set threshold.
  bool saved = false;

  if (updateStatus())
  {
    if (Status.Joined && !Status.RejoinNeeded && Status.saveSettingsNeeded())
    {
      saved = RN2xx3_received_types::determineReceivedDataType(sendRawCommand(F("mac save"))) == RN2xx3_received_types::ok;
      Status.clearSaveSettingsNeeded();
      updateStatus();
    }
  }
  return saved;
}

void rn2xx3_handler::handle_reply_received() {
  const RN2xx3_received_types::received_t received_datatype = RN2xx3_received_types::determineReceivedDataType(_receivedData);

  // Check if the reply is unexpected, so log the command + reply
  bool mustLogAsError = _extensive_debug;

  switch (received_datatype) {
    case RN2xx3_received_types::ok:
    case RN2xx3_received_types::UNKNOWN: // Many get-commands just return a value, so that will be of type UNKNOWN
    case RN2xx3_received_types::accepted:
    case RN2xx3_received_types::mac_tx_ok:
    case RN2xx3_received_types::mac_rx:
    case RN2xx3_received_types::radio_rx:
    case RN2xx3_received_types::radio_tx_ok:
      break;

    default:
      mustLogAsError = true;
      break;
  }

  if (mustLogAsError) {
    String error;
    error.reserve(_sendData.length() + _receivedData.length() + 4);

    if (_processing_cmd == Active_cmd::TX) {
      // TX commands are a lot longer, so do not include complete command
      error += F("mac tx");
    } else {
      error += _sendData;
    }
    error += F(" -> ");
    error += _receivedData;
    setLastError(error);
  }

  switch (received_datatype) {
    case RN2xx3_received_types::UNKNOWN:

      // A reply which is not part of standard replies, so it can be a requested value.
      // Command is now finished.
      set_state(RN_state::reply_received_finished);
      break;
    case RN2xx3_received_types::ok:
    {
      const bool expect_rx2 =
        (_processing_cmd == Active_cmd::TX) ||
        (_processing_cmd == Active_cmd::join);

      if ((get_state() == RN_state::reply_received) && expect_rx2) {
        // "mac tx" and "join otaa" commands may receive a second response if the first one was "ok"
        set_state(RN_state::wait_for_reply_rx2);
      } else {
        set_state(RN_state::reply_received_finished);
      }
      break;
    }

    case RN2xx3_received_types::invalid_param:
    {
      // parameters (<type> <portno> <data>) are not valid
      // should not happen if we typed the commands correctly
      set_state(RN_state::error);
      break;
    }

    case RN2xx3_received_types::not_joined:
    {
      // the network is not joined
      Status.Joined = false;
      set_state(RN_state::must_perform_init);
      break;
    }

    case RN2xx3_received_types::no_free_ch:
    {
      // all channels are busy
      // probably duty cycle limits exceeded.
      // User must retry.
      set_state(RN_state::duty_cycle_exceeded);
      break;
    }

    case RN2xx3_received_types::silent:
    {
      // the module is in a Silent Immediately state
      // This is enforced by the network.
      // To enable:
      // sendRawCommand(F("mac forceENABLE"));
      // N.B. One has to think about why this has happened.
      set_state(RN_state::must_perform_init);
      break;
    }

    case RN2xx3_received_types::frame_counter_err_rejoin_needed:
    {
      // the frame counter rolled over
      set_state(RN_state::must_perform_init);
      break;
    }

    case RN2xx3_received_types::busy:
    {
      // MAC state is not in an Idle state
      _busy_count++;

      // Not sure if this is wise. At low data rates with large packets
      // this can perhaps cause transmissions at more than 1% duty cycle.
      // Need to calculate the correct constant value.
      // But it is wise to have this check and re-init in case the
      // lorawan stack in the RN2xx3 hangs.
      if (_busy_count >= 10)
      {
        set_state(RN_state::must_perform_init);
      }
      else
      {
        delay(100);
      }
      break;
    }

    case RN2xx3_received_types::mac_paused:
    {
      // MAC was paused and not resumed back
      set_state(RN_state::must_perform_init);
      break;
    }

    case RN2xx3_received_types::invalid_data_len:
    {
      if (_state == RN_state::reply_received)
      {
        // application payload length is greater than the maximum application payload length corresponding to the current data rate
      }
      else
      {
        // application payload length is greater than the maximum application payload length corresponding to the current data rate.
        // This can occur after an earlier uplink attempt if retransmission back-off has reduced the data rate.
      }
      set_state(RN_state::error);
      break;
    }

    case RN2xx3_received_types::mac_tx_ok:
    {
      // if uplink transmission was successful and no downlink data was received back from the server
      // SUCCESS!!
      set_state(RN_state::tx_success);
      break;
    }

    case RN2xx3_received_types::mac_rx:
    {
      // mac_rx <portno> <data>
      // transmission was successful
      // <portno>: port number, from 1 to 223
      // <data>: hexadecimal value that was received from theserver
      // example: mac_rx 1 54657374696E6720313233
      _rxMessenge = _receivedData.substring(_receivedData.indexOf(' ', 7) + 1);
      set_state(RN_state::tx_success_with_rx);
      break;
    }

    case RN2xx3_received_types::mac_err:
    {
      set_state(RN_state::must_perform_init);
      break;
    }

    case RN2xx3_received_types::radio_err:
    {
      // transmission was unsuccessful, ACK not received back from the server
      // This should never happen. If it does, something major is wrong.
      set_state(RN_state::must_perform_init);
      break;
    }

    case RN2xx3_received_types::accepted:
      set_state(RN_state::join_accepted);
      break;


    case RN2xx3_received_types::denied:
    case RN2xx3_received_types::keys_not_init:
      set_state(RN_state::error);
      break;

    case RN2xx3_received_types::radio_rx:
    case RN2xx3_received_types::radio_tx_ok:

      // FIXME TD-er: Not sure what to do here.
      break;


      /*
         default:
         {
         // unknown response after mac tx command
         set_state(RN_state::must_perform_init);
         break;
         }
       */
  }
}

int rn2xx3_handler::readIntValue(const String& command)
{
  String value = sendRawCommand(command);

  value.trim();
  return value.toInt();
}

bool rn2xx3_handler::readUIntMacGet(const String& param, uint32_t& value)
{
  String command;

  command.reserve(8 + param.length());
  command  = F("mac get ");
  command += param;
  String value_str = sendRawCommand(command);

  if (value_str.length() == 0)
  {
    return false;
  }
  value = strtoul(value_str.c_str(), 0, 10);
  return true;
}

bool rn2xx3_handler::sendMacSet(const String& param, const String& value)
{
  String command;

  command.reserve(10 + param.length() + value.length());
  command  = F("mac set ");
  command += param;
  command += ' ';
  command += value;

  if (_extensive_debug) {
    setLastError(command);
  }

  return RN2xx3_received_types::determineReceivedDataType(sendRawCommand(command)) == RN2xx3_received_types::ok;
}

bool rn2xx3_handler::sendMacSetEnabled(const String& param, bool enabled)
{
  return sendMacSet(param, enabled ? F("on") : F("off"));
}

bool rn2xx3_handler::sendMacSetCh(const String& param, unsigned int channel, const String& value)
{
  String command;

  command.reserve(20);
  command  = param;
  command += ' ';
  command += channel;
  command += ' ';
  command += value;
  return sendMacSet(F("ch"), command);
}

bool rn2xx3_handler::sendMacSetCh(const String& param, unsigned int channel, uint32_t value)
{
  return sendMacSetCh(param, channel, String(value));
}

bool rn2xx3_handler::setChannelDutyCycle(unsigned int channel, unsigned int dutyCycle)
{
  return sendMacSetCh(F("dcycle"), channel, dutyCycle);
}

bool rn2xx3_handler::setChannelFrequency(unsigned int channel, uint32_t frequency)
{
  return sendMacSetCh(F("freq"), channel, frequency);
}

bool rn2xx3_handler::setChannelDataRateRange(unsigned int channel, unsigned int minRange, unsigned int maxRange)
{
  String value;

  value  = String(minRange);
  value += ' ';
  value += String(maxRange);
  return sendMacSetCh(F("drrange"), channel, value);
}

bool rn2xx3_handler::setChannelEnabled(unsigned int channel, bool enabled)
{
  return sendMacSetCh(F("status"), channel, enabled ? F("on") : F("off"));
}

bool rn2xx3_handler::set2ndRecvWindow(unsigned int dataRate, uint32_t frequency)
{
  String value;

  value  = String(dataRate);
  value += ' ';
  value += String(frequency);
  return sendMacSet(F("rx2"), value);
}

bool rn2xx3_handler::setAdaptiveDataRate(bool enabled)
{
  return sendMacSetEnabled(F("adr"), enabled);
}

bool rn2xx3_handler::setAutomaticReply(bool enabled)
{
  return sendMacSetEnabled(F("ar"), enabled);
}

bool rn2xx3_handler::setTXoutputPower(int pwridx)
{
  // Possible values:

  /*
     433 MHz EU:
     0: 10 dBm
     1:  7 dBm
     2:  4 dBm
     3:  1 dBm
     4: -2 dBm
     5: -5 dBm

     868 MHz EU:
     0: N/A
     1: 14 dBm
     2: 11 dBm
     3: 8 dBm
     4: 5 dBm
     5: 2 dBm

     900 MHz US/AU:
     5 : 20 dBm
     7 : 16 dBm
     8 : 14 dBm
     9 : 12 dBm
     10: 10 dBm
   */
  return sendMacSet(F("pwridx"), String(pwridx));
}

void rn2xx3_handler::sendWakeSequence()
{
  _serial.write(static_cast<uint8_t>(0x00));
  _serial.write(static_cast<uint8_t>(0x55));
  _serial.println();
}

bool rn2xx3_handler::check_set_keys()
{
  // Strings are in HEX, so 1 character per 4 bits.
  // Identifiers:
  // - DevEUI - 64 bit end-device identifier, EUI-64 (unique)
  // - DevAddr - 32 bit device address (non-unique)
  // - AppEUI - 64 bit application identifier, EUI-64 (unique)
  //
  // Security keys: NwkSKey, AppSKey and AppKey.
  // All keys have a length of 128 bits.

  bool otaa_set =
    rn2xx3_helper::isHexStr_of_length(_deveui,  16) &&
    rn2xx3_helper::isHexStr_of_length(_appeui,  16) &&
    rn2xx3_helper::isHexStr_of_length(_appkey,  32);

  bool abp_set =
    rn2xx3_helper::isHexStr_of_length(_nwkskey, 32) &&
    rn2xx3_helper::isHexStr_of_length(_appskey, 32) &&
    rn2xx3_helper::isHexStr_of_length(_devaddr, 8);

  if (!otaa_set && !abp_set) {
    return false;
  }

  if (_otaa && otaa_set) {
    if (!abp_set) {
      if (!rn2xx3_helper::isHexStr_of_length(_nwkskey, 32)) {
        _nwkskey = F("00000000000000000000000000000000");
      }

      if (!rn2xx3_helper::isHexStr_of_length(_appskey, 32)) {
        _appskey = F("00000000000000000000000000000000");
      }

      if (!rn2xx3_helper::isHexStr_of_length(_devaddr, 8))
      {
        // The default address to use on TTN if no address is defined.
        // This one falls in the "testing" address space.
        _devaddr = F("03FFBEEF");
      }
    }
    return true;
  }

  if (!_otaa && abp_set) {
    if (!otaa_set) {
      if (!rn2xx3_helper::isHexStr_of_length(_deveui, 16))
      {
        // if you want to use another DevEUI than the hardware one
        // use this deveui for LoRa WAN
        _deveui = F("0011223344556677");
      }

      if (!rn2xx3_helper::isHexStr_of_length(_appeui, 16)) {
        _appeui = F("0000000000000000");
      }

      if (!rn2xx3_helper::isHexStr_of_length(_appkey, 32)) {
        _appkey = F("00000000000000000000000000000000");
      }
    }
    return true;
  }
  return false;
}
