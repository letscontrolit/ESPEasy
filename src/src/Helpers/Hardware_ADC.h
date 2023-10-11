#ifndef HELPERS_HARDWARE_ADC_H
#define HELPERS_HARDWARE_ADC_H


#include "../../ESPEasy_common.h"

#ifdef ESP8266
# define HAS_ADC2  0
#endif // ifdef ESP8266

#ifdef ESP32

# include "../Helpers/Hardware_ADC_cali.h"

#endif // ifdef ESP32


class Hardware_ADC_t {
public:

  Hardware_ADC_t() = default;
#ifdef ESP8266
  bool init();

  int  read();
#endif // ifdef ESP8266

#ifdef ESP32

  ~Hardware_ADC_t();

  bool init(int         pin,
            adc_atten_t attenuation = ADC_ATTEN_DB_11);

  // Return whether factory calibration is actually enabled.
  // Cannot enable factory calibration when no calibration is present.
  bool        adc_calibration_init();

  int         read(bool readAsTouch = false);

  float       applyFactoryCalibration(float rawValue);


//  bool        hasADC_factory_calibration();

# if ESP_IDF_VERSION_MAJOR >= 5
  static bool adc_calibration_init(
    int                pin,
    adc_atten_t        atten,
    adc_cali_handle_t *out_handle);
# endif // if ESP_IDF_VERSION_MAJOR >= 5

  static float mapADCtoFloat(float float_value,
                             float adc1,
                             float adc2,
                             float out1,
                             float out2);

private:

  int _pin = -1;
  int _adc = -1;

# if HAS_TOUCH_GPIO
  bool _isTouchPin = false;
# endif // if HAS_TOUCH_GPIO
  bool _useFactoryCalibration = false;
  adc_atten_t _attenuation    = ADC_ATTEN_DB_11;


  // ADC Factory calibration definition
  Hardware_ADC_cali_t _adc_cali_handle;

# if ESP_IDF_VERSION_MAJOR >= 5
  adc_channel_t _channel;
  adc_oneshot_unit_handle_t _adc_handle;
# endif // if ESP_IDF_VERSION_MAJOR >= 5

#endif  // ifdef ESP32

  int _lastADCvalue{};
};


#endif // ifndef HELPERS_HARDWARE_ADC_H
