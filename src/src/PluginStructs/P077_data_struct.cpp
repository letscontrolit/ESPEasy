#include "../PluginStructs/P077_data_struct.h"

#ifdef USES_P077

# include <ESPeasySerial.h>


const __FlashStringHelper* Plugin_077_valuename(P077_query value_nr, bool displayString)
{
  const __FlashStringHelper *strings[] = {
    F("Voltage"),        F("V"),
    F("Active Power"),   F("W"),
    F("Current"),        F("A"),
    F("Pulses"),         F("pulses"),
    F("Energy"),         F("kWh"),
    F("Apparent Power"), F("VA"),
    F("Power Factor"),   F("pf"),
    F("Reactive Power"), F("VAR")
  };
  constexpr size_t nrStrings = NR_ELEMENTS(strings);
  const size_t     index     = static_cast<size_t>(value_nr) * 2 + (displayString ? 0 : 1);

  if (index < nrStrings) {
    return strings[index];
  }
  return F("");
}

P077_query Plugin_077_from_valuename(const String& valuename)
{
  for (uint8_t query = 0; query < static_cast<uint8_t>(P077_query::P077_QUERY_NR_OUTPUT_OPTIONS); ++query) {
    if (valuename.equalsIgnoreCase(Plugin_077_valuename(static_cast<P077_query>(query), false))) {
      return static_cast<P077_query>(query);
    }
  }
  return P077_query::P077_QUERY_NR_OUTPUT_OPTIONS;
}

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
  constexpr size_t bufsize = NR_ELEMENTS(serial_in_buffer);

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
    if ((CSE_UREF_PULSE == P077_UREF) || (P077_UREF == 0)) {
      P077_UREF = voltage_coefficient / CSE_UREF;
      V2R       = 1.0f;
    } else {
      V2R = static_cast<float>(P077_UREF * CSE_UREF) / voltage_coefficient;
    }
  }

  if (current_coefficient != 0) {
    if ((CSE_IREF_PULSE == P077_IREF) || (P077_IREF == 0)) {
      P077_IREF = current_coefficient;
      V1R       = 1.0f;
    } else {
      V1R = static_cast<float>(P077_IREF) / current_coefficient;
    }
  }

  if ((CSE_PREF_PULSE == P077_PREF) || (P077_PREF == 0)) {
    P077_PREF = (V1R * V2R * power_coefficient) / CSE_PREF;
  }

  if (P077_PREF != 0) {
    cf_frequency = (1e9f) / (P077_PREF * CSE_PREF);
  }


  adjustment = serial_in_buffer[20];

  const bool voltage_valid = bitRead(adjustment, 6);
  const bool current_valid = bitRead(adjustment, 5);
  const bool power_valid   = bitRead(adjustment, 4);

  // const bool cf_pulse_overflow = bitRead(adjustment, 7);


  if (voltage_valid) {
    const uint32_t voltage_cycle = get_24bit_value(5);

    if (voltage_cycle != 0) {
      setOutputValue(event, P077_query::P077_QUERY_VOLTAGE, static_cast<float>(P077_UREF * CSE_UREF) / static_cast<float>(voltage_cycle));
      newValue = true;
    }
  }

  if (power_valid) {
    const uint32_t cur_millis = millis();

    // Only use CF pulses when a power cycle has completed
    const uint32_t cur_cf_pulses = serial_in_buffer[21] << 8 | serial_in_buffer[22];

    if (last_cf_pulses == 0) {
      last_cf_pulses_moment = cur_millis;
      last_cf_pulses        = cur_cf_pulses;
    }

    uint32_t diff{};

    if (cur_cf_pulses < last_cf_pulses) {
      diff = cur_cf_pulses + (0xFFFF - last_cf_pulses) + 1;
    } else {
      diff = cur_cf_pulses - last_cf_pulses;
    }

    last_cf_pulses = cur_cf_pulses;

    cf_pulses += diff;

    float activePower{};

    if (!power_cycle_exceeds_range) {
      const long power_cycle = get_24bit_value(17);

      if (0 == power_cycle_first) {
        power_cycle_first = power_cycle; // Skip first incomplete power_cycle
      }

      if (power_cycle_first != power_cycle) {
        power_cycle_first = -1;

        if (power_cycle != 0) {
          activePower = static_cast<float>(P077_PREF * CSE_PREF) / static_cast<float>(power_cycle);
        } else {
          if ((diff > 0) && (last_cf_pulses_moment != cur_millis)) {
            const float time_passed_diff_sec = timeDiff(last_cf_pulses_moment, cur_millis) / 1000.0f;
            activePower = (diff / time_passed_diff_sec) / cf_frequency;
          }
        }
        newValue = true;
      }
    }
    setOutputValue(event, P077_query::P077_QUERY_ACTIVE_POWER, activePower);
    last_cf_pulses_moment = cur_millis;
  } else {
    power_cycle_first = 0;

    // Powered on but no load
    setOutputValue(event, P077_query::P077_QUERY_ACTIVE_POWER, 0);
  }

  if (current_valid) {
    newValue = true;
    const uint32_t current_cycle = get_24bit_value(11);

    if (essentiallyZero(getValue(P077_query::P077_QUERY_ACTIVE_POWER)) || (current_cycle == 0)) {
      setOutputValue(event, P077_query::P077_QUERY_CURRENT, 0);
    } else {
      setOutputValue(event, P077_query::P077_QUERY_CURRENT, static_cast<float>(P077_IREF) / static_cast<float>(current_cycle));
    }
  }

  // Update derived values
  {
    const float pulsesPerKwh = cf_frequency * 3600;
    setOutputValue(event, P077_query::P077_QUERY_KWH,    cf_pulses / pulsesPerKwh);
    setOutputValue(event, P077_query::P077_QUERY_PULSES, cf_pulses);

    const float voltage  = getValue(P077_query::P077_QUERY_VOLTAGE);
    const float current  = getValue(P077_query::P077_QUERY_CURRENT);
    const float active   = getValue(P077_query::P077_QUERY_ACTIVE_POWER);
    const float apparent = voltage * current;
    setOutputValue(event, P077_query::P077_QUERY_VA, apparent);

    float reactivePower{};

    if (definitelyGreaterThan(apparent, active)) {
      reactivePower = sqrtf(apparent * apparent - active * active);
    }
    setOutputValue(event, P077_query::P077_QUERY_REACTIVE_POWER, reactivePower);

    float powerfactor = 1.0f;

    if (!essentiallyZero(voltage) && !essentiallyZero(current)) {
      powerfactor = active / voltage / current;
    }
    setOutputValue(event, P077_query::P077_QUERY_PF, powerfactor);
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

bool P077_data_struct::plugin_read(struct EventStruct *event) {
  constexpr uint8_t nrElements = NR_ELEMENTS(_cache);

  for (uint8_t i = 0; i < P077_NR_OUTPUT_VALUES; ++i) {
    const uint8_t pconfigIndex = i + P077_QUERY1_CONFIG_POS;
    const uint8_t index        = static_cast<uint8_t>(PCONFIG(pconfigIndex));

    if (index < nrElements) {
      float value{};

      if (_cache[index].peek(value)) {
        UserVar.setFloat(event->TaskIndex, i,  value);
      }
    }
  }

  for (uint8_t i = 0; i < nrElements; ++i) {
    _cache[i].resetKeepLast();
  }
  const bool res = newValue;

  newValue = false;
  return res;
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
    P077_UREF = CSE_UREF_PULSE;
    P077_IREF = CSE_IREF_PULSE;
    P077_PREF = CSE_PREF_PULSE;
    success   = true;
    changed   = true;
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

    const float Volt        = getValue(P077_query::P077_QUERY_VOLTAGE);
    const float ActivePower = getValue(P077_query::P077_QUERY_ACTIVE_POWER);
    const float Current     = getValue(P077_query::P077_QUERY_CURRENT);

    const bool applyCalibVolt = definitelyGreaterThan(CalibVolt, 0.0f)  && !essentiallyZero(Volt);
    const bool applyCalibCurr = definitelyGreaterThan(CalibCurr, 0.0f)  && !essentiallyZero(Current);
    const bool applyCalibPwr  = definitelyGreaterThan(CalibAcPwr, 0.0f) && !essentiallyZero(ActivePower);


    if (applyCalibVolt) {
      P077_UREF = static_cast<uint16_t>(static_cast<float>(P077_UREF) *
                                        (CalibVolt / Volt));
      changed = true;
    }

    if (applyCalibCurr) {
      P077_IREF = static_cast<uint16_t>(static_cast<float>(P077_IREF) *
                                        (CalibCurr / Current));
      changed = true;
    }

    if (applyCalibPwr) {
      P077_PREF = static_cast<uint16_t>(static_cast<float>(P077_PREF) *
                                        (CalibAcPwr / ActivePower));
      changed = true;
    } else if (changed) {
      // Force reload of factory calibration of pwr corrected with offset for voltage/current
      P077_PREF = CSE_PREF_PULSE;
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

void P077_data_struct::setOutputValue(struct EventStruct *event, P077_query outputType, float value) {
  const uint8_t index          = static_cast<uint8_t>(outputType);
  constexpr uint8_t nrElements = NR_ELEMENTS(_cache);

  if (index < nrElements) {
    _cache[index].add(value);

    for (uint8_t i = 0; i < P077_NR_OUTPUT_VALUES; ++i) {
      const uint8_t pconfigIndex = i + P077_QUERY1_CONFIG_POS;

      if (PCONFIG(pconfigIndex) == index) {
# if FEATURE_PLUGIN_STATS
        PluginStats *stats = getPluginStats(i);

        if (stats != nullptr) {
          stats->trackPeak(value);
        }
# endif // if FEATURE_PLUGIN_STATS

        // Set preliminary averaged value as task value.
        // This way we can see intermediate updates.
        _cache[index].peek(value);
        UserVar.setFloat(event->TaskIndex, i,  value);
      }
    }
  }
}

float P077_data_struct::getValue(P077_query outputType) const
{
  const uint8_t index          = static_cast<uint8_t>(outputType);
  constexpr uint8_t nrElements = NR_ELEMENTS(_cache);

  float res{};

  if (index < nrElements) {
    _cache[index].peek(res);
  }
  return res;
}

#endif // ifdef USES_P077
