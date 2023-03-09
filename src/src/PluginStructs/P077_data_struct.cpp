#include "../PluginStructs/P077_data_struct.h"

#ifdef USES_P077

# include <ESPeasySerial.h>

P077_data_struct::~P077_data_struct() {
  delete easySerial;
  easySerial = nullptr;
}

void P077_data_struct::reset() {
  delete easySerial;
  easySerial = nullptr;
}

bool P077_data_struct::isInitialized() const {
  return easySerial != nullptr;
}

bool P077_data_struct::init(ESPEasySerialPort port, const int16_t serial_rx, const int16_t serial_tx, unsigned long baudrate,
                            uint8_t config) {
  if (serial_rx < 0) {
    return false;
  }
  reset();
  easySerial = new (std::nothrow) ESPeasySerial(port, serial_rx, serial_tx);

  if (isInitialized()) {
    # if defined(ESP8266)
    easySerial->begin(baudrate, (SerialConfig)config);
    # elif defined(ESP32)
    easySerial->begin(baudrate, config);
    # endif // if defined(ESP8266)
    return true;
  }
  return false;
}

bool P077_data_struct::processCseReceived(struct EventStruct *event) {
  uint8_t header = serial_in_buffer[0];

  if ((header & 0xFC) == 0xFC) {
    //  Abnormal hardware
    return false;
  }

  // Get chip calibration data (coefficients) and use as initial defaults
  if (CSE_UREF_PULSE == PCONFIG(0)) {
    long voltage_coefficient = 191200; // uSec

    if (CSE_NOT_CALIBRATED != header) {
      voltage_coefficient = serial_in_buffer[2] << 16 |
                            serial_in_buffer[3] << 8 |
                            serial_in_buffer[4];
    }
    PCONFIG(0) = voltage_coefficient / CSE_UREF;
  }

  if (CSE_IREF_PULSE == PCONFIG(1)) {
    long current_coefficient = 16140; // uSec

    if (CSE_NOT_CALIBRATED != header) {
      current_coefficient = serial_in_buffer[8] << 16 |
                            serial_in_buffer[9] << 8 |
                            serial_in_buffer[10];
    }
    PCONFIG(1) = current_coefficient;
  }

  if (CSE_PREF_PULSE == PCONFIG(2)) {
    long power_coefficient = 5364000; // uSec

    if (CSE_NOT_CALIBRATED != header) {
      power_coefficient = serial_in_buffer[14] << 16 |
                          serial_in_buffer[15] << 8 |
                          serial_in_buffer[16];
    }
    PCONFIG(2) = power_coefficient / CSE_PREF;
  }

  adjustment    = serial_in_buffer[20];
  voltage_cycle = serial_in_buffer[5] << 16 |
                  serial_in_buffer[6] << 8 |
                  serial_in_buffer[7];
  current_cycle = serial_in_buffer[11] << 16 |
                  serial_in_buffer[12] << 8 |
                  serial_in_buffer[13];
  power_cycle = serial_in_buffer[17] << 16 |
                serial_in_buffer[18] << 8 |
                serial_in_buffer[19];
  cf_pulses = serial_in_buffer[21] << 8 |
              serial_in_buffer[22];

  if (adjustment & 0x40) { // Voltage valid
    energy_voltage = static_cast<float>(PCONFIG(0) * CSE_UREF) / static_cast<float>(voltage_cycle);
  }

  if (adjustment & 0x10) {         // Power valid
    if ((header & 0xF2) == 0xF2) { // Power cycle exceeds range
      energy_power = 0;
    } else {
      if (0 == power_cycle_first) {
        power_cycle_first = power_cycle; // Skip first incomplete power_cycle
      }

      if (power_cycle_first != power_cycle) {
        power_cycle_first = -1;
        energy_power      = static_cast<float>(PCONFIG(2) * CSE_PREF) / static_cast<float>(power_cycle);
      } else {
        energy_power = 0;
      }
    }
  } else {
    power_cycle_first = 0;
    energy_power      = 0; // Powered on but no load
  }

  if (adjustment & 0x20) { // Current valid
    if (0 == energy_power) {
      energy_current = 0;
    } else {
      energy_current = static_cast<float>(PCONFIG(1)) / static_cast<float>(current_cycle);
    }
  }

  return true;
}

bool P077_data_struct::processSerialData() {
  long t_start = millis();
  bool found   = false;

  while (isInitialized() && (easySerial->available() > 0) && !found) {
    uint8_t serial_in_byte = easySerial->read();
    count_bytes++;
    checksum -= serial_in_buffer[2];             // substract from checksum data to be removed
    memmove(serial_in_buffer, serial_in_buffer + 1,
            sizeof(serial_in_buffer) - 1);       // scroll buffer
    serial_in_buffer[25] = serial_in_byte;       // add new data
    checksum            += serial_in_buffer[22]; // add online checksum

    if ((checksum == serial_in_buffer[23]) &&
        (serial_in_buffer[1] == 0x5A)) {
      count_pkt++;
      found = true;
    }
  }
  long t_diff = timePassedSince(t_start);

  t_all += t_diff;

  if (count_pkt > 10) { // bypass first 10 pkts
    t_max = max(t_max, t_diff);
  }

  if (found) {
    count_max = max(count_max, count_bytes);
    t_pkt     = t_start - t_pkt_tmp;
    t_pkt_tmp = t_start;
  }

  return found;
}

/**
 * plugin_write: Handle commands
 * csereset: reset calibration values
 * csecalibrate,[voltage],[current],[power]: set calibration values, 0 value(s) will be skipped
 * Saves (all) settings if a calibration value is updated, or csereset is used
 */
bool P077_data_struct::plugin_write(struct EventStruct *event,
                                    String              string) {
  const String cmd = parseString(string, 1);
  bool success     = false;
  bool changed     = false;

  if (equals(cmd, F("csereset"))) { // Reset to defaults
    PCONFIG(0) = CSE_UREF_PULSE;
    PCONFIG(1) = CSE_IREF_PULSE;
    PCONFIG(2) = CSE_PREF_PULSE;
    success    = true;
    changed    = true;
  } else if (equals(cmd, F("csecalibrate"))) { // Set 1 or more calibration values, 0 will skip that value
    success = true;
    float CalibVolt  = 0.0f;
    float CalibCurr  = 0.0f;
    float CalibAcPwr = 0.0f;

    if (validFloatFromString(parseString(string, 2), CalibVolt)) {
      if (validFloatFromString(parseString(string, 3), CalibCurr)) {
        validFloatFromString(parseString(string, 4), CalibAcPwr);
      }
    }

    if (definitelyGreaterThan(CalibVolt, 0.0f)) {
      PCONFIG(0) = static_cast<uint16_t>(static_cast<float>(PCONFIG(0)) * (CalibVolt / energy_voltage));
      changed    = true;
    }

    if (definitelyGreaterThan(CalibCurr, 0.0f)) {
      PCONFIG(1) = static_cast<uint16_t>(static_cast<float>(PCONFIG(1)) * (CalibCurr / energy_current));
      changed    = true;
    }

    if (definitelyGreaterThan(CalibAcPwr, 0.0f)) {
      PCONFIG(2) = static_cast<uint16_t>(static_cast<float>(PCONFIG(2)) * (CalibAcPwr / energy_power));
      changed    = true;
    }
  }

  if (changed) {
    SaveSettings(); // FIXME tonhuisman: Doubting if by default the changes should be saved
  }
  return success;
}

# ifndef BUILD_NO_DEBUG
int P077_data_struct::serial_Available() {
  if (isInitialized()) {
    return easySerial->available();
  }
  return 0;
}

# endif // ifndef BUILD_NO_DEBUG

#endif // ifdef USES_P077
