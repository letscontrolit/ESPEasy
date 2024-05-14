#include "../PluginStructs/P169_data_struct.h"

#ifdef USES_P169

# include "../ESPEasyCore/ESPEasyGPIO.h"

# include <GPIO_Direct_Access.h>


# ifndef CORE_POST_3_0_0
#  ifdef ESP8266
#   define IRAM_ATTR ICACHE_RAM_ATTR
#  endif // ifdef ESP8266
# endif  // ifndef CORE_POST_3_0_0

# define P169_AS3935_TIMEOUT_USEC  2000

// Allow for 3.5% deviation
# define P169_ALLOWED_DEVIATION    0.035f

// Division ratio and nr of samples chosen so we expect a
// 500 kHz LCO measurement to take about 31.25 msec.
// ESP8266 can't handle > 20 kHz interrupt calls very well, therefore set to DR_32
# ifdef ESP32

// Expected LCO frequency for DR_16 = 31250 Hz
#  define P169_LCO_DIVISION_RATIO AS3935MI::AS3935_DR_16
#  define P169_NR_CALIBRATION_SAMPLES 1000ul
# else // ifdef ESP32

// Expected LCO frequency for DR_32 = 15625 Hz
#  define P169_LCO_DIVISION_RATIO AS3935MI::AS3935_DR_32
#  define P169_NR_CALIBRATION_SAMPLES  500ul
# endif // ifdef ESP32

void IRAM_ATTR P169_data_struct::P169_interrupt_ISR(P169_data_struct *self) {
  self->_interrupt_timestamp = millis();
}

void IRAM_ATTR P169_data_struct::P169_calibrate_ISR(P169_data_struct *self) {
  // _interrupt_count is volatile, so we can miss when testing for exactly P169_NR_CALIBRATION_SAMPLES
  if (self->_interrupt_count < P169_NR_CALIBRATION_SAMPLES) {
    ++self->_interrupt_count;
  }
  else if (self->_calibration_end_micros == 0ul) {
    self->_calibration_end_micros = static_cast<uint32_t>(getMicros64());
  }
}

uint32_t P169_data_struct::computeCalibratedFrequency(int32_t divider)
{
  /*
  if ((divider < 16) || (divider > 128)) {
    return 0ul;
  }
  */

  // Need to copy the timestamps first as they are volatile
  const uint32_t start = _calibration_start_micros;
  const uint32_t end   = _calibration_end_micros;

  if ((start == 0ul) || (end == 0ul)) {
    return 0ul;
  }

  const int32_t duration_usec = timeDiff(start, end);

  if (duration_usec <= 0l) {
    return 0ul;
  }

  // Compute measured frequency
  // we have duration of P169_NR_CALIBRATION_SAMPLES pulses in usec, thus measured frequency is:
  // (P169_NR_CALIBRATION_SAMPLES * 1000'000) / duration in usec.
  // Actual frequency should take the division ratio into account.
  uint64_t freq = (static_cast<uint64_t>(divider) * 1000000ull * (P169_NR_CALIBRATION_SAMPLES + 1));

  freq /=  duration_usec;

  addLog(LOG_LEVEL_INFO, strformat(F("AS3935: IRQ pin frequency measured: %u Hz, source freq: %.3f kHz"), static_cast<uint32_t>(freq) / divider, (freq)/1000.0f));

  return static_cast<uint32_t>(freq);
}

bool P169_data_struct::validateCurrentResonanceFrequency(int32_t& frequency)
{
  frequency = measureResonanceFrequency(
    P169_IRQ_frequency_source::LCO,
    _sensor.readAntennaTuning());

  // Check for allowed deviation
  constexpr int allowedDeviation = 500000 * P169_ALLOWED_DEVIATION;

  return abs(500000 - frequency) < allowedDeviation;
}

int32_t P169_data_struct::measureResonanceFrequency(P169_IRQ_frequency_source source)
{
  return measureResonanceFrequency(
    source,
    _sensor.readAntennaTuning());
}

uint32_t P169_data_struct::measureResonanceFrequency(P169_IRQ_frequency_source source, uint8_t tuningCapacitance)
{
  set_P169_interruptMode(P169_InterruptMode::detached);

  // set tuning capacitors
  _sensor.writeAntennaTuning(tuningCapacitance);
//  delayMicroseconds(P169_AS3935_TIMEOUT_USEC);


  unsigned sourceFreq_kHz = 500;
  int32_t divider = 1;

  // display LCO on IRQ
  switch (source) {
    case P169_IRQ_frequency_source::LCO:
      _sensor.displayLCO_on_IRQ(true);
      _sensor.writeDivisionRatio(P169_LCO_DIVISION_RATIO);
      divider = 16 << static_cast<uint32_t>(P169_LCO_DIVISION_RATIO);
      sourceFreq_kHz = 500;
      break;

      // TD-er: Do not try to measure the 1.1 MHz signal as the ESP32 will not be able to keep up with all the interrupts.
    case P169_IRQ_frequency_source::SRCO:
      _sensor.displaySRCO_on_IRQ(true);
      sourceFreq_kHz = 1100;
      break;
    case P169_IRQ_frequency_source::TRCO:
      _sensor.displayTRCO_on_IRQ(true);
      sourceFreq_kHz = 33;
      break;
  }

  set_P169_interruptMode(P169_InterruptMode::calibration);

  // Need to give enough time for the sensor to set the LCO signal on the IRQ pin
  delayMicroseconds(P169_AS3935_TIMEOUT_USEC);
  _calibration_end_micros   = 0ul;
  _interrupt_count          = 0ul;
  _calibration_start_micros = static_cast<uint32_t>(getMicros64());

  // Wait for the amount of samples to be counted (or timeout)
  // Typically this takes 32 msec for the 500 kHz LCO
  const unsigned expectedDuration = (divider * P169_NR_CALIBRATION_SAMPLES) / sourceFreq_kHz;
  const uint32_t timeout          = millis() + (2 * expectedDuration);
  uint32_t freq                   = 0;

  while (freq == 0 && !timeOutReached(timeout)) {
    delay(1);
    freq = computeCalibratedFrequency(divider);
  }

  set_P169_interruptMode(P169_InterruptMode::detached);

  // stop displaying LCO on IRQ
  _sensor.displayLCO_on_IRQ(false);

  return freq;
}

bool P169_data_struct::calibrateResonanceFrequency(int32_t& frequency)
{
  int32_t best_diff = 500000;
  int8_t  best_i    = -1;

  frequency = 0;

  for (uint8_t i = 0; i < 16; i++)
  {
    const uint32_t freq = measureResonanceFrequency(
      P169_IRQ_frequency_source::LCO, i);

    if (freq == 0) {
      return false;
    }
    const int32_t freq_diff = 500000 - freq;

    if (abs(freq_diff) < abs(best_diff)) {
      best_diff = freq_diff;
      best_i    = i;
      frequency = freq;
    }
  }

  if (best_i < 0) {
    frequency = 0;
    return false;
  }

  _sensor.writeAntennaTuning(best_i);

  // Check for allowed deviation
  constexpr int allowedDeviation = 500000 * P169_ALLOWED_DEVIATION;

  return abs(best_diff) < allowedDeviation;
}

void P169_data_struct::set_P169_interruptMode(P169_InterruptMode mode) {
  if (_mode == mode) {
    return;
  }

  // set the IRQ pin as an input pin. do not use INPUT_PULLUP - the AS3935 will pull the pin
  // high if an event is registered.
  pinMode(_irqPin, INPUT);

  if (_mode != P169_InterruptMode::detached) {
    detachInterrupt(_irqPin);
  }
  _interrupt_timestamp = 0;
  _interrupt_count     = 0;
  _mode                = mode;

  switch (mode) {
    case P169_InterruptMode::detached:
      detachInterrupt(_irqPin);
      break;
    case P169_InterruptMode::normal:
      attachInterruptArg(digitalPinToInterrupt(_irqPin),
                         reinterpret_cast<void (*)(void *)>(P169_interrupt_ISR),
                         this,
                         RISING);
      break;
    case P169_InterruptMode::calibration:
      _calibration_start_micros = 0;
      _calibration_end_micros   = 0;
      attachInterruptArg(digitalPinToInterrupt(_irqPin),
                         reinterpret_cast<void (*)(void *)>(P169_calibrate_ISR),
                         this,
                         RISING);
      break;
  }
}

P169_data_struct::P169_data_struct(struct EventStruct *event) :
  _sensor(P169_I2C_ADDRESS, P169_IRQ_PIN),
  _irqPin(P169_IRQ_PIN),
  _mode(P169_InterruptMode::detached)
{
  _sensor.writePowerDown(false);
}

P169_data_struct::~P169_data_struct()
{
  if (_mode != P169_InterruptMode::detached) {
    detachInterrupt(_irqPin);
  }
  _sensor.writePowerDown(true);
}

bool P169_data_struct::loop()
{
  if (_mode == P169_InterruptMode::normal) {
    // FIXME TD-er: Should also check for state of IRQ pin as it may still be high if the interrupt souce isn't checked.
    const uint32_t timestamp = _interrupt_timestamp;
    if ((timestamp != 0ul) || DIRECT_pinRead(_irqPin)) {
      if ((timestamp != 0ul) && (timePassedSince(timestamp) < 2)) {
        // Sensor not yet ready to report what
        return false;
      }
      _interrupt_timestamp = 0;

      // query the interrupt source from the AS3935
      switch (_sensor.readInterruptSource()) {
        case AS3935MI::AS3935_INT_NH:
          // Noise floor too high
          adjustForNoise();
          break;
        case AS3935MI::AS3935_INT_D:
          // Disturbance detected
          // N.B. can be disabled with _sensor.writeMaskDisturbers(true);
          adjustForDisturbances();
          break;
        case AS3935MI::AS3935_INT_L:
          // Lightning detected
          addLog(LOG_LEVEL_INFO, F("AS3935: Lightning detected"));

          return true;
      }
    }
    tryIncreasedSensitivity();
  }
  return false;
}

bool P169_data_struct::plugin_init(struct EventStruct *event)
{
  set_P169_interruptMode(P169_InterruptMode::detached);

  if (!(_sensor.begin() && _sensor.checkConnection()))
  {
    addLog(LOG_LEVEL_ERROR, F("AS3935: Sensor not detected"));
    return false;
  }

  if (!_sensor.checkIRQ())
  {
    addLog(LOG_LEVEL_ERROR, F("AS3935: IRQ pin connection check failed"));

    //    return false;
  }

  // calibrate the resonance frequency. failing the resonance frequency could indicate an issue
  // of the sensor. resonance frequency calibration will take about 600 msec to complete.
  int32_t frequency = 0;

  if (!calibrateResonanceFrequency(frequency))
  {
    addLog(LOG_LEVEL_ERROR,
           strformat(F("AS3935: Resonance Frequency Calibration failed: %d Hz not in range 482500 Hz ... 517500 Hz"), frequency));
    return false;
  }
  addLog(LOG_LEVEL_INFO, strformat(F("AS3935: Resonance Frequency Calibration passed: %d Hz"), frequency));

  // calibrate the RCO.
  if (!_sensor.calibrateRCO())
  {
    // stop displaying LCO on IRQ
    _sensor.displayLCO_on_IRQ(false);
    addLog(LOG_LEVEL_ERROR, F("AS3935: RCO Calibration failed."));
    return false;
  }

  // stop displaying LCO on IRQ
  _sensor.displayLCO_on_IRQ(false);
  addLog(LOG_LEVEL_INFO, F("AS3935: RCO Calibration passed."));
#ifdef ESP32
  {
    // Short test outputting TRCO on IRQ pin
    const uint32_t freq = measureResonanceFrequency(P169_IRQ_frequency_source::TRCO);
    if (freq > 0) 
      addLog(LOG_LEVEL_INFO, strformat(F("AS3935: TRCO frequency: %u Hz"), freq));
  }
/*
  {
    // Short test outputting SRCO on IRQ pin
    const uint32_t freq = measureResonanceFrequency(P169_IRQ_frequency_source::SRCO);
    if (freq > 0) 
      addLog(LOG_LEVEL_INFO, strformat(F("AS3935: SRCO frequency: %u Hz"), freq));
  }
  */
#endif

  // set the analog front end to 'indoors' or 'outdoors'
  _sensor.writeAFE(P169_GET_INDOOR ? AS3935MI::AS3935_INDOORS : AS3935MI::AS3935_OUTDOORS);

  {
    AS3935MI::noise_floor_threshold_t noise_floor_threshold = AS3935MI::AS3935_NFL_2;

    if ((P169_NOISE >= AS3935MI::AS3935_NFL_0) && (P169_NOISE <= AS3935MI::AS3935_NFL_7)) {
      noise_floor_threshold = static_cast<AS3935MI::noise_floor_threshold_t>(P169_NOISE);
    }

    _sensor.writeNoiseFloorThreshold(noise_floor_threshold);
  }
  {
    AS3935MI::wdth_setting_t wdth_setting = AS3935MI::AS3935_WDTH_2;

    if ((P169_WATCHDOG >= AS3935MI::AS3935_WDTH_0) && (P169_WATCHDOG <= AS3935MI::AS3935_WDTH_15)) {
      wdth_setting = static_cast<AS3935MI::wdth_setting_t>(P169_WATCHDOG);
    }
    _sensor.writeWatchdogThreshold(wdth_setting);
  }
  {
    AS3935MI::srej_setting_t srej_setting = AS3935MI::AS3935_SREJ_2;

    if ((P169_SPIKE_REJECTION >= AS3935MI::AS3935_SREJ_0) && (P169_SPIKE_REJECTION <= AS3935MI::AS3935_SREJ_15)) {
      srej_setting = static_cast<AS3935MI::srej_setting_t>(P169_SPIKE_REJECTION);
    }
    _sensor.writeSpikeRejection(srej_setting);
  }
  {
    AS3935MI::min_num_lightnings_t min_num_lightnings = AS3935MI::AS3935_MNL_1;

    if ((P169_LIGHTNING_THRESHOLD >= AS3935MI::AS3935_MNL_1) && (P169_LIGHTNING_THRESHOLD <= AS3935MI::AS3935_MNL_16)) {
      min_num_lightnings = static_cast<AS3935MI::min_num_lightnings_t>(P169_LIGHTNING_THRESHOLD);
    }
    _sensor.writeMinLightnings(min_num_lightnings);
  }

  _sensor.writeMaskDisturbers(P169_GET_MASK_DISTURBANCE);

  set_P169_interruptMode(P169_InterruptMode::normal);
  return true;
}

bool P169_data_struct::plugin_write(struct EventStruct *event, String& string)
{
  return true;
}

int P169_data_struct::getDistance()
{
  const uint8_t dist =  _sensor.readStormDistance();

  if (dist == AS3935MI::AS3935_DST_OOR) { return -1; }
  return dist;
}

uint32_t P169_data_struct::getEnergy()
{
  return _sensor.readEnergy();
}

void P169_data_struct::adjustForNoise()
{
  // if the noise floor threshold setting is not yet maxed out, increase the setting.
  // note that noise floor threshold events can also be triggered by an incorrect
  // analog front end setting.
  if (_sensor.increaseNoiseFloorThreshold()) {
    addLog(LOG_LEVEL_INFO, F("AS3935: Increased noise floor threshold"));
  }
  else {
    addLog(LOG_LEVEL_ERROR, F("AS3935: Noise floor threshold already at maximum"));
  }
}

void P169_data_struct::adjustForDisturbances()
{
  // increasing the Watchdog Threshold and / or Spike Rejection setting improves the AS3935s resistance
  // against disturbers but also decrease the lightning detection efficiency (see AS3935 datasheet)
  const uint8_t wdth = _sensor.readWatchdogThreshold();
  const uint8_t srej = _sensor.readSpikeRejection();


  // FIXME TD-er: Is this a good threshold for auto adjust algorithm?
  if ((wdth < AS3935MI::AS3935_WDTH_10) || (srej < AS3935MI::AS3935_SREJ_10))
  {
    _sense_adj_last = millis();

    // alternatively increase spike rejection and watchdog threshold
    if (srej < wdth)
    {
      if (_sensor.increaseSpikeRejection()) {
        addLog(LOG_LEVEL_INFO, strformat(F("AS3935: Increased spike rejection ratio to: %d"), (srej + 1)));
      }
      else {
        addLog(LOG_LEVEL_ERROR, F("AS3935: Spike rejection ratio already at maximum"));
      }
    }
    else
    {
      if (_sensor.increaseWatchdogThreshold()) {
        addLog(LOG_LEVEL_INFO, strformat(F("AS3935: Increased watchdog threshold to %d"), (wdth + 1)));
      }
      else {
        addLog(LOG_LEVEL_ERROR, F("AS3935: Watchdog threshold already at maximum"));
      }
    }
  }
  else
  {
    int32_t frequency{};

    if (validateCurrentResonanceFrequency(frequency)) {
      // Resonance frequency is still OK, so not much we can do here.
      addLog(LOG_LEVEL_ERROR, strformat(
               F("AS3935: Watchdog Threshold and Spike Rejection settings are already maxed out. Freq = %d"),
               frequency));
    } else {
      addLog(LOG_LEVEL_INFO, strformat(
               F("AS3935: Calibrate Resonance freq. Current frequency: %d"),
               frequency));

      if (calibrateResonanceFrequency(frequency)) {
        addLog(LOG_LEVEL_INFO, strformat(
                 F("AS3935: Calibrate Resonance freq. Current frequency: %d"),
                 frequency));

        // calibrate the RCO.
        if (!_sensor.calibrateRCO())
        {
          addLog(LOG_LEVEL_ERROR, F("AS3935: RCO Calibration failed."));
        } else {
          addLog(LOG_LEVEL_INFO, F("AS3935: RCO Calibration passed."));
        }
      }

      // FIXME TD-er: Should we do anything else here?
    }
    set_P169_interruptMode(P169_InterruptMode::normal);
  }
}

void P169_data_struct::tryIncreasedSensitivity()
{
  // increase sensor sensitivity every once in a while. _sense_increase_interval controls how quickly the code
  // attempts to increase sensitivity.
  if (timePassedSince(_sense_adj_last) > static_cast<long>(_sense_increase_interval))
  {
    _sense_adj_last = millis();

    addLog(LOG_LEVEL_INFO, F("AS3935: No disturber detected, attempting to decrease noise floor threshold."));

    const uint8_t wdth = _sensor.readWatchdogThreshold();
    const uint8_t srej = _sensor.readSpikeRejection();

    if ((wdth > AS3935MI::AS3935_WDTH_0) || (srej > AS3935MI::AS3935_SREJ_0))
    {
      // alternatively decrease spike rejection and watchdog threshold
      if (srej > wdth)
      {
        if (_sensor.decreaseSpikeRejection()) {
          addLog(LOG_LEVEL_INFO, strformat(F("AS3935: Decreased spike rejection ratio to %d"), (srej - 1)));
        }
        # ifndef BUILD_NO_DEBUG
        else {
          addLog(LOG_LEVEL_DEBUG, F("AS3935: Spike rejection ratio already at minimum"));
        }
        # endif // ifndef BUILD_NO_DEBUG
      }
      else
      {
        if (_sensor.decreaseWatchdogThreshold()) {
          addLog(LOG_LEVEL_INFO, strformat(F("AS3935: Decreased watchdog threshold to: %d"), (wdth - 1)));
        }
        # ifndef BUILD_NO_DEBUG
        else {
          addLog(LOG_LEVEL_DEBUG, F("AS3935: Watchdog threshold already at minimum"));
        }
        # endif // ifndef BUILD_NO_DEBUG
      }
    }
  }
}

#endif // ifdef USES_P169
