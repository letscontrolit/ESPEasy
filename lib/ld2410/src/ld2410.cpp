/*
 *	An Arduino library for the Hi-Link LD2410 24Ghz FMCW radar sensor.
 *
 *  This sensor is a Frequency Modulated Continuous Wave radar, which makes it good for presence detection and its sensitivity at different
 *ranges to both static and moving targets can be configured.
 *
 *	The code in this library is based off the manufacturer datasheet and reading of this initial piece of work for ESPHome
 *https://github.com/rain931215/ESPHome-LD2410.
 *
 *	https://github.com/ncmreynolds/ld2410
 *
 *	Released under LGPL-2.1 see https://github.com/ncmreynolds/ld2410/LICENSE for full license
 *
 */
#ifndef ld2410_cpp
#define ld2410_cpp
#include "ld2410.h"


ld2410::ld2410()  // Constructor function
{}

ld2410::~ld2410() // Destructor function
{}

uint16_t ld2410::serial_to_int_(uint8_t index)
{
  return (int16_t)radar_data_frame_[index] + (radar_data_frame_[index + 1] << 8);
}

bool ld2410::debug_command_results_(
  #ifdef LD2410_DEBUG
  const char *title
  #endif
  ) {
  if (latest_command_success_)
  {
    radar_uart_last_packet_ = millis();
    #if defined(LD2410_DEBUG_COMMANDS) && defined(LD2410_DEBUG_PRINT) && defined(LD2410_DEBUG)

    if (debug_uart_ != nullptr)
    {
      debug_uart_->print("\n");
      debug_uart_->print(title);
      debug_uart_->print(" OK\n");
    }
    #endif // if defined(LD2410_DEBUG_COMMANDS) && defined(LD2410_DEBUG_PRINT) && defined(LD2410_DEBUG)
    return true;
  }
  else
  {
  	#ifdef LD2410_DEBUG

    if (debug_uart_ != nullptr)
    {
      debug_uart_->print("\n");
      debug_uart_->print(title);
      debug_uart_->print(" Failed\n");
    }
  	#endif // ifdef LD2410_DEBUG
    return false;
  }
}

bool ld2410::wait_for_command_ack_(uint8_t command) {
  while (millis() - radar_uart_last_command_ < radar_uart_command_timeout_)
  {
    if (read_frame_())
    {
      if (latest_ack_ == command)
      {
        bool rcode = latest_command_success_;
        delay(50);
        leave_configuration_mode_();
        return rcode;
      }
    }
  }
  return false;
}

bool ld2410::begin(Stream& radarStream, bool waitForRadar)      {
  radar_uart_ = &radarStream; // Set the stream used for the LD2410
  #ifdef LD2410_DEBUG

  if (debug_uart_ != nullptr)
  {
    debug_uart_->println(F("ld2410 started"));
  }
  #endif // ifdef LD2410_DEBUG

  if (waitForRadar)
  {
    if (requestRestart()) {
      #ifdef LD2410_DEBUG

      if (debug_uart_ != nullptr) {
        debug_uart_->print(F("\nLD2410 Reset: Ok"));
      }
    } else {
      if (debug_uart_ != nullptr) {
        debug_uart_->print(F("\nLD2410 Reset: No response"));
      }
      #endif // ifdef LD2410_DEBUG
    }
    delay(1500); // allow time for sensor to restart
    #ifdef LD2410_DEBUG

    if (debug_uart_ != nullptr)
    {
      debug_uart_->print(F("\nLD2410 Requesting Configuration: "));
    }
    #endif // ifdef LD2410_DEBUG

    if (requestCurrentConfiguration())
    {
      #ifdef LD2410_DEBUG

      if (debug_uart_ != nullptr)
      {
        debug_uart_->print(F("\nMax gate distance: "));
        debug_uart_->print(max_gate);
        debug_uart_->print(F("\nMax motion detecting gate distance: "));
        debug_uart_->print(max_moving_gate);
        debug_uart_->print(F("\nMax stationary detecting gate distance: "));
        debug_uart_->print(max_stationary_gate);
        debug_uart_->print(F("\nSensitivity per gate"));

        for (uint8_t i = 0; i < sizeof(stationary_sensitivity); ++i)
        {
          debug_uart_->print(F("\nGate "));
          debug_uart_->print(i);
          debug_uart_->print(F(" ("));
          debug_uart_->print(i * 0.75);
          debug_uart_->print('-');
          debug_uart_->print((i + 1) * 0.75);
          debug_uart_->print(F(" metres) Motion: "));
          debug_uart_->print(motion_sensitivity[i]);
          debug_uart_->print(F(" Stationary: "));
          debug_uart_->print(stationary_sensitivity[i]);
        }
        debug_uart_->print(F("\nSensor idle timeout: "));
        debug_uart_->print(sensor_idle_time);
        debug_uart_->println('s');
      }
      #endif // ifdef LD2410_DEBUG
      return true;
    }
    #ifdef LD2410_DEBUG
    else
    {
      if (debug_uart_ != nullptr)
      {
        debug_uart_->print(F("no response"));
      }
    }
    #endif // ifdef LD2410_DEBUG
  }
  else
  {
    #ifdef LD2410_DEBUG

    if (debug_uart_ != nullptr)
    {
      debug_uart_->print(F("\nLD2410 library configured"));
    }
    #endif // ifdef LD2410_DEBUG
    return true;
  }
  return false;
}

#ifdef LD2410_DEBUG
void ld2410::debug(Stream& terminalStream)
{
  debug_uart_ = &terminalStream; // Set the stream used for the terminal
  # if defined(ESP8266)

  if (&terminalStream == &Serial)
  {
    if (debug_uart_ != nullptr) {
      debug_uart_->write(17); // Send an XON to stop the hung terminal after reset on ESP8266
    }
  }
  # endif // if defined(ESP8266)
}

#endif // ifdef LD2410_DEBUG

bool ld2410::isConnected()
{
  if (millis() - radar_uart_last_packet_ < radar_uart_timeout) // Use the last reading
  {
    return true;
  }
  return read_frame_(); // Try and read a frame if the current reading is too old
}

bool ld2410::stationaryTargetDetected()
{
  return (target_type_ & TARGET_STATIONARY);
}

bool ld2410::movingTargetDetected()
{
  return (target_type_ & TARGET_MOVING);
}

String ld2410::cmdFirmwareVersion() {
  String sVersion;

  sVersion  = 'v';
  sVersion += firmware_major_version;
  sVersion += '.';
  sVersion += firmware_minor_version;
  sVersion += '.';
  sVersion += String(firmware_bugfix_version, HEX);

  return sVersion;
}

/* Command / Response / Protocol Frame
 *
 * REQUEST
 * FD FC FB FA -- Header
 *       dd dd -- Frame data length
 *       dd dd -- Command Word
 *         ... -- Command Value nBytes
 * 04 03 02 01 -- Footer
 *
 * RESPONSE
 * FD FC FB FA -- Header
 *       dd dd -- Frame data length
 *       dd dd -- ACK Word
 *         ... -- Response Values nBytes
 * 04 03 02 01 -- Footer
 */
bool ld2410::isProtocolDataFrame_() {
  return radar_data_frame_[0]                              == FRAME_PREFIX_PROTOCOL &&
         radar_data_frame_[1]                              == 0xFC &&
         radar_data_frame_[2]                              == 0xFB &&
         radar_data_frame_[3]                              == 0xFA &&
         radar_data_frame_[radar_data_frame_position_ - 4] == 0x04 &&
         radar_data_frame_[radar_data_frame_position_ - 3] == 0x03 &&
         radar_data_frame_[radar_data_frame_position_ - 2] == 0x02 &&
         radar_data_frame_[radar_data_frame_position_ - 1] == 0x01
  ;
}

/* Data Frame
 *
 * F4 F3 F2 F1 -- header
 *       dd dd -- frame data length
 *          dd -- Type of Data (0x01=Engineering data, 0x02=Target data)
 *        0xAA -- Marker
 *         ... -- target state
 *         ... -- reporting data
 *        0x55 -- Marker
 *        0x00 -- Check flag
 * F8 F7 F6 F5  - Footer
 */
bool ld2410::isReportingDataFrame_() {
  return radar_data_frame_[0]                              == FRAME_PREFIX_REPORTING &&
         radar_data_frame_[1]                              == 0xF3 &&
         radar_data_frame_[2]                              == 0xF2 &&
         radar_data_frame_[3]                              == 0xF1 &&
         radar_data_frame_[radar_data_frame_position_ - 4] == 0xF8 &&
         radar_data_frame_[radar_data_frame_position_ - 3] == 0xF7 &&
         radar_data_frame_[radar_data_frame_position_ - 2] == 0xF6 &&
         radar_data_frame_[radar_data_frame_position_ - 1] == 0xF5
  ;
}

bool ld2410::read_frame_()
{
  if (!radar_uart_->available())
  {
    return false;
  }
  const uint32_t _started = millis();
  int _available          = radar_uart_->available();

  while (_available && (millis() - _started < SERIAL_RECEIVE_MAX_MS)) { // Read for max. N msec
    delay(0);

    if (frame_started_ == false)
    {
      uint8_t byte_read_ = radar_uart_->read();
      --_available; // 1 down

      if (byte_read_ == FRAME_PREFIX_REPORTING)
      {
        radar_data_frame_[radar_data_frame_position_++] = byte_read_;
        frame_started_                                  = true;
        ack_frame_                                      = false;
      }
      else if (byte_read_ == FRAME_PREFIX_PROTOCOL)
      {
        radar_data_frame_[radar_data_frame_position_++] = byte_read_;
        frame_started_                                  = true;
        ack_frame_                                      = true;
      }
      #if (defined(LD2410_DEBUG_DATA) || defined(LD2410_DEBUG_COMMANDS)) && defined(LD2410_DEBUG)

      if (debug_uart_ != nullptr)
      {
        debug_uart_->print(F("\nRcvd : 00 "));
      }
      #endif // if (defined(LD2410_DEBUG_DATA) || defined(LD2410_DEBUG_COMMANDS)) && defined(LD2410_DEBUG)
    }
    else
    {
      if (radar_data_frame_position_ < configuration_buffer_size_)
      {
        #if (defined(LD2410_DEBUG_DATA) || defined(LD2410_DEBUG_COMMANDS)) && defined(LD2410_DEBUG)

        if (debug_uart_ != nullptr)
        {
          if (radar_data_frame_position_ < 0x10)
          {
            debug_uart_->print('0');
          }
          debug_uart_->print(radar_data_frame_position_, HEX);
          debug_uart_->print(' ');
        }
        #endif // if (defined(LD2410_DEBUG_DATA) || defined(LD2410_DEBUG_COMMANDS)) && defined(LD2410_DEBUG)
        radar_data_frame_[radar_data_frame_position_++] = radar_uart_->read();
        --_available;                       // 1 more down

        if (radar_data_frame_position_ > 7) // Can check for start and end
        {
          if (isReportingDataFrame_())
          {
            if (parse_data_frame_())
            {
              #if defined(LD2410_DEBUG_DATA) && defined(LD2410_DEBUG)

              if (debug_uart_ != nullptr)
              {
                debug_uart_->print(F(" parsed data OK"));
              }
              #endif // if defined(LD2410_DEBUG_DATA) && defined(LD2410_DEBUG)
              frame_started_             = false;
              radar_data_frame_position_ = 0;
              return true;
            }
            else
            {
              #if defined(LD2410_DEBUG_DATA) && defined(LD2410_DEBUG)

              if (debug_uart_ != nullptr)
              {
                debug_uart_->print(F(" failed to parse data"));
              }
              #endif // if defined(LD2410_DEBUG_DATA) && defined(LD2410_DEBUG)
              frame_started_             = false;
              radar_data_frame_position_ = 0;
              _errorCount++;
            }
          }
          else if (isProtocolDataFrame_())
          {
            if (parse_command_frame_())
            {
              #if defined(LD2410_DEBUG_COMMANDS) && defined(LD2410_DEBUG)

              if (debug_uart_ != nullptr)
              {
                debug_uart_->print(F(" parsed command OK"));
              }
              #endif // if defined(LD2410_DEBUG_COMMANDS) && defined(LD2410_DEBUG)
              frame_started_             = false;
              radar_data_frame_position_ = 0;
              return true;
            }
            else
            {
              #if defined(LD2410_DEBUG_COMMANDS) && defined(LD2410_DEBUG)

              if (debug_uart_ != nullptr)
              {
                debug_uart_->print(F(" failed to parse command"));
              }
              #endif // if defined(LD2410_DEBUG_COMMANDS) && defined(LD2410_DEBUG)
              frame_started_             = false;
              radar_data_frame_position_ = 0;
              _errorCount++;
            }
          }
        }
      }
      else
      {
        #if (defined(LD2410_DEBUG_DATA) || defined(LD2410_DEBUG_COMMANDS) || defined(LD2410_DEBUG_PARSE)) && \
        defined(LD2410_DEBUG)

        if (debug_uart_ != nullptr)
        {
          debug_uart_->print(F("\nLD2410 frame overran"));
        }
        #endif // if (defined(LD2410_DEBUG_DATA) || defined(LD2410_DEBUG_COMMANDS) || defined(LD2410_DEBUG_PARSE)) && defined(LD2410_DEBUG)
        frame_started_             = false;
        radar_data_frame_position_ = 0;
        _errorCount++;
      }
    }

    if (!_available && (millis() - _started < SERIAL_RECEIVE_GRACE_MS)) { // Data handled and time left? Read some more
      _available = radar_uart_->available();
    }
  }
  return false;
}

uint16_t ld2410::getErrorCountAndReset() {
  uint16_t result = _errorCount;

  _errorCount = 0;
  return result;
}

void ld2410::print_frame_()
{
  #ifdef LD2410_DEBUG

  if (debug_uart_ != nullptr)
  {
    if (ack_frame_ == true)
    {
      debug_uart_->print(F("\nCmnd : "));
    }
    else
    {
      debug_uart_->print(F("\nData : "));
    }

    for (uint8_t i = 0; i < radar_data_frame_position_; ++i)
    {
      if (radar_data_frame_[i] < 0x10)
      {
        debug_uart_->print('0');
      }
      debug_uart_->print(radar_data_frame_[i], HEX);
      debug_uart_->print(' ');
    }
  }
  #endif // ifdef LD2410_DEBUG
}

bool ld2410::parse_data_frame_()
{
  uint16_t intra_frame_data_length_ = serial_to_int_(4); // radar_data_frame_[4] + (radar_data_frame_[5] << 8);

  if (radar_data_frame_position_ != intra_frame_data_length_ + 10)
  {
    #if defined(LD2410_DEBUG_DATA) && defined(LD2410_DEBUG)

    if (debug_uart_ != nullptr)
    {
      debug_uart_->print(F("\nFrame length unexpected: "));
      debug_uart_->print(radar_data_frame_position_);
      debug_uart_->print(F(" not "));
      debug_uart_->print(intra_frame_data_length_ + 10);
    }
    #endif // if defined(LD2410_DEBUG_DATA) && defined(LD2410_DEBUG)
    _errorCount++;
    return false;
  }

  #if (defined(LD2410_DEBUG_DATA) || defined(LD2410_DEBUG_COMMANDS)) && defined(LD2410_DEBUG)

  if (debug_uart_ != nullptr)
  {
    print_frame_();
  }
  #endif // if (defined(LD2410_DEBUG_DATA) || defined(LD2410_DEBUG_COMMANDS)) && defined(LD2410_DEBUG)
  data_ready_ = false;

  if ((radar_data_frame_[6] == FRAME_TYPE_REPORTING) && (radar_data_frame_[7] == FRAME_TYPE_FLAG)) // Engineering mode data
  {
    /*   (Protocol) Target Data Reporting
     * 02 AA         d6,7     data type (target data)
     * 02            d8       target type (stationary target)
     * 51 00         d9,10    stationary target distance
     * 00            d11      stationary target energy
     * 00 00         d12,13   moving target distance
     * 3B            d14      moving target energy
     * 00 00         d15,16   distance detection

       Engineering
     * 08        d17       Max moving distance gate
     * 08        d18       Max static distance gate
     * 3C 22 05 03 03 04 03 06 05       d19,27 Movement distance gate energy
     * 00 00 39 10 13 06 06 08 04       d28,36 Static distance gate energy
     * 03 05     d37,d38    ?? v1283 d37 = lightsensor data, d38 = out pin sensor
     * 55 00     d39,40    Frame flag
     */
    engineering_mode_           = true;
    target_type_                = radar_data_frame_[8];
    moving_target_distance_     = serial_to_int_(9);
    moving_target_energy_       = radar_data_frame_[11];
    stationary_target_distance_ = serial_to_int_(12);
    stationary_target_energy_   = radar_data_frame_[14];
    detection_distance_         = serial_to_int_(15);

    max_moving_distance_gate = radar_data_frame_[17];
    max_static_distance_gate = radar_data_frame_[18];
    light_sensor_data_       = radar_data_frame_[37];
    output_pin_data_         = radar_data_frame_[38];

    uint8_t pos = 19;

    // motion_energy
    for (uint8_t gate = 0; gate < sizeof(movement_distance_gate_energy); ++gate) {
      movement_distance_gate_energy[gate] = radar_data_frame_[pos++];
    }

    // stationary_engergy
    for (uint8_t gate = 0; gate < sizeof(static_distance_gate_engergy); ++gate) {
      static_distance_gate_engergy[gate] = radar_data_frame_[pos++];
    }
    engineering_retain_data_ = serial_to_int_(pos); // radar_data_frame_[pos++] + (radar_data_frame_[pos] << 8); // maybe

    #if defined(LD2410_DEBUG_PARSE) && defined(LD2410_DEBUG)

    if (debug_uart_ != nullptr)
    {
      debug_uart_->print(F("\nEngineering data - "));

      if (target_type_ == TARGET_NONE)
      {
        debug_uart_->print(F(" no target"));
      }
      else if (target_type_ == TARGET_MOVING)
      {
        debug_uart_->print(F(" moving target:"));
      }
      else if (target_type_ == TARGET_STATIONARY)
      {
        debug_uart_->print(F(" stationary target:"));
      }
      else if (target_type_ == TARGET_MOVING_AND_STATIONARY)
      {
        debug_uart_->print(F(" moving & stationary targets:"));
      }
      debug_uart_->print(F(" moving at "));
      debug_uart_->print(moving_target_distance_);
      debug_uart_->print(F("cm power "));
      debug_uart_->print(moving_target_energy_);

      debug_uart_->print(F(" max moving distance gate:"));
      debug_uart_->print(max_moving_distance_gate);
      debug_uart_->print(F(" max static distance gate:"));
      debug_uart_->print(max_static_distance_gate);
      debug_uart_->print(F(" moving/static distance gate energy: "));

      for (uint8_t gate = 0; gate < sizeof(movement_distance_gate_energy); ++gate) {
        debug_uart_->print(gate);
        debug_uart_->print(": [");
        debug_uart_->print(movement_distance_gate_energy[gate]);
        debug_uart_->print(",");
        debug_uart_->print(static_distance_gate_engergy[gate]);
        debug_uart_->print("] ");
      }
      debug_uart_->print("\n");
    }
    #endif // if defined(LD2410_DEBUG_PARSE) && defined(LD2410_DEBUG)

    radar_uart_last_packet_ = millis();
    data_ready_             = true;
    return true;
  }
  else if ((radar_data_frame_[6] == FRAME_TYPE_TARGET) && (radar_data_frame_[7] == FRAME_TYPE_FLAG)) // Normal target data
  {
    // moving_target_distance_ = radar_data_frame_[9] + (radar_data_frame_[10] << 8);
    // stationary_target_distance_ = radar_data_frame_[12] + (radar_data_frame_[13] << 8);
    engineering_mode_           = false;
    target_type_                = radar_data_frame_[8];
    moving_target_distance_     = serial_to_int_(9);
    moving_target_energy_       = radar_data_frame_[11];
    stationary_target_distance_ = serial_to_int_(12);
    stationary_target_energy_   = radar_data_frame_[14];
    detection_distance_         = serial_to_int_(15);
    #if defined(LD2410_DEBUG_PARSE) && defined(LD2410_DEBUG)

    if (debug_uart_ != nullptr)
    {
      debug_uart_->print(F("\nNormal data - "));

      if (target_type_ == TARGET_NONE)
      {
        debug_uart_->print(F(" no target"));
      }
      else if (target_type_ == TARGET_MOVING)
      {
        debug_uart_->print(F(" moving target:"));
      }
      else if (target_type_ == TARGET_STATIONARY)
      {
        debug_uart_->print(F(" stationary target:"));
      }
      else if (target_type_ == TARGET_MOVING_AND_STATIONARY)
      {
        debug_uart_->print(F(" moving & stationary targets:"));
      }

      if (radar_data_frame_[8] & TARGET_MOVING)
      {
        debug_uart_->print(F(" moving at "));
        debug_uart_->print(moving_target_distance_);
        debug_uart_->print(F("cm power "));
        debug_uart_->print(moving_target_energy_);
      }

      if (radar_data_frame_[8] & TARGET_STATIONARY)
      {
        debug_uart_->print(F(" stationary at "));
        debug_uart_->print(stationary_target_distance_);
        debug_uart_->print(F("cm power "));
        debug_uart_->print(stationary_target_energy_);
      }
    }
    #endif // if defined(LD2410_DEBUG_PARSE) && defined(LD2410_DEBUG)
    radar_uart_last_packet_ = millis();
    data_ready_             = true;
    return true;
  }
  else
  {
    #if (defined(LD2410_DEBUG_DATA) || defined(LD2410_DEBUG_COMMANDS) || defined(LD2410_DEBUG_PARSE)) && defined(LD2410_DEBUG)

    if (debug_uart_ != nullptr)
    {
      debug_uart_->print(F("\nUnknown frame type"));
    }
    #endif // if (defined(LD2410_DEBUG_DATA) || defined(LD2410_DEBUG_COMMANDS) || defined(LD2410_DEBUG_PARSE)) &&
                       // defined(LD2410_DEBUG)
    _errorCount++;
    print_frame_();
  }

  return false;
}

bool ld2410::parse_command_frame_()
{

  #if defined(LD2410_DEBUG_COMMANDS) && defined(LD2410_DEBUG)
  uint16_t intra_frame_data_length_ = serial_to_int_(4); // radar_data_frame_[4] + (radar_data_frame_[5] << 8);

  if (debug_uart_ != nullptr)
  {
    print_frame_();
    debug_uart_->print(F("\nACK frame payload: "));
    debug_uart_->print(intra_frame_data_length_);
    debug_uart_->print(F(" bytes"));
  }
  #endif // if defined(LD2410_DEBUG_COMMANDS) && defined(LD2410_DEBUG)
  latest_ack_             = radar_data_frame_[6];
  latest_command_success_ = (radar_data_frame_[8] == 0x00 && radar_data_frame_[9] == 0x00);

  switch (latest_ack_)
  {
    case CMD_CONFIGURATION_ENABLE:

      if (latest_command_success_)
      {
        configuration_protocol_version_ = serial_to_int_(10); // radar_data_frame_[10] + (radar_data_frame_[11] << 8);
        configuration_buffer_size_      = serial_to_int_(12); // radar_data_frame_[12] + (radar_data_frame_[13] << 8);
      }
      return debug_command_results_(
        #ifdef LD2410_DEBUG
        "ACK for entering configuration mode"
        #endif
        );
    case CMD_CONFIGURATION_END:
      return debug_command_results_(
        #ifdef LD2410_DEBUG
        "ACK for leaving configuration mode"
        #endif
        );
    case CMD_MAX_DISTANCE_AND_UNMANNED_DURATION:
      return debug_command_results_(
        #ifdef LD2410_DEBUG
        "ACK for setting max values"
        #endif
        );
    case CMD_READ_PARAMETER:

      if (latest_command_success_)
      {
        max_gate                  = radar_data_frame_[11];
        max_moving_gate           = radar_data_frame_[12];
        max_stationary_gate       = radar_data_frame_[13];
        // Leave optimization to the compiler...
        for (uint8_t n = 0; n < LD2410_MAX_GATES; ++n) {
          motion_sensitivity[n] = radar_data_frame_[14 + n];
        }
        // motion_sensitivity[0]     = radar_data_frame_[14];
        // motion_sensitivity[1]     = radar_data_frame_[15];
        // motion_sensitivity[2]     = radar_data_frame_[16];
        // motion_sensitivity[3]     = radar_data_frame_[17];
        // motion_sensitivity[4]     = radar_data_frame_[18];
        // motion_sensitivity[5]     = radar_data_frame_[19];
        // motion_sensitivity[6]     = radar_data_frame_[20];
        // motion_sensitivity[7]     = radar_data_frame_[21];
        // motion_sensitivity[8]     = radar_data_frame_[22];
        for (uint8_t n = 0; n < LD2410_MAX_GATES; ++n) {
          stationary_sensitivity[n] = radar_data_frame_[23 + n];
        }
        // stationary_sensitivity[0] = radar_data_frame_[23];
        // stationary_sensitivity[1] = radar_data_frame_[24];
        // stationary_sensitivity[2] = radar_data_frame_[25];
        // stationary_sensitivity[3] = radar_data_frame_[26];
        // stationary_sensitivity[4] = radar_data_frame_[27];
        // stationary_sensitivity[5] = radar_data_frame_[28];
        // stationary_sensitivity[6] = radar_data_frame_[29];
        // stationary_sensitivity[7] = radar_data_frame_[30];
        // stationary_sensitivity[8] = radar_data_frame_[31];
        sensor_idle_time          = serial_to_int_(32); // radar_data_frame_[32];
        #if defined(LD2410_DEBUG_COMMANDS) && defined(LD2410_DEBUG)

        if (debug_uart_ != nullptr)
        {
          debug_uart_->print(F("\nMax gate distance: "));
          debug_uart_->print(max_gate);
          debug_uart_->print(F("\nMax motion detecting gate distance: "));
          debug_uart_->print(max_moving_gate);
          debug_uart_->print(F("\nMax stationary detecting gate distance: "));
          debug_uart_->print(max_stationary_gate);
          debug_uart_->print(F("\nSensitivity per gate"));

          for (uint8_t i = 0; i < sizeof(stationary_sensitivity); ++i)
          {
            debug_uart_->print(F("\nGate "));
            debug_uart_->print(i);
            debug_uart_->print(F(" ("));
            debug_uart_->print(i * 0.75);
            debug_uart_->print('-');
            debug_uart_->print((i + 1) * 0.75);
            debug_uart_->print(F(" metres) Motion: "));
            debug_uart_->print(motion_sensitivity[i]);
            debug_uart_->print(F(" Stationary: "));
            debug_uart_->print(stationary_sensitivity[i]);
          }
          debug_uart_->print(F("\nSensor idle timeout: "));
          debug_uart_->print(sensor_idle_time);
          debug_uart_->print('s');
        }
        #endif // if defined(LD2410_DEBUG_COMMANDS) && defined(LD2410_DEBUG)
      } else {
        _errorCount++;
      }
      return debug_command_results_(
        #ifdef LD2410_DEBUG
        "ACK for current configuration"
        #endif
        );
    case CMD_ENGINEERING_ENABLE:
      return debug_command_results_(
        #ifdef LD2410_DEBUG
        "ACK for enable engineering mode"
        #endif
        );
    case CMD_ENGINEERING_END:
      return debug_command_results_(
        #ifdef LD2410_DEBUG
        "ACK for end engineering mode"
        #endif
        );
    case CMD_RANGE_GATE_SENSITIVITY:
      return debug_command_results_(
        #ifdef LD2410_DEBUG
        "ACK for setting sensitivity values"
        #endif
        );
    case CMD_READ_FIRMWARE_VERSION:

      if (latest_command_success_)
      {
        firmware_major_version   = radar_data_frame_[13];
        firmware_minor_version   = radar_data_frame_[12];
        firmware_bugfix_version  = radar_data_frame_[14];
        firmware_bugfix_version += radar_data_frame_[15] << 8;
        firmware_bugfix_version += radar_data_frame_[16] << 16;
        firmware_bugfix_version += radar_data_frame_[17] << 24;
      }
      return debug_command_results_(
        #ifdef LD2410_DEBUG
        "ACK for firmware version"
        #endif
        );
    case CMD_SET_SERIAL_PORT_BAUD:
      return debug_command_results_(
        #ifdef LD2410_DEBUG
        "ACK for setting serial baud rate"
        #endif
        );
    case CMD_FACTORY_RESET:
      return debug_command_results_(
        #ifdef LD2410_DEBUG
        "ACK for factory reset"
        #endif
        );
    case CMD_RESTART:
      return debug_command_results_(
        #ifdef LD2410_DEBUG
        "ACK for restart"
        #endif
        );
    default:
      #if (defined(LD2410_DEBUG_DATA) || defined(LD2410_DEBUG_COMMANDS) || defined(LD2410_DEBUG_PARSE)) && defined(LD2410_DEBUG)

      if (debug_uart_ != nullptr)
      {
        debug_uart_->print(F("\nUnknown command response"));
        print_frame_();
      }
      #endif // if (defined(LD2410_DEBUG_DATA) || defined(LD2410_DEBUG_COMMANDS) || defined(LD2410_DEBUG_PARSE)) &&
                               // defined(LD2410_DEBUG)
      return false;
  }
  return false;
}

/*
 * Specification request command be executed
 * inside configuraiton mode wrapper.  If command fails
 * exit-configuration mode is not required.
 */
void ld2410::send_command_preamble_()
{
  // Command preamble
  const uint8_t preamble [] {0xFD,0xFC,0xFB,0xFA};
  for (uint8_t n = 0; n < 4; ++n) {
    radar_uart_->write(preamble[n]);
  }
  // radar_uart_->write((uint8_t)char(0xFD));
  // radar_uart_->write((uint8_t)char(0xFC));
  // radar_uart_->write((uint8_t)char(0xFB));
  // radar_uart_->write((uint8_t)char(0xFA));
}

void ld2410::send_command_postamble_()
{
  // Command end
  const uint8_t postamble [] { 0x04, 0x03, 0x02, 0x01 };
  for (uint8_t n = 0; n < 4; ++n) {
    radar_uart_->write(postamble[n]);
  }
  // radar_uart_->write((uint8_t)char(0x04));
  // radar_uart_->write((uint8_t)char(0x03));
  // radar_uart_->write((uint8_t)char(0x02));
  // radar_uart_->write((uint8_t)char(0x01));
  radar_uart_->flush();
}

void ld2410::send_2byte_command(uint8_t cmd_byte) {
  uint8_t cmd_array [] = { 0x02, 0x00, 0xFF, 0x00 };
  cmd_array[2] = cmd_byte;
  for (uint8_t n = 0; n < 4u; ++n) {
    radar_uart_->write(cmd_array[n]);
  }
}

void ld2410::send_4byte_command(uint8_t cmd_byte,
                                uint8_t val_byte) {
  uint8_t cmd_array [] = { 0x04, 0x00, 0xFF, 0x00, 0xFF, 0x00 };
  cmd_array[2] = cmd_byte;
  cmd_array[4] = val_byte;
  for (uint8_t n = 0; n < 6u; ++n) {
    radar_uart_->write(cmd_array[n]);
  }
}

/*
 * Wrapper to enable configuration mode for
 * multiple command execution
 */
bool ld2410::requestConfigurationModeBegin() {
  if (configuration_mode_active) { // guard
    return true;
  }
  configuration_mode_active = enter_configuration_mode_();
  return configuration_mode_active;
}

/*
 * Wrapper to disable configuration mode for
 * multiple command execution
 */
bool ld2410::requestConfigurationModeEnd() {
  if (!configuration_mode_active) { // guard
    return true;
  }
  configuration_mode_active = false;
  configuration_mode_active = !leave_configuration_mode_();
  return configuration_mode_active;
}

/*
 * Configuration mode is required to be issued before
 * any command execution.  Multiple commands can be issued
 * once configuraiton mode is enabled.  When complete close with
 * leave_configuration_mode();
 *
 * Configuration mode is cancelled on any error by any
 * given command, and leave is NOT required.
 */
bool ld2410::enter_configuration_mode_()
{
  if (configuration_mode_active) {
    return true;
  }
  send_command_preamble_();

  // Request
  send_4byte_command(CMD_CONFIGURATION_ENABLE, 0x01); // Request enter command mode
  // radar_uart_->write((uint8_t)char(0x04));                     // Command is four bytes long
  // radar_uart_->write((uint8_t)char(0x00));
  // radar_uart_->write((uint8_t)char(CMD_CONFIGURATION_ENABLE)); // Request enter command mode
  // radar_uart_->write((uint8_t)char(0x00));
  // radar_uart_->write((uint8_t)char(0x01));
  // radar_uart_->write((uint8_t)char(0x00));
  send_command_postamble_();
  radar_uart_last_command_ = millis();

  while (millis() - radar_uart_last_command_ < radar_uart_command_timeout_)
  {
    if (read_frame_())
    {
      if (latest_ack_ == CMD_CONFIGURATION_ENABLE)
      {
        return latest_command_success_;
      }
    }
  }
  return false;
}

bool ld2410::leave_configuration_mode_()
{
  if (configuration_mode_active) {
    return true;
  }
  send_command_preamble_();

  // Request firmware
  send_2byte_command(CMD_CONFIGURATION_END); // Request leave command mode
  // radar_uart_->write((uint8_t)char(0x02));                  // Command is two bytes long
  // radar_uart_->write((uint8_t)char(0x00));
  // radar_uart_->write((uint8_t)char(CMD_CONFIGURATION_END)); // Request leave command mode
  // radar_uart_->write((uint8_t)char(0x00));
  send_command_postamble_();
  radar_uart_last_command_ = millis();

  while (millis() - radar_uart_last_command_ < radar_uart_command_timeout_)
  {
    if (read_frame_())
    {
      if (latest_ack_ == CMD_CONFIGURATION_END)
      {
        return latest_command_success_;
      }
    }
  }
  return false;
}

bool ld2410::requestStartEngineeringMode()
{
  if (enter_configuration_mode_())
  {
    delay(50);
    send_command_preamble_();

    // Request firmware
    send_2byte_command(CMD_ENGINEERING_ENABLE); // Request enter engineering mode
    // radar_uart_->write((uint8_t)char(0x02));                   // Command is two bytes long
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(CMD_ENGINEERING_ENABLE)); // Request enter engineering mode
    // radar_uart_->write((uint8_t)char(0x00));
    send_command_postamble_();
    radar_uart_last_command_ = millis();
    return wait_for_command_ack_(CMD_ENGINEERING_ENABLE);
  }
  return false;
}

bool ld2410::requestEndEngineeringMode()
{
  if (enter_configuration_mode_())
  {
    delay(50);
    enter_configuration_mode_();
    send_command_preamble_();

    // Request firmware
    send_2byte_command(CMD_ENGINEERING_END); // Request leave engineering mode
    // radar_uart_->write((uint8_t)char(0x02));                // Command is two bytes long
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(CMD_ENGINEERING_END)); // Request leave engineering mode
    // radar_uart_->write((uint8_t)char(0x00));
    send_command_postamble_();
    radar_uart_last_command_ = millis();
    return wait_for_command_ack_(CMD_ENGINEERING_END);
  }
  return false;
}

bool ld2410::requestCurrentConfiguration()
{
  if (enter_configuration_mode_())
  {
    delay(50);
    send_command_preamble_();

    // Request firmware
    send_2byte_command(CMD_READ_PARAMETER); // Request current configuration
    // radar_uart_->write((uint8_t)char(0x02));               // Command is two bytes long
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(CMD_READ_PARAMETER)); // Request current configuration
    // radar_uart_->write((uint8_t)char(0x00));
    send_command_postamble_();
    radar_uart_last_command_ = millis();
    return wait_for_command_ack_(CMD_READ_PARAMETER);
  }
  return false;
}

bool ld2410::requestFirmwareVersion()
{
  if (enter_configuration_mode_())
  {
    delay(50);
    send_command_preamble_();

    // Request firmware
    send_2byte_command(CMD_READ_FIRMWARE_VERSION); // Request firmware version
    // radar_uart_->write((uint8_t)char(0x02));                      // Command is two bytes long
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(CMD_READ_FIRMWARE_VERSION)); // Request firmware version
    // radar_uart_->write((uint8_t)char(0x00));
    send_command_postamble_();
    radar_uart_last_command_ = millis();
    return wait_for_command_ack_(CMD_READ_FIRMWARE_VERSION);
  }
  return false;
}

bool ld2410::requestRestart()
{
  if (enter_configuration_mode_())
  {
    delay(50);
    send_command_preamble_();

    // Request firmware
    send_2byte_command(CMD_RESTART); // Request restart
    // radar_uart_->write((uint8_t)char(0x02));        // Command is two bytes long
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(CMD_RESTART)); // Request restart
    // radar_uart_->write((uint8_t)char(0x00));
    send_command_postamble_();
    radar_uart_last_command_ = millis();
    return wait_for_command_ack_(CMD_RESTART);
  }
  return false;
}

bool ld2410::requestFactoryReset()
{
  if (enter_configuration_mode_())
  {
    delay(50);
    send_command_preamble_();

    // Request firmware
    send_2byte_command(CMD_FACTORY_RESET); // Request factory reset
    // radar_uart_->write((uint8_t)char(0x02));              // Command is two bytes long
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(CMD_FACTORY_RESET)); // Request factory reset
    // radar_uart_->write((uint8_t)char(0x00));
    send_command_postamble_();
    radar_uart_last_command_ = millis();
    return wait_for_command_ack_(CMD_FACTORY_RESET);
  }
  return false;
}

/*
 * Serial Speed Choices: default is 7
 * 1 =   9600
 * 2 =  19200
 * 3 =  38400
 * 4 =  57600
 * 5 = 115200
 * 6 = 230400
 * 7 = 256000
 * 8 = 460800
 */
bool ld2410::setSerialBaudRate(uint8_t cSpeed)
{
  if ((cSpeed < 0) || (cSpeed > LD2410_MAX_GATES)) {
    return false;
  }

  if (enter_configuration_mode_())
  {
    delay(50);
    send_command_preamble_();

    // Serial baud Rate
    send_4byte_command(CMD_SET_SERIAL_PORT_BAUD, cSpeed);
    // radar_uart_->write((uint8_t)char(0x04)); // Command is four bytes long
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(CMD_SET_SERIAL_PORT_BAUD));
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(cSpeed)); // Set serial baud rate 1-8, 9600-460800 default=7
    // radar_uart_->write((uint8_t)char(0x00));
    send_command_postamble_();
    radar_uart_last_command_ = millis();
    return wait_for_command_ack_(CMD_SET_SERIAL_PORT_BAUD);
  }
  return false;
}

/*
 * Set maximum gates and idle time
 *
 * maximum detection range gate: 2-8
 * unmanned duration: 0-65535 seconds
 */
bool ld2410::setMaxValues(uint16_t moving, uint16_t stationary, uint16_t inactivityTimer)
{
  if (enter_configuration_mode_())
  {
    delay(50);
    send_command_preamble_();
    uint8_t set_zones[] = { 0x14, 0x00, CMD_MAX_DISTANCE_AND_UNMANNED_DURATION,
                            0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0x00,
                            0xFF, 0xFF, 0x00, 0x00, 0x02, 0x00, 0xFF, 0xFF, 0x00, 0x00 };
    set_zones[6]  = moving & 0x00FF;
    set_zones[7]  = (moving & 0xFF00) >> 8;
    set_zones[12] = stationary & 0x00FF;
    set_zones[13] = (stationary & 0xFF00) >> 8;
    set_zones[18] = inactivityTimer & 0x00FF;
    set_zones[19] = (inactivityTimer & 0xFF00) >> 8;
    for (uint8_t n = 0; n < 22u; ++n) {
      radar_uart_->write(set_zones[n]);
    }
    // radar_uart_->write((uint8_t)char(0x14));                                   // Command is 20 bytes long
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(CMD_MAX_DISTANCE_AND_UNMANNED_DURATION)); // Request set max values
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(0x00));                                   // Moving gate command
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(moving & 0x00FF));                        // Moving gate value
    // radar_uart_->write((uint8_t)char((moving & 0xFF00) >> 8));
    // radar_uart_->write((uint8_t)char(0x00));                                   // Spacer
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(0x01));                                   // Stationary gate command
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(stationary & 0x00FF));                    // Stationary gate value
    // radar_uart_->write((uint8_t)char((stationary & 0xFF00) >> 8));
    // radar_uart_->write((uint8_t)char(0x00));                                   // Spacer
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(0x02));                                   // Inactivity timer command
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(inactivityTimer & 0x00FF));               // Inactivity timer
    // radar_uart_->write((uint8_t)char((inactivityTimer & 0xFF00) >> 8));
    // radar_uart_->write((uint8_t)char(0x00));                                   // Spacer
    // radar_uart_->write((uint8_t)char(0x00));
    send_command_postamble_();
    radar_uart_last_command_ = millis();
    return wait_for_command_ack_(CMD_MAX_DISTANCE_AND_UNMANNED_DURATION);
  }
  return false;
}

/*
 * configures the sensitivity of the distance gate
 *
 * Command word:0x0064
 * Command value:
 * individual:
 *     2 bytes distance gate word      0x0000 + 4 bytes distance gate value (2-8),
 *     2 bytes motion sensitivity word 0x0001 + 4 bytes motion sensitivity value. (0-100)
 *     2 bytes static sensitivity word 0x0002 + 4 bytes static sensitivity value. (0-100)
 *  or
 * Grouped: (if input gate equals 255)
 *     2 bytes distance gate:          0x0000 + 4 bytes distance gate value 0xFFFF
 *     2 bytes motion sensitivity word 0x0001 + 4 bytes motion sensitivity value. (0-100)
 *     2 bytes static sensitivity word 0x0002 + 4 bytes static sensitivity value. (0-100)
 *
 * Return value:2 bytes ACK status(0 success, 1 failure)
 */
bool ld2410::setGateSensitivityThreshold(uint8_t gate, uint8_t moving, uint8_t stationary)
{
  if (enter_configuration_mode_())
  {
    delay(50);
    send_command_preamble_();
    uint8_t set_sens[] = { 0x14, 0x00, CMD_RANGE_GATE_SENSITIVITY,
                           0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00,
                           0xFF, 0x00, 0x00, 0x00, 0x02, 0x00, 0xFF, 0x00, 0x00, 0x00 };
    set_sens[6] = gate;
    if (255 == gate) {
      set_sens[7] = gate;
      set_sens[8] = gate;
      set_sens[9] = gate;
    }
    set_sens[12] = moving;
    set_sens[18] = stationary;
    for (uint8_t n = 0; n < 22u; ++n) {
      radar_uart_->write(set_sens[n]);
    }
    // radar_uart_->write((uint8_t)char(0x14));                       // Command is 20 bytes long
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(CMD_RANGE_GATE_SENSITIVITY)); // Request set sensitivity values
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(0x00));                       // Gate command
    // radar_uart_->write((uint8_t)char(0x00));

    // if (gate == 255) {
    //   radar_uart_->write((uint8_t)char(0xFF)); // Gate value
    //   radar_uart_->write((uint8_t)char(0xFF));
    //   radar_uart_->write((uint8_t)char(0xFF));
    //   radar_uart_->write((uint8_t)char(0xFF));
    // } else {
    //   radar_uart_->write((uint8_t)char(gate));     // Gate value
    //   radar_uart_->write((uint8_t)char(0x00));
    //   radar_uart_->write((uint8_t)char(0x00));     // Spacer
    //   radar_uart_->write((uint8_t)char(0x00));
    // }
    // radar_uart_->write((uint8_t)char(0x01));       // Motion sensitivity command
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(moving));     // Motion sensitivity value
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(0x00));       // Spacer
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(0x02));       // Stationary sensitivity command
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(stationary)); // Stationary sensitivity value
    // radar_uart_->write((uint8_t)char(0x00));
    // radar_uart_->write((uint8_t)char(0x00));       // Spacer
    // radar_uart_->write((uint8_t)char(0x00));
    send_command_postamble_();
    radar_uart_last_command_ = millis();
    return wait_for_command_ack_(CMD_RANGE_GATE_SENSITIVITY);
  }
  return false;
}

#endif // ifndef ld2410_cpp
