#ifndef HELPERS_HARDWARE_ADC_CALI_H
#define HELPERS_HARDWARE_ADC_CALI_H

#include "../../ESPEasy_common.h"

#ifdef ESP32

# include "../Helpers/Hardware_defines.h"

// Needed to get ADC Vref
# if ESP_IDF_VERSION_MAJOR >= 5
#  include <soc/soc_caps.h>
#  include <esp_adc/adc_oneshot.h>
#  include <esp_adc/adc_cali.h>
#  include <esp_adc/adc_cali_scheme.h>
# else // if ESP_IDF_VERSION_MAJOR >= 5
#  include <esp_adc_cal.h>
#  include <driver/adc.h>
# endif // if ESP_IDF_VERSION_MAJOR >= 5

# if CONFIG_IDF_TARGET_ESP32C2 || CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C6
/**
 * On ESP32C3, ADC2 is no longer supported, due to its HW limitation.
 * Search for errata on espressif website for more details.
 */
#  define HAS_ADC2  0
# elif (SOC_ADC_PERIPH_NUM >= 2)
#  define HAS_ADC2  1
# else // if (SOC_ADC_PERIPH_NUM >= 2) && !CONFIG_IDF_TARGET_ESP32Cx
#  define HAS_ADC2  0
# endif // if (SOC_ADC_PERIPH_NUM >= 2) && !CONFIG_IDF_TARGET_ESP32Cx


class Hardware_ADC_cali_t {
public:

  Hardware_ADC_cali_t() = default;

  ~Hardware_ADC_cali_t();

  // Return whether factory calibration is actually enabled.
  // Cannot enable factory calibration when no calibration is present.
  bool  init(int         pin,
#if ESP_IDF_VERSION_MAJOR >= 5
             adc_atten_t attenuation = ADC_ATTEN_DB_12);
#else
             adc_atten_t attenuation = ADC_ATTEN_DB_11);
#endif

  bool initialized() const { return _initialized; }

  // Convert the rawValue to milli Volt when factory calibration is present and initialized.
  float applyFactoryCalibration(float rawValue) const;

  float getMinOut() const {
    return _min_out;
  }

  float getMaxOut() const {
    return _max_out;
  }

  bool useFactoryCalibration() const {
    return _useFactoryCalibration;
  }

  const __FlashStringHelper* getADC_factory_calibration_type() const;

private:

# if ESP_IDF_VERSION_MAJOR >= 5
  static bool adc_calibration_init(
    int                pin,
    adc_atten_t        atten,
    adc_cali_handle_t *out_handle);
# endif // if ESP_IDF_VERSION_MAJOR >= 5

  // ADC Factory calibration definition
# if ESP_IDF_VERSION_MAJOR >= 5
  adc_cali_handle_t _adc_cali_handle = nullptr;
# else // if ESP_IDF_VERSION_MAJOR >= 5
  esp_adc_cal_value_t _adc_calibration_type = ESP_ADC_CAL_VAL_NOT_SUPPORTED;
  esp_adc_cal_characteristics_t _adc_chars;
# endif // if ESP_IDF_VERSION_MAJOR >= 5

  bool _useFactoryCalibration   = false;
  bool _useHighResInterpolation = false;

  bool _initialized = false;

  float _min_out{};
  float _max_out = MAX_ADC_VALUE;
};

#endif // ifdef ESP32
#endif // ifndef HELPERS_HARDWARE_ADC_CALI_H
