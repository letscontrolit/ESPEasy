#ifndef RN2XX3_TX_STATE_H
#define RN2XX3_TX_STATE_H

#include <Arduino.h>

#include "rn2xx3_status.h"

// State machine for the RN2483/RN2903 modules
class rn2xx3_handler {
public:

  enum RN_state {
    idle,
    command_set_to_send,
    wait_for_reply,
    wait_for_reply_rx2,
    reply_received,
    reply_received_rx2,
    tx_success,
    tx_success_with_rx,
    reply_received_finished,
    join_accepted,
    timeout,
    duty_cycle_exceeded,
    max_attempt_reached,
    error,
    invalid_char_read,
    must_perform_init,
    must_pause
  };

  rn2xx3_handler(Stream& serial);

  String sendRawCommand(const String& command);

  bool   prepare_raw_command(const String& command);

  bool   prepare_tx_command(const String& command,
                            const String& data,
                            bool          shouldEncode,
                            uint8_t       port);

  bool     prepare_join(bool useOTAA);

  RN_state async_loop();

  // Wait for the command to be handled completely (including reply in RX2 window)
  RN_state wait_command_finished(unsigned long timeout = 10000);

  // Shorter wait, to be used in async mode.
  // This will return early when the message cannot be sent (e.g. due to duty cycle exceeded)
  // It will return when the state waiting for reply_received_rx2 has been reached, or the command has finished (due to error)
  RN_state wait_command_accepted(unsigned long timeout = 10000);

  // Check whether a command has finished.
  bool     command_finished() const;


  /*
   * Initialise the RN2xx3 and join the LoRa network (if applicable).
   * This function can only be called after calling initABP() or initOTAA().
   * The sole purpose of this function is to re-initialise the radio if it
   * is in an unknown state.
   */
  bool init();

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
  bool initABP(const String& addr,
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


  RN2xx3_datatypes::TX_return_type txCommand(const String&,
                                             const String&,
                                             bool,
                                             uint8_t port = 1);

  bool setSF(uint8_t sf);


  /*
   * Change the datarate at which the RN2xx3 transmits.
   * A value of between 0 and 5 can be specified,
   * as is defined in the LoRaWan specs.
   * This can be overwritten by the network when using OTAA.
   * So to force a datarate, call this function after initOTAA().
   */
  bool setDR(int dr);

  void setAsyncMode(bool enabled);

  bool getAsyncMode() const;

  bool useOTAA() const;

  void setLastUsedJoinMode(bool isOTAA);

  // We can't read back from module, we send the one
  // we have memorized if it has been set
  String appkey() const {
    return _appkey;
  }

  String appskey() const {
    return _appskey;
  }

  RN2xx3_datatypes::Model moduleType()
  {
    return _moduleType;
  }

  bool setFrequencyPlan(RN2xx3_datatypes::Freq_plan fp);

private:

  // Return the data to send
  const String& get_send_data() const;

public:

  // Get the received data
  const String& get_received_data() const;

  const String& get_received_data(unsigned long& duration) const;

  // Get the downlink message, received during RX2 after TX command.
  const String& get_rx_message() const;

  // Look at the last error, without clearing it.
  String        peekLastError() const;

  // get and clear the last error.
  String        getLastError();

  // Set specific error string.
  void          setLastError(const String& error);

  RN_state      get_state() const;

  uint8_t       get_busy_count() const;

  String        sysver();

  // delay from last moment of sending to receive RX1 and RX2 window
  bool          getRxDelayValues(uint32_t& rxdelay1,
                                 uint32_t& rxdelay2);

  // Compute the air time for a packet in msec.
  // Formula used from https://www.loratools.nl/#/airtime
  // @param pl   Payload length in bytes
  float getLoRaAirTime(uint8_t  pl) const;


private:

  RN2xx3_datatypes::Model configureModuleType();


  bool                    resetModule();

  // Check to see if the set activation values are of the right size and in HEX notation.
  bool                    check_set_keys();

  void                    set_state(RN_state state);

  // read all available data from serial until '\n'
  bool                    read_line();

  void                    set_timeout(unsigned long timeout);

  bool                    time_out_reached() const;

  void                    handle_reply_received();

  void                    clearSerialBuffer();

  // Read the internal status of the module
  // @retval true when update was successful
  bool updateStatus();

  bool saveUpdatedStatus();


  enum Active_cmd {
    none,
    TX,
    join,
    other
  };


  // OTAA values:
  String _deveui;
  String _appeui;
  String _appkey;

  // ABP values:
  String _nwkskey;
  String _appskey;
  String _devaddr;


  String _receivedData;              // Used as a receive buffer to collect replies from the module
  String _sendData;                  // Complete command to send to the module
  String _rxMessenge;                // Message received (during RX2 window) after a TX
  String _lastError;                 // Last error message received from module (or set by user)
  unsigned long _start_prep  = 0;    // timestamp of preparing command
  unsigned long _start       = 0;    // timestamp of last set timeout
  unsigned long _timeout     = 100;  // timeout duration
  uint32_t _rxdelay1         = 1000; // delay from last moment of sending to receive RX1 window
  uint32_t _rxdelay2         = 2000; // delay from last moment of sending to receive RX2 window
  uint8_t _busy_count        = 0;    // Number of times the module replied with "busy"
  uint8_t _retry_count       = 0;    // Number of retries of current TX command
  RN_state _state            = RN_state::idle;
  Active_cmd _processing_cmd = Active_cmd::none;
  bool _invalid_char_read    = false;
  bool _extensive_debug      = false; // Set this to true to log all steps in _lastError
  uint16_t _max_received_length = 0;


  RN2xx3_datatypes::Model _moduleType = RN2xx3_datatypes::Model::RN_NA;
  RN2xx3_datatypes::Freq_plan _fp     = RN2xx3_datatypes::Freq_plan::TTN_EU;
  uint8_t _sf                         = 7;
  uint8_t _dr                         = 5;

  bool _otaa      = true;  // Switch between OTAA or ABP activation (default OTAA)
  bool _asyncMode = false; // When set, the user must call async_loop() frequently

public:

  RN2xx3_status Status; // Cached result of "mac get status"
  Stream& _serial;      // The serial port used for this module.


  // Convenience functions

  int  readIntValue(const String& command);

  bool readUIntMacGet(const String& param,
                      uint32_t    & value);

  // All "mac set ..." commands return either "ok" or "invalid_param"
  bool sendMacSet(const String& param,
                  const String& value);
  bool sendMacSetEnabled(const String& param,
                         bool          enabled);
  bool sendMacSetCh(const String& param,
                    unsigned int  channel,
                    const String& value);
  bool sendMacSetCh(const String& param,
                    unsigned int  channel,
                    uint32_t      value);
  bool setChannelDutyCycle(unsigned int channel,
                           unsigned int dutyCycle);
  bool setChannelFrequency(unsigned int channel,
                           uint32_t     frequency);
  bool setChannelDataRateRange(unsigned int channel,
                               unsigned int minRange,
                               unsigned int maxRange);

  // Set channel enabled/disabled.
  // Frequency, data range, duty cycle must be issued prior to enabling the status of that channel
  bool setChannelEnabled(unsigned int channel,
                         bool         enabled);

  bool set2ndRecvWindow(unsigned int dataRate,
                        uint32_t     frequency);
  bool setAdaptiveDataRate(bool enabled);
  bool setAutomaticReply(bool enabled);
  bool setTXoutputPower(int pwridx);

  // Send the autobaud sequence to try and wake the module.
  void sendWakeSequence();
};


#endif // RN2XX3_TX_STATE_H
