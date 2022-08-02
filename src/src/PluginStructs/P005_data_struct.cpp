#include "../PluginStructs/P005_data_struct.h"

#ifdef USES_P005


// DEBUG code using logic analyzer for timings
// #define DEBUG_LOGIC_ANALYZER_PIN  27 


// Macros to perform direct access on GPIOs
// Macros written by Paul Stoffregen
// See: https://github.com/PaulStoffregen/OneWire/blob/master/util/
# include <GPIO_Direct_Access.h>

enum struct P005_logNr {
  P005_error_no_reading,
  P005_error_protocol_timeout,
  P005_error_checksum_error,
  P005_error_invalid_NAN_reading,
  P005_info_temperature,
  P005_info_humidity
};

const __FlashStringHelper* P005_logString(P005_logNr logNr) {
  switch (logNr) {
    case P005_logNr::P005_error_no_reading:          return F("No Reading");
    case P005_logNr::P005_error_protocol_timeout:    return F("Protocol Timeout");
    case P005_logNr::P005_error_checksum_error:      return F("Checksum Error");
    case P005_logNr::P005_error_invalid_NAN_reading: return F("Invalid NAN reading");
    case P005_logNr::P005_info_temperature:          return F("Temperature: ");
    case P005_logNr::P005_info_humidity:             return F("Humidity: ");
  }
  return F("");
}

/*********************************************************************************************\
* DHT sub to log an error
\*********************************************************************************************/
void P005_log(struct EventStruct *event, P005_logNr logNr)
{
  bool isError = true;

  switch (logNr) {
    case P005_logNr::P005_info_temperature:
    case P005_logNr::P005_info_humidity:
      isError = false;
      break;

    default:
      UserVar[event->BaseVarIndex]     = NAN;
      UserVar[event->BaseVarIndex + 1] = NAN;
      break;
  }

  if (loglevelActiveFor(isError ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO)) {
    String text;
    text  = F("DHT  : ");
    text += P005_logString(logNr);

    if (logNr == P005_logNr::P005_info_temperature) {
      text += formatUserVarNoCheck(event->TaskIndex, 0);
    }
    else if (logNr == P005_logNr::P005_info_humidity) {
      text += formatUserVarNoCheck(event->TaskIndex, 1);
    }
    addLogMove(isError ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO, text);
  }
}

P005_data_struct::P005_data_struct(struct EventStruct *event) {
  SensorModel = PCONFIG(0);
  DHT_pin = CONFIG_PIN1;
}

/*********************************************************************************************\
* DHT sub to wait until a pin is in a certain state
\*********************************************************************************************/
bool P005_data_struct::waitState(int state)
{
  const uint64_t   timeout          = getMicros64() + 100;

#ifdef DEBUG_LOGIC_ANALYZER_PIN
  // DEBUG code using logic analyzer for timings
  DIRECT_pinWrite(DEBUG_LOGIC_ANALYZER_PIN, 0);
#endif

  while (DIRECT_pinRead(DHT_pin) != state)
  {
    if (usecTimeOutReached(timeout)) { return false; }
  }

#ifdef DEBUG_LOGIC_ANALYZER_PIN
  DIRECT_pinWrite(DEBUG_LOGIC_ANALYZER_PIN, 1);
#endif
  return true;
}

/*********************************************************************************************\
* Perform the actual reading + interpreting of data.
\*********************************************************************************************/
bool P005_data_struct::readDHT(struct EventStruct *event) {
  // Call the "slow" function to make sure the pin is in a defined state.
  // Apparently the pull-up state may not always be in a well known state
  // With the direct pinmode calls we don't set the pull-up or -down resistors.
  pinMode(DHT_pin, INPUT_PULLUP);

#ifdef DEBUG_LOGIC_ANALYZER_PIN
  // DEBUG code using logic analyzer for timings
  DIRECT_PINMODE_OUTPUT(DEBUG_LOGIC_ANALYZER_PIN);
  DIRECT_pinWrite(DEBUG_LOGIC_ANALYZER_PIN, 0);
#endif

  // To begin asking the DHT22 for humidity and temperature data,
  // Start sequence to get data from a DHTxx sensor:
  // Pin must be a logic 0 (low) for at least 500 microseconds (DHT22, others may need different timing)
  // followed by a logic 1 (high).
  DIRECT_PINMODE_OUTPUT(DHT_pin);
  DIRECT_pinWrite(DHT_pin, 0);           // Pull low

  switch (SensorModel) {
    case P005_DHT11:  delay(19); break;  // minimum 18ms
    case P005_DHT22:  delay(2);  break;  // minimum 1ms
    case P005_DHT12:  delay(200); break; // minimum 200ms
    case P005_AM2301: delayMicroseconds(900); break;
    case P005_SI7021: delayMicroseconds(500); break;
  }

  {
#ifdef DEBUG_LOGIC_ANALYZER_PIN
    DIRECT_pinWrite(DEBUG_LOGIC_ANALYZER_PIN, 1);
#endif

    DIRECT_PINMODE_INPUT(DHT_pin);
    // pinMode(DHT_pin, INPUT_PULLUP);  // Way too slow, takes upto 227 usec

#ifdef DEBUG_LOGIC_ANALYZER_PIN
    DIRECT_pinWrite(DEBUG_LOGIC_ANALYZER_PIN, 0);
#endif
  }

  bool readingAborted = false;
  uint8_t dht_dat[5]  = { 0 };

  uint8_t  dht_byte      = 0;
  uint32_t avg_low_total = 0;


  // Response from DHTxx: (N = 80 usec for DHT22)
  // Low for N usec
  // Hight for N usec
  // Low for 50 usec
  bool receive_start;

  uint8_t timings[16] = { 0 };


  noInterrupts();
  receive_start = waitState(0) && waitState(1) && waitState(0);

#ifdef DEBUG_LOGIC_ANALYZER_PIN
  DIRECT_pinWrite(DEBUG_LOGIC_ANALYZER_PIN, 0);
#endif

  if (receive_start) {
    // We know we're now at a "low" state.
    uint64_t last_micros = getMicros64();
    uint64_t prev_edge   = last_micros;

    for (dht_byte = 0; dht_byte < 5 && !readingAborted; ++dht_byte)
    {
      // Start reading next byte
#ifdef DEBUG_LOGIC_ANALYZER_PIN
      DIRECT_pinWrite(DEBUG_LOGIC_ANALYZER_PIN, 1);
#endif
      for (uint8_t t = 0; t < 16 && !readingAborted; ++t) {
        // "even" index = "low" duration
        // "odd"  index = "high" duration
        const uint32_t current_state = (t & 1);

        // Wait till pin state has changed, or timeout.
        while (DIRECT_pinRead(DHT_pin) == current_state && !readingAborted)
        {
          // Keep track of last microsecond the state had not yet changed.
          // This way we are less dependent on any jitter caused by
          // the delay call or rise times of the voltage on the pin.
          last_micros = getMicros64();

          if (timeDiff64(prev_edge, last_micros) > 100) {
            readingAborted = true;
          }
        }

        if (!readingAborted) {
          // We know it is less than 100 usec, so it does fit in the uint8_t timings array.
          timings[t] = usecPassedSince(prev_edge);
          prev_edge  = last_micros;
        } else {
          timings[t] = 255;
        }
      }
#ifdef DEBUG_LOGIC_ANALYZER_PIN
      DIRECT_pinWrite(DEBUG_LOGIC_ANALYZER_PIN, 0);
#endif

      if (!readingAborted) {
        // Evaluate the timings
        // timings on even indices represent "duration low"
        // timings on odd  indices represent "duration high"
        //
        // Timing for a single bit:
        // Logic "1":  50 usec low, 70 usec high
        // Logic "0":  50 usec low, 26 usec high
        // There is a significant difference between the "high" state durations
        // Thus "high duration" > "avg_low duration" means it is an "1".
        //
        // By taking the average low duration, we get rid of
        // critical timing differences among modules and
        // environmental effects which may change these timings.
        // It is all about the relative timings.
        uint32_t avg_low = 0;

        // Don't take the 1st "low" period into account for computing avg_low
        // as there might be an extra wait between bytes.
        // Just to be sure as it is not clear from the documentation if all models act the same.
        for (uint8_t t = 2; t < 16; t += 2) {
          avg_low += timings[t];
        }
        avg_low       /= 7;
        avg_low_total += avg_low;

        dht_dat[dht_byte] = 0;

        for (uint8_t bit = 0; bit < 8; ++bit) {
          if (timings[2 * bit + 1] > avg_low) {
            dht_dat[dht_byte] |= (1 << (7 - bit));
          }
        }
      }
    }
  }
  interrupts();


  if (!receive_start) {
    P005_log(event, P005_logNr::P005_error_no_reading);
    return false;
  }

  # ifndef BUILD_NO_DEBUG

  if (dht_byte != 0) {
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = F("DHT  : ");
      log += F("Avg Low: ");
      log += static_cast<float>(avg_low_total) / dht_byte;
      log += F(" usec ");
      log += dht_byte;
      log += F(" bytes:");
      for (int i = 0; i < dht_byte; ++i) {
        log += ' ';
        log += formatToHex(dht_dat[i]);
      }
      log += F(" timings:");
      for (int i = 0; i < 16; ++i) {
        log += ' ';
        log += timings[i];
      }
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
  }
  # endif // ifndef BUILD_NO_DEBUG


  if (readingAborted) {
    P005_log(event, P005_logNr::P005_error_protocol_timeout);
    return false;
  }

  // Checksum calculation is a Rollover Checksum by design!
  uint8_t dht_check_sum = (dht_dat[0] + dht_dat[1] + dht_dat[2] + dht_dat[3]) & 0xFF; // check check_sum

  if (dht_dat[4] != dht_check_sum)
  {
    P005_log(event, P005_logNr::P005_error_checksum_error);
    return false;
  }

  float temperature = NAN;
  float humidity    = NAN;

  switch (SensorModel) {
    case P005_DHT11:
    case P005_DHT12:
      temperature = float(dht_dat[2] * 10 + (dht_dat[3] & 0x7f)) / 10.0f; // Temperature

      if (dht_dat[3] & 0x80) { temperature = -temperature; } // Negative temperature
      humidity = float(dht_dat[0] * 10 + dht_dat[1]) / 10.0f;             // Humidity
      break;
    case P005_DHT22:
    case P005_AM2301:
    case P005_SI7021:

      if (dht_dat[2] & 0x80) { // negative temperature
        temperature = -0.1f * word(dht_dat[2] & 0x7F, dht_dat[3]);
      }
      else {
        temperature = 0.1f * word(dht_dat[2], dht_dat[3]);
      }
      humidity = 0.1f * word(dht_dat[0], dht_dat[1]); // Humidity
      break;
  }

  if (isnan(temperature) || isnan(humidity))
  {     P005_log(event, P005_logNr::P005_error_invalid_NAN_reading);
        return false; }

  UserVar[event->BaseVarIndex]     = temperature;
  UserVar[event->BaseVarIndex + 1] = humidity;
  P005_log(event, P005_logNr::P005_info_temperature);
  P005_log(event, P005_logNr::P005_info_humidity);
  return true;
}

#endif // ifdef USES_P005
