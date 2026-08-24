#include "../PluginStructs/P188_data_struct.h"

#ifdef USES_P188

# include "../WebServer/DevicesPage.h" // Needed for format_I2C_port_description

const __FlashStringHelper* Plugin_188_output_mapping_name(uint8_t value_nr, bool displayString) {
  const __FlashStringHelper *strings[] {
    F("AIN0 to GND Single-Ended"),          F("V_AIN0"),           // 0
    F("AIN1 to GND Single-Ended"),          F("V_AIN1"),           // 1
    F("AIN2 to GND Single-Ended"),          F("V_AIN2"),           // 2
    F("AIN3 to GND Single-Ended"),          F("V_AIN3"),           // 3
    F("AIN4 to GND Single-Ended"),          F("V_AIN4"),           // 4
    F("AIN5 to GND Single-Ended"),          F("V_AIN5"),           // 5
    F("AIN6 to GND Single-Ended"),          F("V_AIN6"),           // 6
    F("AIN7 to GND Single-Ended"),          F("V_AIN7"),           // 7
    F("ABS(AIN1 - AIN0) (Difference)"),     F("V_abs(AIN1_AIN0)"), // 8
    F("ABS(AIN3 - AIN2) (Difference)"),     F("V_abs(AIN3_AIN2)"), // 9
    F("ABS(AIN5 - AIN4) (Difference)"),     F("V_abs(AIN5_AIN4)"), // 10
    F("ABS(AIN7 - AIN6) (Difference)"),     F("V_abs(AIN7_AIN6)"), // 11
#ifdef P188_FEATURE_RESISTOR_MEASUREMENT
    F("AIN1, AIN0 Resistor Measurement"),   F("R_(AIN1_AIN0)"),    // 12
    F("AIN3, AIN2 Resistor Measurement"),   F("R_(AIN3_AIN2)"),    // 13
    F("AIN5, AIN4 Resistor Measurement"),   F("R_(AIN5_AIN4)"),    // 14
    F("AIN7, AIN6 Resistor Measurement"),   F("R_(AIN7_AIN6)")     // 15
#endif // P188_FEATURE_RESISTOR_MEASUREMENT
  };
  const size_t index         = (2 * value_nr) + (displayString ? 0 : 1);
  constexpr size_t nrStrings = NR_ELEMENTS(strings);

  if (index < nrStrings) {
    return strings[index];
  }
  return F("");
}

P188_CONFIG_BITS_t::P188_CONFIG_BITS_t(int16_t value) : _regValue(value) {}

P188_data_struct::P188_data_struct(struct EventStruct *event) {

}

uint8_t P188_data_struct::TLA2528_read_single_reg(uint8_t i2caddr, uint8_t reg)
{
  uint8_t retVal;
  bool isOK = false;

  I2C_write8_reg(i2caddr, TLA2528_OPC_SINGLE_READ, reg);
  retVal = I2C_read8(i2caddr, &isOK);
  return(retVal);
}

bool P188_data_struct::TLA2528_write_single_reg(uint8_t i2caddr, uint8_t reg, uint8_t data)
{
  return(I2C_write8_reg16(i2caddr, (TLA2528_OPC_SINGLE_WRITE<<8) + reg, data));
}

bool P188_data_struct::init(struct EventStruct *event) {
  if (event != nullptr) {
    LoadCustomTaskSettings(event->TaskIndex, (uint8_t *)&(P188_config), sizeof(P188_config));  // load configuration from flash

    initialized = !I2C_wakeup(P188_config.i2cAddress);
    if (initialized) {

      initialized = initialized && P188_data_struct::TLA2528_write_single_reg(P188_config.i2cAddress, TLA2528_REG_GENERAL_CFG, 0x01);  // reset IC

      _sample_cnt = 0;
      while (TLA2528_BIT_GC_RST & P188_data_struct::TLA2528_read_single_reg(P188_config.i2cAddress, TLA2528_REG_GENERAL_CFG))  { // wait for reset to complete
        delayMicroseconds(500);
        if (_sample_cnt > 9) {
          return(false);
        }     
        _sample_cnt ++;
      }

      initialized = initialized && P188_data_struct::TLA2528_write_single_reg(P188_config.i2cAddress, TLA2528_REG_GENERAL_CFG, 0x02);  // start calibration

      _sample_cnt = 0;
      while (TLA2528_BIT_GC_CAL & P188_data_struct::TLA2528_read_single_reg(P188_config.i2cAddress, TLA2528_REG_GENERAL_CFG)) { // wait for calibration to complete
        delayMicroseconds(500);
        if (_sample_cnt > 9) {
          return(false);
        }      
        _sample_cnt ++;
      }

      initialized = initialized && P188_data_struct::TLA2528_write_single_reg(P188_config.i2cAddress, TLA2528_REG_DATA_CFG, TLA2528_BIT_DC_APPEND_STATUS);  // automaticall append channel ID
    }
    _sample_cnt = 0;
  }
  return(initialized);
}

bool P188_data_struct::sample(void) // called each 100ms, runtime: ~800us
{
  /* each call of P188_data_struct::sample() successive converts 2 ADC input channels as fast */
  /* as possible to (nearly) provide simultaneity, which is needed for ratiometric measurements */
  /* _sample_cnt = 0, 2, 4 or 6 to sample channels 0&1 or 2&3 or 4&5 or 6&7 */

  uint32_t data;
  bool success;

  if ((_sample_cnt & 0x01) || (_sample_cnt > 6))  // invalid sample count, reset to 0
    _sample_cnt = 0;

  success = TLA2528_write_single_reg(P188_config.i2cAddress, TLA2528_REG_AUTO_SEQ_CH_SEL, (0x03 << _sample_cnt));  // configure auto conversion sequence for channels 0/1 or 2/3 or 4/5 or 6/7
  if (success)
    success = TLA2528_write_single_reg(P188_config.i2cAddress, TLA2528_REG_SEQUENCE_CFG, TLA2528_BIT_SC_SEQ_MODE + TLA2528_BIT_SC_SEQ_START);  // start sequence
  if (success)
    data = I2C_read32(P188_config.i2cAddress, &success);  // start conversion & read result
  if (success){
    /* be aware that raw data contains the channel nummer in bits[3:0] */
    raw_samples[_sample_cnt] = data >> 16;
    raw_samples[_sample_cnt + 1] = data & 0xffff;
  }
  success = success && TLA2528_write_single_reg(P188_config.i2cAddress, TLA2528_REG_SEQUENCE_CFG, 0);  // stop sequence

  _sample_cnt += 2;

  return(success);
}

bool P188_data_struct::read_raw(struct EventStruct *event, float& value, uint8_t ch_num) const {
  if (ch_num > 8) {
    return false;
  }
  value = raw_samples[ch_num] >> 4; // remove channel number from raw value
  return(true);
}

#endif // ifdef USES_P188
