#include "../PluginStructs/P028_data_struct.h"

#ifdef USES_P028

# include "../Helpers/Convert.h"

// It takes at least 1.587 sec for valid measurements to complete.
// The datasheet names this the "T63" moment.
// 1 second = 63% of the time needed to perform a measurement.
# define P028_MEASUREMENT_TIMEOUT 1.587f

P028_data_struct::P028_data_struct(uint8_t addr, float tempOffset) :
  i2cAddress(addr), temp_offset(tempOffset) {}


uint8_t P028_data_struct::get_config_settings() const {
  return sensorID == Unknown_DEVICE ? 0u : 0x28; // Tstandby 62.5ms, filter 4, 3-wire SPI Disable
}

uint8_t P028_data_struct::get_control_settings() const {
  return sensorID == Unknown_DEVICE ? 0u : 0x93; // Oversampling: 8x P, 8x T, normal mode
}

const __FlashStringHelper * P028_data_struct::getDeviceName() const {
  switch (sensorID) {
    case BMP280_DEVICE_SAMPLE1:
    case BMP280_DEVICE_SAMPLE2: return F("BMP280 sample");
    case BMP280_DEVICE:         return F("BMP280");
    case BME280_DEVICE:         return F("BME280");
    default: return F("Unknown");
  }
}

bool P028_data_struct::hasHumidity() const {
  return sensorID == BME280_DEVICE;
}

bool P028_data_struct::initialized() const {
  return state != BMx_Uninitialized;
}

void P028_data_struct::setUninitialized() {
  state = BMx_Uninitialized;
}

bool P028_data_struct::measurementInProgress() const {
  if ((state != BMx_Wait_for_samples) || (last_measurement == 0)) { return false; }

  return !timeOutReached(last_measurement + P028_MEASUREMENT_TIMEOUT);
}

void P028_data_struct::startMeasurement() {
  if (measurementInProgress()) { return; }

  if (!initialized()) {
    if (begin()) {
      state            = BMx_Initialized;
      last_measurement = 0;
    }
  }
  check(); // Check id device is present

  if (state != BMx_Error) {
    // Set the Sensor in sleep to be make sure that the following configs will be stored
    I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONTROL, 0x00);

    if (hasHumidity()) {
      I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONTROLHUMID, BME280_CONTROL_SETTING_HUMIDITY);
    }
    I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONFIG,  get_config_settings());
    I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONTROL, get_control_settings());
    state            = BMx_Wait_for_samples;
    last_measurement = millis();
  } else {
    lastMeasurementError = true;
  }
}

// Only perform the measurements with big interval to prevent the sensor from warming up.
bool P028_data_struct::updateMeasurements(taskIndex_t task_index) {
  if ((state != BMx_Wait_for_samples) || measurementInProgress()) {
    // Nothing to do in processing the measurement
    return false;
  }

  if (!readUncompensatedData()) {
    return false;
  }

  // Set to sleep mode again to prevent the sensor from heating up.
  I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONTROL, 0x00);

  lastMeasurementError = false;
  state                = BMx_New_values;
  last_temp_val        = readTemperature();
  last_press_val       = readPressure();
  last_hum_val         = readHumidity();


# ifndef LIMIT_BUILD_SIZE
  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    log.reserve(120); // Prevent re-allocation
    log  = getDeviceName();
    log += ':';
  }
  bool logAdded = false;
# endif // ifndef LIMIT_BUILD_SIZE

  if (hasHumidity()) {
    // Apply half of the temp offset, to correct the dew point offset.
    // The sensor is warmer than the surrounding air, which has effect on the perceived humidity.
    last_dew_temp_val = compute_dew_point_temp(last_temp_val + (temp_offset / 2.0f), last_hum_val);
  } else {
    // No humidity measurement, thus set dew point equal to air temperature.
    last_dew_temp_val = last_temp_val;
  }

  if (!approximatelyEqual(temp_offset, 0.0f)) {
    # ifndef LIMIT_BUILD_SIZE

    // There is some offset to apply.
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F(" Apply temp offset: ");
      log += temp_offset;
      log += 'C';
    }
    # endif // ifndef LIMIT_BUILD_SIZE

    if (hasHumidity()) {
      # ifndef LIMIT_BUILD_SIZE

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F(" humidity: ");
        log += last_hum_val;
      }
      # endif // ifndef LIMIT_BUILD_SIZE
      last_hum_val = compute_humidity_from_dewpoint(last_temp_val + temp_offset, last_dew_temp_val);

      # ifndef LIMIT_BUILD_SIZE

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F("% => ");
        log += last_hum_val;
        log += F("%");
      }
      # endif // ifndef LIMIT_BUILD_SIZE
    } else {
      last_hum_val = 0.0f;
    }

# ifndef LIMIT_BUILD_SIZE

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F(" temperature: ");
      log += last_temp_val;
    }
# endif // ifndef LIMIT_BUILD_SIZE
    last_temp_val = last_temp_val + temp_offset;

# ifndef LIMIT_BUILD_SIZE

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log     += F("C => ");
      log     += last_temp_val;
      log     += 'C';
      logAdded = true;
    }
# endif // ifndef LIMIT_BUILD_SIZE
  }

# ifndef LIMIT_BUILD_SIZE

  if (hasHumidity()) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log     += F(" dew point: ");
      log     += last_dew_temp_val;
      log     += 'C';
      logAdded = true;
    }
  }

  if (logAdded && loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, log);
  }
# endif // ifndef LIMIT_BUILD_SIZE
  return true;
}

// **************************************************************************/
// Check BME280 presence
// **************************************************************************/
bool P028_data_struct::check() {
  bool wire_status      = false;
  const uint8_t chip_id = I2C_read8_reg(i2cAddress, BMx280_REGISTER_CHIPID, &wire_status);

  if (!wire_status) { setUninitialized(); }

  switch (chip_id) {
    case BMP280_DEVICE_SAMPLE1:
    case BMP280_DEVICE_SAMPLE2:
    case BMP280_DEVICE:
    case BME280_DEVICE: {
      if (wire_status) {
        // Store detected chip ID when chip found.
        if (sensorID != chip_id) {
          sensorID = static_cast<BMx_ChipId>(chip_id);
          setUninitialized();

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("BMx280: Detected ");
            log += getDeviceName();
            addLogMove(LOG_LEVEL_INFO, log);
          }
        }
      } else {
        sensorID = Unknown_DEVICE;
      }
      break;
    }
    default:
      sensorID = Unknown_DEVICE;
      break;
  }

  if (sensorID == Unknown_DEVICE) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("BMx280: Unable to detect chip ID (");
      log += chip_id;

      if (!wire_status) {
        log += F(", failed");
      }
      log += ')';
      addLogMove(LOG_LEVEL_INFO, log);
    }

    state = BMx_Error;

    return false;
  }
  return wire_status;
}

bool P028_data_struct::begin() {
  if (!check()) {
    return false;
  }

  // Perform soft reset
  I2C_write8_reg(i2cAddress, BMx280_REGISTER_SOFTRESET, 0xB6);
  delay(2); // Startup time is 2 ms (datasheet)
  readCoefficients();

  //  delay(65); //May be needed here as well to fix first wrong measurement?
  return true;
}

void P028_data_struct::readCoefficients()
{
  calib.dig_T1 = I2C_read16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_T1);
  calib.dig_T2 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_T2);
  calib.dig_T3 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_T3);

  calib.dig_P1 = I2C_read16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P1);
  calib.dig_P2 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P2);
  calib.dig_P3 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P3);
  calib.dig_P4 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P4);
  calib.dig_P5 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P5);
  calib.dig_P6 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P6);
  calib.dig_P7 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P7);
  calib.dig_P8 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P8);
  calib.dig_P9 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P9);

  if (hasHumidity()) {
    calib.dig_H1 = I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H1);
    calib.dig_H2 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_H2);
    calib.dig_H3 = I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H3);
    calib.dig_H4 = (I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H4) << 4) | (I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H4 + 1) & 0xF);
    calib.dig_H5 = (I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H5 + 1) << 4) | (I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H5) >> 4);
    calib.dig_H6 = (int8_t)I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H6);
  }
}

bool P028_data_struct::readUncompensatedData() {
  // wait until measurement has been completed, otherwise we would read
  // the values from the last measurement
  if (I2C_read8_reg(i2cAddress, BMx280_REGISTER_STATUS) & 0x08) {
    return false;
  }

  I2Cdata_bytes BME280_data(BME280_P_T_H_DATA_LEN, BME280_DATA_ADDR);
  const bool    allDataRead = I2C_read_bytes(i2cAddress, BME280_data);

  if (!allDataRead) {
    return false;
  }

  /* Variables to store the sensor data */
  uint32_t data_xlsb;
  uint32_t data_lsb;
  uint32_t data_msb;

  /* Store the parsed register values for pressure data */
  data_msb               = (uint32_t)BME280_data[BME280_DATA_ADDR + 0] << 12;
  data_lsb               = (uint32_t)BME280_data[BME280_DATA_ADDR + 1] << 4;
  data_xlsb              = (uint32_t)BME280_data[BME280_DATA_ADDR + 2] >> 4;
  uncompensated.pressure = data_msb | data_lsb | data_xlsb;

  /* Store the parsed register values for temperature data */
  data_msb                  = (uint32_t)BME280_data[BME280_DATA_ADDR + 3] << 12;
  data_lsb                  = (uint32_t)BME280_data[BME280_DATA_ADDR + 4] << 4;
  data_xlsb                 = (uint32_t)BME280_data[BME280_DATA_ADDR + 5] >> 4;
  uncompensated.temperature = data_msb | data_lsb | data_xlsb;

  /* Store the parsed register values for temperature data */
  data_lsb               = (uint32_t)BME280_data[BME280_DATA_ADDR + 6] << 8;
  data_msb               = (uint32_t)BME280_data[BME280_DATA_ADDR + 7];
  uncompensated.humidity = data_msb | data_lsb;
  return true;
}

float P028_data_struct::readTemperature()
{
  const int32_t adc_T = uncompensated.temperature;

  const int32_t var1 = ((((adc_T >> 3) - ((int32_t)calib.dig_T1 << 1))) *
                        ((int32_t)calib.dig_T2)) >> 11;

  const int32_t var2 = (((((adc_T >> 4) - ((int32_t)calib.dig_T1)) *
                          ((adc_T >> 4) - ((int32_t)calib.dig_T1))) >> 12) *
                        ((int32_t)calib.dig_T3)) >> 14;

  calib.t_fine = var1 + var2;

  return static_cast<float>((calib.t_fine * 5 + 128) >> 8) / 100.0f;
}

float P028_data_struct::readPressure() const
{
  int64_t var1, var2, p;
  int32_t adc_P = uncompensated.pressure;

  var1 = ((int64_t)calib.t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)calib.dig_P6;
  var2 = var2 + ((var1 * (int64_t)calib.dig_P5) << 17);
  var2 = var2 + (((int64_t)calib.dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)calib.dig_P3) >> 8) +
         ((var1 * (int64_t)calib.dig_P2) << 12);
  var1 = ((((((int64_t)1) << 47) + var1)) * ((int64_t)calib.dig_P1)) >> 33;

  if (var1 == 0) {
    return 0.0f; // avoid exception caused by division by zero
  }
  p    = 1048576 - adc_P;
  p    = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)calib.dig_P8) * p) >> 19;

  p = ((p + var1 + var2) >> 8) + (((int64_t)calib.dig_P7) << 4);
  return static_cast<float>(p) / 25600.0f;
}

float P028_data_struct::readHumidity() const
{
  if (!hasHumidity()) {
    // No support for humidity
    return 0.0f;
  }
  const int32_t adc_H = uncompensated.humidity;

  int32_t v_x1_u32r;

  v_x1_u32r = (calib.t_fine - ((int32_t)76800));

  v_x1_u32r = (((((adc_H << 14) - (((int32_t)calib.dig_H4) << 20) -
                  (((int32_t)calib.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
               (((((((v_x1_u32r * ((int32_t)calib.dig_H6)) >> 10) *
                    (((v_x1_u32r * ((int32_t)calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
                  ((int32_t)2097152)) * ((int32_t)calib.dig_H2) + 8192) >> 14));

  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                             ((int32_t)calib.dig_H1)) >> 4));

  v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
  v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
  return static_cast<float>(v_x1_u32r >> 12) / 1024.0f;
}

#endif // ifdef USES_P028
