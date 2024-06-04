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

bool P169_data_struct::loop(struct EventStruct *event)
{
  if (_sensor.getInterruptMode() == AS3935MI::AS3935_INTERRUPT_NORMAL) {
    // FIXME TD-er: Should also check for state of IRQ pin as it may still be high if the interrupt souce isn't checked.
    const uint32_t timestamp = _sensor.getInterruptTimestamp();

    if ((timestamp != 0ul) || DIRECT_pinRead(_irqPin)) {
      if ((timestamp != 0ul) && (timePassedSince(timestamp) < 10)) {
        // Sensor not yet ready to report some data
        return false;
      }

      // query the interrupt source from the AS3935
      switch (_sensor.readInterruptSource()) {
        case AS3935MI::AS3935_INT_NH:

          // Noise floor too high
          adjustForNoise(event);
          break;
        case AS3935MI::AS3935_INT_D:

          // Disturbance detected
          // N.B. can be disabled with _sensor.writeMaskDisturbers(true);
          adjustForDisturbances(event);
          break;
        case AS3935MI::AS3935_INT_L:
        {
          // Lightning detected
          ++_lightningCount;

          // FIXME TD-er: What to do with the "Lightning Threshold" ?
          // If it was > 15 minutes ago since the last detected lightning,
          // or cleared statistics, then we should set _lightningCount to this
          // threshold value and also increment the total counter accordingly.

          const int totalStrikes = UserVar.getFloat(event->TaskIndex, 3) + 1;
          const uint32_t energy  = getEnergy();

          if (energy > _highestEnergy) { _highestEnergy = energy; }

          if (energy < _lowestEnergy) { _lowestEnergy = energy; }
          UserVar.setFloat(event->TaskIndex, 0, computeDistanceFromEnergy(_highestEnergy, NAN));
          UserVar.setFloat(event->TaskIndex, 1, computeDistanceFromEnergy(_lowestEnergy, NAN));
          UserVar.setFloat(event->TaskIndex, 2, _lightningCount);
          UserVar.setFloat(event->TaskIndex, 3, totalStrikes);

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLog(LOG_LEVEL_INFO, strformat(
                     F("AS3935: Lightning detected. DistNear: %.1f, DistFar: %.1f, Count: %u, Total: %d"),
                     computeDistanceFromEnergy(_highestEnergy, -1.0f),
                     computeDistanceFromEnergy(_lowestEnergy,  -1.0f),
                     _lightningCount,
                     totalStrikes));
          }

          if (Settings.UseRules) {
            // Lightning detected, Send event
            // Eventvalues:
            // - Distance
            // - Energy
            // - Lightning count since last PLUGIN_READ
            // - Total Lightning count since this was reset (or power cycle of ESP)
            eventQueue.addMove(
              strformat(
                F("%s#LightningDetected=%.1f,%.1f,%u,%d"),
                getTaskDeviceName(event->TaskIndex).c_str(),
                computeDistanceFromEnergy(_highestEnergy, -1.0f),
                computeDistanceFromEnergy(_lowestEnergy,  -1.0f),
                _lightningCount,
                totalStrikes));
          }

          return true;
        }
        case AS3935MI::AS3935_INT_DUPDATE:
        {
          // FIXME TD-er: No longer needed?


          // Distance updated
          const float distance = getDistance();

          if (_lightningCount > 0) {
            // Do not update until after this task interval or else the reported distance will be too low.
            UserVar.setFloat(event->TaskIndex, 0, distance);
          } else {
            _sensor.clearStatistics();
          }

          if (Settings.UseRules) {
            // Distance updated, Send event
            // Eventvalue:
            // - Distance
            eventQueue.addMove(
              strformat(
                F("%s#DistanceUpdated=%.2f"),
                getTaskDeviceName(event->TaskIndex).c_str(),
                distance));
          }
          break;
        }
      }
    }
    tryIncreasedSensitivity(event);
  }
  return false;
}

void P169_data_struct::html_show_sensor_info(struct EventStruct *event)
{
  addFormSubHeader(F("Current Sensor Data"));
  addRowLabel(F("Calibration"));
  const int8_t ant_cap = _sensor.getCalibratedAntCap();

  if (ant_cap != -1) {
    const float deviation_pct = computeDeviationPct(_sensor.getAntCapFrequency(ant_cap));

    if (fabs(deviation_pct) < (100 * AS3935MI_ALLOWED_DEVIATION)) {
      addEnabled(true);
    } else {
      if (P169_GET_TOLERANT_CALIBRATION_RANGE) {
        addEnabled(true);
        addHtml(F(HTML_SYMBOL_WARNING));
      } else {
        addEnabled(false);
      }
    }

    addRowLabel(F("Best Antenna cap"));
    addHtml(strformat(F("%d (%.2f%%)"), ant_cap, deviation_pct));
  } else {
    addEnabled(false);
  }

  addRowLabel(F("Error % per cap"));
# if FEATURE_CHART_JS
  addCalibrationChart(event);
# else // if FEATURE_CHART_JS

  for (uint8_t i = 0; i < 16; ++i) {
    const int32_t freq = _sensor.getAntCapFrequency(i);

    if (i != 0) {
      addHtml(',');
      addHtml(' ');
    }

    if (freq > 0) {
      addHtml(strformat(F("%.2f%%"), computeDeviationPct(freq)));
    } else {
      addHtml('-');
    }
  }
# endif // if FEATURE_CHART_JS

  addRowLabel(F("Current AFE gain"));
  addHtmlFloat(_afeGain, 2);
  addHtml('x');

  addRowLabel(F("Current Noise Floor Threshold"));
  addHtmlInt(_sensor.readNoiseFloorThreshold());

  addRowLabel(F("Current Watchdog Threshold"));
  addHtmlInt(_sensor.readWatchdogThreshold());

  addRowLabel(F("Current Spike Rejection"));
  addHtmlInt(_sensor.readSpikeRejection());
}

bool P169_data_struct::plugin_init(struct EventStruct *event)
{
  _sensor.setInterruptMode(AS3935MI::AS3935_INTERRUPT_DETACHED);

  if (!(_sensor.begin() && _sensor.checkConnection()))
  {
    addLog(LOG_LEVEL_ERROR, F("AS3935: Sensor not detected"));
    return false;
  }
  addLog(LOG_LEVEL_INFO, F("AS3935: Sensor detected"));

  /*
     if (!_sensor.checkIRQ())
     {
      addLog(LOG_LEVEL_ERROR, F("AS3935: IRQ pin connection check failed"));

      //    return false;
     }
   */

  calibrate(event);

# ifdef ESP32

  if (loglevelActiveFor(LOG_LEVEL_DEBUG))
  {
    // Short test checking effect of nr samples during calibration
    {
      String log = F("AS3935: Calibration test: ");

      for (size_t i = 0; i < 7; ++i) {
        const uint32_t nrSamples = 2048 >> i;
        log += strformat(F(",%d samples"), nrSamples);
      }
      addLogMove(LOG_LEVEL_DEBUG, log);
    }

    for (int antcap = 0; antcap < 16; ++antcap) {
      float deviation_pct[7]{};

      for (size_t i = 0; i < 7; ++i) {
        const uint32_t nrSamples = 2048 >> i;
        _sensor.setFrequencyMeasureNrSamples(nrSamples);
        const uint32_t freq = _sensor.measureResonanceFrequency(AS3935MI::display_frequency_source_t::LCO, antcap);

        if (freq > 0) {
          deviation_pct[i] = computeDeviationPct(freq);
        } else {
          deviation_pct[i] = 0.0f;
        }
      }
      String log = strformat(F("AS3935: LCO: cap %d "), antcap);

      for (size_t i = 0; i < 7; ++i) {
        log += strformat(F(",%.2f%%"), deviation_pct[i]);
      }
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
  }
# endif // ifdef ESP32


  // set the analog front end gain
  _sensor.writeNoiseFloorThreshold(AS3935MI::AS3935_NFL_2);
  _sensor.writeWatchdogThreshold(AS3935MI::AS3935_WDTH_2);
  _sensor.writeSpikeRejection(AS3935MI::AS3935_SREJ_2);
  setAFE_gain(event, P169_AFE_GAIN_LOW);
  {
    AS3935MI::min_num_lightnings_t min_num_lightnings = AS3935MI::AS3935_MNL_1;

    if ((P169_LIGHTNING_THRESHOLD >= AS3935MI::AS3935_MNL_1) && (P169_LIGHTNING_THRESHOLD <= AS3935MI::AS3935_MNL_16)) {
      min_num_lightnings = static_cast<AS3935MI::min_num_lightnings_t>(P169_LIGHTNING_THRESHOLD);
    }
    _sensor.writeMinLightnings(min_num_lightnings);
  }

  _sensor.writeMaskDisturbers(P169_GET_MASK_DISTURBANCE);

  _sensor.setInterruptMode(AS3935MI::AS3935_INTERRUPT_NORMAL);
  return true;
}

const char P169_subcommands[] PROGMEM = "clearstats|calibrate|setgain|setnf|setwd|setsrej";

enum class P169_subcmd_e : int8_t {
  invalid    = -1,
  clearstats = 0,
  calibrate,
  setgain,
  setnf,  // Set noise floor
  setwd,  // Set Watchdog Threshold
  setsrej // Set Spike Rejection
};

/*****************************************************
* plugin_write
*****************************************************/
bool P169_data_struct::plugin_write(struct EventStruct *event,
                                    String            & string) {
  bool success = false;

  const String command = parseString(string, 1);

  if (equals(command, F("as3935"))) {
    const String subcommand   = parseString(string, 2);
    const int    subcommand_i = GetCommandCode(subcommand.c_str(), P169_subcommands);

    if (subcommand_i < 0) { return false; } // Fail fast

    const P169_subcmd_e subcmd = static_cast<P169_subcmd_e>(subcommand_i);
    uint32_t   value{};
    const bool hasValue = validUIntFromString(parseString(string, 3), value);

    switch (subcmd) {
      case P169_subcmd_e::invalid:
        break;
      case P169_subcmd_e::clearstats:
        clearStatistics();
        success = true;
        break;
      case P169_subcmd_e::calibrate:
        calibrate(event);
        _sensor.setInterruptMode(AS3935MI::AS3935_INTERRUPT_NORMAL);

        success = true;
        break;
      case P169_subcmd_e::setgain:

        if (hasValue) {
          success = true;

          // First check if it is a register value or gain factor.
          setAFE_gain(event, AFE_gain_to_regValue(value));
        }
        break;
      case P169_subcmd_e::setnf:

        if (hasValue) {
          success = true;
          setNoiseFloorThreshold(event, value);
        }
        break;
      case P169_subcmd_e::setwd:

        if (hasValue) {
          success = true;
          _sensor.writeWatchdogThreshold(value);
          sendChangeEvent(event);
        }
        break;
      case P169_subcmd_e::setsrej:

        if (hasValue) {
          success = true;
          _sensor.writeSpikeRejection(value);
          sendChangeEvent(event);
        }
        break;
    }
  }
  return success;
}

const char P169_get_config[] PROGMEM = "noisefloor|watchdog|srej|gain";

enum class P169_get_config_e : int8_t {
  invalid    = -1,
  noisefloor = 0, // [<taskname>#noisefloor]
  watchdog,       // [<taskname>#watchdog]
  srej,           // [<taskname>#srej] = current spike rejection
  gain            // [<taskname>#gain]
};


/*****************************************************
* plugin_get_config_value
*****************************************************/
bool P169_data_struct::plugin_get_config_value(struct EventStruct *event,
                                               String            & string) {
  const String var      = parseString(string, 1);
  const int    config_i = GetCommandCode(var.c_str(), P169_get_config);

  if (config_i < 0) { return false; } // Fail fast
  const P169_get_config_e config = static_cast<P169_get_config_e>(config_i);

  switch (config)
  {
    case P169_get_config_e::invalid:
      return false;
    case P169_get_config_e::noisefloor:
      string = _sensor.readNoiseFloorThreshold();
      break;
    case P169_get_config_e::watchdog:
      string = _sensor.readWatchdogThreshold();
      break;
    case P169_get_config_e::srej:
      string = _sensor.readSpikeRejection();
      break;
    case P169_get_config_e::gain:
      string = toString(_afeGain, 2);
      break;
  }
  return true;
}

float P169_data_struct::getDistance()
{
  return computeDistanceFromEnergy(getEnergy(), -1.0f);
}

uint32_t P169_data_struct::getEnergy()
{
  const uint32_t rawEnergy = _sensor.readEnergy();

  if ((rawEnergy == 0) || (rawEnergy == 0xFFFFFFFF)) {
    return 0u;
  }

  const int8_t ant_cap = _sensor.getCalibratedAntCap();

  if (ant_cap != -1) {
    const float deviation_pct = computeDeviationPct(_sensor.getAntCapFrequency(ant_cap));

    // Compute correction factor for loss in reported energy due to offset from perfect calibration.
    // Formula derived by TD-er using chart on this site: (section "Is Tuning Important?")
    // https://sites.google.com/view/as3935workbook/home#h.n9qonjaydsbd
    const float loss =
      (0.0321f * deviation_pct * deviation_pct) +
      (0.0279f * deviation_pct) +
      1.0f;

    return static_cast<uint32_t>((rawEnergy * loss) / _afeGain);
  }

  // No antenna calibration present, so no compensation possible
  return static_cast<uint32_t>(rawEnergy / _afeGain);
}

uint32_t P169_data_struct::getAndClearLightningCount()
{
  const uint32_t res = _lightningCount;

  _lightningCount = 0;
  _highestEnergy  = 0;
  _lowestEnergy   = 0xFFFFFFFF;
  return res;
}

void P169_data_struct::clearStatistics()
{
  _sensor.clearStatistics();
}

float P169_data_struct::computeDeviationPct(uint32_t LCO_freq)
{
  return (LCO_freq / 5000.0f) - 100.0f;
}

float P169_data_struct::computeDistanceFromEnergy(uint32_t energy, float errorValue)
{
  if ((energy == 0) || (energy == 0xFFFFFFFF)) { return errorValue; }

  // TD-er: Distance vs Energy attenuation is roughly X / sqrt(energy) for some factor X.
  // Factor of 2100 was determined experimentally evaluating a number of thunder storms
  // by Michael Gasperi, the author of this site: https://sites.google.com/view/as3935workbook/home
  // Verified by TD-er comparing live data mapped on https://map.blitzortung.org/ and the sensor.
  // LCO calibration offset was taken into account.
  return 2100.0f / sqrtf(energy);
}

bool P169_data_struct::calibrate(struct EventStruct *event)
{
  _sensor.setInterruptMode(AS3935MI::AS3935_INTERRUPT_DETACHED);


  // calibrate the resonance frequency. failing the resonance frequency could indicate an issue
  // of the sensor.
  int32_t frequency = 0;

  _sensor.setCalibrateAllAntCap(P169_GET_SLOW_LCO_CALIBRATION);

  _sensor.setFrequencyMeasureNrSamples(P169_GET_SLOW_LCO_CALIBRATION ? AS3935MI_NR_CALIBRATION_SAMPLES : (AS3935MI_NR_CALIBRATION_SAMPLES /
                                                                                                          2));

  if (!_sensor.calibrateResonanceFrequency(frequency))
  {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      addLog(LOG_LEVEL_ERROR,
             strformat(F("AS3935: Resonance Frequency Calibration failed: %d Hz not in range 482500 Hz ... 517500 Hz"), frequency));
    }
  } else {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO,
             strformat(F("AS3935: Resonance Frequency Calibration passed: ant_cap: %d, %d Hz, deviation: %.2f%%"),
                       _sensor.readAntennaTuning(),
                       frequency, computeDeviationPct(frequency)));
    }
  }

  // calibrate the RCO.
  if (!_sensor.calibrateRCO())
  {
    addLog(LOG_LEVEL_ERROR, F("AS3935: RCO Calibration failed."));
  } else {
    addLog(LOG_LEVEL_INFO, F("AS3935: RCO Calibration passed."));
  }

  // stop displaying LCO on IRQ
  _sensor.displayLcoOnIrq(false);

  return frequency != 0;
}

void P169_data_struct::adjustForNoise(struct EventStruct *event)
{
  // if the noise floor threshold setting is not yet maxed out, increase the setting.
  // note that noise floor threshold events can also be triggered by an incorrect
  // analog front end setting.
  uint8_t nf_lev{};

  if (_sensor.increaseNoiseFloorThreshold(nf_lev)) {
    addLog(LOG_LEVEL_INFO, strformat(F("AS3935: Increased noise floor threshold to: %u"), nf_lev));
    sendChangeEvent(event);
  }
  else {
    addLog(LOG_LEVEL_ERROR, F("AS3935: Noise floor threshold already at maximum"));
  }
}

void P169_data_struct::adjustForDisturbances(struct EventStruct *event)
{
  // increasing the Watchdog Threshold and / or Spike Rejection setting improves the AS3935s resistance
  // against disturbers but also decrease the lightning detection efficiency (see AS3935 datasheet)
  const uint8_t wdth  = _sensor.readWatchdogThreshold();
  const uint8_t srej  = _sensor.readSpikeRejection();
  const uint8_t noise = _sensor.readNoiseFloorThreshold();

  if ((wdth == AS3935MI::AS3935_WDTH_5) ||
      (srej == AS3935MI::AS3935_SREJ_5) ||
      (noise == AS3935MI::AS3935_NFL_5))
  {
    int32_t frequency{};
    const bool valid = _sensor.validateCurrentResonanceFrequency(frequency);

    if (valid || P169_GET_TOLERANT_CALIBRATION_RANGE) {
      // Resonance frequency is still OK, try lowering gain
      uint8_t curGain = _sensor.readAFE();

      if (curGain > P169_AFE_GAIN_LOW) {
        --curGain;

        // Since we change the gain, reset the other values to default
        _sensor.writeNoiseFloorThreshold(AS3935MI::AS3935_NFL_2);
        _sensor.writeWatchdogThreshold(AS3935MI::AS3935_WDTH_2);
        _sensor.writeSpikeRejection(AS3935MI::AS3935_SREJ_2);
        setAFE_gain(event, curGain);
        _sense_adj_last = millis();
      } else
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        addLog(LOG_LEVEL_ERROR, strformat(
                 F("AS3935: Watchdog Threshold and Spike Rejection settings are already maxed out. Freq = %d"),
                 frequency));
      }
    } else if (timePassedSince(_sense_adj_last) > static_cast<long>(_sense_increase_interval))
    {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO, strformat(
                 F("AS3935: Calibrate Resonance freq. Current frequency: %d"),
                 frequency));
      }

      if (_sensor.calibrateResonanceFrequency(frequency)) {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, strformat(
                   F("AS3935: Calibrate Resonance freq. Current frequency: %d"),
                   frequency));
        }

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
    _sensor.setInterruptMode(AS3935MI::AS3935_INTERRUPT_NORMAL);
  }

  // FIXME TD-er: Is this a good threshold for auto adjust algorithm?
  if ((wdth < AS3935MI::AS3935_WDTH_5) ||
      (srej < AS3935MI::AS3935_SREJ_5)

      //      || (noise < AS3935MI::AS3935_NFL_5)
      )
  {
    _sense_adj_last = millis();

    // alternatively increase spike rejection and watchdog threshold
    if (srej < wdth)
    {
      if (_sensor.increaseSpikeRejection()) {
        sendChangeEvent(event);

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, strformat(F("AS3935: Increased spike rejection ratio to: %d"), (srej + 1)));
        }
      }
      else {
        addLog(LOG_LEVEL_ERROR, F("AS3935: Spike rejection ratio already at maximum"));
      }
    }
    else
    {
      if (_sensor.increaseWatchdogThreshold()) {
        sendChangeEvent(event);

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, strformat(F("AS3935: Increased watchdog threshold to %d"), (wdth + 1)));
        }
      }
      else {
        addLog(LOG_LEVEL_ERROR, F("AS3935: Watchdog threshold already at maximum"));
      }
    }
  }
}

void P169_data_struct::tryIncreasedSensitivity(struct EventStruct *event)
{
  // increase sensor sensitivity every once in a while. _sense_increase_interval controls how quickly the code
  // attempts to increase sensitivity.
  if (timePassedSince(_sense_adj_last) > static_cast<long>(_sense_increase_interval))
  {
    _sense_adj_last = millis();

    addLog(LOG_LEVEL_INFO, F("AS3935: No disturber detected, attempting to decrease noise floor threshold."));

    const uint8_t wdth  = _sensor.readWatchdogThreshold();
    const uint8_t srej  = _sensor.readSpikeRejection();
    const uint8_t noise = _sensor.readNoiseFloorThreshold();

    if ((wdth == AS3935MI::AS3935_WDTH_0) ||
        (srej == AS3935MI::AS3935_SREJ_0) ||
        (noise == AS3935MI::AS3935_NFL_0))
    {
      uint8_t curGain = _sensor.readAFE();

      if (curGain < P169_AFE_GAIN_HIGH) {
        ++curGain;

        // Since we change the gain, reset the other values to default
        _sensor.writeNoiseFloorThreshold(AS3935MI::AS3935_NFL_2);
        _sensor.writeWatchdogThreshold(AS3935MI::AS3935_WDTH_2);
        _sensor.writeSpikeRejection(AS3935MI::AS3935_SREJ_2);

        setAFE_gain(event, curGain);
        return;
      }
    }

    if ((noise > srej) && (noise > wdth) && _sensor.decreaseNoiseFloorThreshold()) {
      sendChangeEvent(event);

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO, strformat(F("AS3935: Decreased noise floor to %d"), (noise - 1)));
      }
    }

    // alternatively decrease spike rejection and watchdog threshold
    if (srej > wdth)
    {
      if (_sensor.decreaseSpikeRejection()) {
        sendChangeEvent(event);

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, strformat(F("AS3935: Decreased spike rejection ratio to %d"), (srej - 1)));
        }
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
        sendChangeEvent(event);

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, strformat(F("AS3935: Decreased watchdog threshold to: %d"), (wdth - 1)));
        }
      }
        # ifndef BUILD_NO_DEBUG
      else {
        addLog(LOG_LEVEL_DEBUG, F("AS3935: Watchdog threshold already at minimum"));
      }
        # endif // ifndef BUILD_NO_DEBUG
    }
  }
}

void P169_data_struct::setAFE_gain(struct EventStruct *event, uint8_t gain)
{
  _afeGain       = regValue_AFE_gain_toFloat(gain);
  _afeGainRegval = gain;
  _sensor.writeAFE(gain);

  sendChangeEvent(event);
}

void P169_data_struct::setNoiseFloorThreshold(struct EventStruct *event, uint8_t noiseFloor)
{
  _sensor.writeNoiseFloorThreshold(noiseFloor);
  sendChangeEvent(event);
}

float P169_data_struct::regValue_AFE_gain_toFloat(uint8_t gain)
{
  // Source: https://sites.google.com/view/as3935workbook/home
  float afeGain = 1.0f;

  switch (gain)
  {
    case 10: afeGain = 0.30f; break;
    case 11: afeGain = 0.40f; break;
    case 12: afeGain = 0.55f; break;
    case 13: afeGain = 0.74f; break;
    case 14: afeGain = 1.00f; break; // Datasheet: "Outdoor"
    case 15: afeGain = 1.35f; break;
    case 16: afeGain = 1.83f; break;
    case 17: afeGain = 2.47f; break;
    case 18: afeGain = 3.34f; break; // Datasheet: "Indoor"
  }
  return afeGain;
}

uint8_t P169_data_struct::AFE_gain_to_regValue(float gain)
{
  {
    uint8_t regval = static_cast<uint8_t>(roundf(gain));

    if (regval >= 10) {
      if (regval <= 18) {
        return regval;
      }
      return AS3935MI::AS3935_OUTDOORS;
    }
  }

  float prevAFE_Gain = regValue_AFE_gain_toFloat(10);

  if (gain < prevAFE_Gain) {
    return 10;
  }

  for (uint8_t regval = 11; regval <= 18; ++regval)
  {
    const float afeGain = regValue_AFE_gain_toFloat(regval);

    if (gain < afeGain) {
      // See which is closest, prev or current
      if ((gain - prevAFE_Gain) < (afeGain - gain)) {
        return regval - 1;
      }
      return regval;
    }
    prevAFE_Gain = afeGain;
  }

  return AS3935MI::AS3935_INDOORS;
}

void P169_data_struct::sendChangeEvent(struct EventStruct *event)
{
  if (Settings.UseRules) {
    const   uint8_t noiseFloor = _sensor.readNoiseFloorThreshold();
    const   uint8_t watchdog   = _sensor.readWatchdogThreshold();
    const   uint8_t srej       = _sensor.readSpikeRejection();

    if (((_lastEvent_noiseFloor != noiseFloor)  && ((_lastEvent_noiseFloor > 1) || (noiseFloor > 1))) ||
        ((_lastEvent_watchdog != watchdog) && ((_lastEvent_watchdog > 1) || (watchdog > 1)) ) ||
        (_lastEvent_srej != srej) ||
        (_lastEvent_gain != _afeGainRegval)) {
      // Some value was updated, send event
      // Eventvalue:
      // - Gain
      // - Noise Level
      // - Watchdog Threshold
      // - Spike Rejection
      eventQueue.addMove(
        strformat(
          F("%s#ParamUpdate=%.2f,%u,%u,%u"),
          getTaskDeviceName(event->TaskIndex).c_str(),
          _afeGain,
          noiseFloor,
          watchdog,
          srej));
    }

    _lastEvent_noiseFloor = noiseFloor;
    _lastEvent_watchdog   = watchdog;
    _lastEvent_srej       = srej;
    _lastEvent_gain       = _afeGainRegval;
  }
}

# if FEATURE_CHART_JS
void P169_data_struct::addCalibrationChart(struct EventStruct *event)
{
  const int valueCount = 16;
  int   xAxisValues[valueCount]{};
  float values[valueCount]{};

  int actualValueCount = 0;

  for (int i = 0; i < valueCount; ++i) {
    const int32_t freq = _sensor.getAntCapFrequency(i);

    if (freq > 0) {
      values[actualValueCount]      = computeDeviationPct(freq);
      xAxisValues[actualValueCount] = i;
      ++actualValueCount;
    }
  }

  String axisOptions;

  {
    ChartJS_options_scales scales;
    scales.add({ F("x"), F("Antenna capacitor") });
    scales.add({ F("y"), F("Error (%)") });
    axisOptions = scales.toString();
  }

  add_ChartJS_chart_header(
    F("line"),
    F("lcoCapErrorCurve"),
    { F("LCO Resonance Frequency") },
    500,
    500,
    axisOptions);

  add_ChartJS_chart_labels(
    actualValueCount,
    xAxisValues);

  {
    const ChartJS_dataset_config config(
      F("Error %"),
      F("rgb(255, 99, 132)"));


    add_ChartJS_dataset(
      config,
      values,
      actualValueCount,
      2);
  }
  add_ChartJS_chart_footer();
}

# endif // if FEATURE_CHART_JS


#endif  // ifdef USES_P169
