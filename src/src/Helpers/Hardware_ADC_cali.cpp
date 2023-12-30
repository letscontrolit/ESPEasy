#include "../Helpers/Hardware_ADC_cali.h"

#ifdef ESP32

//# include "../Helpers/ESPEasy_math.h"
# include "../Helpers/Hardware.h"


Hardware_ADC_cali_t::~Hardware_ADC_cali_t()
{
# if ESP_IDF_VERSION_MAJOR >= 5

  if (_useFactoryCalibration) {
#  if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_delete_scheme_curve_fitting(_adc_cali_handle);

#  elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_delete_scheme_line_fitting(_adc_cali_handle);
#  endif // if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
  }

# endif  // if ESP_IDF_VERSION_MAJOR >= 5
}

bool Hardware_ADC_cali_t::init(int         pin,
                               adc_atten_t attenuation)
{
# if ESP_IDF_VERSION_MAJOR >= 5 &&  ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
  _useHighResInterpolation = false;
# elif ESP_IDF_VERSION_MAJOR >= 5
  _useHighResInterpolation = attenuation != adc_atten_t::ADC_ATTEN_DB_12;
# else // if ESP_IDF_VERSION_MAJOR >= 5 &&  ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
  _useHighResInterpolation = attenuation != adc_atten_t::ADC_ATTEN_DB_11;
# endif // if ESP_IDF_VERSION_MAJOR >= 5 &&  ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED

# if ESP_IDF_VERSION_MAJOR >= 5

  if (_adc_cali_handle != nullptr) {
#  if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_delete_scheme_curve_fitting(_adc_cali_handle);

#  elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_delete_scheme_line_fitting(_adc_cali_handle);

#  endif // if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
  }

  _useFactoryCalibration = Hardware_ADC_cali_t::adc_calibration_init(
    pin,
    attenuation,
    &_adc_cali_handle);

  if (_useFactoryCalibration) {
    int tmp{};
    adc_cali_raw_to_voltage(_adc_cali_handle, 0,             &tmp);
    _min_out = tmp;
    adc_cali_raw_to_voltage(_adc_cali_handle, MAX_ADC_VALUE, &tmp);
    _max_out = tmp;
  }

# else // if ESP_IDF_VERSION_MAJOR >= 5
  #  ifndef DEFAULT_VREF
  #   define DEFAULT_VREF 1100
  #  endif // ifndef DEFAULT_VREF
  constexpr adc_bits_width_t adc_bit_width = static_cast<adc_bits_width_t>(ADC_WIDTH_MAX - 1);
  _adc_calibration_type =
    esp_adc_cal_characterize((getADC_num_for_gpio(pin) == 1) ? ADC_UNIT_1 : ADC_UNIT_2,
                             static_cast<adc_atten_t>(attenuation),
                             adc_bit_width,
                             DEFAULT_VREF,
                             &_adc_chars);
  _useFactoryCalibration = esp_adc_cal_check_efuse(_adc_calibration_type) == ESP_OK;

  if (_useFactoryCalibration) {
    _min_out = esp_adc_cal_raw_to_voltage(0, &_adc_chars);
    _max_out = esp_adc_cal_raw_to_voltage(MAX_ADC_VALUE, &_adc_chars);
  }
# endif // if ESP_IDF_VERSION_MAJOR >= 5

  _initialized = true;

  return _useFactoryCalibration;
}

float Hardware_ADC_cali_t::applyFactoryCalibration(float rawValue) const {
  if (!_useFactoryCalibration) {
    return rawValue;
  }

  if (!_useHighResInterpolation) {
    const int raw = rawValue;
# if ESP_IDF_VERSION_MAJOR >= 5
    int res{};
    adc_cali_raw_to_voltage(_adc_cali_handle, raw, &res);
    return res;
# else // if ESP_IDF_VERSION_MAJOR >= 5
    return esp_adc_cal_raw_to_voltage(raw, &_adc_chars);
# endif // if ESP_IDF_VERSION_MAJOR >= 5
  }

  // All other attenuations do appear to have a straight calibration curve.
  // But applying the factory calibration then reduces resolution.
  // So we interpolate using the calibrated extremes

  return mapADCtoFloat(
    rawValue,
    0,
    MAX_ADC_VALUE,
    _min_out,
    _max_out);
}

const __FlashStringHelper * Hardware_ADC_cali_t::getADC_factory_calibration_type() const {
# if ESP_IDF_VERSION_MAJOR >= 5

  if (_useFactoryCalibration) {
    #  if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    return F("Calibration Curve Fitting");
    #  endif // if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    #  if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    return F("Calibration Line Fitting");
    #  endif // if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
  }

# else // if ESP_IDF_VERSION_MAJOR >= 5

  switch (_adc_calibration_type) {
    case ESP_ADC_CAL_VAL_EFUSE_VREF:   return F("V_ref in eFuse");
    case ESP_ADC_CAL_VAL_EFUSE_TP:     return F("Two Point values in eFuse");
    case ESP_ADC_CAL_VAL_DEFAULT_VREF: return F("Default reference voltage");
    case ESP_ADC_CAL_VAL_EFUSE_TP_FIT: return F("Two Point values and fitting curve in eFuse");
    case ESP_ADC_CAL_VAL_NOT_SUPPORTED:
      break;
  }
# endif // if ESP_IDF_VERSION_MAJOR >= 5
  return F("Unknown");
}

# if ESP_IDF_VERSION_MAJOR >= 5
bool Hardware_ADC_cali_t::adc_calibration_init(
  int                pin,
  adc_atten_t        atten,
  adc_cali_handle_t *out_handle)
{
  int ch{};
  const int adc               = getADC_num_for_gpio(pin, ch);
  const adc_channel_t channel = static_cast<adc_channel_t>(ch);

#  if HAS_ADC2
  const adc_unit_t unit = (adc == 1) ? ADC_UNIT_1 : ADC_UNIT_2;
#  else // if HAS_ADC2
  const adc_unit_t unit = ADC_UNIT_1;
#  endif // if HAS_ADC2

  adc_cali_handle_t handle = NULL;
  esp_err_t ret            = ESP_FAIL;
  bool calibrated          = false;

#  if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED

  if (!calibrated) {
    // calibration scheme version: Curve Fitting
    adc_cali_curve_fitting_config_t cali_config = {
      .unit_id  = unit,
      .chan     = channel,
      .atten    = atten,
      .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);

    if (ret == ESP_OK) {
      calibrated = true;
    }
  }
#  endif // if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED

#  if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED

  if (!calibrated) {
    // calibration scheme version: Line Fitting
    adc_cali_line_fitting_config_t cali_config = {
      .unit_id  = unit,
      .atten    = atten,
      .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);

    if (ret == ESP_OK) {
      calibrated = true;
    }
  }
#  endif // if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED

  *out_handle = handle;

  /*
     if (ret == ESP_OK) {
      // Calibration Success
     } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
      // eFuse not burnt, skip software calibration
     } else {
      // Invalid arg or no memory
     }
   */

  return calibrated;
}

# endif // if ESP_IDF_VERSION_MAJOR >= 5

#endif // ifdef ESP32
