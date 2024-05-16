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


P169_data_struct::P169_data_struct(struct EventStruct *event) :
  _sensor(P169_I2C_ADDRESS, P169_IRQ_PIN),
  _irqPin(P169_IRQ_PIN)
{}

P169_data_struct::~P169_data_struct()
{
  _sensor.writePowerDown(true);
}

bool P169_data_struct::loop()
{
  if (_sensor.get_interruptMode() == AS3935MI::interrupt_mode_t::normal) {
    // FIXME TD-er: Should also check for state of IRQ pin as it may still be high if the interrupt souce isn't checked.
    const uint32_t timestamp = _sensor.get_interruptTimestamp();

    if ((timestamp != 0ul) || DIRECT_pinRead(_irqPin)) {
      if ((timestamp != 0ul) && (timePassedSince(timestamp) < 2)) {
        // Sensor not yet ready to report what
        return false;
      }

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
  _sensor.set_interruptMode(AS3935MI::interrupt_mode_t::detached);

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

  _sensor.setFrequencyMeasureNrSamples(256);

  if (!_sensor.calibrateResonanceFrequency(frequency))
  {
    addLog(LOG_LEVEL_ERROR,
           strformat(F("AS3935: Resonance Frequency Calibration failed: %d Hz not in range 482500 Hz ... 517500 Hz"), frequency));
    return false;
  }

  const float deviation_pct = (frequency / 5000.0f) - 100.0f;

  addLog(LOG_LEVEL_INFO,
         strformat(F("AS3935: Resonance Frequency Calibration passed: ant_cap: %d, %d Hz, deviation: %.2f%%"), _sensor.readAntennaTuning(),
                   frequency, deviation_pct));

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

# ifdef ESP32
  {
    // Short test checking effect of nr samples during calibration
    {
      String log = F("AS3935: Calibration test: ");

      for (size_t i = 0; i < 7; ++i) {
        const uint32_t nrSamples = 2048 >> i;
        log                     += strformat(F(",%d samples"), nrSamples);
      }
      addLogMove(LOG_LEVEL_INFO, log);
    }

    for (int antcap = 0; antcap < 16; ++antcap) {
      float deviation_pct[7]{};

      for (size_t i = 0; i < 7; ++i) {
        const uint32_t nrSamples = 2048 >> i;
        _sensor.setFrequencyMeasureNrSamples(nrSamples);
        const uint32_t freq = _sensor.measureResonanceFrequency(AS3935MI::display_frequency_source_t::LCO, antcap);

        if (freq > 0) {
          deviation_pct[i] = (freq / 5000.0f) - 100.0f;
        } else {
          deviation_pct[i] = 0.0f;
        }
      }
      String log = strformat(F("AS3935: LCO: cap %d "), antcap);

      for (size_t i = 0; i < 7; ++i) {
        log += strformat(F(",%.2f%%"), deviation_pct[i]);
      }
      addLogMove(LOG_LEVEL_INFO, log);
    }
  }
# endif // ifdef ESP32


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

  _sensor.set_interruptMode(AS3935MI::interrupt_mode_t::normal);
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

    if (_sensor.validateCurrentResonanceFrequency(frequency)) {
      // Resonance frequency is still OK, so not much we can do here.
      addLog(LOG_LEVEL_ERROR, strformat(
               F("AS3935: Watchdog Threshold and Spike Rejection settings are already maxed out. Freq = %d"),
               frequency));
    } else {
      addLog(LOG_LEVEL_INFO, strformat(
               F("AS3935: Calibrate Resonance freq. Current frequency: %d"),
               frequency));

      if (_sensor.calibrateResonanceFrequency(frequency)) {
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
    _sensor.set_interruptMode(AS3935MI::interrupt_mode_t::normal);
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
