#include "rn2xx3_handler.h"

#include "rn2xx3_received_types.h"


rn2xx3_handler::rn2xx3_handler(Stream& serial) : _serial(serial)
{
  clearSerialBuffer();
}

bool rn2xx3_handler::prepare_raw_command(const String& command)
{
  if (!_command_finished) {
    // Handling of another command has not finished.
    return false; 
  }
  _sendData = command;
  _processing_tx_command   = false;
  _processing_join_command = false;
  _busy_count              = 0;
  _retry_count             = 0;
  set_state(RN_state::command_set_to_send);

  // Set state may set _command_finished to true if no _sendData is set.
  return !_command_finished;
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
    tmpCommand += base16encode(data);
  }
  else
  {
    tmpCommand += data;
  }

  if (!prepare_raw_command(tmpCommand)) {
    return false;
  }
  _processing_tx_command = true;
  return true;
}

bool rn2xx3_handler::prepare_join_command(bool useOTAA) {
  if (!prepare_raw_command(useOTAA ? F("mac join otaa") : F("mac join abp")))
  {
    return false;
  }
  _processing_join_command = true;
  return true;
}

rn2xx3_handler::RN_state rn2xx3_handler::async_loop()
{
  if (_state != RN_state::must_pause) {
    if (!_command_finished && time_out_reached()) {
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
    case RN_state::timeout:
    case RN_state::max_attempt_reached:
    case RN_state::error:
    case RN_state::must_perform_init:
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

    if (command_finished() || get_state() == RN_state::wait_for_reply_rx2) { 
      return get_state(); 
    }
    delay(10);
  }
  return get_state();
}

bool rn2xx3_handler::command_finished() const
{
  return _command_finished;
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
  _lastError += '\n';
  _lastError += error;
}

rn2xx3_handler::RN_state rn2xx3_handler::get_state() const {
  return _state;
}

void rn2xx3_handler::set_state(rn2xx3_handler::RN_state state) {
  // FIXME TD-er: disabled for now, to see what is causing this.
  //  if (state == RN_state::must_perform_init) return;
  _state = state;
  bool old_command_finished = _command_finished;

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
        //
        // TODO: Compute exact time, for now just 2x rxdelay2
        set_timeout(2 * rxdelay2);
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
        _start_prep              = millis();
        _command_finished        = false;
        set_timeout(1500); // Needed for mac save
      }

      break;
    case RN_state::must_pause:
      set_timeout(1000);
      break;

    case RN_state::invalid_char_read:
      if (!_processing_tx_command && !_processing_join_command) {
        // Must retry to run the command again.
        set_state(RN_state::command_set_to_send);
      } else {
        _command_finished = true;
      }
      break;

    case RN_state::idle:

      // ToDo: Add support for sleep mode.
      // Clear the strings to free up some memory.
      _command_finished = true;
      _sendData         = "";
      _receivedData     = "";
      _rxMessenge       = "";
      _lastError        = "";
      break;
    case RN_state::timeout:
    case RN_state::max_attempt_reached:
    case RN_state::error:
    case RN_state::must_perform_init:
    case RN_state::duty_cycle_exceeded:

      // We cannot continue from this error
      _command_finished = true;
      break;
    case RN_state::tx_success:
    case RN_state::tx_success_with_rx:
    case RN_state::reply_received_finished:
    case RN_state::join_accepted:
      _command_finished = true;
      break;

      // Do not use default: here, so the compiler warns when a new state is not yet implemented here.
      //    default:
      //      break;
  }

  if (!old_command_finished && _command_finished) {
    _start                   = 0;
    _processing_tx_command   = false;
    _processing_join_command = false;
    _invalid_char_read       = false;
    _busy_count              = 0;
    _retry_count             = 0;
  }
}

bool rn2xx3_handler::read_line()
{
  while (_serial.available()) {
    int c = _serial.read();

    if (c >= 0) {
      const char character = static_cast<char>(c & 0xFF);
      if (!valid_char(character)) {
        _invalid_char_read = true;
        return false;
      }

      _receivedData += character;

      if (character == '\n') {
        return true;
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

String rn2xx3_handler::base16encode(const String& input_c)
{
  String input(input_c); // Make a deep copy to be able to do trim()

  input.trim();
  const size_t inputLength = input.length();
  String output;
  output.reserve(inputLength * 2);

  for (size_t i = 0; i < inputLength; ++i)
  {
    if (input[i] == '\0') { break; }

    char buffer[3];
    sprintf(buffer, "%02x", static_cast<int>(input[i]));
    output += buffer[0];
    output += buffer[1];
  }
  return output;
}

void rn2xx3_handler::clearSerialBuffer()
{
  while (_serial.available()) {
    _serial.read();
  }
}

bool rn2xx3_handler::valid_hex_char(char ch)
{
  return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f');
}

bool rn2xx3_handler::valid_char(char ch)
{
  switch (ch)
  {
    case '\n':
    case '\r':
    case ' ':
      return true;
  }
  return ch > 32 && ch < 127;
}

void rn2xx3_handler::handle_reply_received() {
  const RN2xx3_received_types::received_t received_datatype = RN2xx3_received_types::determineReceivedDataType(_receivedData);

  // Check if the reply is unexpected, so log the command + reply
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

      if (_processing_tx_command) {
        // TX commands are a lot longer, so do not include complete command
        _lastError += F("mac tx");
      } else {
        _lastError += _sendData;
      }
      _lastError += F(" -> ");
      _lastError += _receivedData;
      break;
  }


  switch (received_datatype) {
    case RN2xx3_received_types::UNKNOWN:

      // A reply which is not part of standard replies, so it can be a requested value.
      // Command is now finished.
      set_state(RN_state::reply_received_finished);
      break;
    case RN2xx3_received_types::ok:
    {
      if ((get_state() == RN_state::reply_received) && (_processing_tx_command || _processing_tx_command)) {
        // "mac tx" commands may receive a second response if the first one was "ok"
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
        delay(1000);
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
