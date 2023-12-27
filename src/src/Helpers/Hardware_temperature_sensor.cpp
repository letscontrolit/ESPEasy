#include "../Helpers/Hardware_temperature_sensor.h"


#if FEATURE_INTERNAL_TEMPERATURE

# include "../Helpers/StringConverter.h"

/**
 * Code based on:
 * https://github.com/esphome/esphome/blob/518ecb4cc4489c8a76b899bfda7576b05d84c226/esphome/components/internal_temperature/internal_temperature.cpp#L40
 */

# ifdef ESP32
#  if defined(ESP32_CLASSIC)

// there is no official API available on the original ESP32
extern "C" {
uint8_t temprature_sens_read();
}
#  elif defined(ESP32C2) || defined(ESP32C3) || defined(ESP32C6) || defined(ESP32S2) || defined(ESP32S3)
#   if ESP_IDF_VERSION_MAJOR < 5
#    include <driver/temp_sensor.h>

// Work-around for bug in ESP-IDF < 5.0
#    if defined(ESP32S3) || defined(ESP32C3)
#     include <esp_efuse_rtc_calib.h>
#    elif defined(ESP32S2)
#     include <esp_efuse_rtc_table.h>
#    endif // if defined(ESP32S3) || defined(ESP32C3)
#   else // if ESP_IDF_VERSION_MAJOR < 5
#    include <driver/temperature_sensor.h>
#   endif  // if ESP_IDF_VERSION_MAJOR < 5
#  endif   // ESP32_CLASSIC
# endif    // ESP32


# ifdef ESP32
#  if defined(ESP32_CLASSIC)


esp_err_t do_read_internal_temperature(float& celsius) {
  esp_err_t result  = ESP_FAIL;
  uint8_t   raw     = 128u;
  int8_t    retries = 2;

  while ((128u == raw) && (0 != retries)) {
    delay(0);
    raw = temprature_sens_read(); // Each reading takes about 112 microseconds
    --retries;
  }
#   ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_DEBUG, concat(F("ESP32: Raw temperature value: "), raw));
#   endif // ifndef BUILD_NO_DEBUG

  if (raw != 128) {
    result = ESP_OK;

    // Raw value is in Fahrenheit
    celsius = (raw - 32) / 1.8f;
  }
  return result;
}

#  elif defined(ESP32C2) || defined(ESP32C3) || defined(ESP32C6) || defined(ESP32S2) || defined(ESP32S3)

esp_err_t do_read_internal_temperature(float& celsius) {
  esp_err_t result = ESP_FAIL;

  celsius = 0.0f; // Make sure it is initialized and within the default range.

#   if ESP_IDF_VERSION_MAJOR < 5

  temp_sensor_config_t tsens = TSENS_CONFIG_DEFAULT();

  temp_sensor_set_config(tsens);
  temp_sensor_start();

  // Work-around for bug in ESP-IDF < 5.0
  // Seems to be fixed in ESP_IDF5.1
  // temp_sensor_get_config always returns ESP_OK
  // Thus dac_offset can be just about anything
  // dac_offset is used as index in an array without bounds checking
  {
#    if defined(ESP32S3) || defined(ESP32C2) || defined(ESP32C3) || defined(ESP32C6)
    static float s_deltaT = (esp_efuse_rtc_calib_get_ver() == 1) ?
                            (esp_efuse_rtc_calib_get_cal_temp(1) / 10.0f) :
                            0.0f;
#    elif defined(ESP32S2)
    static uint32_t version  = esp_efuse_rtc_table_read_calib_version();
    static float    s_deltaT = (version == 1 || version == 2) ?
                               (esp_efuse_rtc_table_get_parsed_efuse_value(RTCCALIB_IDX_TMPSENSOR, false) / 10.0f) :
                               0.0f;
#    endif // if defined(ESP32S3) || defined(ESP32C3)


    /*
        if (isnan(s_deltaT)) { //suggests that the value is not initialized
          uint32_t version = esp_efuse_rtc_calib_get_ver();
          if (version == 1) {
              // fetch calibration value for temp sensor from eFuse
              s_deltaT = esp_efuse_rtc_calib_get_cal_temp(version);
          } else {
              // no value to fetch, use 0.
              s_deltaT = 0;
          }
        }
     */
#    ifndef TSENS_ADC_FACTOR
#     define TSENS_ADC_FACTOR  (0.4386)
#    endif // ifndef TSENS_ADC_FACTOR
#    ifndef TSENS_DAC_FACTOR
#     define TSENS_DAC_FACTOR  (27.88)
#    endif // ifndef TSENS_DAC_FACTOR
#    ifndef TSENS_SYS_OFFSET
#     define TSENS_SYS_OFFSET  (20.52)
#    endif // ifndef TSENS_SYS_OFFSET
    uint32_t tsens_raw{};
    temp_sensor_read_raw(&tsens_raw);
    celsius = (TSENS_ADC_FACTOR * tsens_raw) - s_deltaT - TSENS_SYS_OFFSET;
    result  = ESP_OK;
  }

#   else // if ESP_IDF_VERSION_MAJOR < 5

  //  result = temp_sensor_read_celsius(&celsius);


  static temperature_sensor_handle_t temp_sensor = nullptr;

  // Use range which seems to have the smallest error
  // See: https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/api-reference/peripherals/temp_sensor.html
  static int range_min = -10;
  static int range_max = 80;

  bool must_reinstall = false;

  if (temp_sensor == nullptr) {
    temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(range_min, range_max);
    result = temperature_sensor_install(&temp_sensor_config, &temp_sensor);
  } else {
    result = ESP_OK;
  }

  if (ESP_OK == result) {
    result = temperature_sensor_enable(temp_sensor);

    if (result == ESP_ERR_INVALID_STATE) {
      // Sensor reports to be not enabled
      must_reinstall = true;
    }
  }

  if (ESP_OK == result) {
    result = temperature_sensor_get_celsius(temp_sensor, &celsius);

    // FIXME TD-er: What to do when result == ESP_FAIL (can be indication of out-of-range)
    if (celsius < (range_min + 10)) {
      range_min -= 10;

      if (range_min > celsius) {
        range_min = celsius - 10;
      }
      must_reinstall = true;
    }

    if (celsius > (range_max - 10)) {
      range_max += 10;

      if (range_max < celsius) {
        range_max = celsius + 10;
      }
      must_reinstall = true;
    }
  }

  temperature_sensor_disable(temp_sensor);


  if (must_reinstall) {
    temperature_sensor_uninstall(temp_sensor);
    temp_sensor = nullptr;
  }


#   endif // if ESP_IDF_VERSION_MAJOR < 5

  return result;
}

#  endif // if defined(ESP32_CLASSIC)
# endif  // ifdef ESP32


bool getInternalTemperature(float& temperatureCelsius) {
  static float temperature_filtered = NAN; // Improbable value
  float celsius{};
  esp_err_t result = do_read_internal_temperature(celsius);

  if (ESP_OK == result) {
    if (isnanf(temperature_filtered)) {
      temperature_filtered = celsius;
    } else {
      constexpr float IIR_FACTOR  = 5.0f;
      constexpr float IIR_DIVIDER = IIR_FACTOR + 1.0f;
      temperature_filtered = ((IIR_FACTOR * temperature_filtered) + celsius) / IIR_DIVIDER;
    }
  }
  temperatureCelsius = temperature_filtered;
  return ESP_OK == result;
}

float getInternalTemperature() {
  float temperatureCelsius{};

  getInternalTemperature(temperatureCelsius);
  return temperatureCelsius;
}

#endif // if FEATURE_INTERNAL_TEMPERATURE
