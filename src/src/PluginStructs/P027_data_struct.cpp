#include "P027_data_struct.h"

#ifdef USES_P027


# define INA219_READ                            (0x01)
# define INA219_REG_CONFIG                      (0x00)
# define INA219_CONFIG_RESET                    (0x8000) // Reset Bit

# define INA219_CONFIG_BVOLTAGERANGE_MASK       (0x2000) // Bus Voltage Range Mask
# define INA219_CONFIG_BVOLTAGERANGE_16V        (0x0000) // 0-16V Range
# define INA219_CONFIG_BVOLTAGERANGE_32V        (0x2000) // 0-32V Range

# define INA219_CONFIG_GAIN_MASK                (0x1800) // Gain Mask
# define INA219_CONFIG_GAIN_1_40MV              (0x0000) // Gain 1, 40mV Range
# define INA219_CONFIG_GAIN_2_80MV              (0x0800) // Gain 2, 80mV Range
# define INA219_CONFIG_GAIN_4_160MV             (0x1000) // Gain 4, 160mV Range
# define INA219_CONFIG_GAIN_8_320MV             (0x1800) // Gain 8, 320mV Range

# define INA219_CONFIG_BADCRES_MASK             (0x0780) // Bus ADC Resolution Mask
# define INA219_CONFIG_BADCRES_9BIT             (0x0080) // 9-bit bus res = 0..511
# define INA219_CONFIG_BADCRES_10BIT            (0x0100) // 10-bit bus res = 0..1023
# define INA219_CONFIG_BADCRES_11BIT            (0x0200) // 11-bit bus res = 0..2047
# define INA219_CONFIG_BADCRES_12BIT            (0x0400) // 12-bit bus res = 0..4097

# define INA219_CONFIG_SADCRES_MASK             (0x0078) // Shunt ADC Resolution and Averaging Mask
# define INA219_CONFIG_SADCRES_9BIT_1S_84US     (0x0000) // 1 x 9-bit shunt sample
# define INA219_CONFIG_SADCRES_10BIT_1S_148US   (0x0008) // 1 x 10-bit shunt sample
# define INA219_CONFIG_SADCRES_11BIT_1S_276US   (0x0010) // 1 x 11-bit shunt sample
# define INA219_CONFIG_SADCRES_12BIT_1S_532US   (0x0018) // 1 x 12-bit shunt sample
# define INA219_CONFIG_SADCRES_12BIT_2S_1060US  (0x0048) // 2 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_4S_2130US  (0x0050) // 4 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_8S_4260US  (0x0058) // 8 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_16S_8510US (0x0060) // 16 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_32S_17MS   (0x0068) // 32 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_64S_34MS   (0x0070) // 64 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_128S_69MS  (0x0078) // 128 x 12-bit shunt samples averaged together

# define INA219_CONFIG_MODE_MASK                (0x0007) // Operating Mode Mask
# define INA219_CONFIG_MODE_POWERDOWN           (0x0000)
# define INA219_CONFIG_MODE_SVOLT_TRIGGERED     (0x0001)
# define INA219_CONFIG_MODE_BVOLT_TRIGGERED     (0x0002)
# define INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED (0x0003)
# define INA219_CONFIG_MODE_ADCOFF              (0x0004)
# define INA219_CONFIG_MODE_SVOLT_CONTINUOUS    (0x0005)
# define INA219_CONFIG_MODE_BVOLT_CONTINUOUS    (0x0006)
# define INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS (0x0007)

# define INA219_REG_SHUNTVOLTAGE                (0x01)
# define INA219_REG_BUSVOLTAGE                  (0x02)
# define INA219_REG_POWER                       (0x03)
# define INA219_REG_CURRENT                     (0x04)
# define INA219_REG_CALIBRATION                 (0x05)


P027_data_struct::P027_data_struct(uint8_t i2c_addr) : i2caddr(i2c_addr) {}

void P027_data_struct::setCalibration_32V_2A() {
  calValue = 4027;

  // Set multipliers to convert raw current/power values
  currentDivider_mA = 10; // Current LSB = 100uA per bit (1000/100 = 10)

  // Set Calibration register to 'Cal' calculated above
  wireWriteRegister(INA219_REG_CALIBRATION, calValue);

  // Set Config register to take into account the settings above
  uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
                    INA219_CONFIG_GAIN_8_320MV |
                    INA219_CONFIG_BADCRES_12BIT |
                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
                    INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

  wireWriteRegister(INA219_REG_CONFIG, config);
}

void P027_data_struct::setCalibration_32V_1A() {
  calValue = 10240;

  // Set multipliers to convert raw current/power values
  currentDivider_mA = 25; // Current LSB = 40uA per bit (1000/40 = 25)

  // Set Calibration register to 'Cal' calculated above
  wireWriteRegister(INA219_REG_CALIBRATION, calValue);

  // Set Config register to take into account the settings above
  uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
                    INA219_CONFIG_GAIN_8_320MV |
                    INA219_CONFIG_BADCRES_12BIT |
                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
                    INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

  wireWriteRegister(INA219_REG_CONFIG, config);
}

void P027_data_struct::setCalibration_16V_400mA() {
  calValue = 8192;

  // Set multipliers to convert raw current/power values
  currentDivider_mA = 20; // Current LSB = 50uA per bit (1000/50 = 20)

  // Set Calibration register to 'Cal' calculated above
  wireWriteRegister(INA219_REG_CALIBRATION, calValue);

  // Set Config register to take into account the settings above
  uint16_t config = INA219_CONFIG_BVOLTAGERANGE_16V |
                    INA219_CONFIG_GAIN_1_40MV |
                    INA219_CONFIG_BADCRES_12BIT |
                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
                    INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

  wireWriteRegister(INA219_REG_CONFIG, config);
}

int16_t P027_data_struct::getBusVoltage_raw() {
  uint16_t value;

  wireReadRegister(INA219_REG_BUSVOLTAGE, &value);

  // Shift to the right 3 to drop CNVR and OVF and multiply by LSB
  return (int16_t)((value >> 3) * 4);
}

int16_t P027_data_struct::getShuntVoltage_raw() {
  uint16_t value;

  wireReadRegister(INA219_REG_SHUNTVOLTAGE, &value);
  return (int16_t)value;
}

int16_t P027_data_struct::getCurrent_raw() {
  uint16_t value;

  // Sometimes a sharp load will reset the INA219, which will
  // reset the cal register, meaning CURRENT and POWER will
  // not be available ... avoid this by always setting a cal
  // value even if it's an unfortunate extra step
  wireWriteRegister(INA219_REG_CALIBRATION, calValue);

  // Now we can safely read the CURRENT register!
  wireReadRegister(INA219_REG_CURRENT, &value);

  return (int16_t)value;
}

float P027_data_struct::getShuntVoltage_mV() {
  int16_t value;

  value = P027_data_struct::getShuntVoltage_raw();
  return value * 0.01f;
}

float P027_data_struct::getBusVoltage_V() {
  int16_t value = getBusVoltage_raw();

  return value * 0.001f;
}

float P027_data_struct::getCurrent_mA() {
  float valueDec = getCurrent_raw();

  valueDec /= currentDivider_mA;
  return valueDec;
}

void P027_data_struct::wireWriteRegister(uint8_t reg, uint16_t value)
{
  Wire.beginTransmission(i2caddr);
  Wire.write(reg);                 // Register
  Wire.write((value >> 8) & 0xFF); // Upper 8-bits
  Wire.write(value & 0xFF);        // Lower 8-bits
  Wire.endTransmission();
}

void P027_data_struct::wireReadRegister(uint8_t reg, uint16_t *value)
{
  Wire.beginTransmission(i2caddr);
  Wire.write(reg); // Register
  Wire.endTransmission();

  delay(1);        // Max 12-bit conversion time is 586us per sample

  Wire.requestFrom(i2caddr, (uint8_t)2);

  // Shift values to create properly formed integer
  *value = ((Wire.read() << 8) | Wire.read());
}

#endif // ifdef USES_P027
