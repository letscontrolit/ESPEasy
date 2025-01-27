#include "AXP2101.h"

#ifdef ESP32

// To check if we have implemented all cases of the enums
# pragma GCC diagnostic push
# pragma GCC diagnostic warning "-Wswitch-enum"

bool AXP2101::begin(TwoWire               *wire,
                    uint8_t                addr,
                    AXP2101_device_model_e device) {
  _wire   = wire;
  _addr   = addr;
  _device = device;
  _wire->beginTransmission(_addr);
  return 0 == _wire->endTransmission();
}

void AXP2101::setDevice(AXP2101_device_model_e device) {
  _device = device;
}

bool AXP2101::readRegister(uint8_t  addr,
                           uint8_t  reg,
                           uint8_t *result,
                           uint16_t length) {
  uint8_t index = 0;

  _wire->beginTransmission(addr);
  _wire->write(reg);
  const uint8_t err = _wire->endTransmission();

  _wire->requestFrom(addr, length);

  for (int i = 0; i < length; ++i) {
    result[index++] = _wire->read();
  }

  return err == 0;
}

uint8_t AXP2101::readRegister8(uint8_t addr,
                               uint8_t reg) {
  _wire->beginTransmission(addr);
  _wire->write(reg);
  _wire->endTransmission();
  _wire->requestFrom(addr, 1);
  return _wire->read();
}

bool AXP2101::writeRegister8(uint8_t addr,
                             uint8_t reg,
                             uint8_t data) {
  _wire->beginTransmission(addr);
  _wire->write(reg);
  _wire->write(data);
  return 0 == _wire->endTransmission();
}

bool AXP2101::bitOn(uint8_t addr,
                    uint8_t reg,
                    uint8_t data) {
  const uint8_t temp       = readRegister8(addr, reg);
  const uint8_t write_back = (temp | data);

  return writeRegister8(addr, reg, write_back);
}

bool AXP2101::bitOff(uint8_t addr,
                     uint8_t reg,
                     uint8_t data) {
  const uint8_t temp       = readRegister8(addr, reg);
  const uint8_t write_back = (temp & (~data));

  return writeRegister8(addr, reg, write_back);
}

bool AXP2101::bitGet(uint8_t reg,
                     uint8_t data) {
  const uint8_t temp = readRegister8(AXP2101_ADDR, reg);

  return (temp & data) == data;
}

bool AXP2101::bitOnOff(bool    sw,
                       uint8_t creg,
                       uint8_t mask) {
  bool result = false;

  if (sw) {
    result = bitOn(AXP2101_ADDR, creg, mask);
  } else {
    result = bitOff(AXP2101_ADDR, creg, mask);
  }

  return result;
}

/**
 * Convert a voltage to the indicated register-data, using the matching offset and range(s)
 */
uint8_t AXP2101::voltageToRegister(uint16_t            voltage,
                                   AXP2101_registers_e reg) {
  uint16_t min = 500;
  uint16_t max = 0;

  switch (reg) {
    case AXP2101_registers_e::dcdc2:

      if (0 == max) { max = 1540; }
    case AXP2101_registers_e::dcdc3:

      if (0 == max) { max = 3400; }

      if (voltage <= min) {
        return 0u;
      }
      else if (voltage > max) {
        voltage = max;
      }
      else if ((voltage > 1540) && (voltage < 1600)) {
        voltage = 1540u;
      }

      if (voltage <= 1220) {
        return (voltage - 500) / 10;
      }
      else if (voltage <= 1540) {
        return (voltage - 1220) / 20 + (uint8_t)0b01000111;
      }
      return (voltage - 1600) / 100 + (uint8_t)0b01011000;

    case AXP2101_registers_e::dcdc4:

      if (voltage <= min) {
        return 0;
      }
      else if (voltage > 1840) {
        voltage = 1840u;
      }

      if (voltage <= 1220) {
        return (voltage - 500) / 10;
      }
      return (voltage - 1220) / 20 + (uint8_t)0b01000111;

    case AXP2101_registers_e::dcdc1:

      if (0 == max) {
        min = 1500;
        max = 3400;
      }
    case AXP2101_registers_e::dcdc5:

      if (0 == max) {
        min = 1400;
        max = 3700;
      }
    case AXP2101_registers_e::aldo1:
    case AXP2101_registers_e::aldo2:
    case AXP2101_registers_e::aldo3:
    case AXP2101_registers_e::aldo4:
    case AXP2101_registers_e::bldo1:
    case AXP2101_registers_e::bldo2:
    case AXP2101_registers_e::dldo1:

      if (0 == max) { max = 3400; }
    case AXP2101_registers_e::dldo2:
    case AXP2101_registers_e::cpuldos:

      if (0 == max) { max = 1400; }

      if (voltage <= min) { return 0u; }

      if (voltage > max) { voltage = max; }

      return (voltage - min) / 100;
    case AXP2101_registers_e::chargeled:
    case AXP2101_registers_e::batcharge:
    case AXP2101_registers_e::charging:
    case AXP2101_registers_e::batpresent:
    case AXP2101_registers_e::chipid:
    case AXP2101_registers_e::chargedet:
      break;

    case AXP2101_registers_e::vbat:
    case AXP2101_registers_e::battemp:
    case AXP2101_registers_e::vbus:
    case AXP2101_registers_e::vsys:
    case AXP2101_registers_e::chiptemp:
      break;

  }
  return 0u;
}

/**
 * Convert read data from a register to a voltage for the indicated output
 */
uint16_t AXP2101::registerToVoltage(uint8_t             data,
                                    AXP2101_registers_e reg) {
  uint16_t off  = 0;
  uint8_t  mask = 0;

  switch (reg) {
    case AXP2101_registers_e::dcdc2:
    case AXP2101_registers_e::dcdc3:
      data &= 0x7F;

      if (data < 0b01000111) {
        return static_cast<uint16_t>(data * 10) + 500;
      }
      else if (data < 0b01011000) {
        return static_cast<uint16_t>(data * 20) - 200;
      }
      return static_cast<uint16_t>(data * 100) - 7200;

    case AXP2101_registers_e::dcdc4:

      if (data < 0b01000111) {
        return static_cast<uint16_t>(data * 10) + 500;
      }
      return static_cast<uint16_t>(data * 20) - 200;

    case AXP2101_registers_e::dcdc1:

      if (0 == off) { off = 1500; }
    case AXP2101_registers_e::dcdc5:

      if (0 == off) { off = 1400; }
    case AXP2101_registers_e::aldo1:
    case AXP2101_registers_e::aldo2:
    case AXP2101_registers_e::aldo3:
    case AXP2101_registers_e::aldo4:
    case AXP2101_registers_e::bldo1:
    case AXP2101_registers_e::bldo2:
    case AXP2101_registers_e::dldo1:
    case AXP2101_registers_e::dldo2:
    case AXP2101_registers_e::cpuldos:

      if (0 == off) { off = 500; }
      return off + (data * 100);
    case AXP2101_registers_e::chargeled:
    case AXP2101_registers_e::batcharge:
    case AXP2101_registers_e::charging:
    case AXP2101_registers_e::batpresent:
    case AXP2101_registers_e::chipid:
    case AXP2101_registers_e::chargedet:
      break;

    case AXP2101_registers_e::vbat:
    case AXP2101_registers_e::battemp:
    case AXP2101_registers_e::vbus:
    case AXP2101_registers_e::vsys:
    case AXP2101_registers_e::chiptemp:
      break;
  }
  return 0u;
}

// Values in mVolt, raw ADC data is expressed in 0.5 mV
// LUT is based on 10k NTC with 50 uA current
constexpr uint16_t axp2101_TS_LUT[] {
  3150, // -20
  2508, // -15
  2013, // -10
  1628, // -5
  1325, // 0
  1084, // 5
  889,  // 10
  732,  // 15
  604,  // 20
  500,  // 25
  416,  // 30
  348,  // 35
  292,  // 40
  246,  // 45
  209,  // 50
  177,  // 55
  152}; // 60

uint16_t AXP2101::TS_tempToRegister(float temp_C)
{
  constexpr int NR_LUTS = sizeof(axp2101_TS_LUT) / sizeof(axp2101_TS_LUT[0]);
  const int index_lo = (temp_C + 20) / 5;
  const int index_hi = index_lo + 1;
  if (index_lo < 0) return 0xFFFF;
  if (index_hi >= NR_LUTS) return 0;

  const int mod5 = (static_cast<int>(temp_C)) % 5;
  const int offset = ((axp2101_TS_LUT[index_hi] - axp2101_TS_LUT[index_lo]) * mod5) / 5;

  // Need to apply a factor 2, to convert from mV to regvalue.
  return 2* (axp2101_TS_LUT[index_lo] + offset);
}

float AXP2101::TS_registerToTemp(uint16_t regValue)
{
  regValue /= 2; // Convert from regvalue to mV as used in LUT
  constexpr int NR_LUTS = sizeof(axp2101_TS_LUT) / sizeof(axp2101_TS_LUT[0]);
  if (regValue > axp2101_TS_LUT[0]) return -20.0f;
  for (int index_hi = 1; index_hi < NR_LUTS; ++index_hi) {
    if (regValue > axp2101_TS_LUT[index_hi]) {
      const int index_lo = index_hi - 1;
      const int step_lo = axp2101_TS_LUT[index_lo] - regValue;
      const int step_hi = regValue - axp2101_TS_LUT[index_hi];
      float avg = axp2101_TS_LUT[index_hi] * step_hi + axp2101_TS_LUT[index_lo] * step_lo;
      avg /= (step_hi + step_lo);
      return avg;
    }
  }

  return 60.0f;
}

/**
 * Set a voltage to a port (output pin) of the AXP2101
 */
bool AXP2101::setPortVoltage(uint16_t            voltage,
                             AXP2101_registers_e reg) {
  const uint8_t data = voltageToRegister(voltage, reg);
  const uint8_t creg = static_cast<uint8_t>(reg);

  return writeRegister8(AXP2101_ADDR, creg, data);
}

/**
 * Get the voltage of a port (output pin) of the AXP2101
 */
uint16_t AXP2101::getPortVoltage(AXP2101_registers_e reg) {
  const uint8_t creg = static_cast<uint8_t>(reg);
  const uint8_t data = readRegister8(AXP2101_ADDR,
                                     creg);

  return registerToVoltage(data,
                           reg);
}

/**
 * Set the on/off state of a port (output pin) of the AXP2101
 */
bool AXP2101::setPortState(bool                sw,
                           AXP2101_registers_e reg) {
  uint8_t ctrl   = 0;
  uint8_t mask   = 0;
  bool    result = false;

  getControlRegisterMask(reg, ctrl, mask);

  if (ctrl) {
    result = bitOnOff(sw, ctrl, mask);
  }
  return result;
}

/**
 * Get the on/off state of a port (output pin) of the AXP2101
 */
bool AXP2101::getPortState(AXP2101_registers_e reg) {
  uint8_t ctrl   = 0;
  uint8_t mask   = 0;
  bool    result = false;

  getControlRegisterMask(reg, ctrl, mask);

  if (ctrl) {
    result = bitGet(ctrl, mask);
  }
  return result;
}

bool AXP2101::enableADC(AXP2101_registers_e reg, bool enable)
{
  uint8_t ctrl   = 0;
  uint8_t mask   = 0;
  getControlRegisterMask(reg, ctrl, mask);

  if (ctrl != AXP2101_ADC_ENABLE_REG) {
    return false;
  }

  uint8_t val = readRegister8(AXP2101_ADDR, AXP2101_ADC_ENABLE_REG);

  const bool bit_set = ((val & mask) != 0);
  if (bit_set != enable) {

    if (enable) {
      val |= mask;
    } else {
      val &= ~mask;
    }
    writeRegister8(AXP2101_ADDR, AXP2101_ADC_ENABLE_REG, val);
  }
  return true;
}

uint16_t AXP2101::getADCVoltage(AXP2101_registers_e reg)
{
  if (!enableADC(reg, true)) return 0;
  if (reg == AXP2101_registers_e::vbus && !isVbusIn()) {
    return 0;
  }
  if (reg == AXP2101_registers_e::vbat && !isBatteryDetected()) {
    return 0;
  }

  const uint16_t mask = reg == AXP2101_registers_e::vbat ? 0x1F : 0x3F;

  const uint16_t hi = readRegister8(AXP2101_ADDR, static_cast<uint8_t>(reg));
  const uint16_t lo = readRegister8(AXP2101_ADDR, static_cast<uint8_t>(reg) + 1);
  return ((hi & mask) << 8) | lo;
}

/**
 * Compound functions, device model dependent
 */

// TODO Enable/disable these specific per device/models
void AXP2101::set_bus_3v3(uint16_t voltage) {
  if (AXP2101_device_model_e::M5Stack_Core2_v1_1 == _device) {
    if (!voltage) {
      set_dcdc1_on_off(false);
      set_dcdc3_on_off(false);
    } else {
      set_dcdc1_on_off(true);
      set_dcdc3_on_off(true);
      set_dcdc1_voltage(voltage);
      set_dcdc3_voltage(voltage);
    }
  } // else...
}

void AXP2101::set_lcd_back_light_voltage(uint16_t voltage) {
  if (AXP2101_device_model_e::M5Stack_Core2_v1_1 == _device) {
    if (!voltage) {
      set_bldo1_on_off(false);
    } else {
      set_bldo1_on_off(true);
      set_bldo1_voltage(voltage);
    }
  } // else...
}

void AXP2101::set_bus_5v(uint8_t sw) {
  if (sw) {
    set_bldo2_on_off(true);
    set_bldo2_voltage(3300);
  } else {
    set_bldo2_on_off(false);
  }
}

void AXP2101::set_spk(bool sw) {
  if (AXP2101_device_model_e::M5Stack_Core2_v1_1 == _device) {
    if (sw) {
      set_aldo3_on_off(true);
      set_aldo3_voltage(3300);
    } else {
      set_aldo3_on_off(false);
    }
  } // else...
}

void AXP2101::set_lcd_rst(bool sw) {
  if (AXP2101_device_model_e::M5Stack_Core2_v1_1 == _device) {
    if (sw) {
      set_aldo2_on_off(true);
      set_aldo2_voltage(3300);
    } else {
      set_aldo2_on_off(false);
    }
  } // else...
}

void AXP2101::set_lcd_and_tf_voltage(uint16_t voltage) {
  if (AXP2101_device_model_e::M5Stack_Core2_v1_1 == _device) {
    if (!voltage) {
      set_aldo4_on_off(false);
    } else {
      set_aldo4_on_off(true);
      set_aldo4_voltage(voltage);
    }
  } // else...
}

void AXP2101::set_vib_motor_voltage(uint16_t voltage) {
  if (AXP2101_device_model_e::M5Stack_Core2_v1_1 == _device) {
    if (!voltage) {
      set_dldo1_on_off(false);
    } else {
      set_dldo1_on_off(true);
      set_dldo1_voltage(voltage);
    }
  } // else...
}

/**
 * Universal functions
 */
bool AXP2101::set_sys_led(bool sw) {
  return bitOnOff(sw, AXP2101_CHGLED_REG, 0b00110000);
}

bool AXP2101::setChargeLed(AXP2101_chargeled_d led) {
  if (AXP2101_chargeled_d::Protected != led) {
    const uint8_t temp       = readRegister8(_addr, AXP2101_CHGLED_REG);
    const uint8_t data       = (static_cast<uint8_t>(led) & 0x03) << 4;
    const uint8_t write_back = ((temp & 0b11001111) | data);

    return writeRegister8(_addr, AXP2101_CHGLED_REG, write_back);
  }
  return false;
}

AXP2101_chargeled_d AXP2101::getChargeLed() {
  return static_cast<AXP2101_chargeled_d>((readRegister8(_addr, AXP2101_CHGLED_REG) >> 4) & 0x07);
}

bool AXP2101::getTS_disabled() {
  return bitGet(AXP2101_TS_PIN_CTRL_REG, 0b00010000);  
}

void AXP2101::setTS_disabled(bool val) {
  bitOnOff(val, AXP2101_TS_PIN_CTRL_REG, 0b00010000);
}

// Reg 61: Iprechg Charger settings
uint16_t AXP2101::getPreChargeCurrentLimit()  {
  // AXP2101_IPRECHG_REG
  // bit 3:0
  uint8_t reg = readRegister8(_addr,AXP2101_IPRECHG_REG);
  return (reg & 0b1111) * 25;
}
void AXP2101::setPreChargeCurrentLimit(uint16_t current_mA) {
  if (current_mA > 200) {
    current_mA = 200;
  }
  writeRegister8(_addr, AXP2101_IPRECHG_REG, current_mA / 25);
}

// Reg 62: ICC Charger settings
uint16_t AXP2101::getConstChargeCurrentLimit()  {
  // AXP2101_ICC_CHARGER_SETTING_REG
  // bit 4:0
  uint8_t reg = readRegister8(_addr, AXP2101_ICC_CHARGER_SETTING_REG);
  reg &= 0b11111;
  if (reg <= 8) {
    return reg * 25;
  }
  return (reg - 8) * 100 + 200;
}
void AXP2101::setConstChargeCurrentLimit(uint16_t current_mA) {
  if (current_mA > 1000) {
    current_mA = 1000;
  }

  const uint8_t reg = (current_mA <= 200) 
    ? current_mA / 25
    : ((current_mA - 200) / 100) + 8;
  writeRegister8(_addr, AXP2101_ICC_CHARGER_SETTING_REG, reg); 
}

// Reg 63: Iterm Charger settings and Control
// Enable/Disable via chargeStates.term_cur_lim_en
uint16_t AXP2101::getTerminationChargeCurrentLimit()  {
  // AXP2101_CHARGER_SETTING_REG
  // bit 4: enable/disable
  // bit 3:0
  uint8_t reg = readRegister8(_addr,AXP2101_CHARGER_SETTING_REG);
  return (reg & 0b1111) * 25;
}

void AXP2101::setTerminationChargeCurrentLimit(uint16_t current_mA) {
  constexpr uint8_t enable_mask = 0b00010000;
  const bool enabled = bitGet(AXP2101_CHARGER_SETTING_REG, enable_mask);
  setTerminationChargeCurrentLimit(current_mA, enabled);
}

void AXP2101::setTerminationChargeCurrentLimit(uint16_t current_mA, bool enable) {
  if (current_mA > 200) {
    current_mA = 200;
  }
  uint8_t reg = current_mA / 25;
  if (enable) {
    constexpr uint8_t enable_mask = 0b00010000;
    reg &= enable_mask;
  }
  writeRegister8(_addr, AXP2101_CHARGER_SETTING_REG, reg);
}


// Reg 64: CV Charger Voltage settings
AXP2101_CV_charger_voltage_e AXP2101::getCV_chargeVoltage()  {
  // AXP2101_CV_CHARGER_SETTING_REG
  // bit 2:0
  uint8_t reg = readRegister8(_addr,AXP2101_CV_CHARGER_SETTING_REG);
  reg &= 0b111;
  return static_cast<AXP2101_CV_charger_voltage_e>(reg);
}
void AXP2101::setCV_chargeVoltage(AXP2101_CV_charger_voltage_e voltage_mV) {
  uint8_t reg = static_cast<uint8_t>(voltage_mV);
  if (reg > static_cast<uint8_t>(AXP2101_CV_charger_voltage_e::limit_4_40V)) {
    // Set to a default safe limit
    reg = static_cast<uint8_t>(AXP2101_CV_charger_voltage_e::limit_4_20V);
  }
  writeRegister8(_addr, AXP2101_CV_CHARGER_SETTING_REG, reg);
}

// Reg 14: Minimum System Voltage Control
AXP2101_Linear_Charger_Vsys_dpm_e AXP2101::getLinear_Charger_Vsys_dpm()  {
  // AXP2101_MIN_VSYS_REG
  // bit 6:4
  uint8_t reg = readRegister8(_addr, AXP2101_MIN_VSYS_REG);
  reg &= 0b01110000;
  reg >>= 4;
  return static_cast<AXP2101_Linear_Charger_Vsys_dpm_e>(reg);
}
void AXP2101::setLinear_Charger_Vsys_dpm(AXP2101_Linear_Charger_Vsys_dpm_e voltage) {
  uint8_t reg = static_cast<uint8_t>(voltage);
  reg <<= 4;
  writeRegister8(_addr, AXP2101_MIN_VSYS_REG, reg);
}

// Reg 15: Input Voltage Limit
AXP2101_VINDPM_e AXP2101::getVin_DPM()  {
  // AXP2101_VIN_DPM_REG
  // bit 3:0
  uint8_t reg = readRegister8(_addr, AXP2101_VIN_DPM_REG);
  reg &= 0b00001111;
  return static_cast<AXP2101_VINDPM_e>(reg);
}
void AXP2101::setVin_DPM(AXP2101_VINDPM_e voltage) {
  uint8_t reg = static_cast<uint8_t>(voltage);
  reg &= 0b00001111;
  writeRegister8(_addr, AXP2101_VIN_DPM_REG, reg);
}

// Reg 16: Input Current Limit
AXP2101_InputCurrentLimit_e AXP2101::getInputCurrentLimit()  {
  // AXP2101_IN_CURRENT_LIMIT_REG
  // bit 2:0
  uint8_t reg = readRegister8(_addr, AXP2101_IN_CURRENT_LIMIT_REG);
  reg &= 0b00000111;
  return static_cast<AXP2101_InputCurrentLimit_e>(reg);
}

void AXP2101::setInputCurrentLimit(AXP2101_InputCurrentLimit_e current) {
  uint8_t reg = static_cast<uint8_t>(current);
  reg &= 0b00000111;
  writeRegister8(_addr, AXP2101_IN_CURRENT_LIMIT_REG, reg);
}

uint8_t AXP2101::getBatCharge() {
  return readRegister8(_addr, AXP2101_BAT_CHARGE_REG);
}

AXP2101_chargingState_e AXP2101::getChargingState() {
  const uint8_t level = (readRegister8(_addr, AXP2101_COM_STAT1_REG) >> 5) & 0x03;

  return static_cast<AXP2101_chargingState_e>(0x01 == level ? 1 : (0x02 == level ? -1 : 0));
}

bool AXP2101::isBatteryDetected() {
  return (readRegister8(_addr, AXP2101_COM_STAT0_REG) >> 3) & 0x01;
}

bool AXP2101::isVbusGood() {
  return bitGet(AXP2101_COM_STAT0_REG, (1 << 5));
}


bool AXP2101::isVbusIn() {
  return !bitGet(AXP2101_COM_STAT1_REG, (1 << 3)) && isVbusGood();
}

AXP2101_chargingDetail_e AXP2101::getChargingDetail() {
  return static_cast<AXP2101_chargingDetail_e>(readRegister8(_addr, AXP2101_COM_STAT1_REG) & 0x07);
}

uint8_t AXP2101::getChipIDRaw() {
  return readRegister8(_addr, AXP2101_CHIP_ID_REG);
}

AXP2101_chipid_e AXP2101::getChipID() {
  return static_cast<AXP2101_chipid_e>(getChipIDRaw());
}

bool AXP2101::set_charger_term_current_to_zero(void) {
  return bitOff(AXP2101_ADDR, AXP2101_CHARGER_SETTING_REG, 0b00001111);
}

bool AXP2101::setConstChargeCurrentLimit_to_50mA(void) {
  return writeRegister8(AXP2101_ADDR, AXP2101_ICC_CHARGER_SETTING_REG, 2);
}

void AXP2101::set_bat_charge(bool enable) {
  uint8_t val = 0;

  if (readRegister(AXP2101_ADDR, AXP2101_CHARG_FGAUG_WDOG_REG, &val, 1)) {
    writeRegister8(AXP2101_ADDR, AXP2101_CHARG_FGAUG_WDOG_REG, (val & 0xFD) + (enable << 1));
  }
}

bool AXP2101::enable_pwrok_resets(void) {
  return bitOn(AXP2101_ADDR,
               AXP2101_PMU_CONFIG_REG,
               1 << 3);
}

void AXP2101::set_IRQ_enable_0(uint8_t val) {
  // Clear any IRQ flags
  writeRegister8(AXP2101_ADDR, AXP2101_IRQ_STATUS_0_REG, 0);
  writeRegister8(AXP2101_ADDR, AXP2101_IRQ_STATUS_1_REG, 0);
  writeRegister8(AXP2101_ADDR, AXP2101_IRQ_STATUS_2_REG, 0);
  writeRegister8(AXP2101_ADDR, AXP2101_IRQ_EN_0_REG, val);
}

void AXP2101::power_off(void) {
  // 1. AXP2101 Power off
  bitOn(AXP2101_ADDR, AXP2101_IRQ_EN_1_REG, 1 << 1);                  // POWERON Negative Edge IRQ(ponne_irq_en) enable
  writeRegister8(AXP2101_ADDR, AXP2101_PWROK_PWROFF_REG, 0b00011011); // sleep and wait for wakeup
  delay(100);
  writeRegister8(AXP2101_ADDR, AXP2101_PMU_CONFIG_REG, 0b00110001);   // power off
}

uint8_t AXP2101::get_dcdc_status(void) {
  return readRegister8(_addr,
                       AXP2101_DCDC_CTRL_REG);
}

void AXP2101::getControlRegisterMask(AXP2101_registers_e reg,
                                     uint8_t           & ctrl,
                                     uint8_t           & mask) {
  switch (reg) {
    case AXP2101_registers_e::dcdc1:
      ctrl = AXP2101_DCDC_CTRL_REG;
      mask = AXP2101_DCDC1_CTRL_MASK;
      break;
    case AXP2101_registers_e::dcdc2:
      ctrl = AXP2101_DCDC_CTRL_REG;
      mask = AXP2101_DCDC2_CTRL_MASK;
      break;
    case AXP2101_registers_e::dcdc3:
      ctrl = AXP2101_DCDC_CTRL_REG;
      mask = AXP2101_DCDC3_CTRL_MASK;
      break;
    case AXP2101_registers_e::dcdc4:
      ctrl = AXP2101_DCDC_CTRL_REG;
      mask = AXP2101_DCDC4_CTRL_MASK;
      break;
    case AXP2101_registers_e::dcdc5:
      ctrl = AXP2101_DCDC_CTRL_REG;
      mask = AXP2101_DCDC5_CTRL_MASK;
      break;
    case AXP2101_registers_e::aldo1:
      ctrl = AXP2101_LDO_CTRL_REG;
      mask = AXP2101_ALDO1_CTRL_MASK;
      break;
    case AXP2101_registers_e::aldo2:
      ctrl = AXP2101_LDO_CTRL_REG;
      mask = AXP2101_ALDO2_CTRL_MASK;
      break;
    case AXP2101_registers_e::aldo3:
      ctrl = AXP2101_LDO_CTRL_REG;
      mask = AXP2101_ALDO3_CTRL_MASK;
      break;
    case AXP2101_registers_e::aldo4:
      ctrl = AXP2101_LDO_CTRL_REG;
      mask = AXP2101_ALDO4_CTRL_MASK;
      break;
    case AXP2101_registers_e::bldo1:
      ctrl = AXP2101_LDO_CTRL_REG;
      mask = AXP2101_BLDO1_CTRL_MASK;
      break;
    case AXP2101_registers_e::bldo2:
      ctrl = AXP2101_LDO_CTRL_REG;
      mask = AXP2101_BLDO2_CTRL_MASK;
      break;
    case AXP2101_registers_e::dldo1:
      ctrl = AXP2101_LDO_CTRL_REG;
      mask = AXP2101_DLDO1_CTRL_MASK;
      break;
    case AXP2101_registers_e::dldo2:
      ctrl = AXP2101_LDO_CTRL_REG1;
      mask = AXP2101_DLDO2_CTRL_MASK;
      break;
    case AXP2101_registers_e::cpuldos:
      ctrl = AXP2101_LDO_CTRL_REG;
      mask = AXP2101_CPUSLDO_CTRL_MASK;
      break;
    case AXP2101_registers_e::chargeled:
      ctrl = AXP2101_CHGLED_REG;
      mask = AXP2101_CHGLED_CTRL_MASK;
      break;
    case AXP2101_registers_e::batcharge:
      ctrl = AXP2101_BAT_CHARGE_REG;
      mask = 0xFF;
      break;
    case AXP2101_registers_e::charging:
      ctrl = AXP2101_COM_STAT1_REG;
      mask = 0b01100000;
      break;
    case AXP2101_registers_e::batpresent:
      ctrl = AXP2101_COM_STAT0_REG;
      mask = 0b00001000;
      break;
    case AXP2101_registers_e::chipid:
      ctrl = AXP2101_CHIP_ID_REG;
      mask = 0b11111111;
      break;
    case AXP2101_registers_e::chargedet:
      ctrl = AXP2101_COM_STAT1_REG;
      mask = 0b00000111;
      break;


    case AXP2101_registers_e::vbat:
      ctrl = AXP2101_ADC_ENABLE_REG;
      mask = AXP2101_VBAT_CTRL_MASK;
      break;
    case AXP2101_registers_e::battemp:
      ctrl = AXP2101_ADC_ENABLE_REG;
      mask = AXP2101_BATTEMP_CTRL_MASK;
      break;
    case AXP2101_registers_e::vbus:
      ctrl = AXP2101_ADC_ENABLE_REG;
      mask = AXP2101_VBUS_CTRL_MASK;
      break;
    case AXP2101_registers_e::vsys:
      ctrl = AXP2101_ADC_ENABLE_REG;
      mask = AXP2101_VSYS_CTRL_MASK;
      break;
    case AXP2101_registers_e::chiptemp:
      ctrl = AXP2101_ADC_ENABLE_REG;
      mask = AXP2101_TDIE_CTRL_MASK;
      break;
  }
}

# pragma GCC diagnostic pop

#endif // ifdef ESP32
