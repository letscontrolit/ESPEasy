/*
 * A library for controlling a Microchip rn2xx3 LoRa radio.
 *
 * @Author JP Meijers
 * @Author Nicolas Schteinschraber
 * @Date 18/12/2015
 *
 */

#include "Arduino.h"
#include "rn2xx3.h"
#include "rn2xx3_received_types.h"
#include "rn2xx3_helper.h"

extern "C" {
#include <string.h>
#include <stdlib.h>
}

/*
   @param serial Needs to be an already opened Stream ({Software/Hardware}Serial) to write to and read from.
 */
rn2xx3::rn2xx3(Stream& serial) : _rn2xx3_handler(serial)
{}

bool rn2xx3::autobaud()
{
  String response = "";

  // Try a maximum of 10 times with a 1 second delay
  for (uint8_t i = 0; i < 10 && response.length() == 0; i++)
  {
    if (i != 0)
    {
      delay(1000);
    }
    _rn2xx3_handler._serial.write(   (byte)0x00);
    _rn2xx3_handler._serial.write(0x55);
    _rn2xx3_handler._serial.println();

    // we could use sendRawCommand(F("sys get ver")); here
    _rn2xx3_handler._serial.println(F("sys get ver"));
    response = _rn2xx3_handler._serial.readStringUntil('\n');
  }

  // Returned text should be
  // RN2483 X.Y.Z MMM DD YYYY HH:MM:SS
  // Apparently not always the whole stream is read during autobaud.
  return response.length() > 10;
}

String rn2xx3::sysver()
{
  String ver = sendRawCommand(F("sys get ver"));

  ver.trim();
  return ver;
}

RN2xx3_t rn2xx3::configureModuleType()
{
  String version = sysver();
  String model   = version.substring(2, 6);

  switch (model.toInt()) {
    case 2903:
      _moduleType = RN2903;
      break;
    case 2483:
      _moduleType = RN2483;
      break;
    default:
      _moduleType = RN_NA;
      break;
  }
  return _moduleType;
}

bool rn2xx3::resetModule()
{
  // reset the module - this will clear all keys set previously
  String result;

  switch (configureModuleType())
  {
    case RN2903:
      result = sendRawCommand(F("mac reset"));
      break;
    case RN2483:
      result = sendRawCommand(F("mac reset 868"));
      break;
    default:

      // we shouldn't go forward with the init
      _rn2xx3_handler.setLastError(F("error in reset"));
      return false;
  }

  // _rn2xx3_handler.setLastError(F("success resetmodule"));
  return true;

  //  return RN2xx3_received_types::determineReceivedDataType(result) == ok;
}

String rn2xx3::hweui()
{
  return sendRawCommand(F("sys get hweui"));
}

String rn2xx3::appeui()
{
  return sendRawCommand(F("mac get appeui"));
}

String rn2xx3::appkey()
{
  // We can't read back from module, we send the one
  // we have memorized if it has been set
  return _appskey;
}

String rn2xx3::deveui()
{
  return sendRawCommand(F("mac get deveui"));
}

bool rn2xx3::setSF(uint8_t sf)
{
  if ((sf >= 7) && (sf <= 12))
  {
    int dr = -1;

    switch (_fp)
    {
      case TTN_EU:
      case SINGLE_CHANNEL_EU:
      case DEFAULT_EU:

        //  case TTN_FP_EU868:
        //  case TTN_FP_IN865_867:
        //  case TTN_FP_AS920_923:
        //  case TTN_FP_AS923_925:
        //  case TTN_FP_KR920_923:
        dr = 12 - sf;
        break;
      case TTN_US:

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
  _rn2xx3_handler.setLastError(F("error in setSF"));
  return false;
}

bool rn2xx3::init()
{
  if (!check_set_keys())
  {
    // FIXME TD-er: Do we need to set the state here to idle ???
    // or maybe introduce a new "not_started" ???
    _rn2xx3_handler.setLastError(F("Not all keys are set"));
    return false;
  }

  bool mustInit =
    _rn2xx3_handler.get_state() == rn2xx3_handler::RN_state::must_perform_init ||
    !_rn2xx3_handler.Status.Joined;

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
  setTXoutputPower(_moduleType == RN2903 ? 5 : 1);
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
  // if (_moduleType == RN2483)
  // {
  //   set2ndRecvWindow(3, 869525000);
  // }
  // Disabled for now because an OTAA join seems to work fine without.

  return _rn2xx3_handler.prepare_join(_otaa);
}

bool rn2xx3::initOTAA(const String& AppEUI, const String& AppKey, const String& DevEUI)
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
    _rn2xx3_handler.setLastError(F("InitOTAA: Not all keys are valid."));
    return false;
  }
  _appeui = AppEUI;
  _appkey = AppKey;
  _otaa   = true;
  return init();
}

bool rn2xx3::initOTAA(uint8_t *AppEUI, uint8_t *AppKey, uint8_t *DevEUI)
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

bool rn2xx3::initABP(const String& devAddr, const String& AppSKey, const String& NwkSKey)
{
  _devaddr = devAddr;
  _appskey = AppSKey;
  _nwkskey = NwkSKey;
  _otaa    = false;
  return init();
}

TX_RETURN_TYPE rn2xx3::tx(const String& data, uint8_t port, bool async)
{
  return txUncnf(data, port, async); // we are unsure which mode we're in. Better not to wait for acks.
}

TX_RETURN_TYPE rn2xx3::txBytes(const byte *data, uint8_t size, uint8_t port, bool async)
{
  String dataToTx;

  dataToTx.reserve(size * 2);
  char buffer[3];

  for (unsigned i = 0; i < size; i++)
  {
    sprintf(buffer, "%02X", data[i]);
    dataToTx += buffer[0];
    dataToTx += buffer[1];
  }
  return txCommand("mac tx uncnf ", dataToTx, false, port, async);
}

TX_RETURN_TYPE rn2xx3::txHexBytes(const String& hexEncoded, uint8_t port, bool async)
{
  return txCommand("mac tx uncnf ", hexEncoded, false, port, async);
}

TX_RETURN_TYPE rn2xx3::txCnf(const String& data, uint8_t port, bool async)
{
  return txCommand("mac tx cnf ", data, true, port, async);
}

TX_RETURN_TYPE rn2xx3::txUncnf(const String& data, uint8_t port, bool async)
{
  return txCommand("mac tx uncnf ", data, true, port, async);
}

TX_RETURN_TYPE rn2xx3::txCommand(const String& command, const String& data, bool shouldEncode, uint8_t port, bool async)
{
  if (_rn2xx3_handler.get_state() == rn2xx3_handler::RN_state::must_perform_init) {
    init();
  }

  if (!_rn2xx3_handler.prepare_tx_command(command, data, shouldEncode, port)) {
    return TX_FAIL;
  }

  if (async) {
    // Unlikely the state will be other than an error or wait_for_reply_rx2
    switch (wait_command_accepted()) {
      case rn2xx3_handler::RN_state::wait_for_reply_rx2:
      case rn2xx3_handler::RN_state::tx_success:
      case rn2xx3_handler::RN_state::tx_success_with_rx:
        return TX_SUCCESS;
        break;

      default:
        break;
    }
  } else {
    switch (wait_command_finished()) {
      case rn2xx3_handler::RN_state::tx_success:
        return TX_SUCCESS;
      case rn2xx3_handler::RN_state::tx_success_with_rx:
        return TX_WITH_RX;
        break;

      default:
        break;
    }
  }

  return TX_FAIL;
}

// FIXME TD-er: Move this to the rn2xx3_handler class.
rn2xx3_handler::RN_state rn2xx3::async_loop()
{
  rn2xx3_handler::RN_state newState =  _rn2xx3_handler.async_loop();

  if (newState == rn2xx3_handler::RN_state::must_perform_init) {
    this->init();
  }
  return _rn2xx3_handler.get_state();
}

rn2xx3_handler::RN_state rn2xx3::wait_command_finished(unsigned long timeout)
{
  return _rn2xx3_handler.wait_command_finished(timeout);
}

rn2xx3_handler::RN_state rn2xx3::wait_command_accepted(unsigned long timeout)
{
  return _rn2xx3_handler.wait_command_accepted(timeout);
}

String rn2xx3::getRx() {
  return _rn2xx3_handler.get_rx_message();
}

int rn2xx3::getSNR()
{
  return readIntValue(F("radio get snr"));
}

int rn2xx3::getVbat()
{
  return readIntValue(F("sys get vdd"));
}

String rn2xx3::getDataRate()
{
  String output;

  output.reserve(9);
  output  = sendRawCommand(F("radio get sf"));
  output += "bw";
  output += readIntValue(F("radio get bw"));
  return output;
}

int rn2xx3::getRSSI()
{
  return readIntValue(F("radio get rssi"));
}

bool rn2xx3::setDR(int dr)
{
  if ((dr >= 0) && (dr <= 7))
  {
    return sendMacSet(F("dr"), String(dr));
  }
  return false;
}

void rn2xx3::sleep(long msec)
{
  // FIXME TD-er: Must make this a command that waits for other commands to be finished first.
  _rn2xx3_handler._serial.print("sys sleep ");
  _rn2xx3_handler._serial.println(msec);
}

String rn2xx3::sendRawCommand(const String& command)
{
  return _rn2xx3_handler.sendRawCommand(command);
}

RN2xx3_t rn2xx3::moduleType()
{
  return _moduleType;
}

bool rn2xx3::setFrequencyPlan(FREQ_PLAN fp)
{
  bool returnValue;

  switch (fp)
  {
    case SINGLE_CHANNEL_EU:
    {
      if (_moduleType == RN2483)
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
      else
      {
        returnValue = false;
      }
      break;
    }

    case TTN_EU:
    {
      if (_moduleType == RN2483)
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
         * Most of the TTN_EU frequency plan was copied from:
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
      else
      {
        returnValue = false;
      }

      break;
    }

    case TTN_US:
    {
      /*
       * Most of the TTN_US frequency plan was copied from:
       * https://github.com/TheThingsNetwork/arduino-device-lib
       */
      if (_moduleType == RN2903)
      {
        for (int channel = 0; channel < 72; channel++)
        {
          bool enabled = (channel >= 8 && channel < 16);
          setChannelEnabled(channel, enabled);
        }
        returnValue = true;
      }
      else
      {
        returnValue = false;
      }
      break;
    }

    case DEFAULT_EU:
    {
      if (_moduleType == RN2483)
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
      else
      {
        returnValue = false;
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

int rn2xx3::readIntValue(const String& command)
{
  String value = sendRawCommand(command);

  value.trim();
  return value.toInt();
}

bool rn2xx3::readUIntMacGet(const String& param, uint32_t& value)
{
  return _rn2xx3_handler.readUIntMacGet(param, value);
}

String rn2xx3::peekLastError() const
{
  return _rn2xx3_handler.peekLastError();
}

String rn2xx3::getLastError()
{
  return _rn2xx3_handler.getLastError();
}

bool rn2xx3::getFrameCounters(uint32_t& dnctr, uint32_t& upctr)
{
  return
    readUIntMacGet(F("dnctr"), dnctr) &&
    readUIntMacGet(F("upctr"), upctr);
}

bool rn2xx3::setFrameCounters(uint32_t dnctr, uint32_t upctr)
{
  return
    sendMacSet(F("dnctr"), String(dnctr)) &&
    sendMacSet(F("upctr"), String(upctr));
}

bool rn2xx3::getRxDelayValues(uint32_t& rxdelay1,
                              uint32_t& rxdelay2)
{
  return _rn2xx3_handler.getRxDelayValues(rxdelay1, rxdelay2);
}

const RN2xx3_status& rn2xx3::getStatus() const
{
  return _rn2xx3_handler.Status;
}

bool rn2xx3::sendMacSet(const String& param, const String& value)
{
  return _rn2xx3_handler.sendMacSet(param, value);
}

bool rn2xx3::sendMacSetEnabled(const String& param, bool enabled)
{
  return _rn2xx3_handler.sendMacSetEnabled(param, enabled);
}

bool rn2xx3::sendMacSetCh(const String& param, unsigned int channel, const String& value)
{
  return _rn2xx3_handler.sendMacSetCh(param, channel, value);
}

bool rn2xx3::sendMacSetCh(const String& param, unsigned int channel, uint32_t value)
{
  return _rn2xx3_handler.sendMacSetCh(param, channel, value);
}

bool rn2xx3::setChannelDutyCycle(unsigned int channel, unsigned int dutyCycle)
{
  return _rn2xx3_handler.setChannelDutyCycle(channel, dutyCycle);
}

bool rn2xx3::setChannelFrequency(unsigned int channel, uint32_t frequency)
{
  return _rn2xx3_handler.setChannelFrequency(channel, frequency);
}

bool rn2xx3::setChannelDataRateRange(unsigned int channel, unsigned int minRange, unsigned int maxRange)
{
  return _rn2xx3_handler.setChannelDataRateRange(channel, minRange, maxRange);
}

bool rn2xx3::setChannelEnabled(unsigned int channel, bool enabled)
{
  return _rn2xx3_handler.setChannelEnabled(channel, enabled);
}

bool rn2xx3::set2ndRecvWindow(unsigned int dataRate, uint32_t frequency)
{
  return _rn2xx3_handler.set2ndRecvWindow(dataRate, frequency);
}

bool rn2xx3::setAdaptiveDataRate(bool enabled)
{
  return _rn2xx3_handler.setAdaptiveDataRate(enabled);
}

bool rn2xx3::setAutomaticReply(bool enabled)
{
  return _rn2xx3_handler.setAutomaticReply(enabled);
}

bool rn2xx3::setTXoutputPower(int pwridx)
{
  return _rn2xx3_handler.setTXoutputPower(pwridx);
}

bool rn2xx3::check_set_keys()
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
    rn2xx3_helper::isHexStr_of_length( _deveui, 16) &&
    rn2xx3_helper::isHexStr_of_length( _appeui, 16) &&
    rn2xx3_helper::isHexStr_of_length( _appkey, 32);

  bool abp_set =
    rn2xx3_helper::isHexStr_of_length(_nwkskey, 32) &&
    rn2xx3_helper::isHexStr_of_length(_appskey, 32) &&
    rn2xx3_helper::isHexStr_of_length(_devaddr, 8);

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
