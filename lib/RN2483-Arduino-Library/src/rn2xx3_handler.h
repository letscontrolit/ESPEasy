#ifndef RN2XX3_TX_STATE_H
#define RN2XX3_TX_STATE_H

#include "Arduino.h"

#include "RN2xx3_status.h"

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

  bool     exec_join(bool useOTAA);

  RN_state async_loop();

  // Wait for the command to be handled completely (including reply in RX2 window)
  RN_state wait_command_finished(unsigned long timeout = 10000);

  // Shorter wait, to be used in async mode.
  // This will return early when the message cannot be sent (e.g. due to duty cycle exceeded)
  // It will return when the state waiting for reply_received_rx2 has been reached, or the command has finished (due to error)
  RN_state wait_command_accepted(unsigned long timeout = 10000);

private:

  bool          command_finished() const;

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

  // delay from last moment of sending to receive RX1 and RX2 window
  bool          getRxDelayValues(uint32_t& rxdelay1,
                                 uint32_t& rxdelay2);

private:

  void set_state(RN_state state);

  // read all available data from serial until '\n'
  bool read_line();

  void set_timeout(unsigned long timeout);

  bool time_out_reached() const;

  void handle_reply_received();

  void clearSerialBuffer();

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

public:

  RN2xx3_status Status; // Cached result of "mac get status"
  Stream& _serial;      // The serial port used for this module.


  // Convenience functions


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
};


#endif // RN2XX3_TX_STATE_H
