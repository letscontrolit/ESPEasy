/*
 * A library for controlling a Microchip RN2xx3 LoRa radio.
 *
 * Original:
 * @Author JP Meijers
 * @Author Nicolas Schteinschraber
 * @Date 18/12/2015
 *
 * Rewrite to make it async (non blocking) in handling commands:
 * @Author Gijs Noorlander
 * @Date 16/02/2020
 *
 */

#ifndef rn2xx3_h
#define rn2xx3_h

#include "Arduino.h"

#include "rn2xx3_status.h"
#include "rn2xx3_handler.h"
#include "rn2xx3_helper.h"
#include "rn2xx3_datatypes.h"


class rn2xx3 {
public:

  /*
   * A simplified constructor taking only a Stream ({Software/Hardware}Serial) object.
   * The serial port should already be initialised when initialising this library.
   */
  rn2xx3(Stream& serial);

  /*
   * Set the mode to work in async mode.
   * This requires the user of this library to call async_loop()
   * When set in async mode, the calls to commands which may take a while (e.g. join or "mac tx")
   * will not be blocking anymore.
   */
  void   setAsyncMode(bool enabled);

  bool   getAsyncMode() const;

  /*
   * Transmit the correct sequence to the rn2xx3 to trigger its autobauding feature.
   * After this operation the rn2xx3 should communicate at the same baud rate than us.
   */
  bool   autobaud();

  /*
   * Get the hardware EUI of the radio, so that we can register it on The Things Network
   * and obtain the correct AppKey.
   * You have to have a working serial connection to the radio before calling this function.
   * In other words you have to at least call autobaud() some time before this function.
   */
  String hweui();

  /*
   * Returns the OTAA  AppKey used when initializing the radio.
   */
  String appkey() const;

  /*
   * Returns the ABP AppSKey used when initializing the radio.
   */
  String appskey() const;


  /*
   * In the case of OTAA this function will return the Application EUI used
   * to initialize the radio.
   */
  String appeui();

  /*
   * In the case of OTAA this function will return the Device EUI used to
   * initialize the radio. This is not necessarily the same as the Hardware EUI.
   * To obtain the Hardware EUI, use the hweui() function.
   */
  String deveui();

  /*
   * Get the RN2xx3's hardware and firmware version number. This is also used
   * to detect if the module is either an RN2483 or an RN2903.
   */
  String sysver();

  bool   setSF(uint8_t sf);

  bool   setAdaptiveDataRate(bool enabled);

  /*
   * Initialise the RN2xx3 and join the LoRa network (if applicable).
   * This function can only be called after calling initABP() or initOTAA().
   * The sole purpose of this function is to re-initialise the radio if it
   * is in an unknown state.
   */
  bool   init();

  /*
   * Initialise the RN2xx3 and join a network using personalization.
   *
   * addr: The device address as a HEX string.
   *       Example "0203FFEE"
   * AppSKey: Application Session Key as a HEX string.
   *          Example "8D7FFEF938589D95AAD928C2E2E7E48F"
   * NwkSKey: Network Session Key as a HEX string.
   *          Example "AE17E567AECC8787F749A62F5541D522"
   */
  bool   initABP(const String& addr,
                 const String& AppSKey,
                 const String& NwkSKey);

  // TODO: initABP(uint8_t * addr, uint8_t * AppSKey, uint8_t * NwkSKey)

  /*
   * Initialise the RN2xx3 and join a network using over the air activation.
   *
   * AppEUI: Application EUI as a HEX string.
   *         Example "70B3D57ED00001A6"
   * AppKey: Application key as a HEX string.
   *         Example "A23C96EE13804963F8C2BD6285448198"
   * DevEUI: Device EUI as a HEX string.
   *         Example "0011223344556677"
   * If the DevEUI parameter is omitted, the Hardware EUI from module will be used
   * If no keys, or invalid length keys, are provided, no keys
   * will be configured. If the module is already configured with some keys
   * they will be used. Otherwise the join will fail and this function
   * will return false.
   */
  bool initOTAA(const String& AppEUI = "",
                const String& AppKey = "",
                const String& DevEUI = "");

  /*
   * Initialise the RN2xx3 and join a network using over the air activation,
   * using byte arrays. This is useful when storing the keys in eeprom or flash
   * and reading them out in runtime.
   *
   * AppEUI: Application EUI as a uint8_t buffer
   * AppKey: Application key as a uint8_t buffer
   * DevEui: Device EUI as a uint8_t buffer (optional - set to 0 to use Hardware EUI)
   */
  bool initOTAA(uint8_t *AppEUI,
                uint8_t *AppKey,
                uint8_t *DevEui);

  /*
   * Transmit the provided data. The data is hex-encoded by this library,
   * so plain text can be provided.
   * This function is an alias for txUncnf().
   *
   * Parameter is an ascii text string.
   */
  RN2xx3_datatypes::TX_return_type tx(const String&,
                                      uint8_t port = 1);

  /*
   * Transmit raw byte encoded data via LoRa WAN.
   * This method expects a raw byte array as first parameter.
   * The second parameter is the count of the bytes to send.
   */
  RN2xx3_datatypes::TX_return_type txBytes(const byte *,
                                           uint8_t size,
                                           uint8_t port = 1);

  RN2xx3_datatypes::TX_return_type txHexBytes(const String&,
                                              uint8_t port = 1);

  /*
   * Do a confirmed transmission via LoRa WAN.
   *
   * Parameter is an ascii text string.
   */
  RN2xx3_datatypes::TX_return_type txCnf(const String&,
                                         uint8_t port = 1);

  /*
   * Do an unconfirmed transmission via LoRa WAN.
   *
   * Parameter is an ascii text string.
   */
  RN2xx3_datatypes::TX_return_type txUncnf(const String&,
                                           uint8_t port = 1);

  /*
   * Transmit the provided data using the provided command.
   * Will return after the command has been processed and replies were received
   *
   * String - the tx command to send
            can only be one of "mac tx cnf 1 " or "mac tx uncnf 1 "
   * String - an ascii text string if bool is true. A HEX string if bool is false.
   * bool - should the data string be hex encoded or not
   */
  RN2xx3_datatypes::TX_return_type txCommand(const String&,
                                             const String&,
                                             bool,
                                             uint8_t port = 1);


  /*
   * Call this frequently to process TX commmands when running
   * txCommand with async set
   * This is also called from txCommand, so no need to call it when not in async mode.
   *
   * Return value is the internal state of the TX processing.
   */
  rn2xx3_handler::RN_state async_loop();

  uint8_t get_busy_count() const;

  rn2xx3_handler::RN_state wait_command_finished(unsigned long timeout = 10000);

  rn2xx3_handler::RN_state wait_command_accepted(unsigned long timeout = 10000);

  bool command_finished() const;

  /*
   * Change the datarate at which the RN2xx3 transmits.
   * A value of between 0 and 5 can be specified,
   * as is defined in the LoRaWan specs.
   * This can be overwritten by the network when using OTAA.
   * So to force a datarate, call this function after initOTAA().
   */
  /*
    EU / CN
    Frequencies:

    EU 863-870 MHz (LoRaWAN Specification (2015), Page 35, Table 14)
    CN 779-787 MHz (LoRaWAN Specification (2015), Page 44, Table 25)
    EU 433 MHz (LoRaWAN Specification (2015), Page 48, Table 31)
    DataRate    Modulation  SF  BW  bit/s
    0           LoRa        12  125 250
    1           LoRa        11  125 440
    2           LoRa        10  125 980
    3           LoRa        9   125 1'760
    4           LoRa        8   125 3'125
    5           LoRa        7   125 5'470
    6           LoRa        7   250 11'000
    7           FSK 50 kbps         50'000
    Data rates 8-15 are reserved.

    US
    Frequencies:

    US 902-928 MHz (LoRaWAN Specification (2015), Page 40, Table 18)
    DataRate    Modulation  SF  BW  bit/s
    0           LoRa        10  125 980
    1           LoRa        9   125 1'760
    2           LoRa        8   125 3'125
    3           LoRa        7   125 5'470
    4           LoRa        8   500 12'500
    8           LoRa        12  500 980
    9           LoRa        11  500 1'760
    10          LoRa        10  500 3'900
    11          LoRa        9   500 7'000
    12          LoRa        8   500 12'500
    13          LoRa        7   500 21'900
    Data rates 5-7 and 14-15 are reserved.
  */
  bool                     setDR(int dr);

  /*
   * Put the RN2xx3 to sleep for a specified timeframe.
   * The RN2xx3 accepts values from 100 to 4294967296.
   * Rumour has it that you need to do a autobaud() after the module wakes up again.
   */
  void                     sleep(long msec);

  /*
   * Send a raw command to the RN2xx3 module.
   * Returns the raw string as received back from the RN2xx3.
   * If the RN2xx3 replies with multiple line, only the first line will be returned.
   */
  String                   sendRawCommand(const String& command);

  /*
   * Returns the module type either RN2903 or RN2483, or NA.
   */
  RN2xx3_datatypes::Model  moduleType();

  /*
   * Set the active channels to use.
   * Returns true if setting the channels is possible.
   * Returns false if you are trying to use the wrong channels on the wrong module type.
   */
  bool                     setFrequencyPlan(RN2xx3_datatypes::Freq_plan);

  /*
   * Set version of TTN stack to use.
   */
  bool                     setTTNstack(RN2xx3_datatypes::TTN_stack_version version);

  /*
   * Returns the last downlink message HEX string.
   */
  String                   getRx();

  /*
   * Get the RN2xx3's SNR of the last received packet. Helpful to debug link quality.
   */
  int                      getSNR();

  /*
   * Get the RN2xx3's voltage measurement on the Vdd in mVolt
   * 0–3600 (decimal value from 0 to 3600)
   */
  int                      getVbat();

  /*
   * Return the current data rate formatted like sf7bw125
   * Firmware 1.0.1 returns always "sf9"
   */
  String                   getDataRate();

  /*
   * Return radio Received Signal Strength Indication (rssi) value
   * for the last received frame.
   * Supported since firmware 1.0.5
   */
  int                      getRSSI();

  /*
   * Almost all commands can return "invalid_param"
   * The last command resulting in such an error can be retrieved.
   * Reading this will clear the error.
   */
  String                   getLastError();

  String                   peekLastError() const;

  // Compute the air time for a packet in msec.
  // Formula used from https://www.loratools.nl/#/airtime
  // @param pl   Payload length in bytes
  float getLoRaAirTime(uint8_t  pl) const;

  bool                     hasJoined() const {
    return _rn2xx3_handler.Status.Joined;
  }

  bool useOTAA() const {
    return _rn2xx3_handler.useOTAA();
  }

  // Get the current frame counter values for downlink and uplink
  bool getFrameCounters(uint32_t& dnctr,
                        uint32_t& upctr);

  // Set frame counter values for downlink and uplink
  // E.g. to restore them after a reboot or reset of the module.
  bool setFrameCounters(uint32_t dnctr,
                        uint32_t upctr);

  // delay from last moment of sending to receive RX1 and RX2 window
  bool getRxDelayValues(uint32_t& rxdelay1,
                        uint32_t& rxdelay2);


  // At init() the module is assumed to be joined, which is also checked against the
  // _otaa flag.
  // Allow to set the last used join mode to help prevent unneeded join requests.
  void setLastUsedJoinMode(bool isOTAA) {
    _rn2xx3_handler.setLastUsedJoinMode(isOTAA);
  }

  const RN2xx3_status& getStatus() const;

private:

  // The actual interface to the module, handling the internal states.
  rn2xx3_handler _rn2xx3_handler;
};

#endif // ifndef rn2xx3_h
