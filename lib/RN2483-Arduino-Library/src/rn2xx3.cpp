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

extern "C" {
#include <string.h>
#include <stdlib.h>
}

/*
  @param serial Needs to be an already opened Stream ({Software/Hardware}Serial) to write to and read from.
*/
rn2xx3::rn2xx3(Stream& serial) : _serial(serial)
{
  setSerialTimeout();
}

bool rn2xx3::autobaud()
{
  String response = "";

  // Try a maximum of 10 times with a 1 second delay
  for (uint8_t i=0; i<10 && response.length() == 0; i++)
  {
    if (i != 0)
    { 
      delay(1000); 
    }
    _serial.write((byte)0x00);
    _serial.write(0x55);
    _serial.println();
    clearSerialBuffer();
    
    // we could use sendRawCommand(F("sys get ver")); here
    _serial.println(F("sys get ver"));
    response = _serial.readStringUntil('\n');
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
  String model = version.substring(2,6);
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
      _lastErrorInvalidParam = F("error in reset");
      return false;
  }
  _lastErrorInvalidParam += F("success resetmodule");;
  return true;
//  return determineReceivedDataType(result) == ok;
}

String rn2xx3::hweui()
{
  return (sendRawCommand(F("sys get hweui")));
}

String rn2xx3::appeui()
{
  return ( sendRawCommand(F("mac get appeui") ));
}

String rn2xx3::appkey()
{
  // We can't read back from module, we send the one
  // we have memorized if it has been set
  return _appskey;
}

String rn2xx3::deveui()
{
  return (sendRawCommand(F("mac get deveui")));
}

bool rn2xx3::setSF(uint8_t sf)
{
  if (sf >= 7 && sf <= 12)
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
      //case TTN_FP_US915:
      //case TTN_FP_AU915:
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
  _lastErrorInvalidParam = F("error in setSF");
  return false;
}

bool rn2xx3::init()
{
  if(_appskey=="0") //appskey variable is set by both OTAA and ABP
  {
    return false;
  }
  else if(_otaa)
  {
    return initOTAA(_appeui, _appskey);
  }
  else
  {
    return initABP(_devAddr, _appskey, _nwkskey);
  }
}


bool rn2xx3::initOTAA(const String& AppEUI, const String& AppKey, const String& DevEUI)
{
  // If the Device EUI was given as a parameter, use it
  // otherwise use the Hardware EUI.
  if (DevEUI.length() == 16)
  {
    _deveui = DevEUI;
  }
  else
  {
    String addr = sendRawCommand(F("sys get hweui"));
    if( addr.length() == 16 )
    {
      _deveui = addr;
    }
    else
    {
      //The default address to use on TTN if no address is defined.
      //This one falls in the "testing" address space.
      _devAddr = "03FFBEEF";
    }
  }
    
  if ( AppEUI.length() != 16 || AppKey.length() != 32 || _deveui.length() != 16)
  {
    // No valid config
    _lastErrorInvalidParam = F("InitOTAA: Not all keys are valid.");
    return false;
  }
  _appeui = AppEUI;
  _appskey = AppKey; //reuse the same variable as for ABP

  if (_otaa && Status.Joined) {
    saveUpdatedStatus();
    if (Status.Joined && !Status.RejoinNeeded) {
      return true;
    }
  }

  _otaa = true;
  _nwkskey = "0";

  clearSerialBuffer();

  if (!resetModule()) { return false; }

  sendMacSet(F("deveui"), _deveui);
  sendMacSet(F("appeui"), _appeui);
  sendMacSet(F("appkey"), _appskey);

  if (_moduleType == RN2903)
  {
    setTXoutputPower(5);
  }
  else
  {
    setTXoutputPower(1);
  }
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

  // TODO this is a really long timeout. Will setSerialTimeoutRX2() do?
  _serial.setTimeout(30000);
//  sendRawCommand(F("mac save")); 
  
  // Only try twice to join, then return and let the user handle it.
  Status.Joined = false;
  updateStatus();
  for(int i=0; i<2 && !Status.Joined; i++)
  {
    sendRawCommand(F("mac join otaa"));
    // Parse 2nd response
    String receivedData = _serial.readStringUntil('\n');

    if(determineReceivedDataType(receivedData) == accepted)
    {
      Status.Joined = true;
    } else {
      _lastErrorInvalidParam = receivedData;
    }
    delay(2000); // Needed to make sure even RX2 replies are processed.
    updateStatus();
  }
  setSerialTimeout();
  saveUpdatedStatus();
  return Status.Joined;
}


bool rn2xx3::initOTAA(uint8_t * AppEUI, uint8_t * AppKey, uint8_t * DevEUI)
{
  String app_eui;
  String dev_eui;
  String app_key;
  char buff[3];

  app_eui="";
  for (uint8_t i=0; i<8; i++)
  {
    sprintf(buff, "%02X", AppEUI[i]);
    app_eui += String (buff);
  }

  dev_eui = "0";
  if (DevEUI) //==0
  {
    dev_eui = "";
    for (uint8_t i=0; i<8; i++)
    {
      sprintf(buff, "%02X", DevEUI[i]);
      dev_eui += String (buff);
    }
  }

  app_key="";
  for (uint8_t i=0; i<16; i++)
  {
    sprintf(buff, "%02X", AppKey[i]);
    app_key += String (buff);
  }

  return initOTAA(app_eui, app_key, dev_eui);
}

bool rn2xx3::initABP(const String& devAddr, const String& AppSKey, const String& NwkSKey)
{

  clearSerialBuffer();
  if (!Status.Joined || _otaa) {
    _otaa = false;
    _devAddr = devAddr;
    _appskey = AppSKey;
    _nwkskey = NwkSKey;
    String receivedData;

    if (!resetModule()) { return false; }

    sendMacSet(F("nwkskey"), _nwkskey);
    sendMacSet(F("appskey"), _appskey);
    sendMacSet(F("devaddr"), _devAddr);
    setAdaptiveDataRate(false);

    // Switch off automatic replies, because this library can not
    // handle more than one mac_rx per tx. See RN2483 datasheet,
    // 2.4.8.14, page 27 and the scenario on page 19.
    setAutomaticReply(false);

    if (_moduleType == RN2903)
    {
      setTXoutputPower(5);
    }
    else
    {
      setTXoutputPower(1);
    }
    setSF(_sf);

    // TODO Determine proper delay for this timeout.
    // Is this as long as for a normal RX2 delay?
    // setSerialTimeoutRX2();
    _serial.setTimeout(60000);
    sendRawCommand(F("mac join abp"));
    // Wait for the 2nd response.
    receivedData = _serial.readStringUntil('\n');

    setSerialTimeout();
    //with abp we can always join successfully as long as the keys are valid
    if (determineReceivedDataType(receivedData) != accepted) {
      _lastErrorInvalidParam = receivedData;
      Status.Joined = false;
    }
    delay(2000); // Needed to make sure even RX2 replies are processed.
  }
  saveUpdatedStatus();
  return Status.Joined;
}

TX_RETURN_TYPE rn2xx3::tx(const String& data, uint8_t port)
{
  return txUncnf(data); //we are unsure which mode we're in. Better not to wait for acks.
}

TX_RETURN_TYPE rn2xx3::txBytes(const byte* data, uint8_t size, uint8_t port)
{
  String dataToTx;
  dataToTx.reserve(size * 2);
  char buffer[3];
  for (unsigned i=0; i<size; i++)
  {
    sprintf(buffer, "%02X", data[i]);
    dataToTx += buffer[0];
    dataToTx += buffer[1];
  }
  return txCommand("mac tx uncnf ", dataToTx, false, port);
}

TX_RETURN_TYPE rn2xx3::txHexBytes(const String& hexEncoded, uint8_t port)
{
  return txCommand("mac tx uncnf ", hexEncoded, false, port);
}

TX_RETURN_TYPE rn2xx3::txCnf(const String& data, uint8_t port)
{
  return txCommand("mac tx cnf ", data, true, port);
}

TX_RETURN_TYPE rn2xx3::txUncnf(const String& data, uint8_t port)
{
  return txCommand("mac tx uncnf ", data, true, port);
}

TX_RETURN_TYPE rn2xx3::txCommand(const String& command, const String& data, bool shouldEncode, uint8_t port)
{
  updateStatus();
  bool send_success = false;
  uint8_t busy_count = 0;
  uint8_t retry_count = 0;

  clearSerialBuffer();

  while(!send_success)
  {
    //retransmit a maximum of 10 times
    retry_count++;
    if(retry_count>10)
    {
      return TX_FAIL;
    }

    _serial.print(command);
    if (command.endsWith(F("cnf "))) {
      // No port was given in the command, so add the port.
      _serial.print(port);
      _serial.print(' ');
    }
    if(shouldEncode)
    {
      sendEncoded(data);
    }
    else
    {
      _serial.print(data);
    }
    _serial.println();

    String receivedData = _serial.readStringUntil('\n');
    //TODO: Debug print on receivedData

    // "mac tx" commands may receive a second response if the first one was "ok"
    bool firstResponseAfterSendingCommand = true;
    if (determineReceivedDataType(receivedData) == rn2xx3::ok) {
      // parameters and configurations are valid and the packet was forwarded to the radio transceiver for transmission
      setSerialTimeoutRX2();
      receivedData = _serial.readStringUntil('\n');
      setSerialTimeout();
      firstResponseAfterSendingCommand = false;
    }

    switch (determineReceivedDataType(receivedData))
    {
      case rn2xx3::ok:
      {
        // Already handled.
        break;
      }

      case rn2xx3::invalid_param:
      {
        // parameters (<type> <portno> <data>) are not valid
        // should not happen if we typed the commands correctly
        send_success = true;
        return TX_FAIL;
      }

      case rn2xx3::not_joined:
      {
        // the network is not joined
        _lastErrorInvalidParam = receivedData;
        Status.Joined = false;
        init();
        break;
      }

      case rn2xx3::no_free_ch:
      {
        // all channels are busy
        // probably duty cycle limits exceeded.
        //retry
        _lastErrorInvalidParam = receivedData;
        delay(1000);
        break;
      }

      case rn2xx3::silent:
      {
        // the module is in a Silent Immediately state
        // This is enforced by the network.
        // To enable: 
        // sendRawCommand(F("mac forceENABLE"));
        // N.B. One has to think about why this has happened.
        _lastErrorInvalidParam = receivedData;
        init();
        break;
      }

      case rn2xx3::frame_counter_err_rejoin_needed:
      {
        // the frame counter rolled over
        _lastErrorInvalidParam = receivedData;
        init();
        break;
      }

      case rn2xx3::busy:
      {
        // MAC state is not in an Idle state
        busy_count++;

        // Not sure if this is wise. At low data rates with large packets
        // this can perhaps cause transmissions at more than 1% duty cycle.
        // Need to calculate the correct constant value.
        // But it is wise to have this check and re-init in case the
        // lorawan stack in the RN2xx3 hangs.
        if(busy_count>=10)
        {
          init();
        }
        else
        {
          delay(1000);
        }
        break;
      }

      case rn2xx3::mac_paused:
      {
        // MAC was paused and not resumed back
        _lastErrorInvalidParam = receivedData;
        init();
        break;
      }

      case rn2xx3::invalid_data_len:
      {
        if (firstResponseAfterSendingCommand)
        {
          // application payload length is greater than the maximum application payload length corresponding to the current data rate

        }
        else 
        {
          // application payload length is greater than the maximum application payload length corresponding to the current data rate. 
          // This can occur after an earlier uplink attempt if retransmission back-off has reduced the data rate.

        }
        _lastErrorInvalidParam = receivedData;
        send_success = true;
        return TX_FAIL;
      }

      case rn2xx3::mac_tx_ok:
      {
        // if uplink transmission was successful and no downlink data was received back from the server
        //SUCCESS!!
        send_success = true;
        return TX_SUCCESS;
      }

      case rn2xx3::mac_rx:
      {
        // mac_rx <portno> <data> 
        // transmission was successful
        // <portno>: port number, from 1 to 223
        // <data>: hexadecimal value that was received from theserver
        //example: mac_rx 1 54657374696E6720313233
        _rxMessenge = receivedData.substring(receivedData.indexOf(' ', 7)+1);
        send_success = true;
        return TX_WITH_RX;
      }

      case rn2xx3::mac_err:
      {
        _lastErrorInvalidParam = receivedData;
        init();
        break;
      }

      case rn2xx3::radio_err:
      {
        // transmission was unsuccessful, ACK not received back from the server
        // This should never happen. If it does, something major is wrong.
        _lastErrorInvalidParam = receivedData;
        init();
        break;
      }
      default:
      {
        //unknown response after mac tx command
        _lastErrorInvalidParam = receivedData;
        init();
        break;
      }
    }
  }

  return TX_FAIL; //should never reach this
}

void rn2xx3::sendEncoded(const String& input)
{
  char buffer[3];
  for (unsigned i=0; i<input.length(); i++)
  {
    sprintf(buffer, "%02x", static_cast<int>(input.charAt(i)));
    _serial.print(buffer);
  }
}

String rn2xx3::base16encode(const String& input_c)
{
  String input(input_c); // Make a deep copy to be able to do trim()
  input.trim();
  const size_t inputLength = input.length();
  String output;
  output.reserve(inputLength * 2);
  
  for(size_t i = 0; i < inputLength; ++i)
  {
    if(input[i] == '\0') break;

    char buffer[3];
    sprintf(buffer, "%02x", static_cast<int>(input[i]));
    output += buffer[0];
    output += buffer[1];
  }
  return output;
}

String rn2xx3::getRx() {
  return _rxMessenge;
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
  output = sendRawCommand(F("radio get sf"));
  output += "bw";
  output += readIntValue(F("radio get bw"));
  return output;
}

int rn2xx3::getRSSI()
{
  return readIntValue(F("radio get rssi"));
}

String rn2xx3::base16decode(const String& input_c)
{
  if (!isHexStr(input_c)) return "";
  String input(input_c); // Make a deep copy to be able to do trim()
  input.trim();
  const size_t inputLength = input.length();
  const size_t outputLength = inputLength / 2;
  String output;
  output.reserve(outputLength);

  for(size_t i = 0; i < outputLength; ++i)
  {
    char toDo[3];
    toDo[0] = input[i*2];
    toDo[1] = input[i*2+1];
    toDo[2] = '\0';
    unsigned long out = strtoul(toDo, 0, 16);
    if(out <= 0xFF)
    {
      output += char(out & 0xFF);
    }
  }
  return output;
}

bool rn2xx3::setDR(int dr)
{
  if(dr>=0 && dr<=7)
  {
    return sendMacSet(F("dr"), String(dr));
  }
  return false;
}

void rn2xx3::sleep(long msec)
{
  _serial.print("sys sleep ");
  _serial.println(msec);
}

String rn2xx3::sendRawCommand(const String& command)
{
//  delay(100);
  clearSerialBuffer();
  _serial.println(command);

  String ret = _serial.readStringUntil('\n');
  ret.trim();

  switch (determineReceivedDataType(ret))
  {
    case ok:
    case UNKNOWN:
    case accepted:
      break;
    default:
      _lastErrorInvalidParam = command;    
  }
  /*
  String log = F("SendRaw: ");
  log += command;
  log += F(" -> ");
  log += ret;
  _lastErrorInvalidParam = log;

  //TODO: Add debug print
   */

  return ret;
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
      if(_moduleType == RN2483)
      {
        //mac set rx2 <dataRate> <frequency>
        //set2ndRecvWindow(5, 868100000); //use this for "strict" one channel gateways
        set2ndRecvWindow(3, 869525000); //use for "non-strict" one channel gateways
        setChannelDutyCycle(0, 99); //1% duty cycle for this channel
        setChannelDutyCycle(1, 65535); //almost never use this channel
        setChannelDutyCycle(2, 65535); //almost never use this channel
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
      if(_moduleType == RN2483)
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
          setChannelEnabled(ch, true);  // frequency, data rate and duty cycle must be set first.
        }

        //RX window 2
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
      if(_moduleType == RN2903)
      {
        for(int channel=0; channel<72; channel++)
        {
          bool enabled = (channel>=8 && channel<16);
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
      if(_moduleType == RN2483)
      {
        for(int channel=0; channel<8; channel++)
        {
          if (channel < 3) {
            //fix duty cycle - 1% = 0.33% per channel
            setChannelDutyCycle(channel, 799);
            setChannelEnabled(channel, true);
          } else {
            //disable non-default channels
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
      //set default channels 868.1, 868.3 and 868.5?
      returnValue = false; //well we didn't do anything, so yes, false
      break;
    }
  }

  return returnValue;
}


rn2xx3::received_t rn2xx3::determineReceivedDataType(const String& receivedData) {
  if (receivedData.length() != 0) {
    #define MATCH_STRING(S) \
    if (receivedData.startsWith(F(#S))) return (rn2xx3::S);

    switch (receivedData[0]) {
      case 'a': 
        MATCH_STRING(accepted);
        break;
      case 'b': 
        MATCH_STRING(busy);
        break;
      case 'd': 
        MATCH_STRING(denied);
        break;
      case 'f': 
        MATCH_STRING(frame_counter_err_rejoin_needed);
        break;
      case 'i': 
        MATCH_STRING(invalid_data_len);
        MATCH_STRING(invalid_param);
        break;
      case 'k': 
        MATCH_STRING(keys_not_init);
        break;
      case 'm': 
        MATCH_STRING(mac_err);
        MATCH_STRING(mac_paused);
        MATCH_STRING(mac_rx);
        MATCH_STRING(mac_tx_ok);
        break;
      case 'n': 
        MATCH_STRING(no_free_ch);
        MATCH_STRING(not_joined);
        break;
      case 'o': 
        MATCH_STRING(ok);
        break;
      case 'r': 
        MATCH_STRING(radio_err);
        MATCH_STRING(radio_tx_ok);
        break;
      case 's': 
        MATCH_STRING(silent);
        break;
    }
    #undef MATCH_STRING
  }
  return rn2xx3::UNKNOWN;
}


int rn2xx3::readIntValue(const String& command)
{
  String value = sendRawCommand(command);
  value.trim();
  return value.toInt();
}

bool rn2xx3::readUIntMacGet(const String& param, uint32_t &value)
{
  String command;
  command.reserve(8 + param.length());
  command = F("mac get ");
  command += param;
  String value_str = sendRawCommand(command);
  if (value_str.length() == 0)
  {
    return false;
  }
  value = strtoul(value_str.c_str(), 0, 10);
  return true;
}

String rn2xx3::peekLastErrorInvalidParam()
{
  return _lastErrorInvalidParam;;
}

String rn2xx3::getLastErrorInvalidParam() 
{
  String res = _lastErrorInvalidParam;
  _lastErrorInvalidParam = "";
  return res;
}

bool rn2xx3::getFrameCounters(uint32_t &dnctr, uint32_t &upctr)
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

bool rn2xx3::sendMacSet(const String& param, const String& value)
{
  String command;
  command.reserve(10 + param.length() + value.length());
  command = F("mac set ");
  command += param;
  command += ' ';
  command += value;

  return determineReceivedDataType(sendRawCommand(command)) == ok;
}

bool rn2xx3::sendMacSetEnabled(const String& param, bool enabled)
{
  return sendMacSet(param, enabled ? F("on") : F("off"));
}

bool rn2xx3::sendMacSetCh(const String& param, unsigned int channel, const String& value)
{
  String command;
  command.reserve(20);
  command = param;
  command += ' ';
  command += channel;
  command += ' ';
  command += value;
  return sendMacSet(F("ch"), command);
}

bool rn2xx3::sendMacSetCh(const String& param, unsigned int channel, uint32_t value)
{
  return sendMacSetCh(param, channel, String(value));
}

bool rn2xx3::setChannelDutyCycle(unsigned int channel, unsigned int dutyCycle)
{
  return sendMacSetCh(F("dcycle"), channel, dutyCycle);
}

bool rn2xx3::setChannelFrequency(unsigned int channel, uint32_t frequency)
{
  return sendMacSetCh(F("freq"), channel, frequency);
}

bool rn2xx3::setChannelDataRateRange(unsigned int channel, unsigned int minRange, unsigned int maxRange)
{
  String value;
  value = String(minRange);
  value += ' ';
  value += String(maxRange);
  return sendMacSetCh(F("drrange"), channel, value);
}

bool rn2xx3::setChannelEnabled(unsigned int channel, bool enabled)
{
  return sendMacSetCh(F("status"), channel, enabled ? F("on") : F("off"));
}

bool rn2xx3::set2ndRecvWindow(unsigned int dataRate, uint32_t frequency)
{
  String value;
  value = String(dataRate);
  value += ' ';
  value += String(frequency);
  return sendMacSet(F("rx2"), value);
}

bool rn2xx3::setAdaptiveDataRate(bool enabled)
{
  return sendMacSetEnabled(F("adr"), enabled);
}

bool rn2xx3::setAutomaticReply(bool enabled)
{
  return sendMacSetEnabled(F("ar"), enabled);
}

bool rn2xx3::setTXoutputPower(int pwridx)
{
  return sendMacSet(F("pwridx"), String(pwridx));
}

bool rn2xx3::updateStatus()
{
  const String status_str = sendRawCommand(F("mac get status"));
  const size_t strlength = status_str.length();
  if (strlength != 8 || !isHexStr(status_str)) { 
    _lastErrorInvalidParam = F("mac get status  : No valid hex string");
    return false; 
  }
  uint32_t status_value = strtoul(status_str.c_str(), 0, 16);
  Status.decode(status_value);
  if (rxdelay1 == 0 || rxdelay2 == 0 || Status.SecondReceiveWindowParamUpdated)
  {
    readUIntMacGet(F("rxdelay1"), rxdelay1);
    readUIntMacGet(F("rxdelay2"), rxdelay2);
    Status.SecondReceiveWindowParamUpdated = false;
  }  
  return true;
}

bool rn2xx3::saveUpdatedStatus()
{

  // Only save to the eeprom when really needed.
  // No need to store the current config when there is no active connection.
  // Todo: Must keep track of last saved counters and decide to update when current counter differs more than set threshold.
  bool saved = false;
  if (updateStatus())
  {
    if (Status.Joined && !Status.RejoinNeeded && Status.saveSettingsNeeded())
    {
      saved = determineReceivedDataType(sendRawCommand(F("mac save"))) == ok;
      Status.clearSaveSettingsNeeded();
      updateStatus();
    }
  }
  return saved;
}

void rn2xx3::setSerialTimeout()
{
  // Enough time to wait for:
  // sending the command module + reading reply
  // TODO Determine correct delay based on baud rate + response time of module
  _serial.setTimeout(2000);
}

void rn2xx3::setSerialTimeoutRX2() 
{
  // Enough time to wait for:
  // Transmit Time On Air + receive_delay2 + receiving RX2 packet.
  // 
  // TODO: Compute exact time, for now just 2x rxdelay2
  _serial.setTimeout(2 * rxdelay2);
}

void rn2xx3::clearSerialBuffer()
{
  while(_serial.available())
    _serial.read();
}

bool rn2xx3::isHexStr(const String& str)
{
  const size_t strlength = str.length();
  if (strlength != 8) { return false; }
  for (size_t i = 0; i < strlength; ++i) {
    const char ch = str[i];
    bool valid = (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f');
    if (!valid)
    {
      return false;      
    }
  }
  return true;
}