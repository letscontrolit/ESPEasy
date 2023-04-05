#include "../PluginStructs/P025_data_struct.h"

#ifdef USES_P025

# include "src/WebServer/DevicesPage.h" // Needed for format_I2C_port_description


# define P025_CONVERSION_REGISTER  0x00
# define P025_CONFIG_REGISTER      0x01


P025_data_struct::P025_data_struct(struct EventStruct *event) {
  _i2cAddress = P025_I2C_ADDR;

  constexpr uint16_t defaultValue =
    (0x0003)    | // Disable the comparator (default val)
    (0x0000)    | // Non-latching (default val)
    (0x0000)    | // Alert/Rdy active low   (default val)
    (0x0000)    | // Traditional comparator (default val)
    (0x0080)    | // 128 samples per second (default)
    (0x0100)    | // Single-shot mode (default)
    (0x8000);     // Start a single conversion

  _configRegisterValue = defaultValue |
                         (static_cast<uint16_t>(P025_GAIN) << 9) |
                         (static_cast<uint16_t>(P025_MUX) << 12);
  _fullScaleFactor = 1.0f;

  if (P025_VOLT_OUT_GET) {
    if (P025_GAIN == 0) { 
      _fullScaleFactor = 6.144 / 32768.0f; 
    } else {
      const uint8_t shift = 13 - P025_GAIN;
      _fullScaleFactor = (1 << shift) / 32768000.0f;
    }
  }
}

bool P025_data_struct::read(float& value) const {
  if (!waitReady025(5)) { return false; }

  if (!I2C_write16_reg(_i2cAddress, P025_CONFIG_REGISTER, _configRegisterValue)) {
    return false;
  }

  // See https://github.com/letscontrolit/ESPEasy/issues/3159#issuecomment-660546091
  if (!waitReady025(10)) { return false; }

  int16_t raw = 0;
  if (!readConversionRegister025(raw)) return false;

  value = _fullScaleFactor * raw;
  return true;
}

bool P025_data_struct::readConversionRegister025(int16_t& value) const {
  bool is_ok = false;

  // Conversion register represents 2-complement format.
  value = (int16_t)I2C_read16_reg(_i2cAddress, P025_CONVERSION_REGISTER, &is_ok);
  return is_ok;
}

bool P025_data_struct::waitReady025(unsigned long timeout_ms) const {
  const unsigned long timeout = millis() + timeout_ms;

  while (!timeOutReached(timeout)) {
    bool is_ok = false;

    // bit15=0 performing a conversion   =1 not performing a conversion
    const bool ready = (I2C_read16_reg(_i2cAddress, P025_CONFIG_REGISTER, &is_ok) & 0x8000) != 0;

    if (!is_ok) { return false; }

    if (ready) { return true; }
    delay(1);
  }
  return false;
}

bool P025_data_struct::webformLoad(struct EventStruct *event)
{
  const uint8_t port = CONFIG_PORT;

  if (port > 0) // map old port logic to new gain and mode settings
  {
    P025_GAIN     = PCONFIG(0) / 2;
    P025_I2C_ADDR = 0x48 + ((port - 1) / 4);
    P025_MUX      = ((port - 1) & 3) | 4;
    CONFIG_PORT   = 0;
  }

  {
    const __FlashStringHelper *pgaOptions[] = {
      F("2/3x gain (FS=6.144V)"),
      F("1x gain (FS=4.096V)"),
      F("2x gain (FS=2.048V)"),
      F("4x gain (FS=1.024V)"),
      F("8x gain (FS=0.512V)"),
      F("16x gain (FS=0.256V)")
    };

    constexpr size_t ADS1115_PGA_OPTIONS = sizeof(pgaOptions) / sizeof(pgaOptions[0]);
    addFormSelector(F("Gain"), F("gain"), ADS1115_PGA_OPTIONS, pgaOptions, nullptr, P025_GAIN);
  }
  {
    const __FlashStringHelper *P025_muxOptions[] = {
      F("AIN0 - AIN1 (Differential)"),
      F("AIN0 - AIN3 (Differential)"),
      F("AIN1 - AIN3 (Differential)"),
      F("AIN2 - AIN3 (Differential)"),
      F("AIN0 - GND (Single-Ended)"),
      F("AIN1 - GND (Single-Ended)"),
      F("AIN2 - GND (Single-Ended)"),
      F("AIN3 - GND (Single-Ended)"),
    };
    constexpr size_t ADS1115_MUX_OPTIONS = sizeof(P025_muxOptions) / sizeof(P025_muxOptions[0]);

    addFormSelector(F("Input Multiplexer"), F("mux"), ADS1115_MUX_OPTIONS, P025_muxOptions, nullptr, P025_MUX);
  }

  addFormCheckBox(F("Convert to Volt"), F("volt"), P025_VOLT_OUT_GET);


  addFormSubHeader(F("Two Point Calibration"));

  addFormCheckBox(F("Calibration Enabled"), F("cal"), P025_CAL_GET);

  addFormNumericBox(F("Point 1"), F("adc1"), P025_CAL_ADC1, -32768, 32767);
  html_add_estimate_symbol();
  addTextBox(F("out1"), toString(P025_CAL_OUT1, 3), 10);

  addFormNumericBox(F("Point 2"), F("adc2"), P025_CAL_ADC2, -32768, 32767);
  html_add_estimate_symbol();
  addTextBox(F("out2"), toString(P025_CAL_OUT2, 3), 10);
  return true;
}

bool P025_data_struct::webformSave(struct EventStruct *event)
{
  P025_I2C_ADDR = getFormItemInt(F("i2c_addr"));

  P025_GAIN = getFormItemInt(F("gain"));

  P025_MUX = getFormItemInt(F("mux"));

  P025_VOLT_OUT_SET(isFormItemChecked(F("volt")));

  P025_CAL_SET(isFormItemChecked(F("cal")));

  P025_CAL_ADC1 = getFormItemInt(F("adc1"));
  P025_CAL_OUT1 = getFormItemFloat(F("out1"));

  P025_CAL_ADC2 = getFormItemInt(F("adc2"));
  P025_CAL_OUT2 = getFormItemFloat(F("out2"));

  return true;
}

bool P025_data_struct::webform_showConfig(struct EventStruct *event)
{
  format_I2C_port_description(event->TaskIndex);

  const __FlashStringHelper *P025_muxOptions[] = {
    F("AIN0/1 (Diff)"),
    F("AIN0/3 (Diff)"),
    F("AIN1/3 (Diff)"),
    F("AIN2/3 (Diff)"),
    F("AIN0"),
    F("AIN1"),
    F("AIN2"),
    F("AIN3"),
  };
  constexpr size_t ADS1115_MUX_OPTIONS = sizeof(P025_muxOptions) / sizeof(P025_muxOptions[0]);

  if ((P025_MUX >= 0) && (P025_MUX < static_cast<int>(ADS1115_MUX_OPTIONS))) {
    addHtml(F("<br>"));
    addHtml(P025_muxOptions[P025_MUX]);
  }

  return true;
}

#endif // ifdef USES_P025
