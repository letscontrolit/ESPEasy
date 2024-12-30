#include "../Helpers/Hardware_ADC.h"


#ifdef ESP32
# include "../Helpers/Hardware_GPIO.h"
#endif // ifdef ESP32

#ifdef ESP8266
#include "../Globals/ESPEasyWiFiEvent.h"


bool Hardware_ADC_t::init() {
  return true;
}


int  Hardware_ADC_t::read() {
  if (!WiFiEventData.wifiConnectInProgress) {
# if FEATURE_ADC_VCC
    _lastADCvalue = ESP.getVcc();
# else // if FEATURE_ADC_VCC
    _lastADCvalue = analogRead(A0);
# endif // if FEATURE_ADC_VCC
  }
  return _lastADCvalue;
}

#endif // ifdef ESP8266


#ifdef ESP32
Hardware_ADC_t::~Hardware_ADC_t()
{
# if ESP_IDF_VERSION_MAJOR >= 5
  adc_oneshot_del_unit(_adc_handle);
# else
  // FIXME TD-er: Should we reset the GPIO config for this pin?
# endif // if ESP_IDF_VERSION_MAJOR >= 5
}

bool Hardware_ADC_t::init(int pin, adc_atten_t attenuation)
{
  int ch{};
  int t = -1;

  if (!getADC_gpio_info(pin, _adc, ch, t)) {
    return false;
  }
  _pin = pin;
#if defined(SOC_TOUCH_SENSOR_SUPPORTED) && SOC_TOUCH_SENSOR_SUPPORTED
  _isTouchPin = t >= 0;
#endif
  _attenuation = attenuation;

# if ESP_IDF_VERSION_MAJOR >= 5
  _channel = static_cast<adc_channel_t>(ch);
#  if HAS_ADC2
  const adc_unit_t unit = (_adc == 1) ? ADC_UNIT_1 : ADC_UNIT_2;
#  else // if HAS_ADC2

  if (_adc != 1) {
    return false;
  }
  const adc_unit_t unit = ADC_UNIT_1;
#  endif // if HAS_ADC2

  adc_oneshot_unit_init_cfg_t init_config = {
    .unit_id = unit
  };

  if (ESP_OK != adc_oneshot_new_unit(&init_config, &_adc_handle)) {
    return false;
  }

  adc_oneshot_chan_cfg_t config = {
    .atten    = attenuation,
    .bitwidth = ADC_BITWIDTH_DEFAULT,
  };
  return ESP_OK == adc_oneshot_config_channel(_adc_handle, _channel, &config);

# else // if ESP_IDF_VERSION_MAJOR >= 5

  if ((_adc == 1) || (_adc == 2)) {
    analogSetPinAttenuation(_pin, static_cast<adc_attenuation_t>(_attenuation));
  }

  return true;
# endif // if ESP_IDF_VERSION_MAJOR >= 5
}

bool Hardware_ADC_t::adc_calibration_init()
{
  _useFactoryCalibration = _adc_cali_handle.init(_pin, _attenuation);
  return _useFactoryCalibration;
}

int Hardware_ADC_t::read(bool readAsTouch) {
# if HAS_HALL_EFFECT_SENSOR

  if (_adc == 0) {
    return hallRead();
  }
# endif // if HAS_HALL_EFFECT_SENSOR

# if ESP_IDF_VERSION_MAJOR >= 5
  // Starting with IDF 5.x, you can apparently now use ADC2 along with WiFi running.
  // See: https://docs.espressif.com/projects/esp-idf/en/v5.1.1/esp32/api-reference/peripherals/adc_oneshot.html#hardware-limitations
  const bool canread = true;
#else
  // ADC2 and WiFi affect each other.
  // See:
  // https://docs.espressif.com/projects/esp-idf/en/v4.4.6/esp32/api-reference/peripherals/adc.html#adc-limitations
  // ADC2 is shared with WiFi, so don't read ADC2 when WiFi is on.
  const bool canread = _adc == 1 || WiFi.getMode() == WIFI_OFF;
#endif

  if (canread) {
#if defined(SOC_TOUCH_SENSOR_SUPPORTED) && SOC_TOUCH_SENSOR_SUPPORTED

    if (readAsTouch && _isTouchPin) {
      return touchRead(_pin);
    }
# endif

# if ESP_IDF_VERSION_MAJOR >= 5

    int adc_raw{};

    if (ESP_OK == adc_oneshot_read(_adc_handle, _channel, &adc_raw)) {
      _lastADCvalue = adc_raw;
    }
# else // if ESP_IDF_VERSION_MAJOR >= 5

    _lastADCvalue = analogRead(_pin);

# endif // if ESP_IDF_VERSION_MAJOR >= 5
  }
  return _lastADCvalue;
}

float Hardware_ADC_t::applyFactoryCalibration(float rawValue) {
  return _adc_cali_handle.applyFactoryCalibration(rawValue);
}

#endif // ifdef ESP32
