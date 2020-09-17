#include "P028_data_struct.h"

#ifdef USES_P028

# include "../Helpers/Convert.h"


P028_data_struct::P028_data_struct(uint8_t addr) :
  last_hum_val(0.0f),
  last_press_val(0.0f),
  last_temp_val(0.0f),
  last_dew_temp_val(0.0f),
  last_measurement(0),
  sensorID(Unknown_DEVICE),
  i2cAddress(addr),
  state(BMx_Uninitialized) {}


byte P028_data_struct::get_config_settings() const {
  switch (sensorID) {
    case BMP280_DEVICE_SAMPLE1:
    case BMP280_DEVICE_SAMPLE2:
    case BMP280_DEVICE:
    case BME280_DEVICE:  return 0x28; // Tstandby 62.5ms, filter 4, 3-wire SPI Disable
    default: return 0;
  }
}

byte P028_data_struct::get_control_settings() const {
  switch (sensorID) {
    case BMP280_DEVICE_SAMPLE1:
    case BMP280_DEVICE_SAMPLE2:
    case BMP280_DEVICE:
    case BME280_DEVICE:  return 0x93; // Oversampling: 8x P, 8x T, normal mode
    default: return 0;
  }
}

String P028_data_struct::getFullDeviceName() const {
  String devicename = getDeviceName();

  if ((sensorID == BMP280_DEVICE_SAMPLE1) ||
      (sensorID == BMP280_DEVICE_SAMPLE2))
  {
    devicename += F(" sample");
  }
  return devicename;
}

String P028_data_struct::getDeviceName() const {
  switch (sensorID) {
    case BMP280_DEVICE_SAMPLE1:
    case BMP280_DEVICE_SAMPLE2:
    case BMP280_DEVICE:  return F("BMP280");
    case BME280_DEVICE:  return F("BME280");
    default: return F("Unknown");
  }
}

boolean P028_data_struct::hasHumidity() const {
  switch (sensorID) {
    case BMP280_DEVICE_SAMPLE1:
    case BMP280_DEVICE_SAMPLE2:
    case BMP280_DEVICE:  return false;
    case BME280_DEVICE:  return true;
    default: return false;
  }
}

bool P028_data_struct::initialized() const {
  return state != BMx_Uninitialized;
}

void P028_data_struct::setUninitialized() {
  state = BMx_Uninitialized;
}

// Only perform the measurements with big interval to prevent the sensor from warming up.
bool P028_data_struct::updateMeasurements(float tempOffset, unsigned long task_index) {
  const unsigned long current_time = millis();

  check(); // Check id device is present

  if (!initialized()) {
    if (!begin()) {
      return false;
    }
    state            = BMx_Initialized;
    last_measurement = 0;
  }

  if (state != BMx_Wait_for_samples) {
    if ((last_measurement != 0) &&
        !timeOutReached(last_measurement + (Settings.TaskDeviceTimer[task_index] * 1000))) {
      // Timeout has not yet been reached.
      return false;
    }

    last_measurement = current_time;

    // Set the Sensor in sleep to be make sure that the following configs will be stored
    I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONTROL, 0x00);

    if (hasHumidity()) {
      I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONTROLHUMID, BME280_CONTROL_SETTING_HUMIDITY);
    }
    I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONFIG,  get_config_settings());
    I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONTROL, get_control_settings());
    state = BMx_Wait_for_samples;
    return false;
  }

  // It takes at least 1.587 sec for valit measurements to complete.
  // The datasheet names this the "T63" moment.
  // 1 second = 63% of the time needed to perform a measurement.
  if (!timeOutReached(last_measurement + 1587)) {
    return false;
  }

  if (!readUncompensatedData()) {
    return false;
  }

  // Set to sleep mode again to prevent the sensor from heating up.
  I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONTROL, 0x00);

  last_measurement = current_time;
  state            = BMx_New_values;
  last_temp_val    = readTemperature();
  last_press_val   = ((float)readPressure()) / 100.0f;
  last_hum_val     = ((float)readHumidity());


  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    log.reserve(120); // Prevent re-allocation
    log  = getDeviceName();
    log += F(":");
  }
  boolean logAdded = false;

  if (hasHumidity()) {
    // Apply half of the temp offset, to correct the dew point offset.
    // The sensor is warmer than the surrounding air, which has effect on the perceived humidity.
    last_dew_temp_val = compute_dew_point_temp(last_temp_val + (tempOffset / 2.0f), last_hum_val);
  } else {
    // No humidity measurement, thus set dew point equal to air temperature.
    last_dew_temp_val = last_temp_val;
  }

  if ((tempOffset > 0.1f) || (tempOffset < -0.1f)) {
    // There is some offset to apply.
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F(" Apply temp offset ");
      log += tempOffset;
      log += F("C");
    }

    if (hasHumidity()) {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F(" humidity ");
        log += last_hum_val;
      }
      last_hum_val = compute_humidity_from_dewpoint(last_temp_val + tempOffset, last_dew_temp_val);

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F("% => ");
        log += last_hum_val;
        log += F("%");
      }
    } else {
      last_hum_val = 0.0f;
    }

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F(" temperature ");
      log += last_temp_val;
    }
    last_temp_val = last_temp_val + tempOffset;

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log     += F("C => ");
      log     += last_temp_val;
      log     += F("C");
      logAdded = true;
    }
  }

  if (hasHumidity()) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log     += F(" dew point ");
      log     += last_dew_temp_val;
      log     += F("C");
      logAdded = true;
    }
  }

  if (logAdded && loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, log);
  }
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
          String log = F("BMx280 : Detected ");
          log += getFullDeviceName();
          addLog(LOG_LEVEL_INFO, log);
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
    String log = F("BMx280 : Unable to detect chip ID (");
    log += chip_id;
    if (!wire_status) {
      log += F(", failed");
    }
    log += ')';
    addLog(LOG_LEVEL_INFO, log);
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
  bool allDataRead = I2C_read_bytes(i2cAddress, BME280_data);

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
  int32_t var1, var2;
  int32_t adc_T = uncompensated.temperature;

  var1 = ((((adc_T >> 3) - ((int32_t)calib.dig_T1 << 1))) *
          ((int32_t)calib.dig_T2)) >> 11;

  var2 = (((((adc_T >> 4) - ((int32_t)calib.dig_T1)) *
            ((adc_T >> 4) - ((int32_t)calib.dig_T1))) >> 12) *
          ((int32_t)calib.dig_T3)) >> 14;

  calib.t_fine = var1 + var2;

  float T = (calib.t_fine * 5 + 128) >> 8;

  return T / 100;
}

float P028_data_struct::readPressure()
{
  int64_t var1, var2, p;
  int32_t adc_P = uncompensated.pressure;

  var1 = ((int64_t)calib.t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)calib.dig_P6;
  var2 = var2 + ((var1 * (int64_t)calib.dig_P5) << 17);
  var2 = var2 + (((int64_t)calib.dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)calib.dig_P3) >> 8) +
         ((var1 * (int64_t)calib.dig_P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib.dig_P1) >> 33;

  if (var1 == 0) {
    return 0; // avoid exception caused by division by zero
  }
  p    = 1048576 - adc_P;
  p    = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)calib.dig_P8) * p) >> 19;

  p = ((p + var1 + var2) >> 8) + (((int64_t)calib.dig_P7) << 4);
  return (float)p / 256;
}

float P028_data_struct::readHumidity()
{
  if (!hasHumidity()) {
    // No support for humidity
    return 0.0f;
  }
  int32_t adc_H = uncompensated.humidity;

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
  float h = (v_x1_u32r >> 12);

  return h / 1024.0f;
}

float P028_data_struct::Plugin_028_readAltitude(float seaLevel)
{
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude.  See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

  float atmospheric = readPressure() / 100.0f;

  return 44330.0f * (1.0f - pow(atmospheric / seaLevel, 0.1903f));
}

float P028_data_struct::pressureElevation(int altitude) {
  return last_press_val / pow(1.0f - (altitude / 44330.0f), 5.255f);
}

#endif // ifdef USES_P028
