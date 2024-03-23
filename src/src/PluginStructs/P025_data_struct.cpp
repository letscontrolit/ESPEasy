#include "../PluginStructs/P025_data_struct.h"

#ifdef USES_P025

# include "../WebServer/DevicesPage.h" // Needed for format_I2C_port_description


# define P025_CONVERSION_REGISTER  0x00
# define P025_CONFIG_REGISTER      0x01


P025_VARIOUS_BITS_t::P025_VARIOUS_BITS_t(int16_t value) {
  memcpy(this, &value, sizeof(int16_t));
}

int16_t P025_VARIOUS_BITS_t::pconfigvalue() const {
  int16_t value{};
  memcpy(&value, this, sizeof(int16_t));
  return value;
}

const __FlashStringHelper* Plugin_025_valuename(uint8_t value_nr, bool displayString) {
  const __FlashStringHelper *strings[] {
    F("AIN0 - AIN1 (Differential)"),     F("AIN0_1"),
    F("AIN0 - AIN3 (Differential)"),     F("AIN0_3"),
    F("AIN1 - AIN3 (Differential)"),     F("AIN1_3"),
    F("AIN2 - AIN3 (Differential)"),     F("AIN2_3"),
    F("AIN0 - GND (Single-Ended)"),      F("AIN0"),
    F("AIN1 - GND (Single-Ended)"),      F("AIN1"),
    F("AIN2 - GND (Single-Ended)"),      F("AIN2"),
    F("AIN3 - GND (Single-Ended)"),      F("AIN3")
  };
  const size_t index         = (2 * value_nr) + (displayString ? 0 : 1);
  constexpr size_t nrStrings = NR_ELEMENTS(strings);

  if (index < nrStrings) {
    return strings[index];
  }
  return F("");
}

const __FlashStringHelper* toString(P025_sensorType sensorType)
{
  if (P025_sensorType::None == sensorType) { return F("None"); }
  return (P025_sensorType::ADS1015 == sensorType) ?
         F("ADS1015") : F("ADS1115");
}

struct P025_config_register {
  struct {
    uint16_t comp_que        : 2;
    uint16_t comp_lat        : 1;
    uint16_t comp_pol        : 1;
    uint16_t compMode        : 1;
    uint16_t datarate        : 3;
    uint16_t mode            : 1;
    uint16_t PGA             : 3;
    uint16_t MUX             : 3;
    uint16_t operatingStatus : 1;
  };

  P025_config_register(uint16_t regval) {
    memcpy(this, &regval, sizeof(uint16_t));
  }

  void setRegval(uint16_t regval) {
    memcpy(this, &regval, sizeof(uint16_t));
  }

  uint16_t getRegval() const {
    uint16_t regval{};
    memcpy(&regval, this, sizeof(uint16_t));
    return regval;
  }

  String toString() const {
    return strformat(F("reg: %X OS: %d MUX: %d PGA: %d mode: %d DR: %d"),
                     getRegval(), operatingStatus, MUX, PGA, mode, datarate
                     );
  }
};


P025_data_struct::P025_data_struct(struct EventStruct *event) {
  _i2cAddress = P025_I2C_ADDR;

  const P025_VARIOUS_BITS_t p025_variousBits(P025_VARIOUS_BITS);

  constexpr uint16_t defaultValue =
    (0x0003)    | // Disable the comparator (default val)
    (0x0000)    | // Non-latching (default val)
    (0x0000)    | // Alert/Rdy active low   (default val)
    (0x0000)    | // Traditional comparator (default val)
    (0x0100)    | // Single-shot mode (default)
    (0x8000);     // Start a single conversion

  P025_config_register reg(defaultValue);

  reg.datarate         = p025_variousBits.getSampleRate();
  reg.PGA              = P025_GAIN;
  _configRegisterValue = reg.getRegval();

  _fullScaleFactor = 1.0f;

  if (p025_variousBits.outputVolt) {
    if (reg.PGA == 0) {
      _fullScaleFactor = 6144;
    } else {
      const uint8_t shift = 13 - reg.PGA;
      _fullScaleFactor = (1 << shift);
    }
    _fullScaleFactor /= 32768000.0f;
  }

  for (taskVarIndex_t i = 0; i < VARS_PER_TASK; ++i) {
    _mux[i] = P025_MUX(i);
  }
}

bool P025_data_struct::read(float& value, taskVarIndex_t index) const {
  if (!validTaskVarIndex(index)) {
    return false;
  }
  P025_config_register reg(_configRegisterValue);

  reg.MUX = _mux[index];

  if (!startMeasurement(_i2cAddress, reg.getRegval())) {
    return false;
  }

  if (!I2C_write16_reg(_i2cAddress, P025_CONFIG_REGISTER, reg.getRegval())) {
# ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLog(LOG_LEVEL_DEBUG,
             concat(F("ADS1x15: Start measurement failed"),
                    reg.toString()));
    }
# endif // ifndef BUILD_NO_DEBUG

    return false;
  }

  const long sds = waitReady025(_i2cAddress);

  if (sds == 0) {
# ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLog(LOG_LEVEL_DEBUG,
             concat(F("ADS1x15: Not Ready after start measurement"),
                    reg.toString()));
    }
# endif // ifndef BUILD_NO_DEBUG

    return false;
  }

# ifndef BUILD_NO_DEBUG
  else {
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLog(LOG_LEVEL_DEBUG,
             concat(F("ADS1x15: Detected SPS: "), sds));
    }
  }
# endif // ifndef BUILD_NO_DEBUG


  int16_t raw = 0;

  if (!readConversionRegister025(raw)) {
# ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLog(LOG_LEVEL_DEBUG,
             concat(F("ADS1x15: Cannot read from conversion register"),
                    reg.toString()));
    }
# endif // ifndef BUILD_NO_DEBUG
    return false;
  }

  value = _fullScaleFactor * raw;

  addLog(LOG_LEVEL_INFO, strformat(F("ADS1x15: RAW value: %d, output value: %f"), raw, value));
  return true;
}

bool P025_data_struct::readConversionRegister025(int16_t& value) const {
  bool is_ok = false;

  // Conversion register represents 2-complement format.
  value = (int16_t)I2C_read16_reg(_i2cAddress, P025_CONVERSION_REGISTER, &is_ok);
  return is_ok;
}

P025_sensorType P025_data_struct::detectType(uint8_t i2cAddress)
{
  P025_sensorType sensorType = P025_sensorType::None;

  constexpr uint16_t defaultValue =
    (0x0003)    | // Disable the comparator (default val)
    (0x0000)    | // Non-latching (default val)
    (0x0000)    | // Alert/Rdy active low   (default val)
    (0x0000)    | // Traditional comparator (default val)
    (0x00A0)    | // 250 (ADS1115) or 2400 (ADS1015) samples per second
    (0x0100)    | // Single-shot mode (default)
    (0x0000)    | // PGA to max FSR of 6.144V
    (0x0000)    | // MUX: differential AIN0/AIN1 (default val)
    (0x8000);     // Start a single conversion

  if (startMeasurement(i2cAddress, defaultValue)) {
    const long sps = waitReady025(i2cAddress);

    if (sps > 0) {
      // Sample rate is either 250 (ADS1115) or 2400 (ADS1015)
      sensorType = (sps < 500) ? P025_sensorType::ADS1115 : P025_sensorType::ADS1015;
    }
  }
  return sensorType;
}

bool P025_data_struct::startMeasurement(uint8_t i2cAddress, uint16_t configRegisterValue)
{
  if (I2C_write16_reg(i2cAddress, P025_CONFIG_REGISTER, configRegisterValue)) {
    return true;
  }
# ifndef BUILD_NO_DEBUG

  const P025_config_register reg(configRegisterValue);

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLog(LOG_LEVEL_DEBUG,
           concat(F("ADS1x15: Start measurement failed"),
                  reg.toString()));
  }
# endif // ifndef BUILD_NO_DEBUG
  return false;
}

long P025_data_struct::waitReady025(uint8_t i2cAddress)
{
  const uint64_t start   = getMicros64();
  unsigned long  timeout = millis();

  bool is_ok = false;

  // Only need to set the Address Pointer Register once
  P025_config_register reg(I2C_read16_reg(i2cAddress, P025_CONFIG_REGISTER, &is_ok));

  if (!is_ok) {
    return 0;
  }

  // Compute expected timeout
  // Rough estimate of SPS = (1 << (DR + 3))
  // Add margin of roughly 50%
  // Add 1 msec as minimum, due to rounding errors at highest frame rate
  // See https://github.com/letscontrolit/ESPEasy/issues/3159#issuecomment-660546091
  timeout += 1500 / (1 << (reg.datarate + 3)) + 1;

  while (!timeOutReached(timeout)) {
    // bit15=0 performing a conversion   =1 not performing a conversion
    const bool ready = reg.operatingStatus == 1;

    if (ready && is_ok) {
      const long res = usecPassedSince(start);
      const long sps = (res > 0) ? 1000000 / res : 0;
# ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        addLog(LOG_LEVEL_DEBUG,
               concat(F("ADS1x15: waitReady OK, Config_reg: "),
                      reg.toString()));
      }
# endif // ifndef BUILD_NO_DEBUG

      return sps;
    }
    delay(0);

    // Address Pointer Register is the same, so only need to read bytes again
    reg.setRegval(I2C_read16(i2cAddress, &is_ok));
  }

# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLog(LOG_LEVEL_DEBUG,
           concat(F("ADS1x15: waitReady timeout, Config_reg: "),
                  reg.toString()));
  }
# endif // ifndef BUILD_NO_DEBUG
  return 0;
}

bool P025_data_struct::webformLoad(struct EventStruct *event)
{
  const uint8_t port = CONFIG_PORT;

  if (port > 0) // map old port logic to new gain and mode settings
  {
    P025_GAIN                       = PCONFIG(0) / 2;
    P025_I2C_ADDR                   = 0x48 + ((port - 1) / 4);
    P025_MUX(0)                     = ((port - 1) & 3) | 4;
    CONFIG_PORT                     = 0;
    PCONFIG(P025_SENSOR_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_SINGLE);
  }

  const P025_sensorType detectedType = detectType(P025_I2C_ADDR);

  addRowLabel(F("Detected Sensor Type"));
  addHtml(toString(detectedType));

  const P025_VARIOUS_BITS_t p025_variousBits(P025_VARIOUS_BITS);

  {
    const __FlashStringHelper *pgaOptions[] = {
      F("2/3x gain (FS=6.144V)"),
      F("1x gain (FS=4.096V)"),
      F("2x gain (FS=2.048V)"),
      F("4x gain (FS=1.024V)"),
      F("8x gain (FS=0.512V)"),
      F("16x gain (FS=0.256V)")
    };

    constexpr size_t ADS1115_PGA_OPTIONS = NR_ELEMENTS(pgaOptions);
    addFormSelector(F("Gain"), F("gain"), ADS1115_PGA_OPTIONS, pgaOptions, nullptr, P025_GAIN);
    addFormNote(F("Do not apply more than VDD + 0.3 V to the analog inputs of the device."));
  }
  {
    const __FlashStringHelper *P025_SPSOptions[] = {
      F("8 / 128"),
      F("16 / 250"),
      F("32 / 490"),
      F("64 / 920"),
      F("128 / 1600"),
      F("250 / 2400"),
      F("475 / 3300"),
      F("860 / 3300"),
    };
    constexpr size_t NR_OPTIONS = NR_ELEMENTS(P025_SPSOptions);

    addFormSelector(F("Sample Rate"), F("sps"), NR_OPTIONS, P025_SPSOptions, nullptr, p025_variousBits.getSampleRate());
    addUnit(F("SPS"));
    addFormNote(F("Lower values for ADS1115, higher values for ADS1015"));

    if (P025_sensorType::ADS1015 != detectedType) {
      addFormNote(F("Sample rates < 64 may cause issues"));
    }
  }

  addFormCheckBox(F("Convert to Volt"), F("volt"), p025_variousBits.outputVolt);


  addFormSubHeader(F("Two Point Calibration"));

  addFormCheckBox(F("Calibration Enabled"), F("cal"), p025_variousBits.cal);

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
  for (uint8_t i = 0; i < P025_NR_OUTPUT_VALUES; i++) {
    const uint8_t pconfigIndex = P025_PCONFIG_INDEX(i);
    const uint8_t choice       = PCONFIG(pconfigIndex);
    sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i,
                                        Plugin_025_valuename(choice, false));
  }

  P025_I2C_ADDR = getFormItemInt(F("i2c_addr"));

  P025_GAIN = getFormItemInt(F("gain"));

  P025_VARIOUS_BITS_t p025_variousBits(P025_VARIOUS_BITS);

  p025_variousBits.setSampleRate(getFormItemInt(F("sps")));
  p025_variousBits.outputVolt = isFormItemChecked(F("volt"));
  p025_variousBits.cal        = isFormItemChecked(F("cal"));
  P025_VARIOUS_BITS           = p025_variousBits.pconfigvalue();

  P025_CAL_ADC1 = getFormItemInt(F("adc1"));
  P025_CAL_OUT1 = getFormItemFloat(F("out1"));

  P025_CAL_ADC2 = getFormItemInt(F("adc2"));
  P025_CAL_OUT2 = getFormItemFloat(F("out2"));

  return true;
}

bool P025_data_struct::webform_showConfig(struct EventStruct *event)
{
  format_I2C_port_description(event->TaskIndex);

  for (uint8_t i = 0; i < P025_NR_OUTPUT_VALUES; i++) {
    const uint8_t choice = PCONFIG(P025_PCONFIG_INDEX(i));

    if ((choice >= 0) && (choice < 8)) {
      addHtml(F("<br>"));
      addHtml(Plugin_025_valuename(choice, false));
    }
  }

  return true;
}

#endif // ifdef USES_P025
