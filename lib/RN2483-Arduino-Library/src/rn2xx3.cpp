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
rn2xx3::rn2xx3(Stream& serial):
_serial(serial)
{
  _serial.setTimeout(2000);
}

//TODO: change to a boolean
void rn2xx3::autobaud()
{
  String response = "";

  // Try a maximum of 10 times with a 1 second delay
  for (uint8_t i=0; i<10 && response==""; i++)
  {
    delay(1000);
    _serial.write((byte)0x00);
    _serial.write(0x55);
    _serial.println();
    // we could use sendRawCommand(F("sys get ver")); here
    _serial.println(F("sys get ver"));
    response = _serial.readStringUntil('\n');
  }
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

bool rn2xx3::init()
{
  if(_appskey=="0") //appskey variable is set by both OTAA and ABP
  {
    return false;
  }
  else if(_otaa==true)
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
  _otaa = true;
  _nwkskey = "0";
  String receivedData;

  //clear serial buffer
  while(_serial.available())
    _serial.read();

  // detect which model radio we are using
  configureModuleType();

  // reset the module - this will clear all keys set previously
  switch (_moduleType)
  {
    case RN2903:
      sendRawCommand(F("mac reset"));
      break;
    case RN2483:
      sendRawCommand(F("mac reset 868"));
      break;
    default:
      // we shouldn't go forward with the init
      return false;
  }

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
    // else fall back to the hard coded value in the header file
  }

  sendMacSet(F("deveui"), _deveui);

  // A valid length App EUI was given. Use it.
  if ( AppEUI.length() == 16 )
  {
      _appeui = AppEUI;
      sendMacSet(F("appeui"), _appeui);
  }

  // A valid length App Key was give. Use it.
  if ( AppKey.length() == 32 )
  {
    _appskey = AppKey; //reuse the same variable as for ABP
    sendMacSet(F("appkey"), _appskey);
  }

  if (_moduleType == RN2903)
  {
    setTXoutputPower(5);
  }
  else
  {
    setTXoutputPower(1);
  }

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

  _serial.setTimeout(30000);
  sendRawCommand(F("mac save"));

  bool joined = false;

  // Only try twice to join, then return and let the user handle it.
  for(int i=0; i<2 && !joined; i++)
  {
    sendRawCommand(F("mac join otaa"));
    // Parse 2nd response
    receivedData = _serial.readStringUntil('\n');

    if(receivedData.startsWith(F("accepted")))
    {
      joined=true;
      delay(1000);
    }
    else
    {
      delay(1000);
    }
  }
  _serial.setTimeout(2000);
  return joined;
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
  _otaa = false;
  _devAddr = devAddr;
  _appskey = AppSKey;
  _nwkskey = NwkSKey;
  String receivedData;

  //clear serial buffer
  while(_serial.available())
    _serial.read();

  configureModuleType();

  switch (_moduleType) {
    case RN2903:
      sendRawCommand(F("mac reset"));
      break;
    case RN2483:
      sendRawCommand(F("mac reset 868"));
      // set2ndRecvWindow(3, 869525000);
      // In the past we set the downlink channel here,
      // but setFrequencyPlan is a better place to do it.
      break;
    default:
      // we shouldn't go forward with the init
      return false;
  }

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
  sendMacSet(F("dr"), String(5)); //0= min, 7=max

  _serial.setTimeout(60000);
  sendRawCommand(F("mac save"));
  sendRawCommand(F("mac join abp"));
  receivedData = _serial.readStringUntil('\n');

  _serial.setTimeout(2000);
  delay(1000);

  if(receivedData.startsWith(F("accepted")))
  {
    return true;
    //with abp we can always join successfully as long as the keys are valid
  }
  else
  {
    return false;
  }
}

TX_RETURN_TYPE rn2xx3::tx(const String& data)
{
  return txUncnf(data); //we are unsure which mode we're in. Better not to wait for acks.
}

TX_RETURN_TYPE rn2xx3::txBytes(const byte* data, uint8_t size)
{
  char msgBuffer[size*2 + 1];

  char buffer[3];
  for (unsigned i=0; i<size; i++)
  {
    sprintf(buffer, "%02X", data[i]);
    memcpy(&msgBuffer[i*2], &buffer, sizeof(buffer));
  }
  String dataToTx(msgBuffer);
  return txCommand("mac tx uncnf 1 ", dataToTx, false);
}

TX_RETURN_TYPE rn2xx3::txCnf(const String& data)
{
  return txCommand("mac tx cnf 1 ", data, true);
}

TX_RETURN_TYPE rn2xx3::txUncnf(const String& data)
{
  return txCommand("mac tx uncnf 1 ", data, true);
}

TX_RETURN_TYPE rn2xx3::txCommand(const String& command, const String& data, bool shouldEncode)
{
  bool send_success = false;
  uint8_t busy_count = 0;
  uint8_t retry_count = 0;

  //clear serial buffer
  while(_serial.available())
    _serial.read();

  while(!send_success)
  {
    //retransmit a maximum of 10 times
    retry_count++;
    if(retry_count>10)
    {
      return TX_FAIL;
    }

    _serial.print(command);
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

    switch (determineReceivedDataType(receivedData))
    {
      case rn2xx3::ok:
      {
        _serial.setTimeout(30000);
        receivedData = _serial.readStringUntil('\n');
        _serial.setTimeout(2000);

        //TODO: Debug print on receivedData

        switch (determineReceivedDataType(receivedData))
        {
          case rn2xx3::mac_tx_ok:
          {
            //SUCCESS!!
            send_success = true;
            return TX_SUCCESS;
          }

          case rn2xx3::mac_rx:
          {
            //example: mac_rx 1 54657374696E6720313233
            _rxMessenge = receivedData.substring(receivedData.indexOf(' ', 7)+1);
            send_success = true;
            return TX_WITH_RX;
          }

          case rn2xx3::mac_err:
          {
            init();
            break;
          }

          case rn2xx3::invalid_data_len:
          {
            //this should never happen if the prototype worked
            send_success = true;
            return TX_FAIL;
          }

          case rn2xx3::radio_tx_ok:
          {
            //SUCCESS!!
            send_success = true;
            return TX_SUCCESS;
          }

          case rn2xx3::radio_err:
          {
            //This should never happen. If it does, something major is wrong.
            init();
            break;
          }

          default:
          {
            //unknown response
            //init();
          }
        } // End while after "ok"
        break;
      }

      case rn2xx3::invalid_param:
      {
        //should not happen if we typed the commands correctly
        send_success = true;
        return TX_FAIL;
      }

      case rn2xx3::not_joined:
      {
        init();
        break;
      }

      case rn2xx3::no_free_ch:
      {
        //retry
        delay(1000);
        break;
      }

      case rn2xx3::silent:
      {
        init();
        break;
      }

      case rn2xx3::frame_counter_err_rejoin_needed:
      {
        init();
        break;
      }

      case rn2xx3::busy:
      {
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
        init();
        break;
      }

      case rn2xx3::invalid_data_len:
      {
        //should not happen if the prototype worked
        send_success = true;
        return TX_FAIL;
      }

      default:
      {
        //unknown response after mac tx command
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

String rn2xx3::base16decode(const String& input_c)
{
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
    int out = strtoul(toDo, 0, 16);
    if((out & 0xFF) == 0)
    {
      output += char(out);
    }
  }
  return output;
}

void rn2xx3::setDR(int dr)
{
  if(dr>=0 && dr<=5)
  {
    sendMacSet(F("dr"), String(dr));
  }
}

void rn2xx3::sleep(long msec)
{
  _serial.print("sys sleep ");
  _serial.println(msec);
}

String rn2xx3::sendRawCommand(const String& command)
{
  delay(100);
  while(_serial.available())
    _serial.read();
  _serial.println(command);

  String ret = _serial.readStringUntil('\n');
  ret.trim();

  if (ret.equals(F("invalid_param")))
  {
    _lastErrorInvalidParam = command;
  }

  //TODO: Add debug print

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
      case 'b': 
        MATCH_STRING(busy);
        break;
      case 'f': 
        MATCH_STRING(frame_counter_err_rejoin_needed);
        break;
      case 'i': 
        MATCH_STRING(invalid_data_len);
        MATCH_STRING(invalid_param);
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

String rn2xx3::getLastErrorInvalidParam() 
{
  String res = _lastErrorInvalidParam;
  _lastErrorInvalidParam = "";
  return res;
}

bool rn2xx3::sendMacSet(const String& param, const String& value)
{
  String command;
  command.reserve(10 + param.length() + value.length());
  command = F("mac set ");
  command += param;
  command += ' ';
  command += value;

  return sendRawCommand(command).equals(F("ok"));
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