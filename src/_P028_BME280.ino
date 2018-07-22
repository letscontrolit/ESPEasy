#ifdef USES_P028
//#######################################################################################################
//#################### Plugin 028 BME280 I2C Temp/Hum/Barometric Pressure Sensor  #######################
//#######################################################################################################

//#include <math.h>
#include <Arduino.h>
#include <map>

#define PLUGIN_028
#define PLUGIN_ID_028        28
#define PLUGIN_NAME_028       "Environment - BMx280"
#define PLUGIN_VALUENAME1_028 "Temperature"
#define PLUGIN_VALUENAME2_028 "Humidity"
#define PLUGIN_VALUENAME3_028 "Pressure"

#define PLUGIN_028_BME280_DEVICE "BME280"
#define PLUGIN_028_BMP280_DEVICE "BMP280"

#define BMx280_REGISTER_DIG_T1           0x88
#define BMx280_REGISTER_DIG_T2           0x8A
#define BMx280_REGISTER_DIG_T3           0x8C

#define BMx280_REGISTER_DIG_P1           0x8E
#define BMx280_REGISTER_DIG_P2           0x90
#define BMx280_REGISTER_DIG_P3           0x92
#define BMx280_REGISTER_DIG_P4           0x94
#define BMx280_REGISTER_DIG_P5           0x96
#define BMx280_REGISTER_DIG_P6           0x98
#define BMx280_REGISTER_DIG_P7           0x9A
#define BMx280_REGISTER_DIG_P8           0x9C
#define BMx280_REGISTER_DIG_P9           0x9E

#define BMx280_REGISTER_DIG_H1           0xA1
#define BMx280_REGISTER_DIG_H2           0xE1
#define BMx280_REGISTER_DIG_H3           0xE3
#define BMx280_REGISTER_DIG_H4           0xE4
#define BMx280_REGISTER_DIG_H5           0xE5
#define BMx280_REGISTER_DIG_H6           0xE7

#define BMx280_REGISTER_CHIPID           0xD0
#define BMx280_REGISTER_VERSION          0xD1
#define BMx280_REGISTER_SOFTRESET        0xE0

#define BMx280_REGISTER_CAL26            0xE1  // R calibration stored in 0xE1-0xF0

#define BMx280_REGISTER_CONTROLHUMID     0xF2
#define BMx280_REGISTER_STATUS           0xF3
#define BMx280_REGISTER_CONTROL          0xF4
#define BMx280_REGISTER_CONFIG           0xF5
#define BMx280_REGISTER_PRESSUREDATA     0xF7
#define BMx280_REGISTER_TEMPDATA         0xFA
#define BMx280_REGISTER_HUMIDDATA        0xFD

#define BME280_CONTROL_SETTING_HUMIDITY  0x02 // Oversampling: 2x H

#define BME280_TEMP_PRESS_CALIB_DATA_ADDR	0x88
#define BME280_HUMIDITY_CALIB_DATA_ADDR		0xE1
#define BME280_DATA_ADDR					        0xF7

#define BME280_TEMP_PRESS_CALIB_DATA_LEN	26
#define BME280_HUMIDITY_CALIB_DATA_LEN		7
#define BME280_P_T_H_DATA_LEN				      8

typedef struct
{
  uint16_t dig_T1;
  int16_t  dig_T2;
  int16_t  dig_T3;

  uint16_t dig_P1;
  int16_t  dig_P2;
  int16_t  dig_P3;
  int16_t  dig_P4;
  int16_t  dig_P5;
  int16_t  dig_P6;
  int16_t  dig_P7;
  int16_t  dig_P8;
  int16_t  dig_P9;

  uint8_t  dig_H1;
  int16_t  dig_H2;
  uint8_t  dig_H3;
  int16_t  dig_H4;
  int16_t  dig_H5;
  int8_t   dig_H6;
  int32_t  t_fine;
} bme280_calib_data;

struct bme280_uncomp_data {
	/*! un-compensated pressure */
	uint32_t pressure;
	/*! un-compensated temperature */
	uint32_t temperature;
	/*! un-compensated humidity */
	uint32_t humidity;
};

enum BMx_ChipId {
  Unknown_DEVICE = 0,
  BMP280_DEVICE_SAMPLE1 = 0x56,
  BMP280_DEVICE_SAMPLE2 = 0x57,
  BMP280_DEVICE = 0x58,
  BME280_DEVICE = 0x60
};

enum BMx_state {
  BMx_Uninitialized = 0,
  BMx_Initialized,
  BMx_Wait_for_samples,
  BMx_New_values,
  BMx_Values_read
};

struct P028_sensordata {
  P028_sensordata() :
    last_hum_val(0.0),
    last_press_val(0.0),
    last_temp_val(0.0),
    last_dew_temp_val(0.0),
    last_measurement(0),
    sensorID(Unknown_DEVICE),
    i2cAddress(0),
    state(BMx_Uninitialized) {}

    byte get_config_settings() const {
      switch (sensorID) {
        case BMP280_DEVICE_SAMPLE1:
        case BMP280_DEVICE_SAMPLE2:
        case BMP280_DEVICE:  return 0x28; // Tstandby 62.5ms, filter 4, 3-wire SPI Disable
        case BME280_DEVICE:  return 0x28; // Tstandby 62.5ms, filter 4, 3-wire SPI Disable
        default: return 0;
      }
    }

    byte get_control_settings() const {
      switch (sensorID) {
        case BMP280_DEVICE_SAMPLE1:
        case BMP280_DEVICE_SAMPLE2:
        case BMP280_DEVICE:  return 0x93; // Oversampling: 8x P, 8x T, normal mode
        case BME280_DEVICE:  return 0x93; // Oversampling: 8x P, 8x T, normal mode
        default: return 0;
      }
    }

    String getFullDeviceName() const {
      String devicename = getDeviceName();
      if (sensorID == BMP280_DEVICE_SAMPLE1 ||
          sensorID == BMP280_DEVICE_SAMPLE2)
      {
        devicename += " sample";
      }
      return devicename;
    }

    String getDeviceName() const {
      switch (sensorID) {
        case BMP280_DEVICE_SAMPLE1:
        case BMP280_DEVICE_SAMPLE2:
        case BMP280_DEVICE:  return PLUGIN_028_BMP280_DEVICE;
        case BME280_DEVICE:  return PLUGIN_028_BME280_DEVICE;
        default: return "Unknown";
      }
    }

    boolean hasHumidity() const {
      switch (sensorID) {
        case BMP280_DEVICE_SAMPLE1:
        case BMP280_DEVICE_SAMPLE2:
        case BMP280_DEVICE:  return false;
        case BME280_DEVICE:  return true;
        default: return false;
      }
    }

    bool initialized() const {
      return state != BMx_Uninitialized;
    }

    void setUninitialized() {
      state = BMx_Uninitialized;
    }

  bme280_uncomp_data uncompensated;
  bme280_calib_data calib;
  float last_hum_val;
  float last_press_val;
  float last_temp_val;
  float last_dew_temp_val;
  unsigned long last_measurement;
  BMx_ChipId sensorID;
  uint8_t i2cAddress;
  unsigned long moment_next_step;
  BMx_state state;
};

std::map<uint8_t, P028_sensordata> P028_sensors;

int Plugin_28_i2c_addresses[2] = { 0x76, 0x77 };

uint8_t Plugin_028_i2c_addr(struct EventStruct *event) {
  uint8_t i2cAddress = (uint8_t)Settings.TaskDevicePluginConfig[event->TaskIndex][0];
  if (i2cAddress != Plugin_28_i2c_addresses[0] && i2cAddress != Plugin_28_i2c_addresses[1]) {
    // Set to default address
    i2cAddress = Plugin_28_i2c_addresses[0];
  }
  if (P028_sensors.count(i2cAddress) == 0) {
    P028_sensors[i2cAddress] = P028_sensordata();
  }
  return i2cAddress;
}

boolean Plugin_028(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_028;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_TEMP_HUM_BARO;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_028);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_028));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_028));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_028));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        const uint8_t i2cAddress = Plugin_028_i2c_addr(event);
        P028_sensordata& sensor = P028_sensors[i2cAddress];
        addFormSelectorI2C(F("plugin_028_bme280_i2c"), 2, Plugin_28_i2c_addresses, i2cAddress);
        if (sensor.sensorID != Unknown_DEVICE) {
          String detectedString = F("Detected: ");
          detectedString += sensor.getFullDeviceName();
          addUnit(detectedString);
        }
        addFormNote(F("SDO Low=0x76, High=0x77"));

        addFormNumericBox(F("Altitude"), F("plugin_028_bme280_elev"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
        addUnit(F("m"));

        addFormNumericBox(F("Temperature offset"), F("plugin_028_bme280_tempoffset"), Settings.TaskDevicePluginConfig[event->TaskIndex][2]);
        addUnit(F("x 0.1C"));
        String offsetNote = F("Offset in units of 0.1 degree Celcius");
        if (sensor.hasHumidity()) {
          offsetNote += F(" (also correct humidity)");
        }
        addFormNote(offsetNote);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        const uint8_t i2cAddress = getFormItemInt(F("plugin_028_bme280_i2c"));
        Plugin_028_check(i2cAddress); // Check id device is present
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = i2cAddress;
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_028_bme280_elev"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("plugin_028_bme280_tempoffset"));
        success = true;
        break;
      }
    case PLUGIN_ONCE_A_SECOND:
      {
        const uint8_t i2cAddress = Plugin_028_i2c_addr(event);
        const float tempOffset = Settings.TaskDevicePluginConfig[event->TaskIndex][2] / 10.0;
        Plugin_028_update_measurements(i2cAddress, tempOffset);
        break;
      }

    case PLUGIN_READ:
      {
        const uint8_t i2cAddress = Plugin_028_i2c_addr(event);
        P028_sensordata& sensor = P028_sensors[i2cAddress];
        if (sensor.state != BMx_New_values) {
          success = false;
          break;
        }
        sensor.state = BMx_Values_read;
        if (!sensor.hasHumidity()) {
          // Patch the sensor type to output only the measured values.
          event->sensorType = SENSOR_TYPE_TEMP_EMPTY_BARO;
        }
        UserVar[event->BaseVarIndex] = sensor.last_temp_val;
        UserVar[event->BaseVarIndex + 1] = sensor.last_hum_val;
        const int elev = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        if (elev) {
           UserVar[event->BaseVarIndex + 2] = Plugin_028_pressureElevation(sensor.last_press_val, elev);
        } else {
           UserVar[event->BaseVarIndex + 2] = sensor.last_press_val;
        }
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log;
          log.reserve(40); // Prevent re-allocation
          log = sensor.getDeviceName();
          log += F(" : Address: 0x");
          log += String(i2cAddress,HEX);
          addLog(LOG_LEVEL_INFO, log);
          log = sensor.getDeviceName();
          log += F(" : Temperature: ");
          log += UserVar[event->BaseVarIndex];
          addLog(LOG_LEVEL_INFO, log);
          if (sensor.hasHumidity()) {
            log = sensor.getDeviceName();
            log += F(" : Humidity: ");
            log += UserVar[event->BaseVarIndex + 1];
            addLog(LOG_LEVEL_INFO, log);
          }
          log = sensor.getDeviceName();
          log += F(" : Barometric Pressure: ");
          log += UserVar[event->BaseVarIndex + 2];
          addLog(LOG_LEVEL_INFO, log);
        }
        success = true;
        break;
      }
      case PLUGIN_EXIT:
      {
        const uint8_t i2cAddress = Plugin_028_i2c_addr(event);
        P028_sensors.erase(i2cAddress);
        break;
      }
  }
  return success;
}


// Only perform the measurements with big interval to prevent the sensor from warming up.
bool Plugin_028_update_measurements(const uint8_t i2cAddress, float tempOffset) {
  P028_sensordata& sensor = P028_sensors[i2cAddress];
  const unsigned long current_time = millis();
  Plugin_028_check(i2cAddress); // Check id device is present
  if (!sensor.initialized()) {
    if (!Plugin_028_begin(i2cAddress)) {
      return false;
    }
    sensor.state = BMx_Initialized;
    sensor.last_measurement = 0;
  }
  if (sensor.state != BMx_Wait_for_samples) {
    if (sensor.last_measurement != 0 &&
        !timeOutReached(sensor.last_measurement + (Settings.MessageDelay * 1000))) {
      // Timeout has not yet been reached.
      return false;
    }

    sensor.last_measurement = current_time;
    // Set the Sensor in sleep to be make sure that the following configs will be stored
    I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONTROL, 0x00);
    if (sensor.hasHumidity()) {
      I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONTROLHUMID, BME280_CONTROL_SETTING_HUMIDITY);
    }
    I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONFIG, sensor.get_config_settings());
    I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONTROL, sensor.get_control_settings());
    sensor.state = BMx_Wait_for_samples;
    return false;
  }

  if (!timeOutReached(sensor.last_measurement + 1000)) {
    // Must wait one second to make sure the filtered values stabilize.
    return false;
  }
  if (!Plugin_028_readUncompensatedData(i2cAddress)) {
    return false;
  }
  // Set to sleep mode again to prevent the sensor from heating up.
  I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONTROL, 0x00);

  sensor.last_measurement = current_time;
  sensor.state = BMx_New_values;
  sensor.last_temp_val = Plugin_028_readTemperature(i2cAddress);
  sensor.last_press_val = ((float)Plugin_028_readPressure(i2cAddress)) / 100;
  sensor.last_hum_val = ((float)Plugin_028_readHumidity(i2cAddress));


  String log;
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    log.reserve(120); // Prevent re-allocation
    log = sensor.getDeviceName();
    log += F(":");
  }
  boolean logAdded = false;
  if (sensor.hasHumidity()) {
    // Apply half of the temp offset, to correct the dew point offset.
    // The sensor is warmer than the surrounding air, which has effect on the perceived humidity.
    sensor.last_dew_temp_val = compute_dew_point_temp(sensor.last_temp_val + (tempOffset / 2.0), sensor.last_hum_val);
  } else {
    // No humidity measurement, thus set dew point equal to air temperature.
    sensor.last_dew_temp_val = sensor.last_temp_val;
  }
  if (tempOffset > 0.1 || tempOffset < -0.1) {
    // There is some offset to apply.
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F(" Apply temp offset ");
      log += tempOffset;
      log += F("C");
    }
    if (sensor.hasHumidity()) {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F(" humidity ");
        log += sensor.last_hum_val;
      }
      sensor.last_hum_val = compute_humidity_from_dewpoint(sensor.last_temp_val + tempOffset, sensor.last_dew_temp_val);
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F("% => ");
        log += sensor.last_hum_val;
        log += F("%");
      }
    } else {
      sensor.last_hum_val = 0.0;
    }
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F(" temperature ");
      log += sensor.last_temp_val;
    }
    sensor.last_temp_val = sensor.last_temp_val + tempOffset;
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F("C => ");
      log += sensor.last_temp_val;
      log += F("C");
      logAdded = true;
    }
  }
  if (sensor.hasHumidity()) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F(" dew point ");
      log += sensor.last_dew_temp_val;
      log += F("C");
      logAdded = true;
    }
  }
  if (logAdded && loglevelActiveFor(LOG_LEVEL_INFO))
    addLog(LOG_LEVEL_INFO, log);
  return true;
}


//**************************************************************************/
// Check BME280 presence
//**************************************************************************/
bool Plugin_028_check(uint8_t i2cAddress) {
  bool wire_status = false;
  const uint8_t chip_id = I2C_read8_reg(i2cAddress, BMx280_REGISTER_CHIPID, &wire_status);
  P028_sensordata& sensor = P028_sensors[i2cAddress];
  if (!wire_status) sensor.setUninitialized();
  switch (chip_id) {
    case BMP280_DEVICE_SAMPLE1:
    case BMP280_DEVICE_SAMPLE2:
    case BMP280_DEVICE:
    case BME280_DEVICE: {
      if (wire_status) {
        // Store detected chip ID when chip found.
        if (sensor.sensorID != chip_id) {
          sensor.sensorID = static_cast<BMx_ChipId>(chip_id);
          sensor.setUninitialized();
          String log = F("BMx280 : Detected ");
          log += sensor.getFullDeviceName();
          addLog(LOG_LEVEL_INFO, log);
        }
      } else {
        sensor.sensorID = Unknown_DEVICE;
      }
      break;
    }
    default:
      sensor.sensorID = Unknown_DEVICE;
      break;
  }
  if (sensor.sensorID == Unknown_DEVICE) {
    String log = F("BMx280 : Unable to detect chip ID");
    addLog(LOG_LEVEL_INFO, log);
    return false;
  }
  return wire_status;
}

//**************************************************************************/
// Initialize BME280
//**************************************************************************/
bool Plugin_028_begin(uint8_t i2cAddress) {
  if (! Plugin_028_check(i2cAddress))
    return false;
  // Perform soft reset
  I2C_write8_reg(i2cAddress, BMx280_REGISTER_SOFTRESET, 0xB6);
  delay(2);  // Startup time is 2 ms (datasheet)
  Plugin_028_readCoefficients(i2cAddress);
  delay(65); //May be needed here as well to fix first wrong measurement?
  return true;
}


//**************************************************************************/
// Reads the factory-set coefficients
//**************************************************************************/
void Plugin_028_readCoefficients(uint8_t i2cAddress)
{
  P028_sensordata& sensor = P028_sensors[i2cAddress];
  sensor.calib.dig_T1 = I2C_read16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_T1);
  sensor.calib.dig_T2 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_T2);
  sensor.calib.dig_T3 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_T3);

  sensor.calib.dig_P1 = I2C_read16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P1);
  sensor.calib.dig_P2 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P2);
  sensor.calib.dig_P3 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P3);
  sensor.calib.dig_P4 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P4);
  sensor.calib.dig_P5 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P5);
  sensor.calib.dig_P6 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P6);
  sensor.calib.dig_P7 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P7);
  sensor.calib.dig_P8 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P8);
  sensor.calib.dig_P9 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P9);

  if (sensor.hasHumidity()) {
    sensor.calib.dig_H1 = I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H1);
    sensor.calib.dig_H2 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_H2);
    sensor.calib.dig_H3 = I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H3);
    sensor.calib.dig_H4 = (I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H4) << 4) | (I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H4 + 1) & 0xF);
    sensor.calib.dig_H5 = (I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H5 + 1) << 4) | (I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H5) >> 4);
    sensor.calib.dig_H6 = (int8_t)I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H6);
  }
}

bool Plugin_028_readUncompensatedData(uint8_t i2cAddress) {
  // wait until measurement has been completed, otherwise we would read
  // the values from the last measurement
  while (I2C_read8_reg(i2cAddress, BMx280_REGISTER_STATUS) & 0x08)
    delay(1);

  I2Cdata_bytes BME280_data(BME280_P_T_H_DATA_LEN, BME280_DATA_ADDR);
  bool allDataRead = I2C_read_bytes(i2cAddress, BME280_data);
  if (!allDataRead) {
    return false;
  }
  /* Variables to store the sensor data */
	uint32_t data_xlsb;
	uint32_t data_lsb;
	uint32_t data_msb;

  P028_sensordata& sensor = P028_sensors[i2cAddress];

	/* Store the parsed register values for pressure data */
	data_msb = (uint32_t)BME280_data[BME280_DATA_ADDR + 0] << 12;
	data_lsb = (uint32_t)BME280_data[BME280_DATA_ADDR + 1] << 4;
	data_xlsb = (uint32_t)BME280_data[BME280_DATA_ADDR + 2] >> 4;
	sensor.uncompensated.pressure = data_msb | data_lsb | data_xlsb;

	/* Store the parsed register values for temperature data */
	data_msb = (uint32_t)BME280_data[BME280_DATA_ADDR + 3] << 12;
	data_lsb = (uint32_t)BME280_data[BME280_DATA_ADDR + 4] << 4;
	data_xlsb = (uint32_t)BME280_data[BME280_DATA_ADDR + 5] >> 4;
	sensor.uncompensated.temperature = data_msb | data_lsb | data_xlsb;

	/* Store the parsed register values for temperature data */
	data_lsb = (uint32_t)BME280_data[BME280_DATA_ADDR + 6] << 8;
	data_msb = (uint32_t)BME280_data[BME280_DATA_ADDR + 7];
	sensor.uncompensated.humidity = data_msb | data_lsb;
  return true;
}

//**************************************************************************/
// Read temperature
//**************************************************************************/
float Plugin_028_readTemperature(uint8_t i2cAddress)
{
  P028_sensordata& sensor = P028_sensors[i2cAddress];
  int32_t var1, var2;
  int32_t adc_T = sensor.uncompensated.temperature;
  var1  = ((((adc_T >> 3) - ((int32_t)sensor.calib.dig_T1 << 1))) *
           ((int32_t)sensor.calib.dig_T2)) >> 11;

  var2  = (((((adc_T >> 4) - ((int32_t)sensor.calib.dig_T1)) *
             ((adc_T >> 4) - ((int32_t)sensor.calib.dig_T1))) >> 12) *
           ((int32_t)sensor.calib.dig_T3)) >> 14;

  sensor.calib.t_fine = var1 + var2;

  float T  = (sensor.calib.t_fine * 5 + 128) >> 8;
  return T / 100;
}

//**************************************************************************/
// Read pressure
//**************************************************************************/
float Plugin_028_readPressure(uint8_t i2cAddress)
{
  P028_sensordata& sensor = P028_sensors[i2cAddress];
  int64_t var1, var2, p;
  int32_t adc_P = sensor.uncompensated.pressure;

  var1 = ((int64_t)sensor.calib.t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)sensor.calib.dig_P6;
  var2 = var2 + ((var1 * (int64_t)sensor.calib.dig_P5) << 17);
  var2 = var2 + (((int64_t)sensor.calib.dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)sensor.calib.dig_P3) >> 8) +
         ((var1 * (int64_t)sensor.calib.dig_P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)sensor.calib.dig_P1) >> 33;

  if (var1 == 0) {
    return 0;  // avoid exception caused by division by zero
  }
  p = 1048576 - adc_P;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)sensor.calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)sensor.calib.dig_P8) * p) >> 19;

  p = ((p + var1 + var2) >> 8) + (((int64_t)sensor.calib.dig_P7) << 4);
  return (float)p / 256;
}

//**************************************************************************/
// Read humidity
//**************************************************************************/
float Plugin_028_readHumidity(uint8_t i2cAddress)
{
  P028_sensordata& sensor = P028_sensors[i2cAddress];
  if (!sensor.hasHumidity()) {
    // No support for humidity
    return 0.0;
  }
  // It takes at least 1.587 sec for valit measurements to complete.
  // The datasheet names this the "T63" moment.
  // 1 second = 63% of the time needed to perform a measurement.
  unsigned long difTime = millis() - sensor.last_measurement;
  if (difTime < 1587) {
    delay(1587 - difTime);
  }
  int32_t adc_H = sensor.uncompensated.humidity;

  int32_t v_x1_u32r;

  v_x1_u32r = (sensor.calib.t_fine - ((int32_t)76800));

  v_x1_u32r = (((((adc_H << 14) - (((int32_t)sensor.calib.dig_H4) << 20) -
                  (((int32_t)sensor.calib.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
               (((((((v_x1_u32r * ((int32_t)sensor.calib.dig_H6)) >> 10) *
                    (((v_x1_u32r * ((int32_t)sensor.calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
                  ((int32_t)2097152)) * ((int32_t)sensor.calib.dig_H2) + 8192) >> 14));

  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                             ((int32_t)sensor.calib.dig_H1)) >> 4));

  v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
  v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
  float h = (v_x1_u32r >> 12);
  return  h / 1024.0;
}

//**************************************************************************/
// Calculates the altitude (in meters) from the specified atmospheric
//    pressure (in hPa), and sea-level pressure (in hPa).
//    @param  seaLevel      Sea-level pressure in hPa
//    @param  atmospheric   Atmospheric pressure in hPa
//**************************************************************************/
float Plugin_028_readAltitude(uint8_t i2cAddress, float seaLevel)
{
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude.  See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

  float atmospheric = Plugin_028_readPressure(i2cAddress) / 100.0F;
  return 44330.0 * (1.0 - pow(atmospheric / seaLevel, 0.1903));
}

//**************************************************************************/
// MSL pressure formula
//**************************************************************************/
float Plugin_028_pressureElevation(float atmospheric, int altitude) {
  return atmospheric / pow(1.0 - (altitude/44330.0), 5.255);
}
#endif // USES_P028
