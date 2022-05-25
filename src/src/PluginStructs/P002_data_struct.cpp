#include "../PluginStructs/P002_data_struct.h"

#ifdef USES_P002

# include "../Helpers/Rules_calculate.h"


void P002_formatStatistics(const __FlashStringHelper *label, int raw, float float_value) {
  addRowLabel(label);
  addHtmlInt(raw);
  html_add_estimate_symbol();
  addHtml(toString(float_value, 3));
}

P002_data_struct::P002_data_struct(struct EventStruct *event)
{
  _sampleMode = P002_OVERSAMPLING;

  # ifdef ESP8266
  _pin_analogRead = A0;
  # endif // if defined(ESP8266)
  # if defined(ESP32)
  _pin_analogRead = CONFIG_PIN1;
  # endif // if defined(ESP32)

  if (P002_CALIBRATION_ENABLED) {
    _calib_adc1 = P002_CALIBRATION_POINT1;
    _calib_adc2 = P002_CALIBRATION_POINT2;
    _calib_out1 = P002_CALIBRATION_VALUE1;
    _calib_out2 = P002_CALIBRATION_VALUE2;
  }

  LoadTaskSettings(event->TaskIndex);
  _nrDecimals        = ExtraTaskSettings.TaskDeviceValueDecimals[0];
  _nrMultiPointItems = P002_NR_MULTIPOINT_ITEMS;

  load(event);

  /*

     // hard-code some values of a wind-vane to test
     _multipoint.emplace_back(33000,  0);
     _multipoint.emplace_back(6570,   22.5);
     _multipoint.emplace_back(8200,   45);
     _multipoint.emplace_back(891,    67.5);
     _multipoint.emplace_back(1000,   90);
     _multipoint.emplace_back(688,    112.5);
     _multipoint.emplace_back(2200,   135);
     _multipoint.emplace_back(1410,   157.5);
     _multipoint.emplace_back(3900,   180);
     _multipoint.emplace_back(3140,   202.5);
     _multipoint.emplace_back(16000,  225);
     _multipoint.emplace_back(14120,  247.5);
     _multipoint.emplace_back(120000, 270);
     _multipoint.emplace_back(42120,  292.5);
     _multipoint.emplace_back(64900,  315);
     _multipoint.emplace_back(21880,  337.5);

     std::sort(_multipoint.begin(), _multipoint.end());

     _binning.resize(_multipoint.size(), 0);
     _binningRange.resize(_multipoint.size());

     _formula = RulesCalculate_t::preProces(String(F("(-10000*%value%)/(%value%-3.3)")));
   */
}

void P002_data_struct::load(struct EventStruct *event)
{
  const size_t nr_lines = P002_Nlines;

  {
    String lines[nr_lines];
    LoadCustomTaskSettings(event->TaskIndex, lines, nr_lines, 0);
    const int stored_nr_lines = lines[P002_SAVED_NR_LINES].toInt();
    _formula              = lines[P002_LINE_INDEX_FORMULA];
    _formula_preprocessed = RulesCalculate_t::preProces(_formula);

    for (size_t i = P002_LINE_IDX_FIRST_MP; i < nr_lines && i < stored_nr_lines; i += P002_STRINGS_PER_MP) {
      float adc, value = 0.0f;

      if (validFloatFromString(lines[i], adc) && validFloatFromString(lines[i + 1], value)) {
        _multipoint.emplace_back(adc, value);
      }
    }
  }
  std::sort(_multipoint.begin(), _multipoint.end());

  _binning.resize(_multipoint.size(), 0);
  _binningRange.resize(_multipoint.size());
}

void P002_data_struct::webformLoad(struct EventStruct *event)
{
          # if defined(ESP32)
  addHtml(F("<TR><TD>Analog Pin:<TD>"));
  addADC_PinSelect(AdcPinSelectPurpose::ADC_Touch_HallEffect, F("taskdevicepin1"), CONFIG_PIN1);

      # endif // if defined(ESP32)

  {
    const __FlashStringHelper *outputOptions[] = {
      F("Use Current Sample"),
      F("Oversampling"),
      F("Binning")
    };
    const int outputOptionValues[] = {
      P002_USE_CURENT_SAMPLE,
      P002_USE_OVERSAMPLING,
      P002_USE_BINNING };
    addFormSelector(F("Oversampling"), F("p002_oversampling"), 3, outputOptions, outputOptionValues, P002_OVERSAMPLING);
  }

  addFormSubHeader(F("Two Point Calibration"));

  addFormCheckBox(F("Calibration Enabled"), F("p002_cal"), P002_CALIBRATION_ENABLED);

  addFormNumericBox(F("Point 1"), F("p002_adc1"), P002_CALIBRATION_POINT1, 0, P002_MAX_ADC_VALUE);
  html_add_estimate_symbol();
  addTextBox(F("p002_out1"), toString(P002_CALIBRATION_VALUE1, 3), 10);

  addFormNumericBox(F("Point 2"), F("p002_adc2"), P002_CALIBRATION_POINT2, 0, P002_MAX_ADC_VALUE);
  html_add_estimate_symbol();
  addTextBox(F("p002_out2"), toString(P002_CALIBRATION_VALUE2, 3), 10);

  {
    // Output the statistics for the current settings.
    int raw_value     = 0;
    const float value = P002_data_struct::getCurrentValue(event, raw_value);
    P002_formatStatistics(F("Current"), raw_value, value);

    if (P002_CALIBRATION_ENABLED) {
      P002_formatStatistics(F("Minimum"),   0,                  P002_data_struct::applyCalibration(event, 0));
      P002_formatStatistics(F("Maximum"),   P002_MAX_ADC_VALUE, P002_data_struct::applyCalibration(event, P002_MAX_ADC_VALUE));

      float stepsize = P002_data_struct::applyCalibration(event, 1.0f) - P002_data_struct::applyCalibration(event, 0.0f);
      P002_formatStatistics(F("Step size"), 1,                  stepsize);
    }
  }

  addFormSubHeader(F("Multipoint Processing"));
  addFormCheckBox(F("Multipoint Processing Enabled"), F("p002_multi_en"), P002_MULTIPOINT_ENABLED);

  addFormTextBox(F("Binning Formula"), getPluginCustomArgName(P002_LINE_INDEX_FORMULA), _formula, P002_MAX_FORMULA_LENGTH);

  addFormNumericBox(F("Nr Multipoint Fields"), F("p002_nr_mp"), P002_NR_MULTIPOINT_ITEMS, 0, P002_MAX_NR_MP_ITEMS);

  // Checkbox needed to explicitly allow to split-paste over each field
  addFormCheckBox(F("Split-Paste Multipoint Fields"), F("splitpaste"), false);
  addFormNote(F("When checked, a set of tab, space or newline separated values can be pasted at once."));

  size_t line_nr = 0;

  for (size_t varNr = P002_LINE_IDX_FIRST_MP; varNr < P002_Nlines; varNr += P002_STRINGS_PER_MP)
  {
    const String label = String(F("Point ")) + String(line_nr + 1);
    const double adc   = _multipoint[line_nr]._adc;
    const double val   = _multipoint[line_nr]._value;
    addFormTextBox(F("query-input widenumber"),
                   label,
                   getPluginCustomArgName(varNr),
                   _multipoint.size() > line_nr ? doubleToString(adc, _nrDecimals, true) : EMPTY_STRING,
                   0);
    html_add_estimate_symbol();
    addTextBox(getPluginCustomArgName(varNr + 1),
               _multipoint.size() > line_nr ?  doubleToString(val, _nrDecimals, true) : EMPTY_STRING,
               0,
               false,
               false,
               EMPTY_STRING,
               F("query-input widenumber"));

    ++line_nr;
  }
}

String P002_data_struct::webformSave(struct EventStruct *event)
{
  P002_OVERSAMPLING = getFormItemInt(F("p002_oversampling"));

  P002_CALIBRATION_ENABLED = isFormItemChecked(F("p002_cal"));

  P002_CALIBRATION_POINT1 = getFormItemInt(F("p002_adc1"));
  P002_CALIBRATION_VALUE1 = getFormItemFloat(F("p002_out1"));

  P002_CALIBRATION_POINT2 = getFormItemInt(F("p002_adc2"));
  P002_CALIBRATION_VALUE2 = getFormItemFloat(F("p002_out2"));

  P002_MULTIPOINT_ENABLED = isFormItemChecked(F("p002_multi_en"));

  P002_NR_MULTIPOINT_ITEMS = getFormItemInt(F("p002_nr_mp"));

  const size_t nr_lines = P002_Nlines;
  String lines[nr_lines];

  // Store nr of lines that were saved, so no 'old' data will be read when nr of multi-point items has changed.
  lines[P002_SAVED_NR_LINES]     = String(nr_lines);
  lines[P002_LINE_INDEX_FORMULA] = webArg(getPluginCustomArgName(P002_LINE_INDEX_FORMULA));

  const int nrDecimals = webArg(F("TDVD1")).toInt();

  for (size_t varNr = P002_LINE_IDX_FIRST_MP; varNr < nr_lines; varNr += P002_STRINGS_PER_MP)
  {
    float adc, value = 0.0f;
    const String adc_str = webArg(getPluginCustomArgName(varNr));
    const String val_str = webArg(getPluginCustomArgName(varNr + 1));

    if (validFloatFromString(adc_str, adc) && validFloatFromString(val_str, value)) {
      // Only store valid floats
      lines[varNr]     = adc_str;
      lines[varNr + 1] = val_str;
    }
  }

  return SaveCustomTaskSettings(event->TaskIndex, lines, nr_lines, 0);
}

void P002_data_struct::takeSample()
{
  switch (_sampleMode) {
    case P002_USE_OVERSAMPLING:
      addOversamplingValue(espeasy_analogRead(_pin_analogRead));
      break;
    case P002_USE_BINNING:
      addBinningValue(espeasy_analogRead(_pin_analogRead));
      break;
  }
}

bool P002_data_struct::getValue(float& float_value,
                                int  & raw_value) const
{
  bool mustTakeSample = false;

  switch (_sampleMode) {
    case P002_USE_OVERSAMPLING:

      if (getOversamplingValue(float_value, raw_value)) { return true; }
      mustTakeSample = true;
      break;
    case P002_USE_BINNING:

      if (getBinnedValue(float_value, raw_value)) { return true; }
      mustTakeSample = true;
      break;
    case P002_USE_CURENT_SAMPLE:
      mustTakeSample = true;
      break;
  }

  if (!mustTakeSample) {
    return false;
  }

  raw_value   = espeasy_analogRead(_pin_analogRead);
  float_value = static_cast<float>(raw_value);
  float_value = applyMultiPointInterpolation(applyCalibration(float_value));
  return true;
}

void P002_data_struct::reset()
{
  switch (_sampleMode) {
    case P002_USE_OVERSAMPLING:
      resetOversampling();
      break;
    case P002_USE_BINNING:
    {
      for (auto it = _binning.begin(); it != _binning.end(); ++it) {
        *it = 0;
      }

      break;
    }
  }
}

void P002_data_struct::resetOversampling() {
  OversamplingValue  = 0;
  OversamplingCount  = 0;
  OversamplingMinVal = P002_MAX_ADC_VALUE;
  OversamplingMaxVal = -P002_MAX_ADC_VALUE;
}

void P002_data_struct::addOversamplingValue(int currentValue) {
  // Extra check to only add min or max readings once.
  // They will be taken out of the averaging only one time.
  if ((currentValue == 0) && (currentValue == OversamplingMinVal)) {
    return;
  }

  if ((currentValue == P002_MAX_ADC_VALUE) && (currentValue == OversamplingMaxVal)) {
    return;
  }

  OversamplingValue += currentValue;
  ++OversamplingCount;

  if (currentValue > OversamplingMaxVal) {
    OversamplingMaxVal = currentValue;
  }

  if (currentValue < OversamplingMinVal) {
    OversamplingMinVal = currentValue;
  }
}

bool P002_data_struct::getOversamplingValue(float& float_value, int& raw_value) const {
  if (OversamplingCount > 0) {
    float sum   = static_cast<float>(OversamplingValue);
    float count = static_cast<float>(OversamplingCount);

    if (OversamplingCount >= 3) {
      sum   -= OversamplingMaxVal;
      sum   -= OversamplingMinVal;
      count -= 2;
    }
    float_value = sum / count;
    raw_value   = static_cast<int>(float_value);

    // We counted the raw oversampling values, so now we need to apply the calibration and multi-point processing
    float_value = applyMultiPointInterpolation(applyCalibration(float_value));

    return true;
  }
  return false;
}

int P002_data_struct::getBinIndex(float currentValue) const
{
  const size_t mp_size = _multipoint.size();

  if (mp_size == 0) { return -1; }

  if (mp_size == 1) { return 0; }

  if (currentValue <= _multipoint[0]._adc) { return 0; }

  const size_t last_mp_index = mp_size - 1;

  if (currentValue >= _multipoint[last_mp_index]._adc) { return last_mp_index; }

  for (unsigned int i = 0; i < last_mp_index; ++i) {
    const float dist_left  = currentValue - _multipoint[i]._adc;
    const float dist_right = _multipoint[i + 1]._adc - currentValue;

    if ((dist_left >= 0) && (dist_right >= 0)) {
      // Inbetween 2 points of the multipoint array
      return (dist_left < dist_right) ? i : i + 1;
    }
  }

  return -1;
}

void P002_data_struct::addBinningValue(int currentValue)
{
  for (int index = 0; index < _binningRange.size(); ++index) {
    if (_binningRange[index].inRange(currentValue)) {
      ++_binning[index];
      return;
    }
  }

  // First apply calibration, then find the bin index
  float calibrated_value = applyCalibration(static_cast<float>(currentValue));

  // FIXME TD-er: hard-coded formula, must be computed before binning
  String formula = _formula_preprocessed;

  formula.replace(F("%value%"), toString(calibrated_value, _nrDecimals));

  double result = 0;

  if (!isError(RulesCalculate.doCalculate(parseTemplate(formula).c_str(), &result))) {
    calibrated_value = result;
  }

  const int index = getBinIndex(calibrated_value);

  if ((index >= 0) && (_binning.size() > index)) {
    _binningRange[index].set(currentValue);
    ++_binning[index];
  }
}

bool P002_data_struct::getBinnedValue(float& float_value, int& raw_value) const
{
  unsigned int highest_bin_count = 0;

  for (unsigned int i = 0; i < _binning.size(); ++i) {
    if (_binning[i] > highest_bin_count) {
      highest_bin_count = _binning[i];
      float_value       = _multipoint[i]._value;
      raw_value         = _multipoint[i]._adc;
    }
  }
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("ADC getBinnedValue: bin cnt: ");

    log += highest_bin_count;
    log += F(" Value: ");
    log += float_value;
    log += F(" RAW: ");
    log += raw_value;
    addLog(LOG_LEVEL_DEBUG, log);
  }
  # endif // ifndef BUILD_NO_DEBUG

  return highest_bin_count != 0;
}

float P002_data_struct::applyCalibration(struct EventStruct *event, float float_value) {
  if (P002_CALIBRATION_ENABLED)
  {
    float_value = mapADCtoFloat(float_value,
                                P002_CALIBRATION_POINT1,
                                P002_CALIBRATION_POINT2,
                                P002_CALIBRATION_VALUE1,
                                P002_CALIBRATION_VALUE2);
  }
  return float_value;
}

float P002_data_struct::getCurrentValue(struct EventStruct *event, int& raw_value)
{
  # ifdef ESP8266
  const int pin = A0;
  # endif // if defined(ESP8266)
  # if defined(ESP32)
  const int pin = CONFIG_PIN1;
  # endif // if defined(ESP32)

  raw_value = espeasy_analogRead(pin);
  return applyCalibration(event, static_cast<float>(raw_value));
}

float P002_data_struct::applyCalibration(float float_value) const
{
  return mapADCtoFloat(
    float_value,
    _calib_adc1,
    _calib_adc2,
    _calib_out1,
    _calib_out2);
}

float P002_data_struct::applyMultiPointInterpolation(float float_value) const
{
  // First find the surrounding bins
  const size_t mp_size = _multipoint.size();

  if (mp_size == 0) { return float_value; }

  if (float_value <= _multipoint[0]._adc) {
    if (mp_size > 1) {
      // Just extrapolate the first multipoint line segment.
      return mapADCtoFloat(
        float_value,
        _multipoint[0]._adc,
        _multipoint[1]._adc,
        _multipoint[0]._value,
        _multipoint[1]._value);
    }

    // just one point, so all we can do is consider it to be a slight deviation of the calibration.
    return mapADCtoFloat(
      float_value,
      0,
      _multipoint[0]._adc,
      applyCalibration(0),
      _multipoint[0]._value);
  }

  const size_t last_mp_index = mp_size - 1;

  if (float_value >= _multipoint[last_mp_index]._adc)
  {
    if (mp_size > 1) {
      // Just extrapolate the last multipoint line segment.
      return mapADCtoFloat(
        float_value,
        _multipoint[last_mp_index - 1]._adc,
        _multipoint[last_mp_index]._adc,
        _multipoint[last_mp_index - 1]._value,
        _multipoint[last_mp_index]._value);
    }

    // just one point, so all we can do is consider it to be a slight deviation of the calibration.
    return mapADCtoFloat(
      float_value,
      _multipoint[last_mp_index]._adc,
      P002_MAX_ADC_VALUE,
      _multipoint[last_mp_index]._value,
      applyCalibration(P002_MAX_ADC_VALUE));
  }

  for (unsigned int i = 0; i < last_mp_index; ++i) {
    const float dist_left  = float_value - _multipoint[i]._adc;
    const float dist_right = _multipoint[i + 1]._adc - float_value;

    if ((dist_left >= 0) && (dist_right >= 0) &&
        (_multipoint[i]._adc != _multipoint[i + 1]._adc)) {
      // Inbetween 2 points of the multipoint array
      return mapADCtoFloat(
        float_value,
        _multipoint[i]._adc,
        _multipoint[i + 1]._adc,
        _multipoint[i]._value,
        _multipoint[i + 1]._value);
    }
  }

  return float_value;
}

float P002_data_struct::mapADCtoFloat(float float_value,
                                      int   adc1,
                                      int   adc2,
                                      float out1,
                                      float out2)
{
  if (adc1 != adc2)
  {
    const float normalized = static_cast<float>(float_value - adc1) / static_cast<float>(adc2 - adc1);
    float_value = normalized * (out2 - out1) + out1;
  }
  return float_value;
}

#endif // ifdef USES_P002
