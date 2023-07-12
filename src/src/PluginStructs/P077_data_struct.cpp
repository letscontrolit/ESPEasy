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

uint32_t P077_data_struct::get_24bit_value(uint8_t offset) const {
  uint32_t res{};
  constexpr size_t bufsize = sizeof(serial_in_buffer) / sizeof(serial_in_buffer[0]);

  if ((offset + 2u) < bufsize) {
    res = serial_in_buffer[offset] << 16 |
          serial_in_buffer[offset + 1] << 8 |
          serial_in_buffer[offset + 2];
  }
  return res;
}

bool P077_data_struct::processCseReceived(struct EventStruct *event) {
  const uint8_t header = serial_in_buffer[0];

  const bool voltage_cycle_exceeds_range = (header & 0xF8) == 0xF8;
  const bool current_cycle_exceeds_range = (header & 0xF4) == 0xF4;
  const bool power_cycle_exceeds_range   = (header & 0xF2) == 0xF2;


  if (header != 0x55) {
    if ((header & 0xF0) == 0xF0) {
      // Header1 = FxH
      if (voltage_cycle_exceeds_range || current_cycle_exceeds_range) {
        // Abnormal external circuit or chip damage
        return false;
      }
    } else {
      return false;
    }
  }

  // Get chip calibration data (coefficients) and use as initial defaults
  long voltage_coefficient = 191200;  // uSec
  long current_coefficient = 16140;   // uSec
  long power_coefficient   = 5364000; // uSec

  // V1R = shunt resistor
  // V2R = Voltage resistor
  float V1R = 1.0f; // With 1mR manganin resistor at V1P and V1N, V1R=1
  float V2R = 1.0f; // With 1M resistor at V2P, V2R=1


  if (CSE_NOT_CALIBRATED != header) {
    voltage_coefficient = get_24bit_value(2);
    current_coefficient = get_24bit_value(8);
    power_coefficient   = get_24bit_value(14);
  }

  if (voltage_coefficient != 0) {
    if (CSE_UREF_PULSE == PCONFIG(0)) {
      PCONFIG(0) = voltage_coefficient / CSE_UREF;
      V2R        = 1.0f;
    } else {
      V2R = static_cast<float>(PCONFIG(0) * CSE_UREF) / voltage_coefficient;
    }
  }

  if (current_coefficient != 0) {
    if (CSE_IREF_PULSE == PCONFIG(1)) {
      PCONFIG(1) = current_coefficient;
      V1R        = 1.0f;
    } else {
      V1R = static_cast<float>(PCONFIG(1)) / current_coefficient;
    }
  }

  if (CSE_PREF_PULSE == PCONFIG(2)) {
    PCONFIG(2) = (V1R * V2R * power_coefficient) / CSE_PREF;
  }

  if (PCONFIG(2) != 0) {
    cf_frequency = (1e9f) / (PCONFIG(2) * CSE_PREF);
  }


  adjustment = serial_in_buffer[20];

  const bool voltage_valid = bitRead(adjustment, 6);
  const bool current_valid = bitRead(adjustment, 5);
  const bool power_valid   = bitRead(adjustment, 4);

  // const bool cf_pulse_overflow = bitRead(adjustment, 7);


  if (voltage_valid) {
    const uint32_t voltage_cycle = get_24bit_value(5);

    if (voltage_cycle != 0) {
      energy_voltage = static_cast<float>(PCONFIG(0) * CSE_UREF) / static_cast<float>(voltage_cycle);
      newValue       = true;
    }
  }

  if (power_valid) {
    // Only use CF pulses when a power cycle has completed
    const uint32_t cur_cf_pulses = serial_in_buffer[21] << 8 | serial_in_buffer[22];

    if (last_cf_pulses == 0) {
      last_cf_pulses = cur_cf_pulses;
    }

    uint32_t diff{};

    if (cur_cf_pulses < last_cf_pulses) {
      diff = cur_cf_pulses + (0xFFFF - last_cf_pulses) + 1;
    } else {
      diff = cur_cf_pulses - last_cf_pulses;
    }

    last_cf_pulses = cur_cf_pulses;

    cf_pulses += diff;

    if (power_cycle_exceeds_range) {
      _activePower = 0;
    } else {
      const long power_cycle = get_24bit_value(17);

      if (0 == power_cycle_first) {
        power_cycle_first = power_cycle; // Skip first incomplete power_cycle
      }

      if ((power_cycle_first != power_cycle) && (power_cycle != 0)) {
        power_cycle_first = -1;
        _activePower      = static_cast<float>(PCONFIG(2) * CSE_PREF) / static_cast<float>(power_cycle);
        newValue          = true;
      } else {
        _activePower = 0;
      }
    }
  } else {
    power_cycle_first = 0;
    _activePower      = 0; // Powered on but no load
  }

  if (current_valid) {
    newValue = true;
    const uint32_t current_cycle = get_24bit_value(11);

    if ((0 == _activePower) || (current_cycle == 0)) {
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

  if (isInitialized()) {
    int available = easySerial->available();

    while ((available > 0) && !found) {
      uint8_t serial_in_byte = easySerial->read();
      --available;
      count_bytes++;
      memmove(serial_in_buffer, serial_in_buffer + 1,
              sizeof(serial_in_buffer) - 1); // scroll buffer
      serial_in_buffer[23] = serial_in_byte; // add new data

      if ((serial_in_buffer[1] == 0x5A)      // Packet header2 = 5AH？
          && checksumMatch()) {
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
  }

  return found;
}

bool P077_data_struct::checksumMatch() const
{
  uint8_t checksum = 0;

  // Sum of all data, except for 2 byte header and tail (checksum)
  for (uint8_t i = 2; i < 23; i++) {
    checksum += serial_in_buffer[i];
  }
  return checksum == serial_in_buffer[23];
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

    const bool hasCalibVolt = definitelyGreaterThan(CalibVolt, 0.0f);
    const bool hasCalibCurr = definitelyGreaterThan(CalibCurr, 0.0f);
    const bool hasCalibPwr  = definitelyGreaterThan(CalibAcPwr, 0.0f);

    if (hasCalibVolt) {
      PCONFIG(0) = static_cast<uint16_t>(static_cast<float>(PCONFIG(0)) * (CalibVolt / energy_voltage));
      changed    = true;
    }

    if (hasCalibCurr) {
      PCONFIG(1) = static_cast<uint16_t>(static_cast<float>(PCONFIG(1)) * (CalibCurr / energy_current));
      changed    = true;
    }

    if (hasCalibPwr) {
      PCONFIG(2) = static_cast<uint16_t>(static_cast<float>(PCONFIG(2)) * (CalibAcPwr / _activePower));
      changed    = true;
    } else if (changed) {
      // Force reload of factory calibration of pwr corrected with offset for voltage/current
      PCONFIG(2) = CSE_PREF_PULSE;
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

#endif  // ifdef USES_P077
