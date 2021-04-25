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

void rn2xx3::setAsyncMode(bool enabled) {
  _rn2xx3_handler.setAsyncMode(enabled);
}

bool rn2xx3::getAsyncMode() const {
  return _rn2xx3_handler.getAsyncMode();
}

bool rn2xx3::autobaud()
{
  // FIXME TD-er: Must fix this, as it is not working well.
  String response = "";

  // Try a maximum of 10 times with a 1 second delay
  for (uint8_t i = 0; i < 10 && response.length() == 0; i++)
  {
    if (i != 0)
    {
      delay(1000);
    }
    _rn2xx3_handler._serial.write((byte)0x00);
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
  return _rn2xx3_handler.sysver();
}

String rn2xx3::hweui()
{
  return sendRawCommand(F("sys get hweui"));
}

String rn2xx3::appeui()
{
  return sendRawCommand(F("mac get appeui"));
}

String rn2xx3::appkey() const
{
  // We can't read back from module, we send the one
  // we have memorized if it has been set
  return _rn2xx3_handler.appkey();
}

String rn2xx3::appskey() const
{
  // We can't read back from module, we send the one
  // we have memorized if it has been set
  return _rn2xx3_handler.appskey();
}

String rn2xx3::deveui()
{
  return sendRawCommand(F("mac get deveui"));
}

bool rn2xx3::setSF(uint8_t sf)
{
  return _rn2xx3_handler.setSF(sf);
}

bool rn2xx3::init()
{
  return _rn2xx3_handler.init();
}

bool rn2xx3::initOTAA(const String& AppEUI, const String& AppKey, const String& DevEUI)
{
  return _rn2xx3_handler.initOTAA(AppEUI, AppKey, DevEUI);
}

bool rn2xx3::initOTAA(uint8_t *AppEUI, uint8_t *AppKey, uint8_t *DevEUI)
{
  return _rn2xx3_handler.initOTAA(AppEUI, AppKey, DevEUI);
}

bool rn2xx3::initABP(const String& devAddr, const String& AppSKey, const String& NwkSKey)
{
  return _rn2xx3_handler.initABP(devAddr, AppSKey, NwkSKey);
}

RN2xx3_datatypes::TX_return_type rn2xx3::tx(const String& data, uint8_t port)
{
  return txUncnf(data, port); // we are unsure which mode we're in. Better not to wait for acks.
}

RN2xx3_datatypes::TX_return_type rn2xx3::txBytes(const byte *data, uint8_t size, uint8_t port)
{
  const String dataToTx = rn2xx3_helper::base16encode(data, size);
  return txCommand(F("mac tx uncnf "), dataToTx, false, port);
}

RN2xx3_datatypes::TX_return_type rn2xx3::txHexBytes(const String& hexEncoded, uint8_t port)
{
  return txCommand(F("mac tx uncnf "), hexEncoded, false, port);
}

RN2xx3_datatypes::TX_return_type rn2xx3::txCnf(const String& data, uint8_t port)
{
  return txCommand(F("mac tx cnf "), data, true, port);
}

RN2xx3_datatypes::TX_return_type rn2xx3::txUncnf(const String& data, uint8_t port)
{
  return txCommand(F("mac tx uncnf "), data, true, port);
}

RN2xx3_datatypes::TX_return_type rn2xx3::txCommand(const String& command, const String& data, bool shouldEncode, uint8_t port)
{
  return _rn2xx3_handler.txCommand(command, data, shouldEncode, port);
}


// FIXME TD-er: Move this to the handler class.
rn2xx3_handler::RN_state rn2xx3::async_loop()
{
  rn2xx3_handler::RN_state newState =  _rn2xx3_handler.async_loop(); 
 
  if (newState == rn2xx3_handler::RN_state::must_perform_init) { 
    _rn2xx3_handler.init(); 
  } 
  return _rn2xx3_handler.get_state();
}

uint8_t rn2xx3::get_busy_count() const
{
  return _rn2xx3_handler.get_busy_count();
}

rn2xx3_handler::RN_state rn2xx3::wait_command_finished(unsigned long timeout)
{
  return _rn2xx3_handler.wait_command_finished(timeout);
}

rn2xx3_handler::RN_state rn2xx3::wait_command_accepted(unsigned long timeout)
{
  return _rn2xx3_handler.wait_command_accepted(timeout);
}

bool rn2xx3::command_finished() const
{
  return _rn2xx3_handler.command_finished();
}

String rn2xx3::getRx() {
  return _rn2xx3_handler.get_rx_message();
}

int rn2xx3::getSNR()
{
  return _rn2xx3_handler.readIntValue(F("radio get snr"));
}

int rn2xx3::getVbat()
{
  return _rn2xx3_handler.readIntValue(F("sys get vdd"));
}

String rn2xx3::getDataRate()
{
  String output;

  output.reserve(9);
  output  = sendRawCommand(F("radio get sf"));
  output += "bw";
  output += _rn2xx3_handler.readIntValue(F("radio get bw"));
  return output;
}

int rn2xx3::getRSSI()
{
  return _rn2xx3_handler.readIntValue(F("radio get rssi"));
}

bool rn2xx3::setDR(int dr)
{
  return _rn2xx3_handler.setDR(dr);
}

void rn2xx3::sleep(long msec)
{
  // FIXME TD-er: Must make this a command that waits for other commands to be finished first.
  _rn2xx3_handler._serial.print(F("sys sleep "));
  _rn2xx3_handler._serial.println(msec);
}

String rn2xx3::sendRawCommand(const String& command)
{
  return _rn2xx3_handler.sendRawCommand(command);
}

RN2xx3_datatypes::Model rn2xx3::moduleType()
{
  return _rn2xx3_handler.moduleType();
}

bool rn2xx3::setFrequencyPlan(RN2xx3_datatypes::Freq_plan fp)
{
  return _rn2xx3_handler.setFrequencyPlan(fp);
}

String rn2xx3::peekLastError() const
{
  return _rn2xx3_handler.peekLastError();
}

float rn2xx3::getLoRaAirTime(uint8_t  pl) const
{
  return _rn2xx3_handler.getLoRaAirTime(pl);
}

String rn2xx3::getLastError()
{
  return _rn2xx3_handler.getLastError();
}

bool rn2xx3::getFrameCounters(uint32_t& dnctr, uint32_t& upctr)
{
  return
    _rn2xx3_handler.readUIntMacGet(F("dnctr"), dnctr) &&
    _rn2xx3_handler.readUIntMacGet(F("upctr"), upctr);
}

bool rn2xx3::setFrameCounters(uint32_t dnctr, uint32_t upctr)
{
  return
    _rn2xx3_handler.sendMacSet(F("dnctr"), String(dnctr)) &&
    _rn2xx3_handler.sendMacSet(F("upctr"), String(upctr));
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
