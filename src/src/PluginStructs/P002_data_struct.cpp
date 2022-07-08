#include "../PluginStructs/P002_data_struct.h"

#ifdef USES_P002

# include "../Helpers/Rules_calculate.h"


# ifndef DEFAULT_VREF
#  define DEFAULT_VREF 1100
# endif // ifndef DEFAULT_VREF


P002_data_struct::P002_data_struct(struct EventStruct *event)
{
  initPluginStats(0);
  _sampleMode = P002_OVERSAMPLING;

  # ifdef ESP8266
  _pin_analogRead = A0;
  # endif // ifdef ESP8266
  # ifdef ESP32
  _pin_analogRead        = CONFIG_PIN1;
  _useFactoryCalibration = applyFactoryCalibration(event);
  _attenuation           = getAttenuation(event);
  const int adc = getADC_num_for_gpio(_pin_analogRead);

  if ((adc == 1) || (adc == 2)) {
    analogSetPinAttenuation(_pin_analogRead, static_cast<adc_attenuation_t>(_attenuation));
  }

  # endif // ifdef ESP32

  if (P002_CALIBRATION_ENABLED) {
    _use2pointCalibration = true;
    _calib_adc1           = P002_CALIBRATION_POINT1;
    _calib_adc2           = P002_CALIBRATION_POINT2;
    _calib_out1           = P002_CALIBRATION_VALUE1;
    _calib_out2           = P002_CALIBRATION_VALUE2;
  }
# ifndef LIMIT_BUILD_SIZE
  LoadTaskSettings(event->TaskIndex);
  _nrDecimals        = ExtraTaskSettings.TaskDeviceValueDecimals[0];
  _nrMultiPointItems = P002_NR_MULTIPOINT_ITEMS;
  _useMultipoint     = P002_MULTIPOINT_ENABLED;

  load(event);
# endif // ifndef LIMIT_BUILD_SIZE
}

# ifndef LIMIT_BUILD_SIZE
void P002_data_struct::load(struct EventStruct *event)
{
  const size_t nr_lines = P002_Nlines;

  {
    String lines[nr_lines];
    LoadCustomTaskSettings(event->TaskIndex, lines, nr_lines, 0);
    const int stored_nr_lines = lines[P002_SAVED_NR_LINES].toInt();
    _formula              = lines[P002_LINE_INDEX_FORMULA];
    _formula_preprocessed = RulesCalculate_t::preProces(_formula);

    for (size_t i = P002_LINE_IDX_FIRST_MP; i < nr_lines && static_cast<int>(i) < stored_nr_lines; i += P002_STRINGS_PER_MP) {
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

# endif // ifndef LIMIT_BUILD_SIZE

void P002_data_struct::webformLoad(struct EventStruct *event)
{
  // Output the statistics for the current settings.
  int raw_value            = 0;
  const float currentValue = P002_data_struct::getCurrentValue(event, raw_value);

  if (_plugin_stats[0] != nullptr) {
    _plugin_stats[0]->trackPeak(raw_value);
  }

# ifdef ESP32
  addRowLabel(F("Analog Pin"));
  addADC_PinSelect(AdcPinSelectPurpose::ADC_Touch_HallEffect, F("taskdevicepin1"), CONFIG_PIN1);

  {
    const __FlashStringHelper *outputOptions[] = {
      F("11 dB"),
      F("6 dB"),
      F("2.5 dB"),
      F("0 dB")
    };
    const int outputOptionValues[] = {
      P002_ADC_11db,
      P002_ADC_6db,
      P002_ADC_2_5db,
      P002_ADC_0db
    };
    addFormSelector(F("Attenuation"), F("p002_attn"), 4, outputOptions, outputOptionValues, P002_ATTENUATION);
  }

# endif // ifdef ESP32

  {
    const __FlashStringHelper *outputOptions[] = {
      F("Use Current Sample"),
      F("Oversampling")
# ifndef LIMIT_BUILD_SIZE
      , F("Binning")
# endif // ifndef LIMIT_BUILD_SIZE
    };
    const int outputOptionValues[] = {
      P002_USE_CURENT_SAMPLE,
      P002_USE_OVERSAMPLING
# ifndef LIMIT_BUILD_SIZE
      , P002_USE_BINNING
# endif // ifndef LIMIT_BUILD_SIZE
    };
# ifndef LIMIT_BUILD_SIZE
    const int nrOptions = 3;
# else // ifndef LIMIT_BUILD_SIZE
    const int nrOptions = 2;
# endif // ifndef LIMIT_BUILD_SIZE
    addFormSelector(F("Oversampling"), F("p002_oversampling"), nrOptions, outputOptions, outputOptionValues, P002_OVERSAMPLING);
  }

# ifdef ESP32
  addFormSubHeader(F("Factory Calibration"));
  addFormCheckBox(F("Apply Factory Calibration"), F("p002_fac_cal"), P002_APPLY_FACTORY_CALIB, !hasADC_factory_calibration());
  addFormNote(F("When checked, reading is in mV"));

  if (hasADC_factory_calibration()) {
    addRowLabel(F("Factory Calibration Type"));
    addHtml(getADC_factory_calibration_type());
    #  ifndef LIMIT_BUILD_SIZE
    webformLoad_calibrationCurve(event);
    #  endif // ifndef LIMIT_BUILD_SIZE
    formatADC_statistics(F("Current ADC to mV"), raw_value);

    for (size_t att = 0; att < ADC_ATTEN_MAX; ++att) {
      const int   low  = esp_adc_cal_raw_to_voltage(0, &adc_chars[att]);
      const int   high = esp_adc_cal_raw_to_voltage(MAX_ADC_VALUE, &adc_chars[att]);
      const float step = static_cast<float>(high - low) / MAX_ADC_VALUE;

      String rowlabel = F("Attenuation @");
      rowlabel += AttenuationToString(static_cast<adc_atten_t>(att));
      addRowLabel(rowlabel);
      addHtml(F("Range / Step: "));
      addHtmlInt(low);
      addHtml(F(" ... "));
      addHtmlInt(high);
      addUnit(F("mV"));
      addHtml(F(" / "));
      addHtmlFloat(step, 3);
      addUnit(F("mV"));
    }
  }
# endif // ifdef ESP32

  addFormSubHeader(F("Two Point Calibration"));

  addFormCheckBox(F("Calibration Enabled"), F("p002_cal"), P002_CALIBRATION_ENABLED);

  addFormNumericBox(F("Point 1"), F("p002_adc1"), P002_CALIBRATION_POINT1, 0, MAX_ADC_VALUE);
  html_add_estimate_symbol();
  addTextBox(F("p002_out1"), toString(P002_CALIBRATION_VALUE1, 3), 10);

  addFormNumericBox(F("Point 2"), F("p002_adc2"), P002_CALIBRATION_POINT2, 0, MAX_ADC_VALUE);
  html_add_estimate_symbol();
  addTextBox(F("p002_out2"), toString(P002_CALIBRATION_VALUE2, 3), 10);

  {
    // Output the statistics for the current settings.
    if (P002_CALIBRATION_ENABLED) {
      # ifndef LIMIT_BUILD_SIZE
      webformLoad_2pt_calibrationCurve(event);
      # endif // ifndef LIMIT_BUILD_SIZE

      int minInputValue, maxInputValue;
      getInputRange(event, minInputValue, maxInputValue);

      const float minY_value         = P002_data_struct::applyCalibration(event, minInputValue);
      const float maxY_value         = P002_data_struct::applyCalibration(event, maxInputValue);
      const float current_calibrated = P002_data_struct::applyCalibration(event, currentValue);

      format_2point_calib_statistics(F("Current"), currentValue,  current_calibrated);
      format_2point_calib_statistics(F("Minimum"), minInputValue, minY_value);
      format_2point_calib_statistics(F("Maximum"), maxInputValue, maxY_value);

      const float stepsize = (maxY_value - minY_value) / (MAX_ADC_VALUE + 1);
      addRowLabel(F("Step Size"));
      addHtmlFloat(stepsize, 3);
    } else {
      addRowLabel(F("Current"));
      addHtmlFloat(currentValue, 3);
    }
  }
# ifndef LIMIT_BUILD_SIZE
  addFormSubHeader(F("Multipoint Processing"));
  addFormCheckBox(F("Multipoint Processing Enabled"), F("p002_multi_en"), P002_MULTIPOINT_ENABLED);


  addFormTextBox(F("Binning Formula"), getPluginCustomArgName(P002_LINE_INDEX_FORMULA), _formula, P002_MAX_FORMULA_LENGTH);

  if (P002_OVERSAMPLING != P002_USE_BINNING) {
    addFormNote(F("Formula only used when binning"));
  }

  addFormNumericBox(F("Nr Multipoint Fields"), F("p002_nr_mp"), P002_NR_MULTIPOINT_ITEMS, 0, P002_MAX_NR_MP_ITEMS);

  // Checkbox needed to explicitly allow to split-paste over each field
  addFormCheckBox(F("Split-Paste Multipoint Fields"), F("splitpaste"), false);
  addFormNote(F("When checked, a set of tab, space or newline separated values can be pasted at once."));

  size_t line_nr = 0;

  for (int varNr = P002_LINE_IDX_FIRST_MP; varNr < P002_Nlines; varNr += P002_STRINGS_PER_MP)
  {
    const String label = String(F("Point ")) + String(line_nr + 1);
    addFormTextBox(F("query-input widenumber"),
                   label,
                   getPluginCustomArgName(varNr),
                   _multipoint.size() > line_nr ? doubleToString(static_cast<double>(_multipoint[line_nr]._adc), _nrDecimals,
                                                                 true) : EMPTY_STRING,
                   0);
    html_add_estimate_symbol();
    addTextBox(getPluginCustomArgName(varNr + 1),
               _multipoint.size() > line_nr ?  doubleToString(static_cast<double>(_multipoint[line_nr]._value), _nrDecimals,
                                                              true) : EMPTY_STRING,
               0,
               false,
               false,
               EMPTY_STRING,
               F("query-input widenumber"));

    ++line_nr;
  }
  webformLoad_multipointCurve(event);
# endif // ifndef LIMIT_BUILD_SIZE

  if (_plugin_stats[0] != nullptr) {
    if (_plugin_stats[0]->hasPeaks()) {
      addFormSubHeader(F("Statistics"));

      if (_plugin_stats[0]->hasPeaks()) {
        formatADC_statistics(F("ADC Peak Low"),  _plugin_stats[0]->getPeakLow());
        formatADC_statistics(F("ADC Peak High"), _plugin_stats[0]->getPeakHigh());
        addFormNote(F("Peak values recorded since last \"resetpeaks\"."));
      }

      if (_plugin_stats[0]->getNrSamples() > 0) {
        addRowLabel(F("Avg. ouput value"));
        addHtmlFloat(_plugin_stats[0]->getSampleAvg());
        {
          String note = F("Average over last ");
          note += _plugin_stats[0]->getNrSamples();
          note += F(" samples");
          addFormNote(note);
        }
      }
    }
  }
}

# ifndef LIMIT_BUILD_SIZE
#  ifdef ESP32
void P002_data_struct::webformLoad_calibrationCurve(struct EventStruct *event)
{
  if (!hasADC_factory_calibration()) { return; }

  addRowLabel(F("Calibration Curve"));

  const int valueCount = 33;
  int xAxisValues[valueCount];

  getChartRange(event, xAxisValues, valueCount, true);

  String axisOptions;

  {
    const ChartJS_title xAxisTitle(F("ADC Value"));
    const ChartJS_title yAxisTitle(F("Input Voltage (mV)"));
    axisOptions = make_ChartJS_scale_options(xAxisTitle, yAxisTitle);
  }
  add_ChartJS_chart_header(
    F("line"),
    F("fact_cal"),
    F("Factory Calibration per Attenuation"),
    500,
    500,
    axisOptions);

  add_ChartJS_chart_labels(
    valueCount,
    xAxisValues);

  const __FlashStringHelper *colors[] = { F("#A52422"), F("#BEA57D"), F("#EFF2C0"), F("#A4BAB7") };

  size_t current_attenuation = getAttenuation(event);

  if (current_attenuation >= ADC_ATTEN_MAX) { current_attenuation = ADC_ATTEN_DB_11; }

  for (size_t att = 0; att < ADC_ATTEN_MAX; ++att)
  {
    float values[valueCount];

    for (int i = 0; i < valueCount; ++i) {
      values[i] = esp_adc_cal_raw_to_voltage(xAxisValues[i], &adc_chars[att]);
    }

    add_ChartJS_dataset(
      AttenuationToString(static_cast<adc_atten_t>(att)),
      colors[att],
      values,
      valueCount,
      att != current_attenuation);
  }
  add_ChartJS_chart_footer();
}

#  endif // ifdef ESP32
# endif  // ifndef LIMIT_BUILD_SIZE

# ifndef LIMIT_BUILD_SIZE
const __FlashStringHelper * P002_data_struct::getChartXaxisLabel(struct EventStruct *event)
{
  #  ifdef ESP32

  if (applyFactoryCalibration(event)) {
    // reading in mVolt, not ADC
    return F("Input Voltage (mV)");
  }
  #  endif // ifdef ESP32
  return F("ADC Value");
}

# endif // ifndef LIMIT_BUILD_SIZE

void P002_data_struct::getInputRange(struct EventStruct *event, int& minInputValue, int& maxInputValue, bool ignoreCalibration)
{
  minInputValue = 0;
  maxInputValue = MAX_ADC_VALUE;
  # ifdef ESP32

  if (applyFactoryCalibration(event) && !ignoreCalibration) {
    // reading in mVolt, not ADC
    const size_t attenuation = getAttenuation(event);
    minInputValue = esp_adc_cal_raw_to_voltage(0, &adc_chars[attenuation]);
    maxInputValue = esp_adc_cal_raw_to_voltage(MAX_ADC_VALUE, &adc_chars[attenuation]);
  }
  # endif // ifdef ESP32
}

# ifndef LIMIT_BUILD_SIZE

void P002_data_struct::getChartRange(struct EventStruct *event, int values[], int count, bool ignoreCalibration)
{
  int minInputValue, maxInputValue;

  getInputRange(event, minInputValue, maxInputValue, ignoreCalibration);

  const float stepSize = static_cast<float>(maxInputValue + 1 - minInputValue) / (count - 1);

  for (int i = 0; i < count; ++i) {
    values[i] = minInputValue + i * stepSize;
  }
}

void P002_data_struct::webformLoad_2pt_calibrationCurve(struct EventStruct *event)
{
  addRowLabel(F("Two Point Calibration"));

  const int valueCount = 33;
  int xAxisValues[valueCount];

  getChartRange(event, xAxisValues, valueCount);

  String axisOptions;

  {
    const ChartJS_title xAxisTitle(getChartXaxisLabel(event));
    const ChartJS_title yAxisTitle(F("Calibrated Output"));
    axisOptions = make_ChartJS_scale_options(xAxisTitle, yAxisTitle);
  }


  add_ChartJS_chart_header(
    F("line"),
    F("twoPointCurve"),
    F("Two Point Calibration Curve"),
    500,
    500,
    axisOptions);

  add_ChartJS_chart_labels(
    valueCount,
    xAxisValues);

  {
    float values[valueCount];

    for (int i = 0; i < valueCount; ++i) {
      values[i] = P002_data_struct::applyCalibration(event, xAxisValues[i]);
    }

    add_ChartJS_dataset(
      F("2 Point Calibration"),
      F("rgb(255, 99, 132)"),
      values,
      valueCount);
  }
  add_ChartJS_chart_footer();
}

# endif // ifndef LIMIT_BUILD_SIZE

void P002_data_struct::formatADC_statistics(const __FlashStringHelper *label, int raw) const
{
  addRowLabel(label);
  addHtmlInt(raw);
# ifdef ESP32

  if (_useFactoryCalibration) {
    html_add_estimate_symbol();
    addHtmlInt(esp_adc_cal_raw_to_voltage(raw, &adc_chars[_attenuation]));
    addUnit(F("mV"));
  }
# endif // ifdef ESP32
}

void P002_data_struct::format_2point_calib_statistics(const __FlashStringHelper *label, int raw, float float_value) const
{
  addRowLabel(label);
  addHtmlInt(raw);
  # ifdef ESP32
  addUnit(_useFactoryCalibration ? F("mV") : F("raw"));
  # else // ifdef ESP32
  addUnit(F("raw"));
  # endif // ifdef ESP32
  html_add_estimate_symbol();
  addHtmlFloat(float_value, 3);
}

# ifdef ESP32
const __FlashStringHelper * P002_data_struct::AttenuationToString(adc_atten_t attenuation) {
  const __FlashStringHelper *datalabels[] = { F("0 dB"), F("2.5 dB"), F("6 dB"), F("11 dB") };

  if (attenuation < 4) { return datalabels[attenuation]; }
  return F("Unknown");
}

adc_atten_t P002_data_struct::getAttenuation(struct EventStruct *event) {
  if ((P002_ATTENUATION >= P002_ADC_0db) && (P002_ATTENUATION <= P002_ADC_11db)) {
    // Make sure the attenuation is only set to correct values or else it may damage the board
    return static_cast<adc_atten_t>(P002_ATTENUATION - 10);
  }
  P002_ATTENUATION = P002_ADC_11db;
  return ADC_ATTEN_DB_11;
}

# endif // ifdef ESP32

# ifndef LIMIT_BUILD_SIZE
void P002_data_struct::webformLoad_multipointCurve(struct EventStruct *event) const
{
  if (P002_MULTIPOINT_ENABLED)
  {
    addRowLabel(F("Multipoint Curve"));
    const bool useBinning = P002_OVERSAMPLING == P002_USE_BINNING;
    add_ChartJS_chart_header(
      useBinning ? F("bar") : F("line"),
      F("mpcurve"),
      F("Multipoint Curve"),
      500,
      500);

    // Add labels
    for (size_t i = 0; i < _multipoint.size(); ++i) {
      if (i != 0) {
        addHtml(',');
      }
      addHtmlFloat(_multipoint[i]._adc, 3);
    }
    addHtml(F("],datasets: ["));

    add_ChartJS_dataset_header(
      F("Multipoint Values"),
      F("rgb(255, 99, 132)"));

    for (int i = 0; i < _multipoint.size(); ++i) {
      if (i != 0) {
        addHtml(',');
      }
      addHtmlFloat(_multipoint[i]._value, 3);
    }
    add_ChartJS_dataset_footer();
    add_ChartJS_chart_footer();

    if (!useBinning) {
      // Try to compute the expected mapping from ADC to multipoint values
      addRowLabel(F("Input to Output Curve"));
      const int valueCount = 33;
      int xAxisValues[valueCount];
      getChartRange(event, xAxisValues, valueCount);

      String axisOptions;

      {
        const ChartJS_title xAxisTitle(getChartXaxisLabel(event));
        const ChartJS_title yAxisTitle(F("Output"));
        axisOptions = make_ChartJS_scale_options(xAxisTitle, yAxisTitle);
      }
      add_ChartJS_chart_header(
        F("line"),
        F("mpCurveSimulated"),
        F("Simulated Input to Output Curve"),
        500,
        500,
        axisOptions);

      add_ChartJS_chart_labels(
        valueCount,
        xAxisValues);

      float values[valueCount];
      const __FlashStringHelper *label = F("2 Point Calibration");
      const __FlashStringHelper *color = F("rgb(255, 99, 132)");

      bool useFactoryCalib = false;
      bool hidden          = true;

      #  ifdef ESP32
      const bool   hasFactoryCalibration = hasADC_factory_calibration();
      const size_t attenuation           = getAttenuation(event);
      #  else // ifdef ESP32
      const bool hasFactoryCalibration = false;
      #  endif // ifdef ESP32


      for (int step = 0; step < 4; ++step)
      {
        switch (step) {
          case 0:
            useFactoryCalib = false;
            hidden          = true;
            break;
          case 1:
            label           = F("Multipoint & 2 Point Calibration");
            color           = F("rgb(54, 162, 235)");
            useFactoryCalib = false;
            hidden          = false;
            break;
          case 2:
            label           = F("Factory & 2 Point Calibration");
            color           = F("rgb(153, 102, 255)");
            useFactoryCalib = true;
            hidden          = true;
            break;
          case 3:
            label           = F("Multipoint & Factory & 2 Point Calibration");
            color           = F("rgb(54, 235, 235)");
            useFactoryCalib = true;
            hidden          = false;
            break;
        }

        if (hasFactoryCalibration || !useFactoryCalib) {
          for (int i = 0; i < valueCount; ++i) {
            switch (step) {
              case 0:
                values[i] = P002_data_struct::applyCalibration(event, xAxisValues[i]);
                break;
              case 2:
                #  ifdef ESP32
                values[i] = P002_data_struct::applyCalibration(
                  event,
                  esp_adc_cal_raw_to_voltage(xAxisValues[i], &adc_chars[attenuation]));
                #  endif // ifdef ESP32
                break;
              case 1:
              case 3:
                values[i] = applyMultiPointInterpolation(values[i]);
                break;
            }
          }

          add_ChartJS_dataset(
            label,
            color,
            values,
            valueCount,
            hidden);
        }
      }
      add_ChartJS_chart_footer();
    }
  }
}

# endif // ifndef LIMIT_BUILD_SIZE

String P002_data_struct::webformSave(struct EventStruct *event)
{
  P002_OVERSAMPLING = getFormItemInt(F("p002_oversampling"), 0); // Set a default for LIMIT_BUILD_SIZE

  P002_CALIBRATION_ENABLED = isFormItemChecked(F("p002_cal"));
  # ifdef ESP32
  P002_APPLY_FACTORY_CALIB = isFormItemChecked(F("p002_fac_cal"));
  P002_ATTENUATION         = getFormItemInt(F("p002_attn"));
  # endif // ifdef ESP32

  P002_CALIBRATION_POINT1 = getFormItemInt(F("p002_adc1"));
  P002_CALIBRATION_VALUE1 = getFormItemFloat(F("p002_out1"));

  P002_CALIBRATION_POINT2 = getFormItemInt(F("p002_adc2"));
  P002_CALIBRATION_VALUE2 = getFormItemFloat(F("p002_out2"));

# ifndef LIMIT_BUILD_SIZE
  P002_MULTIPOINT_ENABLED = isFormItemChecked(F("p002_multi_en"));

  P002_NR_MULTIPOINT_ITEMS = getFormItemInt(F("p002_nr_mp"));

  const size_t nr_lines = P002_Nlines;
  String lines[nr_lines];

  // Store nr of lines that were saved, so no 'old' data will be read when nr of multi-point items has changed.
  lines[P002_SAVED_NR_LINES]     = String(nr_lines);
  lines[P002_LINE_INDEX_FORMULA] = webArg(getPluginCustomArgName(P002_LINE_INDEX_FORMULA));

  // const int nrDecimals = webArg(F("TDVD1")).toInt();

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
# else // ifndef LIMIT_BUILD_SIZE
  return EMPTY_STRING;
# endif // ifndef LIMIT_BUILD_SIZE
}

void P002_data_struct::takeSample()
{
  if (_sampleMode == P002_USE_CURENT_SAMPLE) { return; }
  int raw = espeasy_analogRead(_pin_analogRead);

  if (_plugin_stats[0] != nullptr) {
    _plugin_stats[0]->trackPeak(raw);
  }
# ifdef ESP32

  if (_useFactoryCalibration) {
    raw = esp_adc_cal_raw_to_voltage(raw, &adc_chars[_attenuation]);
  }
# endif // ifdef ESP32


  switch (_sampleMode) {
    case P002_USE_OVERSAMPLING:
      addOversamplingValue(raw);
      break;
# ifndef LIMIT_BUILD_SIZE
    case P002_USE_BINNING:
      addBinningValue(raw);
      break;
# endif // ifndef LIMIT_BUILD_SIZE
  }
}

bool P002_data_struct::getValue(float& float_value,
                                int  & raw_value) const
{
  bool mustTakeSample = false;

  switch (_sampleMode) {
    case P002_USE_OVERSAMPLING:

      if (getOversamplingValue(float_value, raw_value)) {
        return true;
      }
      mustTakeSample = true;
      break;
# ifndef LIMIT_BUILD_SIZE
    case P002_USE_BINNING:

      if (getBinnedValue(float_value, raw_value)) {
        return true;
      }
      mustTakeSample = true;
      break;
# endif // ifndef LIMIT_BUILD_SIZE
    case P002_USE_CURENT_SAMPLE:
      mustTakeSample = true;
      break;
  }

  if (!mustTakeSample) {
    return false;
  }

  raw_value = espeasy_analogRead(_pin_analogRead);

  if (_plugin_stats[0] != nullptr) {
    _plugin_stats[0]->trackPeak(raw_value);
  }
  # ifdef ESP32

  if (_useFactoryCalibration) {
    raw_value = esp_adc_cal_raw_to_voltage(raw_value, &adc_chars[_attenuation]);
  }
  # endif // ifdef ESP32

  float_value = applyCalibration(raw_value);

# ifndef LIMIT_BUILD_SIZE
  float_value = applyMultiPointInterpolation(float_value);
# endif // ifndef LIMIT_BUILD_SIZE

  return true;
}

void P002_data_struct::reset()
{
# ifndef LIMIT_BUILD_SIZE

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
# else // ifndef LIMIT_BUILD_SIZE
  resetOversampling();
# endif // ifndef LIMIT_BUILD_SIZE
}

void P002_data_struct::resetOversampling() {
  OversamplingValue  = 0;
  OversamplingCount  = 0;
  OversamplingMinVal = MAX_ADC_VALUE;
  OversamplingMaxVal = -MAX_ADC_VALUE;
}

void P002_data_struct::addOversamplingValue(int currentValue) {
  // Extra check to only add min or max readings once.
  // They will be taken out of the averaging only one time.
  if ((currentValue == 0) && (currentValue == OversamplingMinVal)) {
    return;
  }

  if ((currentValue == MAX_ADC_VALUE) && (currentValue == OversamplingMaxVal)) {
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
    float_value = applyCalibration(float_value);
# ifndef LIMIT_BUILD_SIZE
    float_value = applyMultiPointInterpolation(float_value);
# endif // ifndef LIMIT_BUILD_SIZE

    return true;
  }
  return false;
}

# ifndef LIMIT_BUILD_SIZE
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
  for (size_t index = 0; index < _binningRange.size(); ++index) {
    if (_binningRange[index].inRange(currentValue)) {
      ++_binning[index];
      return;
    }
  }

  // First apply calibration, then find the bin index
  float calibrated_value = static_cast<float>(currentValue);

  calibrated_value = applyCalibration(static_cast<float>(currentValue));

  // FIXME TD-er: hard-coded formula, must be computed before binning
  String formula = _formula_preprocessed;

  formula.replace(F("%value%"), toString(calibrated_value, _nrDecimals));

  double result = 0;

  if (!isError(RulesCalculate.doCalculate(parseTemplate(formula).c_str(), &result))) {
    calibrated_value = result;
  }

  const int index = getBinIndex(calibrated_value);

  if ((index >= 0) && (static_cast<int>(_binning.size()) > index)) {
    _binningRange[index].set(currentValue);
    ++_binning[index];
  }
}

bool P002_data_struct::getBinnedValue(float& float_value, int& raw_value) const
{
  unsigned int highest_bin_count = 0;

  const size_t nr_bin_elements = std::min(_binning.size(), _multipoint.size());

  for (size_t i = 0; i < nr_bin_elements; ++i) {
    if (_binning[i] > highest_bin_count) {
      highest_bin_count = _binning[i];
      float_value       = _multipoint[i]._value;
      raw_value         = _multipoint[i]._adc;
    }
  }
  #  ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("ADC getBinnedValue: bin cnt: ");

    log += highest_bin_count;
    log += F(" Value: ");
    log += float_value;
    log += F(" RAW: ");
    log += raw_value;
    addLog(LOG_LEVEL_DEBUG, log);
  }
  #  endif // ifndef BUILD_NO_DEBUG

  return highest_bin_count != 0;
}

# endif // ifndef LIMIT_BUILD_SIZE

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
  # endif // ifdef ESP8266
  # ifdef ESP32
  const int pin = CONFIG_PIN1;
  # endif // ifdef ESP32

  raw_value = espeasy_analogRead(pin);

  # ifdef ESP32

  if (applyFactoryCalibration(event)) {
    const size_t attenuation = getAttenuation(event);
    return esp_adc_cal_raw_to_voltage(raw_value, &adc_chars[attenuation]);
  }
  # endif // ifdef ESP32

  return raw_value;
}

float P002_data_struct::applyCalibration(float float_value) const
{
  if (!_use2pointCalibration) { return float_value; }
  return mapADCtoFloat(
    float_value,
    _calib_adc1,
    _calib_adc2,
    _calib_out1,
    _calib_out2);
}

# ifdef ESP32
bool P002_data_struct::applyFactoryCalibration(struct EventStruct *event) {
  if (P002_APPLY_FACTORY_CALIB) {
    const int adc_num = getADC_num_for_gpio(CONFIG_PIN1);

    if ((adc_num == 1) || (adc_num == 2)) {
      return true;
    }
  }
  return false;
}

# endif // ifdef ESP32

# ifndef LIMIT_BUILD_SIZE
float P002_data_struct::applyMultiPointInterpolation(float float_value) const
{
  if (!_useMultipoint) { return float_value; }

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
      MAX_ADC_VALUE,
      _multipoint[last_mp_index]._value,
      applyCalibration(MAX_ADC_VALUE));
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

# endif // ifndef LIMIT_BUILD_SIZE

float P002_data_struct::mapADCtoFloat(float float_value,
                                      float adc1,
                                      float adc2,
                                      float out1,
                                      float out2)
{
  if (!approximatelyEqual(adc1, adc2))
  {
    const float normalized = static_cast<float>(float_value - adc1) / static_cast<float>(adc2 - adc1);
    float_value = normalized * (out2 - out1) + out1;
  }
  return float_value;
}

#endif // ifdef USES_P002
