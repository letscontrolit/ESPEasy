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

/** Changelog:
 * 2024-01-13 tonhuisman: Replace separate serial->write() commands and c-style casts, by loops using an uint8_t array,
 *                        saving ca. 1100 bytes on ESP8266 and ca. 700 bytes on ESP32 binaries
 */

#pragma once

#include <Arduino.h>

#if __cplusplus >= 202002L
# include <atomic>
typedef std::atomic<bool> atomic_bool;
#else // if __cplusplus >= 202002L
typedef volatile bool atomic_bool;
#endif // if __cplusplus >= 202002L

// #define LD2410_DEBUG											// Send any debug to serial?
// #define LD2410_DEBUG_DATA                //Debug all Data Frames
// #define LD2410_DEBUG_COMMANDS            //Debug Command Acks
// #define LD2410_DEBUG_PARSE               //Debug Reporting Frames

// Exclude DEBUG logging when requested externally
#if defined(LIBRARIES_NO_LOG) && LIBRARIES_NO_LOG
  # ifdef LD2410_DEBUG
        #  undef LD2410_DEBUG
        # endif // ifdef LD2410_DEBUG
#endif // if defined(LIBRARIES_NO_LOG) && LIBRARIES_NO_LOG

/*
 * Protocol Command Words
 */
#define LD2410_MAX_FRAME_LENGTH   0x40 // or 64 bytes
#define LD2410_MAX_GATES             9 // 0 - 8 gates

#define CMD_CONFIGURATION_ENABLE  0xFF
#define CMD_CONFIGURATION_END     0xFE
#define CMD_MAX_DISTANCE_AND_UNMANNED_DURATION  0x60
#define CMD_READ_PARAMETER        0x61
#define CMD_ENGINEERING_ENABLE    0x62
#define CMD_ENGINEERING_END       0x63
#define CMD_RANGE_GATE_SENSITIVITY 0x64
#define CMD_READ_FIRMWARE_VERSION 0xA0
#define CMD_SET_SERIAL_PORT_BAUD  0xA1
#define CMD_FACTORY_RESET         0xA2
#define CMD_RESTART               0xA3

/*
 * Data Frame Formats
 */
#define FRAME_TYPE_REPORTING      0x01
#define FRAME_TYPE_TARGET         0x02
#define FRAME_PREFIX_PROTOCOL     0xFD
#define FRAME_PREFIX_REPORTING    0xF4
#define FRAME_TYPE_FLAG           0xAA
#define FRAME_TYPE_MARKER         0x55

/*
 * Target State Constants
 */
#define TARGET_NONE                  0x00
#define TARGET_MOVING                0x01
#define TARGET_STATIONARY            0x02
#define TARGET_MOVING_AND_STATIONARY 0x03


#define SERIAL_RECEIVE_MAX_MS     5 // Read for max. N milliseconds, we'd need (64*(8+2))*(1/256000)=2.5msec to read an entire buffer
#define SERIAL_RECEIVE_GRACE_MS   2 // Read more if still this amount of milliseconds available

class ld2410    {
public:

  ld2410();  // Constructor function
  ~ld2410(); // Destructor function

  /*
   * Primary APIs */
  bool begin(Stream&,
             bool waitForRadar = true); // Start the ld2410
                #ifdef LD2410_DEBUG
  void debug(Stream&);                  // Start debugging on a stream
                #endif // ifdef LD2410_DEBUG

  bool ld2410_loop() {
    return read_frame_();
  } // Sensor loop service

  bool presenceDetected() {
    return target_type_ != 0;
  } // last report data had a type

  bool isConnected();
  bool isStationary() {
    return stationaryTargetDetected();
  }

  bool isMoving() {
    return movingTargetDetected();
  }

  uint16_t detectionDistance() {
    return detection_distance_;
  } // Target Reporting Data

  bool dataReady() {
    return data_ready_;
  }

  /*
   * Utilities -- depreciation candidates */
  uint8_t reportingDataComposition() {
    return target_type_;
  } // Target data state 0-3

  bool isEngineeringMode() {
    return engineering_mode_;
  } // Reporting Data

  bool movingTargetDetected();
  bool stationaryTargetDetected();
  bool read() {
    return read_frame_();
  }

  /*
   * primary sensor responses */
  uint16_t stationaryTargetDistance() {
    return stationary_target_distance_;
  } // Target Reporting Data

  uint8_t stationaryTargetEnergy() {
    return stationary_target_energy_;
  } // Target Reporting Data

  uint16_t movingTargetDistance() {
    return moving_target_distance_;
  } // Target Reporting Data

  uint8_t movingTargetEnergy() {
    return moving_target_energy_;
  } // Target Reporting Data

  /*
   * available if engineering mode is active */
  uint8_t engMovingDistanceGateEnergy(uint8_t gate) {
    return (gate < LD2410_MAX_GATES) ? movement_distance_gate_energy[gate] : 255;
  } // Engineering Reporting Data

  uint8_t engStaticDistanceGateEnergy(uint8_t gate) {
    return (gate < LD2410_MAX_GATES) ? static_distance_gate_engergy[gate] : 255;
  } // Engineering Reporting Data

  uint16_t engMaxMovingDistanceGate() {
    return max_moving_distance_gate;
  } // Engineering Reporting Data

  uint16_t engMaxStaticDistanceGate() {
    return max_static_distance_gate;
  } // Engineering Reporting Data

  uint16_t engRetainDataValue() {
    return engineering_retain_data_;
  } // Engineering Reporting Data, last value

  uint8_t engLightSensorValue() {
    return light_sensor_data_;
  } // Engineering Reporting Data, light sensor (undocumented)

  uint8_t engOutputPinState() {
    return output_pin_data_;
  } // Engineering Reporting Data, output pin state (undocumented)

  /*
   * Commands */
  bool     requestRestart();
  bool     requestFactoryReset();
  bool     requestFirmwareVersion();
  bool     requestCurrentConfiguration();
  bool     requestStartEngineeringMode();
  bool     requestEndEngineeringMode();
  bool     setSerialBaudRate(uint8_t cSpeed);
  bool     setMaxValues(uint16_t moving,
                        uint16_t stationary,
                        uint16_t inactivityTimer); // Realistically gate values are 0-8 but sent as uint16_t
  bool     setGateSensitivityThreshold(uint8_t gate,
                                       uint8_t moving,
                                       uint8_t stationary);
  bool     requestConfigurationModeBegin(); // support multi command executions BEGIN
  bool     requestConfigurationModeEnd();   // support multi command executions END

  /*
   * available after related command has been executed */
  uint16_t cmdProtocolVersion() {
    return configuration_protocol_version_;
  } // Configuration mode response

  uint16_t cmdCommunicationBufferSize() {
    return configuration_buffer_size_;
  }                             // Configuration mode response

  String  cmdFirmwareVersion(); // Returns value from command

  /*
   * available after Read Parameter command has been executed */
  uint8_t cfgMaxGate() {
    return max_gate;
  } // Read Parameters command response

  uint8_t cfgMaxMovingGate() {
    return max_moving_gate;
  } // Read Parameters command response

  uint8_t cfgMaxStationaryGate() {
    return max_stationary_gate;
  } // Read Parameters command response

  uint16_t cfgSensorIdleTimeInSeconds() {
    return sensor_idle_time;
  } // Read Parameters command response

  uint8_t cfgMovingGateSensitivity(uint8_t gate) {
    return (gate < LD2410_MAX_GATES) ? motion_sensitivity[gate] : 255;
  } // Read Parameters command response

  uint8_t cfgStationaryGateSensitivity(uint8_t gate) {
    return (gate < LD2410_MAX_GATES) ? stationary_sensitivity[gate] : 255;
  } // Read Parameters command response

  uint16_t getErrorCountAndReset();

protected:

  /*
   * Request Firmware Version command responses */
  char firmwareBuffer[LD2410_MAX_FRAME_LENGTH];                                            // 64 byte buffer
  uint8_t firmware_major_version   = 0;                                                    // Reported major version
  uint8_t firmware_minor_version   = 0;                                                    // Reported minor version
  uint32_t firmware_bugfix_version = 0;                                                    // Reported bugfix version (coded as hex)

  /*
   * Read Parameter command response data */
  uint8_t max_gate                                 = 0;                                    // Read parameter data
  uint8_t max_moving_gate                          = 0;                                    // Read parameter data
  uint8_t max_stationary_gate                      = 0;                                    // Read parameter data
  uint16_t sensor_idle_time                        = 0;                                    // Read parameter data
  uint8_t motion_sensitivity[LD2410_MAX_GATES]     = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };        // Read parameter data
  uint8_t stationary_sensitivity[LD2410_MAX_GATES] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };        // Read parameter data

  /*
   * Protocol & Engineering Frame Data */
  uint16_t moving_target_distance_                        = 0;                             // protocol mode info
  uint8_t moving_target_energy_                           = 0;                             // protocol mode info
  uint16_t stationary_target_distance_                    = 0;                             // protocol mode info
  uint8_t stationary_target_energy_                       = 0;                             // protocol mode info
  uint16_t detection_distance_                            = 0;                             // protocol & engineering mode info
  uint8_t max_moving_distance_gate                        = 0;                             // engineering mode info
  uint8_t max_static_distance_gate                        = 0;                             // engineering mode info
  uint8_t movement_distance_gate_energy[LD2410_MAX_GATES] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // Engineering mode info
  uint8_t static_distance_gate_engergy[LD2410_MAX_GATES]  = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // Engineering mode info
  uint16_t engineering_retain_data_                       = 0;                             // last value in engineering data frame, extra
                                                                                           // retain ??
  uint8_t light_sensor_data_                              = 0;                             // engineering mode, light sensor (undocumented)
  uint8_t output_pin_data_                                = 0;                             // engineering mode, output pin state
                                                                                           // (undocumented)

  /*
   * Configuration mode response info */
  uint16_t configuration_protocol_version_ = 0;                                            // From Enter Configuration Mode Response
  uint16_t configuration_buffer_size_      = LD2410_MAX_FRAME_LENGTH;                      // From Enter Configuration Mode Response

private:

  /*
   * feature control variables */
  Stream *radar_uart_ = nullptr;
  #ifdef LD2410_DEBUG
  Stream *debug_uart_ = nullptr;              // The stream used for the debugging
  #endif // ifdef LD2410_DEBUG

  uint32_t radar_uart_timeout          = 250; // How long to give up on receiving some useful data from the LD2410
  uint32_t radar_uart_last_packet_     = 0;   // Time of the last packet from the radar
  uint32_t radar_uart_last_command_    = 0;   // Time of the last command sent to the radar
  uint32_t radar_uart_command_timeout_ = 250; // Timeout for sending commands

  uint16_t _errorCount = 0;                   // Internal errorcounter, managed via getErrorCountAndReset()

  uint8_t latest_ack_                = 0;
  uint8_t target_type_               = 0;
  uint8_t radar_data_frame_position_ = 0;             // Where in the frame we are currently writing
  uint8_t radar_data_frame_[LD2410_MAX_FRAME_LENGTH]; // Store the incoming data from the radar, to check it's in a valid format

  bool frame_started_            = false;             // Whether a frame is currently being read
  bool ack_frame_                = false;             // Whether the incoming frame is LIKELY an ACK frame
  bool waiting_for_ack_          = false;             // Whether a command has just been sent
  bool engineering_mode_         = false;             // Wheter engineering mode is active
  bool latest_command_success_   = false;
  bool configuration_mode_active = false;             // Configuration state (multi-mode)
  atomic_bool data_ready_        = false;             // Can we read the current data?

  /*
   * feature management functions */
  uint16_t serial_to_int_(uint8_t index);             // Unpack bytes
  bool     debug_command_results_(
                  #ifdef LD2410_DEBUG
                  const char *title
                  #endif
                  );
  bool     wait_for_command_ack_(uint8_t command);
  bool     isProtocolDataFrame_();                    // Command -Determine type of Frame
  bool     isReportingDataFrame_();                   // Data - Determine type of Frame

  bool     read_frame_();                             // Try to read a frame from the UART
  bool     parse_data_frame_();                       // Is the current data frame valid?
  bool     parse_command_frame_();                    // Is the current command frame valid?
  void     print_frame_();                            // Print the frame for debugging
  void     send_command_preamble_();                  // Commands have the same preamble
  void     send_command_postamble_();                 // Commands have the same postamble
  void     send_2byte_command(uint8_t cmd_byte);      // 2-byte commands have a single command byte
  void     send_4byte_command(uint8_t cmd_byte, 
                              uint8_t val_byte);      // 4-byte commands have a singel command byte and a single value byte
  bool     enter_configuration_mode_();               // Necessary before sending any command
  bool     leave_configuration_mode_();               // Will not read values without leaving command mode
};
