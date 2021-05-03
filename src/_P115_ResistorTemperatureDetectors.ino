#include "_Plugin_Helper.h"
#ifdef USES_P115

// #######################################################################################################
// ######################## Plugin 115: Resistor Temperature Detectors (MAX31865) ########################
// #######################################################################################################

// Original work by chri.kai.in
//  - based upon _P039_Thermocouple by Dominik
//  - and Adafruit C++ library for MAX 31865 shield

// Plugin Description
//  This Plugin reads the data from Resistor Temperature Detectors. You have to use an Adapter Board with a
//  MAX 31865 in order to read the values. Take a look at the usual sources to find such boards :-)
//  You can only use ESP8266 boards which expose the SPI Interface. This Plugin uses only the Hardware
//  SPI Interface - no software SPI at the moment.
//  But nevertheless you need at least 3 Pins to use SPI. So using an very simple ESP-01 is no option - Sorry.
//  The Wiring is straight forward ...
//
//  Have fun ... Christoph

// Wiring
// https://de.wikipedia.org/wiki/Serial_Peripheral_Interface
// You need an ESP8266 device with accessible SPI Pins. These are:
// Name   Description     GPIO      NodeMCU   Notes
// MOSI   Master Output   GPIO13    D7        Not used (No Data sending to MAX)
// MISO   Master Input    GPIO12    D6        Hardware SPI
// SCK    Clock Output    GPIO14    D5        Hardware SPI
// CS     Chip Select     GPIO15    D8        Hardware SPI (CS is configurable through the web interface)
//
// remark: some SPI modules provide pullup resistor on CS input (e.g. 10k)
//         -> using GPIO15/D8 for CS then might require pull down resistor (e.g. 4,7k) to GND to ensure proper booting of ESP

// Resistor Temperature Detectors Infos
// http://www.bristolwatch.com/ele2/therc.htm

// Chips
// MAX31865 - RTD-to-Digital Converter, capable of handling PT100 and PT1000 temperature sensors in 2-, 3- or 4-wire connections
//            https://datasheets.maximintegrated.com/en/ds/MAX31865.pdf

#include <SPI.h>

#define PLUGIN_115  
#define PLUGIN_ID_115               115
#define PLUGIN_NAME_115             "Environment - Resistor Temperature Detectors"
#define PLUGIN_VALUENAME1_115       "Temperature"

#define P115_MAX_TYPE               PCONFIG(0)
#define P115_RTD_TYPE               PCONFIG(1)
#define P115_RTD_CON_TYPE           PCONFIG(2)
#define P115_RTD_FILT_TYPE          PCONFIG(3)
#define P115_RTD_RES                PCONFIG(4)
#define P115_RTD_LM_TYPE            PCONFIG(5)
#define P115_RTD_LM_SHTDWN          PCONFIG(6)
#define P115_RTD_OFFSET             PCONFIG_FLOAT(0)

#define CS_Delay()                  delayMicroseconds(500)
#define MAX31865_CONVERSION_BREAK   100

#define MAX31865_PT100              0
#define MAX31865_PT1000             1

#define P115_MAX31865               0
#define P115_LM7x                   1

#define P115_READ_ADDR_BASE         0x00
#define P115_WRITE_ADDR_BASE        0x80

#define P115_CONFIG                 0
#define P115_RTD_MSB                1
#define P115_RTD_LSB                2
#define P115_HFT_MSB                3
#define P115_HFT_LSB                4
#define P115_LFT_MSB                5
#define P115_LFT_LSB                6
#define P115_FAULT                  7

#define P115_NO_REG                 8

// all failure defines
#define MAX31865_FAULT_HIGHTHRESH   0x80
#define MAX31865_FAULT_LOWTHRESH    0x40
#define MAX31865_FAULT_REFINLOW     0x20
#define MAX31865_FAULT_REFINHIGH    0x10
#define MAX31865_FAULT_RTDINLOW     0x08
#define MAX31865_FAULT_OVUV         0x04

#define MAX31865_SET_50HZ           0x01
#define MAX31865_CLEAR_FAULTS       0x02
#define MAX31865_FAULT_CTRL_MASK    0x0C
#define MAX31865_SET_3WIRE          0x10
#define MAX31865_SET_ONE_SHOT       0x20
#define MAX31865_SET_CONV_AUTO      0x40
#define MAX31865_SET_VBIAS_ON       0x80

#define LM7x_CONV_RDY               0x02

#define LM7x_SD70                   0x00
#define LM7x_SD71                   0x01
#define LM7x_SD74                   0x04
#define LM7x_SD121                  0x05
#define LM7x_SD122                  0x06
#define LM7x_SD123                  0x07
#define LM7x_SD124                  0x08
#define LM7x_SD125                  0x09

boolean Plugin_115(byte function, struct EventStruct *event, String &string)
{
  boolean success = false;

  switch (function)
  {
  case PLUGIN_DEVICE_ADD:
  {
    Device[++deviceCount].Number = PLUGIN_ID_115;
    Device[deviceCount].Type = DEVICE_TYPE_SPI;
    Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_SINGLE;
    Device[deviceCount].Ports = 0;
    Device[deviceCount].PullUpOption = false;
    Device[deviceCount].InverseLogicOption = false;
    Device[deviceCount].FormulaOption = true;
    Device[deviceCount].ValueCount = 1;
    Device[deviceCount].SendDataOption = true;
    Device[deviceCount].TimerOption = true;
    Device[deviceCount].GlobalSyncOption = true;
    break;
  }

  case PLUGIN_GET_DEVICENAME:
  {
    string = F(PLUGIN_NAME_115);
    break;
  }

  case PLUGIN_GET_DEVICEVALUENAMES:
  {
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_115));
    break;
  }

  case PLUGIN_GET_DEVICEGPIONAMES:
  {
    event->String1 = formatGpioName_output(F("CS"));
    break;
  }

  case PLUGIN_INIT:
  {
    uint8_t CS_pin_no = Plugin_115_Get_SPI_CS_Pin(event);

    // set the slaveSelectPin as an output:
    pinMode(CS_pin_no, OUTPUT);

    // initialize SPI:
    SPI.setHwCs(false);
    // SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE3);
    SPI.begin();

    if (P115_MAX_TYPE == P115_MAX31865)
    {

      // activate 50Hz filter, clear all faults, no auto conversion, no one shot started
      chooseFilterType(CS_pin_no, 0x01);

      // configure 2/4-wire mode as default
      setConType(CS_pin_no, 0x00);

      // set HighFault Threshold MSB
      write8BitRegister(CS_pin_no, (P115_WRITE_ADDR_BASE + P115_HFT_MSB), 0xFF);

      // set HighFault Threshold LSB
      write8BitRegister(CS_pin_no, (P115_WRITE_ADDR_BASE + P115_HFT_LSB), 0xFF);

      // set LowFault Threshold MSB
      write8BitRegister(CS_pin_no, (P115_WRITE_ADDR_BASE + P115_LFT_MSB), 0x00);

      // set LowFault Threshold LSB
      write8BitRegister(CS_pin_no, (P115_WRITE_ADDR_BASE + P115_LFT_LSB), 0x00);

      // clear all faults
      clearFaults(CS_pin_no);
    }

    if (P115_MAX_TYPE == P115_LM7x)
    {
    }

    addLog(LOG_LEVEL_INFO, F("P115 : MAX31865 Init"));
    success = true;
    break;
  }

  case PLUGIN_WEBFORM_LOAD:
  {
    // FIXME TD-er: Why is this list needed? GPIO selector should provide this info.
#ifdef ESP8266
    addFormNote(F("<b>1st GPIO</b> = CS (Usable GPIOs : 0, 2, 4, 5, 15)"));
#endif // ifdef ESP8266
#ifdef ESP32
    addFormNote(F("<b>1st GPIO</b> = CS (Usable GPIOs : 0, 2, 4, 5, 15..19, 21..23, 25..27, 32, 33)"));
#endif // ifdef ESP32

    // addHtml(F("<TR><TD>Info GPIO:<TD><b>1st GPIO</b> = CS (Usable GPIOs : 0, 2, 4, 5, 15)"));

    addFormSubHeader(F("Device Selection"));

    const byte choice = P115_MAX_TYPE;
    {
      const String TPoptions[2] = {F("MAX 31865"), F("LM7x")};
      const int TPoptionValues[2] = {P115_MAX31865, P115_LM7x};
      addFormSelector(F("Adapter IC"), F("P115_maxtype"), 2, TPoptions, TPoptionValues, choice);
      addFormNote(F("Set used RTD Converter Module. Currently only MAX31865 is fully supported. LM7x derivatives are untested and experimental."));
    }

    if (choice == P115_MAX31865)
    {
      const String PToptions[2] = {F("PT100"), F("PT1000")};
      const int PToptionValues[2] = {MAX31865_PT100, MAX31865_PT1000};
      addFormSelector(F("Resistor Type"), F("P115_rtdtype"), 2, PToptions, PToptionValues, P115_RTD_TYPE);
      addFormNote(F("Set Resistor Type for MAX31865"));

      const String Coptions[2] = {F("2-/4-wire"), F("3-wire")};
      const int CoptionValues[2] = {0, 1};
      addFormSelector(F("Connection Type"), F("P115_contype"), 2, Coptions, CoptionValues, P115_RTD_CON_TYPE);
      addFormNote(F("Set Connection Type for MAX31865"));

      const String FToptions[2] = {F("60 Hz"), F("50 Hz")};
      const int FToptionValues[2] = {0, 1};
      addFormSelector(F("Supply Frequency Filter"), F("P115_filttype"), 2, FToptions, FToptionValues, P115_RTD_FILT_TYPE);
      addFormNote(F("Set FIlter Frequency for Supply Voltage. Choose appropriate to your power net frequency (50/60 Hz)"));

      addFormNumericBox(F("Reference Resistor"), F("P115_res"), P115_RTD_RES, 0);
      addFormNote(F("Set Reference Resistor for MAX31865"));

      addFormFloatNumberBox(F("Offset [K]"), F("P115_offset"), P115_RTD_OFFSET, -50.0, 50.0, 2, 0.01);
      addFormNote(F("Set Offset [K] for MAX31865. Valid values: [-50.0...50.0 K], min. stepsize: [0.01]"));
    }

    if (choice == P115_LM7x)
    {

      const String PToptions[8] = {F("LM70"), F("LM71"), F("LM74"), F("TMP121"), F("TMP122"), F("TMP123"), F("TMP124"), F("TMP125")};
      const int PToptionValues[8] = {LM7x_SD70, LM7x_SD71, LM7x_SD74, LM7x_SD121, LM7x_SD122, LM7x_SD123, LM7x_SD124, LM7x_SD125};
      addFormSelector(F("LM7x device details"), F("P115_rtd_lm_type"), 8, PToptions, PToptionValues, P115_RTD_LM_TYPE);
      addFormNote(F("Choose LM7x device details to allow handling of device specifics,TMP122/124 not yet supported with all options -> fixed 12 Bit resolution, no advanced options active"));

      addFormCheckBox(F("Enable Shutdown Mode"), F("P115_rtd_lm_shtdwn"), P115_RTD_LM_SHTDWN, false);
      addFormNote(F("Enable shutdown mode for LM7x devices. Device is set to shutdown between sample cycles. Useful for very long call cycles, to save power.\n\r Without LM7x device conversion happens in between call cycles. Call Cylces should therefore not become lower than 350ms."));
    }
    success = true;
    break;
  }

  case PLUGIN_WEBFORM_SAVE:
  {
    P115_MAX_TYPE = getFormItemInt(F("P115_maxtype"));
    P115_RTD_TYPE = getFormItemInt(F("P115_rtdtype"));
    P115_RTD_CON_TYPE = getFormItemInt(F("P115_contype"));
    P115_RTD_FILT_TYPE = getFormItemInt(F("P115_filttype"));
    P115_RTD_RES = getFormItemInt(F("P115_res"));
    P115_RTD_OFFSET = getFormItemFloat(F("P115_offset"));
    P115_RTD_LM_TYPE = getFormItemInt(F("P115_rtd_lm_type"));
    P115_RTD_LM_SHTDWN = getFormItemInt(F("P115_rtd_lm_shtdwn"));

    success = true;
    break;
  }

  case PLUGIN_READ:
  {
    // Get the MAX Type (31865)
    byte MaxType = P115_MAX_TYPE;

    float Plugin_115_Celsius = NAN;

    switch (MaxType)
    {
    case P115_MAX31865:
      Plugin_115_Celsius = readMax31865(event);
      break;
    case P115_LM7x:
      Plugin_115_Celsius = readLM7x(event);
      break;
    }

    if (loglevelActiveFor(LOG_LEVEL_INFO))
    {
      String log = F("P115 : Temperature :");
      log += F(" MaxType: ");
      log += String(MaxType, DEC);
      log += F(" Plugin_115_Celsius: ");
      log += String(Plugin_115_Celsius, DEC);
      log += F(" P115_RTD_TYPE: ");
      log += String(P115_RTD_TYPE, DEC);
      log += F(" P115_RTD_RES: ");
      log += String(P115_RTD_RES, DEC);
      addLog(LOG_LEVEL_INFO, log);
    }

    if (Plugin_115_Celsius != NAN)
    {
      UserVar[event->BaseVarIndex] = Plugin_115_Celsius;

      if (loglevelActiveFor(LOG_LEVEL_INFO))
      {
        String log = F("P115 : Temperature ");
        log += String(UserVar[event->BaseVarIndex], DEC);
        log += F(" Valid : ");
        log += formatUserVarNoCheck(event->TaskIndex, 0);
        addLog(LOG_LEVEL_INFO, log);
      }
      success = true;
    }
    else
    {
      UserVar[event->BaseVarIndex] = NAN;
      UserVar[event->BaseVarIndex + 1] = NAN;
      addLog(LOG_LEVEL_INFO, F("P115 : No Sensor attached !"));
      success = false;
    }

    break;
  }
  }
  return success;
}

float readLM7x(struct EventStruct *event)
{

  float temperature = 0;
  uint16_t device_id = 0;
  uint16_t rawValue = 0;

  uint8_t CS_pin_no = Plugin_115_Get_SPI_CS_Pin(event);

  // operate LM7x devices in polling mode, assuming conversion is ready with every call of this read function ( >=210ms call cycle)
  // this allows usage of multiples generations of LM7x devices, that doe not provde conversion ready information in temperature register

  rawValue = readLM7xRegisters(CS_pin_no, P115_RTD_LM_TYPE, P115_RTD_LM_SHTDWN, &device_id);

  temperature = convertLM7xTemp(rawValue, P115_RTD_LM_TYPE);

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log;
    log.reserve(66);
    log = F("P115 : LM7x : readLM7x : ");
    log += F(" rawValue: ");
    log += String(rawValue, DEC);
    log += F(" device_id: 0x");
    log += String(device_id, HEX);
    log += F(" temperature: ");
    log += String(temperature, DEC);
    addLog(LOG_LEVEL_INFO, log);
  }

  return (temperature);
}

float convertLM7xTemp(uint16_t l_rawValue, uint16_t l_LM7xsubtype)
{
  float l_returnValue = 0;
  float l_lsbvalue = 0;
  uint8_t l_noBits = 0;
  int l_intTemperature = 0;

  switch (l_LM7xsubtype)
  {
  case LM7x_SD70:
    l_rawValue >>= 5;
    l_lsbvalue = 0.25;
    l_noBits = 11;
    break;
  case LM7x_SD71:
    l_rawValue >>= 2;
    l_lsbvalue = 0.03125;
    l_noBits = 14;
    break;
  case LM7x_SD74:
    l_rawValue >>= 3;
    l_lsbvalue = 0.0625;
    l_noBits = 13;
    break;
  case LM7x_SD121:
  case LM7x_SD122:
  case LM7x_SD123:
  case LM7x_SD124:
    l_rawValue >>= 4;
    l_lsbvalue = 0.0625;
    l_noBits = 12;
    break;
  case LM7x_SD125:
    l_rawValue >>= 5;
    l_lsbvalue = 0.25;
    l_noBits = 10;
    break;
  default: // use lowest resolution as fallback if no device has been configured
    l_rawValue >>= 5;
    l_lsbvalue = 0.25;
    l_noBits = 11;
    break;
  }

  l_intTemperature = P115_convert_two_complement(l_rawValue, l_noBits);

  l_returnValue = l_intTemperature * l_lsbvalue;

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log;
    log.reserve(66);
    log = F("P115 : LM7x : convertLM7xTemp : ");
    log += F(" l_returnValue: ");
    log += String(l_returnValue, DEC);
    log += F(" l_LM7xsubtype: 0x");
    log += String(l_LM7xsubtype, HEX);
    log += F(" l_rawValue: 0x");
    log += String(l_rawValue, HEX);
    log += F(" l_noBits: ");
    log += String(l_noBits, DEC);
    log += F(" l_lsbvalue: ");
    log += String(l_lsbvalue, DEC);
    addLog(LOG_LEVEL_INFO, log);
  }

  return (l_returnValue);
}

uint16_t readLM7xRegisters(uint8_t l_CS_pin_no, uint8_t l_LM7xsubType, uint8_t l_runMode, uint16_t *l_device_id)
{
  uint16_t l_returnValue = 0;
  uint16_t l_mswaitTime = 0;

  switch (l_LM7xsubType)
  {
  case LM7x_SD70:
  case LM7x_SD71:
  case LM7x_SD74:
    l_mswaitTime = 300;
    break;
  case LM7x_SD121:
  case LM7x_SD122:
  case LM7x_SD123:
  case LM7x_SD124:
    l_mswaitTime = 320;
    break;
  case LM7x_SD125:
    l_mswaitTime = 100;
    break;
  default:
    l_mswaitTime = 500;
    break;
  }

  CS_Delay(); // tCWH (min) = 400ns
  digitalWrite(l_CS_pin_no, LOW);
  CS_Delay(); // tCC (min) = 400ns

  if (l_runMode)
  {
    // shutdown mode active -> conversion when called

    // wakeup device and start conversion
    // initial read of covnersion result is obsolete
    SPI.transfer(0xFF);
    SPI.transfer(0xFF);

    // (wakeup device with "all zero2 message in the last 8 bits
    SPI.transfer(0xFF);
    SPI.transfer(0x00);

    //wait specific ms for conversion to be ready (TI datasheet per devices)
    delay(l_mswaitTime);

    //read temperature value (16 Bit)
    l_returnValue = SPI.transfer(0x00);
    l_returnValue <<= 8;
    l_returnValue = SPI.transfer(0x00);

    // set device to shutdown with "all one" message in the last 8 bits
    SPI.transfer(0xFF);
    SPI.transfer(0xFF);

    // read Manufatures/Device ID (16 Bit)
    *(l_device_id) = SPI.transfer(0x00);
    *(l_device_id) <<= 8;
    *(l_device_id) = SPI.transfer(0x00);

    // set device to shutdown with "all one" message in the last 8 bits ( maybe redundant, check with test)
    SPI.transfer(0xFF);
    SPI.transfer(0xFF);
  }
  else
  {
    // shutdown mode inactive -> normal background conversion during call cycle

    l_returnValue = SPI.transfer(0x00); //read temperature value (16 Bit)
    l_returnValue <<= 8;
    l_returnValue = SPI.transfer(0x00);

    // set device to shutdown
    SPI.transfer(0xFF);

    // read Manufatures/Device ID (16 Bit)
    *(l_device_id) = SPI.transfer(0x00);
    *(l_device_id) <<= 8;
    *(l_device_id) = SPI.transfer(0x00);

    // start conversion until next read  (8 Bit sufficient)
    // 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F allowed - else device goe to test mode (not desirable here)
    SPI.transfer(0x00);
    // SPI.transfer (0x00);
  }

  CS_Delay(); // tCCH (min) = 100ns
  digitalWrite(l_CS_pin_no, HIGH);
  CS_Delay(); // tCWH (min) = 400ns

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log;
    log.reserve(66);
    log = F("P115 : LM7x : readLM7xRegisters : ");
    log += F(" l_returnValue: 0x");
    log += String(l_returnValue, HEX);
    log += F(" l_device_id: 0x");
    log += String(*(l_device_id), HEX);
    addLog(LOG_LEVEL_INFO, log);
  }

  return (l_returnValue);
}

float readMax31865(struct EventStruct *event)
{

  uint8_t registers[P115_NO_REG] = {0};
  uint16_t rawValue = 0;

  uint8_t CS_pin_no = Plugin_115_Get_SPI_CS_Pin(event);

  // clear all faults
  clearFaults(CS_pin_no);

  // set frequency filter
  chooseFilterType(CS_pin_no, P115_RTD_FILT_TYPE);

  //activate BIAS short before read, to reduce power consumption
  handleBias(CS_pin_no, true);

  // wait for external capacities to load
  delay(15);

  // configure read access with configuration from web interface
  setConType(CS_pin_no, P115_RTD_CON_TYPE);

  //activate one shot conversion
  startOneShotConversion(CS_pin_no);

  // wait for 100ms -> conversion to be ready
  delay(100);

  uint32_t breakValue = 0;
  // wait for conversion to be ready

  // while((read8BitRegister(CS_pin_no, (P115_READ_ADDR_BASE + P115_CONFIG)) & MAX31865_SET_ONE_SHOT)) {
  //   breakValue++;
  //   if (breakValue >= MAX31865_CONVERSION_BREAK)
  //     break;
  // }

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log;
    log.reserve(66);

    log = F("P115 : MAX31865 : ");
    log += F("breakValue: ");
    log += String(breakValue, DEC);

    addLog(LOG_LEVEL_INFO, log);
  }
  // delay(70);

  // read conversion result
  rawValue = read16BitRegister(CS_pin_no, (P115_READ_ADDR_BASE + P115_RTD_MSB));

  //deactivate BIAS short after read, to reduce power consumption
  handleBias(CS_pin_no, false);

  registers[P115_FAULT] = read8BitRegister(CS_pin_no, (P115_READ_ADDR_BASE + P115_FAULT));

  //# ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log;
    log.reserve(66);

    for (int i = 0; i < P115_NO_REG; ++i)
    {
      registers[i] = read8BitRegister(CS_pin_no, (P115_READ_ADDR_BASE + i));
    }

    log = F("P115 : MAX31865 :");

    for (int i = 0; i < P115_NO_REG; ++i)
    {
      log += F(" 0x");
      log += String(registers[i], HEX);
    }

    addLog(LOG_LEVEL_INFO, log);
  }
  //# endif // ifndef BUILD_NO_DEBUG

  // clear all faults
  clearFaults(CS_pin_no);

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    if (registers[P115_FAULT])
    {
      String log = F("P115 : MAX31865 : ");

      log += F("Fault : 0x");
      log += String(registers[P115_FAULT], HEX);
      log += F(" :");

      if (registers[P115_FAULT] & MAX31865_FAULT_OVUV)
      {
        log += F(" Under/Over voltage");
      }

      if (registers[P115_FAULT] & MAX31865_FAULT_RTDINLOW)
      {
        log += F(" RTDIN- < 0.85 x Bias - FORCE- open");
      }

      if (registers[P115_FAULT] & MAX31865_FAULT_REFINHIGH)
      {
        log += F(" REFIN- < 0.85 x Bias - FORCE- open");
      }

      if (registers[P115_FAULT] & MAX31865_FAULT_REFINLOW)
      {
        log += F(" REFIN- > 0.85 x Bias");
      }

      if (registers[P115_FAULT] & MAX31865_FAULT_LOWTHRESH)
      {
        log += F(" RTD Low Threshold");
      }

      if (registers[P115_FAULT] & MAX31865_FAULT_HIGHTHRESH)
      {
        log += F(" RTD High Threshold");
      }
      addLog(LOG_LEVEL_INFO, log);
    }
  }
  bool ValueValid = false;

  if (registers[P115_FAULT] == 0x00)
    ValueValid = true;

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log = F("P115 : Temperature :");
    log += F(" registers[P115_FAULT]: ");
    log += String(registers[P115_FAULT], HEX);
    log += F(" ValueValid: ");
    log += String(ValueValid, BIN);
    addLog(LOG_LEVEL_INFO, log);
  }

  if (ValueValid)
  {
    rawValue >>= 1; // bottom fault bits is unused
    float temperature = Plugin_115_convert_to_temperature(rawValue, getNomResistor(P115_RTD_TYPE), P115_RTD_RES);

    if (loglevelActiveFor(LOG_LEVEL_INFO))
    {
      String log = F("P115 : Temperature :");
      log += F(" rawValue: ");
      log += String(rawValue, DEC);
      log += F(" temperature: ");
      log += String(temperature, DEC);
      log += F(" P115_RTD_TYPE: ");
      log += String(P115_RTD_TYPE, DEC);
      log += F(" P115_RTD_RES: ");
      log += String(P115_RTD_RES, DEC);
      addLog(LOG_LEVEL_INFO, log);
    }
    //TODO CK: add offset handling from configuration webpage
    temperature += P115_RTD_OFFSET;

    // Calculate Celsius
    return temperature;
  }
  else
  {
    // Fault state, thus output no value.
    return NAN;
  }
}

void clearFaults(uint8_t l_CS_pin_no)
{
  uint8_t l_reg = 0;

  // read in config register
  l_reg = read8BitRegister(l_CS_pin_no, (P115_READ_ADDR_BASE + P115_CONFIG));

  // clear all faults ( write "0" to D2, D3, D5; write "1" to D2)
  l_reg &= ~(MAX31865_SET_ONE_SHOT | MAX31865_FAULT_CTRL_MASK);
  l_reg |= MAX31865_CLEAR_FAULTS;

  // write configuration to MAX31865 to enable VBIAS
  write8BitRegister(l_CS_pin_no, (P115_WRITE_ADDR_BASE + P115_CONFIG), l_reg);
}

void handleBias(uint8_t l_CS_pin_no, bool l_active)
{
  uint8_t l_reg = 0;

  // read in config register
  l_reg = read8BitRegister(l_CS_pin_no, (P115_READ_ADDR_BASE + P115_CONFIG));

  if (l_active)
  {
    // activate BIAS
    l_reg |= MAX31865_SET_VBIAS_ON;
  }
  else
  {
    // deactivate BIAS
    l_reg &= ~MAX31865_SET_VBIAS_ON;
  }
  // write configuration to MAX31865 to enable VBIAS
  write8BitRegister(l_CS_pin_no, (P115_WRITE_ADDR_BASE + P115_CONFIG), l_reg);
}

void chooseFilterType(uint8_t l_CS_pin_no, uint8_t l_filtType)
{
  uint8_t l_reg = 0;

  // read in config register
  l_reg = read8BitRegister(l_CS_pin_no, (P115_READ_ADDR_BASE + P115_CONFIG));

  // configure access to sensor (2/4-wire OR 3 wire)
  switch (l_filtType)
  {
  case 0:
    l_reg &= ~MAX31865_SET_50HZ;
    break;
  case 1:
    l_reg |= MAX31865_SET_50HZ;
    break;
  default:
    l_reg &= ~MAX31865_SET_50HZ;
    break;
  }

  // write to configuration register
  write8BitRegister(l_CS_pin_no, (P115_WRITE_ADDR_BASE + P115_CONFIG), l_reg);
}

void setConType(uint8_t l_CS_pin_no, uint8_t l_conType)
{
  uint8_t l_reg = 0;

  // read in config register
  l_reg = read8BitRegister(l_CS_pin_no, (P115_READ_ADDR_BASE + P115_CONFIG));

  // configure access to sensor (2/4-wire OR 3 wire)
  switch (l_conType)
  {
  case 0:
    l_reg &= ~MAX31865_SET_3WIRE;
    break;
  case 1:
    l_reg |= MAX31865_SET_3WIRE;
    break;
  default:
    l_reg &= ~MAX31865_SET_3WIRE;
    break;
  }

  // write to configuration register
  write8BitRegister(l_CS_pin_no, (P115_WRITE_ADDR_BASE + P115_CONFIG), l_reg);
}

void startOneShotConversion(uint8_t l_CS_pin_no)
{
  uint8_t l_reg = 0;

  // read in config register
  l_reg = read8BitRegister(l_CS_pin_no, (P115_READ_ADDR_BASE + P115_CONFIG));

  //activate one shot conversion
  l_reg |= MAX31865_SET_ONE_SHOT;

  // write to configuration register
  write8BitRegister(l_CS_pin_no, (P115_WRITE_ADDR_BASE + P115_CONFIG), l_reg);
}

/**************************************************************************/
/*!
    @brief Read the temperature in C from the RTD through calculation of the
    resistance. Uses
   http://www.analog.com/media/en/technical-documentation/application-notes/AN709_0.pdf
   technique
    @param RTDnominal The 'nominal' resistance of the RTD sensor, usually 100
    or 1000
    @param refResistor The value of the matching reference resistor, usually
    430 or 4300
    @returns Temperature in C
*/
/**************************************************************************/
float Plugin_115_convert_to_temperature(uint32_t l_rawvalue, float RTDnominal, float refResistor)
{

#define RTD_A 3.9083e-3
#define RTD_B -5.775e-7

  float Z1, Z2, Z3, Z4, Rt, temp;

  Rt = l_rawvalue;
  Rt /= 32768;
  Rt *= refResistor;

  Z1 = -RTD_A;
  Z2 = RTD_A * RTD_A - (4 * RTD_B);
  Z3 = (4 * RTD_B) / RTDnominal;
  Z4 = 2 * RTD_B;

  temp = Z2 + (Z3 * Rt);
  temp = (sqrt(temp) + Z1) / Z4;

  if (temp >= 0)
    return temp;

  Rt /= RTDnominal;
  Rt *= 100; // normalize to 100 ohm

  float rpoly = Rt;

  temp = -242.02;
  temp += 2.2228 * rpoly;
  rpoly *= Rt; // square
  temp += 2.5859e-3 * rpoly;
  rpoly *= Rt; // ^3
  temp -= 4.8260e-6 * rpoly;
  rpoly *= Rt; // ^4
  temp -= 2.8183e-8 * rpoly;
  rpoly *= Rt; // ^5
  temp += 1.5243e-10 * rpoly;

  return temp;
}

uint16_t getNomResistor(uint8_t l_RType)
{
  uint16_t l_returnValue = 100;

  switch (l_RType)
  {
  case MAX31865_PT100:
    l_returnValue = 100;
    break;
  case MAX31865_PT1000:
    l_returnValue = 1000;
    break;
  default:
    l_returnValue = 100;
    break;
  }
  return (l_returnValue);
}

int Plugin_115_Get_SPI_CS_Pin(struct EventStruct *event)
{
  // If no Pin is in Config we use 15 as default -> Hardware Chip Select on ESP8266
  if (CONFIG_PIN1 != 0)
  {
    return CONFIG_PIN1;
  }
  return 15; // D8
}

void write8BitRegister(uint8_t l_CS_pin_no, uint8_t l_address, uint8_t value)
{
  CS_Delay(); // tCWH (min) = 400ns
  digitalWrite(l_CS_pin_no, LOW);
  CS_Delay(); // tCC (min) = 400ns

  SPI.transfer(l_address);
  SPI.transfer(value);

  CS_Delay(); // tCCH (min) = 100ns
  digitalWrite(l_CS_pin_no, HIGH);
  CS_Delay(); // tCWH (min) = 400ns

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log;
    log.reserve(66);
    log = F("P115 : MAX31865 : write8BitRegister : ");
    log += F("l_address: 0x");
    log += String(l_address, HEX);
    log += F(" value: 0x");
    log += String(value, HEX);
    addLog(LOG_LEVEL_INFO, log);
  }
}

void write16BitRegisters(uint8_t l_CS_pin_no, uint8_t l_address, uint16_t value)
{
  CS_Delay(); // tCWH (min) = 400ns
  digitalWrite(l_CS_pin_no, LOW);
  CS_Delay(); // tCC (min) = 400ns

  SPI.transfer(l_address);
  SPI.transfer16(value);

  CS_Delay(); // tCCH (min) = 100ns
  digitalWrite(l_CS_pin_no, HIGH);
  CS_Delay(); // tCWH (min) = 400ns

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log;
    log.reserve(66);
    log = F("P115 : MAX31865 : write16BitRegister : ");
    log += F("l_address: 0x");
    log += String(l_address, HEX);
    log += F(" value: 0x");
    log += String(value, HEX);
    addLog(LOG_LEVEL_INFO, log);
  }
}

uint8_t read8BitRegister(uint8_t l_CS_pin_no, uint8_t l_address)
{
  uint8_t l_returnValue = 0;

  CS_Delay(); // tCWH (min) = 400ns
  digitalWrite(l_CS_pin_no, LOW);
  CS_Delay(); // tCC (min) = 400ns

  SPI.transfer(l_address);
  l_returnValue = SPI.transfer(0x00);

  CS_Delay(); // tCCH (min) = 100ns
  digitalWrite(l_CS_pin_no, HIGH);
  CS_Delay(); // tCWH (min) = 400ns

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log;
    log.reserve(66);
    log = F("P115 : MAX31865 : read8BitRegister : ");
    log += F("l_address: 0x");
    log += String(l_address, HEX);
    log += F(" l_returnValue: 0x");
    log += String(l_returnValue, HEX);
    addLog(LOG_LEVEL_INFO, log);
  }

  return (l_returnValue);
}

uint16_t read16BitRegister(uint8_t l_CS_pin_no, uint8_t l_address)
{
  uint16_t l_rawValue = 0;

  CS_Delay();
  digitalWrite(l_CS_pin_no, LOW);
  CS_Delay();

  SPI.transfer(l_address);
  l_rawValue = SPI.transfer(0x00);
  l_rawValue <<= 8;
  l_rawValue |= SPI.transfer(0x00);

  CS_Delay();
  digitalWrite(l_CS_pin_no, HIGH);
  CS_Delay();

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log;
    log.reserve(66);
    log = F("P115 : MAX31865 : read16BitRegister : ");
    log += F("l_address: 0x");
    log += String(l_address, HEX);
    log += F(" l_rawValue: 0x");
    log += String(l_rawValue, HEX);
    addLog(LOG_LEVEL_INFO, log);
  }

  return l_rawValue;
}

int P115_convert_two_complement(uint16_t value, uint8_t nr_bits)
{
  const bool negative = (value & (1 << (nr_bits - 1))) != 0;
  int l_returnValue = 0;

  if (negative)
  {
    // Add zeroes to the left to create the proper negative native-sized integer.
    l_returnValue = value | ~((1 << nr_bits) - 1);
  }
  else
  {
    l_returnValue = value;
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log;
    log.reserve(66);
    log = F("P115 : LM7x : P115_convert_two_complement : ");
    log += F(" l_returnValue: ");
    log += String(l_returnValue, DEC);
    log += F(" value: 0x");
    log += String(value, HEX);
    log += F(" nr_bits: ");
    log += String(nr_bits, DEC);
    log += F(" negative: ");
    log += String(negative, BIN);
    addLog(LOG_LEVEL_INFO, log);
  }

  return (l_returnValue);
}

#endif // USES_P115