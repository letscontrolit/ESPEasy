#include "../PluginStructs/P039_data_struct.h"

#ifdef USES_P039

P039_data_struct::P039_data_struct(struct EventStruct*event) {
  # ifdef ESP32
  const uint8_t spi_bus = Settings.getSPIBusForTask(event->TaskIndex);
  _spi = 0 == spi_bus ? SPI : SPIe;
  # endif // ifdef ESP32
}

bool P039_data_struct::begin(struct EventStruct*event) {
  bool   success   = false;
  int8_t CS_pin_no = get_SPI_CS_Pin(event);

  // set the slaveSelectPin as an output:
  init_SPI_CS_Pin(CS_pin_no);

  // ensure MODE3 access to SPI device
  _spi.setDataMode(SPI_MODE3);

  if (P039_MAX_TYPE == P039_MAX31855) {
    sensorFault = false;
  }


  if (P039_MAX_TYPE == P039_MAX31856) {
    // init string - content accoring to inital implementation of P039 - MAX31856 read function
    // write to Adress 0x80
    // activate 50Hz filter in CR0, choose averaging and TC type from configuration in CR1, activate OV/UV/OC faults, write defaults to
    // CJHF, CJLF, LTHFTH, LTHFTL, LTLFTH, LTLFTL, CJTO
    uint8_t sendBuffer[11] =
    { 0x80, static_cast<uint8_t>(P039_RTD_FILT_TYPE), static_cast<uint8_t>((P039_CONFIG_4 << 4) | P039_TC_TYPE), 0xFC, 0x7F, 0xC0, 0x7F,
      0xFF, 0x80,                                     0x00,                                                      0x00 };

    transfer_n_ByteSPI(CS_pin_no, 11, &sendBuffer[0]);

    sensorFault = false;

    // start on shot conversion for upcoming read cycle
    change8BitRegister(CS_pin_no,
                       (MAX31856_READ_ADDR_BASE + MAX31856_CR0),
                       (MAX31856_WRITE_ADDR_BASE + MAX31856_CR0),
                       MAX31856_SET_ONE_SHOT,
                       P039_SET);
  }


  if (P039_MAX_TYPE == P039_MAX31865) {
    // two step initialization buffer
    uint8_t initSendBufferHFTH[3] = { (MAX31865_WRITE_ADDR_BASE + MAX31865_HFT_MSB), 0xFF, 0xFF };
    uint8_t initSendBufferLFTH[3] = { (MAX31865_WRITE_ADDR_BASE + MAX31865_HFT_MSB), 0xFF, 0xFF };

    // write intially 0x00 to CONFIG register
    write8BitRegister(CS_pin_no, (MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG), 0x00u);

    // activate 50Hz filter, clear all faults, no auto conversion, no conversion started
    change8BitRegister(CS_pin_no,
                       MAX31865_RD_ADDRESS(MAX31865_CONFIG),
                       MAX31865_WR_ADDRESS(MAX31865_CONFIG),
                       MAX31865_SET_50HZ,
                       static_cast<bool>(P039_RTD_FILT_TYPE));

    // configure 2/4-wire sensor connection as default
    MAX31865_setConType(CS_pin_no, P039_CONFIG_4);

    // set HighFault Threshold
    transfer_n_ByteSPI(CS_pin_no, 3, &initSendBufferHFTH[0]);

    // set LowFault Threshold
    transfer_n_ByteSPI(CS_pin_no, 3, &initSendBufferLFTH[0]);

    // clear all faults
    MAX31865_clearFaults(CS_pin_no);

    // activate BIAS short before read, to reduce power consumption
    change8BitRegister(CS_pin_no,
                       (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG),
                       (MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG),
                       MAX31865_SET_VBIAS_ON,
                       P039_SET);

    // save current timer for next calculation
    timer = millis();

    // start time to follow up on BIAS activation before starting the conversion
    // and start conversion sequence via TIMER API

    Scheduler.setPluginTaskTimer(MAX31865_BIAS_WAIT_TIME, event->TaskIndex, MAX31865_BIAS_ON_STATE);
  }

  /*
        if (P039_MAX_TYPE == P039_LM7x)
        {
          // TODO: c.k.i.: more detailed inits depending on the sub devices expected , e.g. TMP 122/124
        }
   */
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("P039 : %s : SPI Init - DONE"), getTaskDeviceName(event->TaskIndex).c_str()));
  }
  # endif // ifndef BUILD_NO_DEBUG

  return success;
}

bool P039_data_struct::read(EventStruct *event) {
  bool success = false;

  // Get the MAX Type (6675 / 31855 / 31856)
  uint8_t MaxType = P039_MAX_TYPE;

  float Plugin_039_Celsius = NAN;

  switch (MaxType) {
    case P039_MAX6675:
      Plugin_039_Celsius = readMax6675(event);
      break;
    case P039_MAX31855:
      Plugin_039_Celsius = readMax31855(event);
      break;
    case P039_MAX31856:
      Plugin_039_Celsius = readMax31856(event);
      break;
    case P039_MAX31865:
      Plugin_039_Celsius = readMax31865(event);
      break;
    case P039_LM7x:
      Plugin_039_Celsius = readLM7x(event);
      break;
  }

  if (isValidFloat(Plugin_039_Celsius))
  {
    UserVar.setFloat(event->TaskIndex, 0, Plugin_039_Celsius);

    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = strformat(F("P039 : %s :"), getTaskDeviceName(event->TaskIndex).c_str());

      const uint8_t valueCount = getValueCountForTask(event->TaskIndex);

      for (uint8_t i = 0; i < valueCount; ++i)
      {
        log += strformat(
          F(" %s: %s"),
          Cache.getTaskDeviceValueName(event->TaskIndex, i).c_str(),
          formatUserVarNoCheck(event, i).c_str());
      }
      addLogMove(LOG_LEVEL_INFO, log);
    }
    # endif // ifndef BUILD_NO_DEBUG

    if (definitelyGreaterThan(Plugin_039_Celsius, P039_TEMP_THRESHOLD)) {
      success = true;
    }
  }
  else
  {
    UserVar.setFloat(event->TaskIndex, 0, NAN);
    UserVar.setFloat(event->TaskIndex, 1, NAN);

    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      addLog(LOG_LEVEL_ERROR, strformat(F("P039 : %s : No Sensor attached!"), getTaskDeviceName(event->TaskIndex).c_str()));
    }
    success = false;
  }
  return success;
}

bool P039_data_struct::plugin_tasktimer_in(EventStruct *event) {
  bool   success   = false;
  int8_t CS_pin_no = get_SPI_CS_Pin(event);

  // Get the MAX Type (6675 / 31855 / 31856)
  uint8_t MaxType = P039_MAX_TYPE;

  switch (MaxType)
  {
    case P039_MAX31865:
    {
      switch (event->Par1)
      {
        case MAX31865_BIAS_ON_STATE:
        {
          # ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            addLog(LOG_LEVEL_DEBUG, strformat(
                     F("P039 : %s : current state: MAX31865_BIAS_ON_STATE; delta: %d ms"),
                     getTaskDeviceName(event->TaskIndex).c_str(),
                     timePassedSince(timer))); // calc delta since last call
          }
          # endif // ifndef BUILD_NO_DEBUG

          // save current timer for next calculation
          timer = millis();

          // activate one shot conversion
          change8BitRegister(CS_pin_no,
                             (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG),
                             (MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG),
                             MAX31865_SET_ONE_SHOT,
                             P039_SET);

          // set next state in sequence -> READ STATE
          // start time to follow up on conversion and read the conversion result
          convReady = false;
          Scheduler.setPluginTaskTimer(MAX31865_CONVERSION_TIME, event->TaskIndex, MAX31865_RD_STATE);

          # ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            addLog(LOG_LEVEL_DEBUG, strformat(
                     F("P039 : %s : Next State: %d"),
                     getTaskDeviceName(event->TaskIndex).c_str(),
                     event->Par1));
          }
          # endif // ifndef BUILD_NO_DEBUG

          break;
        }
        case MAX31865_RD_STATE:
        {
          # ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            addLog(LOG_LEVEL_DEBUG, strformat(
                     F("P039 : %s : current state: MAX31865_RD_STATE; delta: %d ms"),
                     getTaskDeviceName(event->TaskIndex).c_str(),
                     timePassedSince(timer))); // calc delta since last call
          }
          # endif // ifndef BUILD_NO_DEBUG

          // save current timer for next calculation
          timer = millis();

          // read conversion result
          conversionResult = read16BitRegister(CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_RTD_MSB));

          // deactivate BIAS short after read, to reduce power consumption
          change8BitRegister(CS_pin_no,
                             (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG),
                             (MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG),
                             MAX31865_SET_VBIAS_ON,
                             P039_RESET);

          // read fault register to get a full picture
          deviceFaults = read8BitRegister(CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_FAULT));

          // mark conversion as ready
          convReady = true;

          # ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            addLog(LOG_LEVEL_DEBUG,
                   strformat(F("P039 : %s : conversionResult: %s; deviceFaults: %s; Next State: %d"),
                             getTaskDeviceName(event->TaskIndex).c_str(),
                             formatToHex_decimal(conversionResult).c_str(),
                             formatToHex_decimal(deviceFaults).c_str(),
                             event->Par1));
          }
          # endif // ifndef BUILD_NO_DEBUG


          break;
        }
        case MAX31865_INIT_STATE:
        default:
        {
          // clear all faults
          MAX31865_clearFaults(CS_pin_no);

          // activate BIAS short before read, to reduce power consumption
          change8BitRegister(CS_pin_no,
                             (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG),
                             (MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG),
                             MAX31865_SET_VBIAS_ON,
                             P039_SET);


          # ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            addLog(LOG_LEVEL_DEBUG, strformat(F("P039 : %s : current state: MAX31865_INIT_STATE, "
                                                "default; next state: MAX31865_BIAS_ON_STATE"),
                                              getTaskDeviceName(event->TaskIndex).c_str()));

            // FIXME: Shouldn't this be in active code? // save current timer for next calculation
            timer = millis();
          }
          # endif // ifndef BUILD_NO_DEBUG

          // start time to follow up on BIAS activation before starting the conversion
          // and start conversion sequence via TIMER API
          // set next state in sequence -> BIAS ON STATE

          Scheduler.setPluginTaskTimer(MAX31865_BIAS_WAIT_TIME, event->TaskIndex, MAX31865_BIAS_ON_STATE);


          break;
        }
      }
      break;
    }
    default:
    {
      break;
    }
  }
  return success;
}

void P039_data_struct::AddMainsFrequencyFilterSelection(struct EventStruct *event)
{
  const __FlashStringHelper *FToptions[] = { F("60"), F("50") };
  const int FToptionValues[]             = { 0, 1 };

  const FormSelectorOptions selector(NR_ELEMENTS(FToptions), FToptions, FToptionValues);

  selector.addFormSelector(F("Supply Frequency Filter"), F("filttype"), P039_RTD_FILT_TYPE);
  addUnit(F("Hz"));
  # ifndef LIMIT_BUILD_SIZE
  addFormNote(F("Filter power net frequency (50/60 Hz)"));
  # else // ifndef LIMIT_BUILD_SIZE
  addUnit(F("net frequency"));
  # endif // ifndef LIMIT_BUILD_SIZE
}

float P039_data_struct::readMax6675(struct EventStruct *event)
{
  int8_t CS_pin_no = get_SPI_CS_Pin(event);

  uint8_t  messageBuffer[2] = { 0 };
  uint16_t rawvalue         = 0u;


  // "transfer" 2 bytes to SPI to get 16 Bit return value
  transfer_n_ByteSPI(CS_pin_no, 2, &messageBuffer[0]);

  // merge 16Bit return value from messageBuffer
  rawvalue = ((messageBuffer[0] << 8) | messageBuffer[1]);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG))
  {
    addLog(LOG_LEVEL_DEBUG, strformat(F("P039 : MAX6675 : RAW - BIN: %s HEX: %s DEC: %d MSB: %s LSB: %s"),
                                      String(rawvalue, BIN).c_str(),
                                      formatToHex(rawvalue).c_str(),
                                      rawvalue,
                                      formatToHex_decimal(messageBuffer[0]).c_str(),
                                      formatToHex_decimal(messageBuffer[1]).c_str()));
  }

  # endif // ifndef BUILD_NO_DEBUG

  // Open Thermocouple
  // Bit D2 is normally low and goes high if the thermocouple input is open. In order to allow the operation of the
  // open  thermocouple  detector,  T-  must  be  grounded. Make  the  ground  connection  as  close  to  the  GND  pin
  // as possible.
  // 2021-05-11: FIXED: c.k.i.: OC Flag already checked; migrated to #define for improved maintenance
  const bool Plugin_039_SensorAttached = !(rawvalue & MAX6675_TC_OC);

  if (Plugin_039_SensorAttached)
  {
    // shift RAW value 3 Bits to the right to get the data
    rawvalue >>= 3;

    // calculate Celsius with device resolution 0.25 K/bit
    return rawvalue * 0.25f;
  }
  else
  {
    return NAN;
  }
}

float P039_data_struct::readMax31855(struct EventStruct *event)
{
  uint8_t messageBuffer[4] = { 0 };

  int8_t CS_pin_no = get_SPI_CS_Pin(event);

  // "transfer" 0x0 and read the 32 Bit conversion register from the Chip
  transfer_n_ByteSPI(CS_pin_no, 4, &messageBuffer[0]);

  // merge rawvalue from 4 bytes of messageBuffer
  uint32_t rawvalue =
    ((static_cast<uint32_t>(messageBuffer[0]) <<
      24) |
     (static_cast<uint32_t>(messageBuffer[1]) <<
      16) | (static_cast<uint32_t>(messageBuffer[2]) << 8) | static_cast<uint32_t>(messageBuffer[3]));


  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG))
  {
    String log;

    if (log.reserve(200u)) { // reserve value derived from example log file
      log = strformat(F("P039 : MAX31855 : RAW - BIN: %s rawvalue,HEX: %s rawvalue,DEC: %d messageBuffer[],HEX:"),
                      String(rawvalue, BIN).c_str(),
                      formatToHex(rawvalue).c_str(),
                      rawvalue);

      for (size_t i = 0u; i < 4; ++i)
      {
        log += ' ';                                   // 1 char
        log += formatToHex_decimal(messageBuffer[i]); // 9 char
      }
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
  }

  # endif // ifndef BUILD_NO_DEBUG

  // check for fault flags in LSB of 32 Bit messageBuffer
  if (sensorFault != ((rawvalue & (MAX31855_TC_SCVCC | MAX31855_TC_SC | MAX31855_TC_OC)) == 0)) {
    // Fault code changed, log them
    sensorFault = ((rawvalue & (MAX31855_TC_SCVCC | MAX31855_TC_SC | MAX31855_TC_OC)) == 0);

    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
    {
      String log;

      if (log.reserve(120u)) { // reserve value derived from example log file
        log = F("P039 : MAX31855 : ");

        if (sensorFault) {
          log += F("Fault resolved");
        } else {
          log += F("Fault code :");

          if (rawvalue & MAX31855_TC_OC) {
            log += F(" Open (no connection)");
          }

          if (rawvalue & MAX31855_TC_SC) {
            log += F(" Short-circuit to GND");
          }

          if (rawvalue & MAX31855_TC_SCVCC) {
            log += F(" Short-circuit to Vcc");
          }
        }
        addLogMove(LOG_LEVEL_DEBUG_MORE, log);
      }
    }
    # endif // ifndef BUILD_NO_DEBUG

    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG))
    {
      addLog(LOG_LEVEL_DEBUG, strformat(F("P039 : MAX31855 : rawvalue: %s sensorFault: "),
                                        formatToHex_decimal(rawvalue).c_str(),
                                        formatToHex_decimal(sensorFault).c_str()));
    }

    # endif // ifndef BUILD_NO_DEBUG
  }

  // D16 - This bit reads at 1 when any of the SCV, SCG, or OC faults are active. Default value is 0.
  // 2020-05-11: FIXED: c.k.i.: migrated plain flag mask to #defines to enhance maintainability; added all fault flags for safety reasons
  const bool Plugin_039_SensorAttached = !(rawvalue & (MAX31855_TC_GENFLT | MAX31855_TC_SCVCC | MAX31855_TC_SC | MAX31855_TC_OC));

  if (Plugin_039_SensorAttached)
  {
    // Data is D[31:18]
    // Shift RAW value 18 Bits to the right to get the data
    rawvalue >>= 18;

    // Check for negative Values
    //  +25.00    0000 0001 1001 00
    //    0.00    0000 0000 0000 00
    //   -0.25    1111 1111 1111 11
    //   -1.00    1111 1111 1111 00
    // -250.00    1111 0000 0110 00
    // We're left with (32 - 18 =) 14 bits
    int temperature = convert_two_complement(rawvalue, 14);

    // Calculate Celsius
    return temperature * 0.25f;
  }
  else
  {
    // Fault state, thus output no value.
    return NAN;
  }
}

float P039_data_struct::readMax31856(struct EventStruct *event)
{
  int8_t CS_pin_no = get_SPI_CS_Pin(event);


  uint8_t registers[MAX31856_NO_REG]         = { 0 };
  uint8_t messageBuffer[MAX31856_NO_REG + 1] = { 0 };

  messageBuffer[0] = MAX31856_READ_ADDR_BASE;

  // "transfer" 0x0 starting at address 0x00 and read the all registers from the Chip
  transfer_n_ByteSPI(CS_pin_no, (MAX31856_NO_REG + 1), &messageBuffer[0]);

  // transfer data from messageBuffer and get rid of initial address uint8_t
  for (uint8_t i = 0u; i < MAX31856_NO_REG; ++i) {
    registers[i] = messageBuffer[i + 1];
  }

  // configure device for next conversion
  // activate frequency filter according to configuration
  change8BitRegister(CS_pin_no,
                     (MAX31856_READ_ADDR_BASE + MAX31856_CR0),
                     (MAX31856_WRITE_ADDR_BASE + MAX31856_CR0),
                     MAX31856_SET_50HZ,
                     static_cast<bool>(P039_RTD_FILT_TYPE));

  // set averaging and TC type
  write8BitRegister(CS_pin_no, (MAX31856_WRITE_ADDR_BASE + MAX31856_CR1), static_cast<uint8_t>((P039_CONFIG_4 << 4) | P039_TC_TYPE));


  // start on shot conversion for next read cycle
  change8BitRegister(CS_pin_no,
                     (MAX31856_READ_ADDR_BASE + MAX31856_CR0),
                     (MAX31856_WRITE_ADDR_BASE + MAX31856_CR0),
                     MAX31856_SET_ONE_SHOT,
                     P039_SET);


  // now derive raw value from respective registers
  uint32_t rawvalue = static_cast<uint32_t>(registers[MAX31856_LTCBH]);

  rawvalue = (rawvalue << 8) | static_cast<uint32_t>(registers[MAX31856_LTCBM]);
  rawvalue = (rawvalue << 8) | static_cast<uint32_t>(registers[MAX31856_LTCBL]);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG))
  {
    String log;

    if (log.reserve(210u)) { // reserve value derived from example log file
      log = F("P039 : MAX31856 :");

      for (uint8_t i = 0; i < MAX31856_NO_REG; ++i) {
        log += ' ';
        log += formatToHex_decimal(registers[i]);
      }
      log += F(" rawvalue: ");
      log += formatToHex_decimal(rawvalue);
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
  }

  # endif // ifndef BUILD_NO_DEBUG


  // ignore TC Range Bit in case Voltage Modes are used
  // datasheet:
  // Thermocouple Out-of-Range fault.
  //    0 = The Thermocouple Hot Junction temperature is within the normal operating range (see Table 1).
  //    1 = The Thermocouple Hot Junction temperature is outside of the normal operating range.
  //  Note: The TC Range bit should be ignored in voltage mode.
  uint8_t sr = registers[MAX31856_SR];

  if ((8u == P039_TC_TYPE) || (12u == P039_TC_TYPE)) {
    sr &= ~MAX31856_TC_TCRANGE;
  }


  // FIXED: c.k.i. : moved static fault flag to instance data structure

  sensorFault = (sr != 0); // Set new state

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
  {
    // FIXME TD-er: Part of expression is always false (sr == 0)
    const bool faultResolved = sensorFault && (sr == 0);

    if (sensorFault || faultResolved) {
      String log;

      if (log.reserve(140u)) { // reserve value derived from example log file
        log = F("P039 : MAX31856 : ");

        if (!sensorFault) {
          log += F("Fault resolved");
        } else {
          log += F("Fault :");

          if (sr & MAX31856_TC_OC) {
            log += F(" Open (no connection)");
          }

          if (sr & MAX31856_TC_OVUV) {
            log += F(" Over/Under Voltage");
          }

          if (sr & MAX31856_TC_TCLOW) {
            log += F(" TC Low");
          }

          if (sr & MAX31856_TC_TCLHIGH) {
            log += F(" TC High");
          }

          if (sr & MAX31856_TC_CJLOW) {
            log += F(" CJ Low");
          }

          if (sr & MAX31856_TC_CJHIGH) {
            log += F(" CJ High");
          }

          if (sr & MAX31856_TC_TCRANGE) {
            log += F(" TC Range");
          }

          if (sr & MAX31856_TC_CJRANGE) {
            log += F(" CJ Range");
          }
          addLogMove(LOG_LEVEL_DEBUG_MORE, log);
        }
      }
    }
  }
  # endif // ifndef BUILD_NO_DEBUG


  const bool Plugin_039_SensorAttached = (sr == 0);

  if (Plugin_039_SensorAttached)
  {
    rawvalue >>= 5; // bottom 5 bits are unused
    // We're left with (24 - 5 =) 19 bits

    {
      float temperature = 0;

      switch (P039_TC_TYPE)
      {
        case 8:
        {
          temperature = rawvalue / 1677721.6f; // datasheet: rawvalue = 8 x 1.6 x 2^17 x VIN -> VIN = rawvalue / (8 x 1.6 x 2^17)
          break;
        }
        case 12:
        {
          temperature = rawvalue / 6710886.4f; // datasheet: rawvalue = 32 x 1.6 x 2^17 x VIN -> VIN = rawvalue / (32 x 1.6 x 2^17)
          break;
        }
        default:
        {
          temperature = convert_two_complement(rawvalue, 19);

          // Calculate Celsius
          temperature /= 128.0f;
          break;
        }
      }

      return temperature;
    }
  }
  else
  {
    // Fault state, thus output no value.
    return NAN;
  }
}

float P039_data_struct::readMax31865(struct EventStruct *event)
{
  uint8_t  registers[MAX31865_NO_REG] = { 0 };
  uint16_t rawValue                   = 0u;

  int8_t CS_pin_no = get_SPI_CS_Pin(event);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG))
  {
    String log;

    if (log.reserve(80u)) { // reserve value derived from example log file
      log  = F("P039 : MAX31865 :");
      log += F(" P039_data->convReady: ");
      log += boolToString(convReady);

      addLogMove(LOG_LEVEL_DEBUG, log);
    }
  }

  # endif // ifndef BUILD_NO_DEBUG


  // read conversion result and faults from plugin data structure
  // if pointer exists and conversion has been finished
  if (convReady) {
    rawValue                  = conversionResult;
    registers[MAX31865_FAULT] = deviceFaults;
  }


  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
  {
    String log;

    if (log.reserve(160u)) { // reserve value derived from example log file
      for (uint8_t i = 0u; i < MAX31865_NO_REG; ++i)
      {
        registers[i] = read8BitRegister(CS_pin_no, (MAX31865_READ_ADDR_BASE + i));
      }

      log = F("P039 : MAX31865 :");

      for (uint8_t i = 0u; i < MAX31865_NO_REG; ++i)
      {
        log += ' ';
        log += formatToHex_decimal(registers[i]);
      }

      addLogMove(LOG_LEVEL_DEBUG_MORE, log);
    }
  }

  # endif // ifndef BUILD_NO_DEBUG

  // Prepare and start next conversion, before handling faults and rawValue
  // clear all faults
  MAX31865_clearFaults(CS_pin_no);

  // set frequency filter
  change8BitRegister(CS_pin_no,
                     (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG),
                     (MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG),
                     MAX31865_SET_50HZ,
                     static_cast<bool>(P039_RTD_FILT_TYPE));


  // configure read access with configuration from web interface
  MAX31865_setConType(CS_pin_no, P039_CONFIG_4);

  // activate BIAS short before read, to reduce power consumption
  change8BitRegister(CS_pin_no,
                     (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG),
                     (MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG),
                     MAX31865_SET_VBIAS_ON,
                     P039_SET);

  // start time to follow up on BIAS activation before starting the conversion
  // and start conversion sequence via TIMER API
  // save current timer for next calculation
  timer = millis();

  // set next state to MAX31865_BIAS_ON_STATE

  Scheduler.setPluginTaskTimer(MAX31865_BIAS_WAIT_TIME, event->TaskIndex, MAX31865_BIAS_ON_STATE);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
  {
    if (registers[MAX31865_FAULT])
    {
      String log;

      if (log.reserve(210u)) { // reserve value derived from example log file
        log = F("P039 : MAX31865 : ");

        log += F("Fault : ");
        log += formatToHex_decimal(registers[MAX31865_FAULT]);
        log += F(" :");

        if (registers[MAX31865_FAULT] & MAX31865_FAULT_OVUV)
        {
          log += F(" Under/Over voltage");
        }

        if (registers[MAX31865_FAULT] & MAX31865_FAULT_RTDINLOW)
        {
          log += F(" RTDIN- < 0.85 x Bias - FORCE- open");
        }

        if (registers[MAX31865_FAULT] & MAX31865_FAULT_REFINHIGH)
        {
          log += F(" REFIN- < 0.85 x Bias - FORCE- open");
        }

        if (registers[MAX31865_FAULT] & MAX31865_FAULT_REFINLOW)
        {
          log += F(" REFIN- > 0.85 x Bias");
        }

        if (registers[MAX31865_FAULT] & MAX31865_FAULT_LOWTHRESH)
        {
          log += F(" RTD Low Threshold");
        }

        if (registers[MAX31865_FAULT] & MAX31865_FAULT_HIGHTHRESH)
        {
          log += F(" RTD High Threshold");
        }
        addLogMove(LOG_LEVEL_DEBUG_MORE, log);
      }
    }
  }
  # endif // ifndef BUILD_NO_DEBUG


  bool ValueValid = false;

  if (registers[MAX31865_FAULT] == 0x00u) {
    ValueValid = true;
  }

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG))
  {
    addLog(LOG_LEVEL_DEBUG, strformat(F("P039 : Temperature : registers[MAX31865_FAULT]: %s ValueValid: %s"),
                                      formatToHex_decimal(registers[MAX31865_FAULT]).c_str(),
                                      FsP(boolToString(ValueValid))));
  }

  # endif // ifndef BUILD_NO_DEBUG

  if (ValueValid)
  {
    rawValue >>= 1; // bottom fault bits is unused

    float temperature = convert_to_temperature(rawValue, getNomResistor(P039_RTD_TYPE), P039_RTD_RES);

    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG))
    {
      addLog(LOG_LEVEL_DEBUG, strformat(F("P039 : Temperature : rawValue: %s temperature: %.3f P039_RTD_TYPE: %d P039_RTD_RES: %d"),
                                        formatToHex_decimal(rawValue).c_str(),
                                        temperature,
                                        P039_RTD_TYPE,
                                        P039_RTD_RES));
    }

    # endif // ifndef BUILD_NO_DEBUG

    // add offset handling from configuration webpage
    temperature += P039_RTD_OFFSET;

    // Calculate Celsius
    return temperature;
  }
  else
  {
    // Fault state, thus output no value.
    return NAN;
  }
}

void P039_data_struct::MAX31865_clearFaults(int8_t l_CS_pin_no)
{
  uint8_t l_reg = 0u;

  // read in config register
  l_reg = read8BitRegister(l_CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG));


  // clear all faults ( write "0" to D2, D3, D5; write "1" to D2)
  l_reg &= ~(MAX31865_SET_ONE_SHOT | MAX31865_FAULT_CTRL_MASK);
  l_reg |= MAX31865_CLEAR_FAULTS;

  // write configuration
  write8BitRegister(l_CS_pin_no, (MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG), l_reg);
}

void P039_data_struct::MAX31865_setConType(int8_t l_CS_pin_no, uint8_t l_conType)
{
  bool l_set_reset = false;

  // configure if 3 WIRE bit will be set/reset
  switch (l_conType)
  {
    case 0:
      l_set_reset = P039_RESET;
      break;
    case 1:
      l_set_reset = P039_SET;
      break;
    default:
      l_set_reset = P039_RESET;
      break;
  }

  // change to configuration register
  change8BitRegister(l_CS_pin_no,
                     (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG),
                     (MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG),
                     MAX31865_SET_3WIRE,
                     l_set_reset);
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
float P039_data_struct::convert_to_temperature(uint32_t l_rawvalue, float RTDnominal, float refResistor)
{
  # define RTD_A 3.9083e-3f
  # define RTD_B -5.775e-7f

  float Z1, Z2, Z3, Z4, Rt, temp;

  Rt  = l_rawvalue;
  Rt /= 32768u;
  Rt *= refResistor;

  Z1 = -RTD_A;
  Z2 = RTD_A * RTD_A - (4 * RTD_B);
  Z3 = (4 * RTD_B) / RTDnominal;
  Z4 = 2 * RTD_B;

  temp = Z2 + (Z3 * Rt);
  temp = (sqrtf(temp) + Z1) / Z4;

  if (temp >= 0) {
    return temp;
  }

  Rt /= RTDnominal;
  Rt *= 100; // normalize to 100 ohm

  float rpoly = Rt;

  temp   = -242.02f;
  temp  += 2.2228f * rpoly;
  rpoly *= Rt; // square
  temp  += 2.5859e-3f * rpoly;
  rpoly *= Rt; // ^3
  temp  -= 4.8260e-6f * rpoly;
  rpoly *= Rt; // ^4
  temp  -= 2.8183e-8f * rpoly;
  rpoly *= Rt; // ^5
  temp  += 1.5243e-10f * rpoly;

  return temp;
}

uint16_t P039_data_struct::getNomResistor(uint8_t l_RType)
{
  uint16_t l_returnValue = 100u;

  switch (l_RType)
  {
    case MAX31865_PT1000:
      l_returnValue = 1000u;
      break;
    case MAX31865_PT100: // Fall through
    default:
      break;
  }
  return l_returnValue;
}

int P039_data_struct::convert_two_complement(uint32_t value, int nr_bits) {
  const bool negative = (value & (1 << (nr_bits - 1))) != 0;
  int nativeInt;

  if (negative) {
    // Add zeroes to the left to create the proper negative native-sized integer.
    nativeInt = value | ~((1 << nr_bits) - 1);
  } else {
    nativeInt = value;
  }
  return nativeInt;
}

float P039_data_struct::readLM7x(struct EventStruct *event)
{
  float temperature  = 0.0f;
  uint16_t device_id = 0u;
  uint16_t rawValue  = 0u;

  int8_t CS_pin_no = get_SPI_CS_Pin(event);

  // operate LM7x devices in polling mode, assuming conversion is ready with every call of this read function ( >=210ms call cycle)
  // this allows usage of multiples generations of LM7x devices, that doe not provde conversion ready information in temperature register

  rawValue = readLM7xRegisters(CS_pin_no, P039_RTD_LM_TYPE, P039_RTD_LM_SHTDWN, &device_id);

  temperature = convertLM7xTemp(rawValue, P039_RTD_LM_TYPE);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG))
  {
    addLog(LOG_LEVEL_DEBUG, strformat(F("P039 : LM7x : readLM7x :  rawValue: %s device_id: %s temperature: %.3f"),
                                      formatToHex_decimal(rawValue).c_str(),
                                      formatToHex(device_id).c_str(),
                                      temperature));
  }
  # endif // ifndef BUILD_NO_DEBUG

  return temperature;
}

float P039_data_struct::convertLM7xTemp(uint16_t l_rawValue, uint16_t l_LM7xsubtype)
{
  float   l_returnValue    = 0.0f;
  float   l_lsbvalue       = 0.0f;
  uint8_t l_noBits         = 0u;
  int     l_intTemperature = 0;

  switch (l_LM7xsubtype)
  {
    case LM7x_SD70:
      l_rawValue >>= 5;
      l_lsbvalue   = 0.25f;
      l_noBits     = 11u;
      break;
    case LM7x_SD71:
      l_rawValue >>= 2;
      l_lsbvalue   = 0.03125f;
      l_noBits     = 14u;
      break;
    case LM7x_SD74:
      l_rawValue >>= 3;
      l_lsbvalue   = 0.0625f;
      l_noBits     = 13u;
      break;
    case LM7x_SD121:
    case LM7x_SD122:
    case LM7x_SD123:
    case LM7x_SD124:
      l_rawValue >>= 4;
      l_lsbvalue   = 0.0625f;
      l_noBits     = 12u;
      break;
    case LM7x_SD125:
      l_rawValue >>= 5;
      l_lsbvalue   = 0.25f;
      l_noBits     = 10u;
      break;
    default: // use lowest resolution as fallback if no device has been configured
      l_rawValue >>= 5;
      l_lsbvalue   = 0.25f;
      l_noBits     = 11u;
      break;
  }

  l_intTemperature = convert_two_complement(l_rawValue, l_noBits);

  l_returnValue = l_intTemperature * l_lsbvalue;

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
  {
    addLog(LOG_LEVEL_DEBUG_MORE,
           strformat(F("P039 : LM7x : convertLM7xTemp :  l_returnValue: %s l_LM7xsubtype: %s l_rawValue: %s l_noBits: %d l_lsbvalue: %.3f"),
                     formatToHex_decimal(l_returnValue).c_str(),
                     formatToHex_decimal(l_LM7xsubtype).c_str(),
                     formatToHex_decimal(l_rawValue).c_str(),
                     l_noBits,
                     l_lsbvalue));
  }

  # endif // ifndef BUILD_NO_DEBUG

  return l_returnValue;
}

uint16_t P039_data_struct::readLM7xRegisters(int8_t l_CS_pin_no, uint8_t l_LM7xsubType, uint8_t l_runMode, uint16_t *l_device_id)
{
  uint16_t l_returnValue = 0u;
  uint16_t l_mswaitTime  = 0u;


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

  // // activate communication -> CS low
  // handle_SPI_CS_Pin(l_CS_pin_no, LOW);

  if (l_runMode)
  {
    // shutdown mode active -> conversion when called
    uint8_t messageBuffer[12] = { 0xFF, 0xFF, 0xFF, 0X00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF };

    // send inital 4 bytes to wake the device and start the conversion
    transfer_n_ByteSPI(l_CS_pin_no, 4, &messageBuffer[0]);

    // wait specific ms for conversion to be ready (TI datasheet per devices)
    delay(l_mswaitTime);

    // send remaining 8 bytes to read the device ID and shutdown the device
    transfer_n_ByteSPI(l_CS_pin_no, 8, &messageBuffer[4]);

    // read temperature value (16 Bit)
    l_returnValue = ((messageBuffer[4] << 8) | messageBuffer[5]);

    // read Manufatures/Device ID (16 Bit)
    *(l_device_id) = ((messageBuffer[8] << 8) | messageBuffer[9]);
  }
  else
  {
    // shutdown mode inactive -> normal background conversion during call cycle
    uint8_t messageBuffer[8] = { 0x00, 0x00, 0xFF, 0XFF, 0x00, 0x00, 0x00, 0x00 };

    transfer_n_ByteSPI(l_CS_pin_no, 8, &messageBuffer[0]);

    // read temperature value (16 Bit)
    l_returnValue = ((messageBuffer[0] << 8) | messageBuffer[1]);

    // read Manufatures/Device ID (16 Bit)
    *(l_device_id) = ((messageBuffer[4] << 8) | messageBuffer[5]);
  }

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
  {
    addLog(LOG_LEVEL_DEBUG_MORE, strformat(F("P039 : LM7x : readLM7xRegisters :  l_returnValue: %s l_device_id: %s"),
                                           formatToHex_decimal(l_returnValue).c_str(),
                                           formatToHex(*(l_device_id)).c_str()));
  }

  # endif // ifndef BUILD_NO_DEBUG

  return l_returnValue;
}

// POSSIBLE START OF GENERIC SPI HIGH LEVEL FUNCTIONS WITH POTENTIAL OF SYSTEM WIDE RE-USE

/**************************************************************************/

/*!
    @brief generic high level library to access SPI interface from plugins
    with GPIO pin handled as CS - chri.kai.in 2021

    Initial Revision - chri.kai.in 2021

    TODO: c.k.i.: make it generic and carve out to generic _SPI_helper.c library


   **************************************************************************/


/**************************************************************************/

/*!

    @brief Identifying the CS pin from the event basic data structure
    @param event pointer to the event structure; default GPIO is chosen as GPIO 15

    @returns

    Initial Revision - chri.kai.in 2021

   **************************************************************************/
int P039_data_struct::get_SPI_CS_Pin(struct EventStruct *event) { // If no Pin is in Config we use 15 as default -> Hardware Chip Select on
                                                                  // ESP8266
  if (CONFIG_PIN1 != -1) {
    return CONFIG_PIN1;
  }
  return 15; // D8
}

/**************************************************************************/

/*!
    @brief Initializing GPIO as OUTPUT for CS for SPI communication
    @param l_CS_pin_no the GPIO pin number used as CS

    @returns

    Initial Revision - chri.kai.in 2021

   **************************************************************************/
void P039_data_struct::init_SPI_CS_Pin(int8_t l_CS_pin_no) {
  // set the slaveSelectPin as an output:
  pinMode(l_CS_pin_no, OUTPUT);
}

/**************************************************************************/

/*!
    @brief Handling GPIO as CS for SPI communication
    @param l_CS_pin_no the GPIO pin number used as CS
    @param l_state the state of the CS pin: "HIGH/LOW" reflecting the physical level

    @returns

    Initial Revision - chri.kai.in 2021

   **************************************************************************/
void P039_data_struct::handle_SPI_CS_Pin(int8_t l_CS_pin_no, bool l_state) {
  P039_CS_Delay(); // tCWH (min) >= x00ns
  digitalWrite(l_CS_pin_no, l_state);
  P039_CS_Delay(); // tCC (min) >= x00ns
}

/**************************************************************************/

/*!
    @brief write 8 bits to adress l_address on the SPI interface, handling a GPIO CS
    @param l_CS_pin_no the GPIO pin number used as CS
    @param l_address the register addess of the connected SPI device
    @param value the unsigned 8 Bit message to be transferred

    @returns

    Initial Revision - chri.kai.in 2021

   **************************************************************************/
void P039_data_struct::write8BitRegister(int8_t l_CS_pin_no, uint8_t l_address, uint8_t value)
{
  uint8_t l_messageBuffer[2] = { l_address, value };

  transfer_n_ByteSPI(l_CS_pin_no, 2, l_messageBuffer);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
  {
    addLog(LOG_LEVEL_DEBUG_MORE, strformat(F("P039 : SPI : write8BitRegister : l_address: %s value: %s"),
                                           formatToHex(l_address).c_str(),
                                           formatToHex_decimal(value).c_str()));
  }

  # endif // ifndef BUILD_NO_DEBUG
}

/**************************************************************************/

/*!
    @brief write 16 bits to adress l_address on the SPI interface, handling a GPIO CS
    @param l_CS_pin_no the GPIO pin number used as CS
    @param l_address the register addess of the connected SPI device
    @param value the unsigned 16 Bit message to be transferred

    @returns

    Initial Revision - chri.kai.in 2021

   **************************************************************************/
void P039_data_struct::write16BitRegister(int8_t l_CS_pin_no, uint8_t l_address, uint16_t value)
{
  uint8_t l_messageBuffer[3] = { l_address, static_cast<uint8_t>((value >> 8) & 0xFF), static_cast<uint8_t>(value & 0xFF) };

  transfer_n_ByteSPI(l_CS_pin_no, 3, l_messageBuffer);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
  {
    addLog(LOG_LEVEL_DEBUG_MORE, strformat(F("P039 : SPI : write16BitRegister : l_address: %s value: %s"),
                                           formatToHex(l_address).c_str(),
                                           formatToHex_decimal(value).c_str()));
  }

  # endif // ifndef BUILD_NO_DEBUG
}

/**************************************************************************/

/*!
    @brief read 8 bits from adress l_address on the SPI interface, handling a GPIO CS
    @param l_CS_pin_no the GPIO pin number used as CS
    @param l_address the register addess of the connected SPI device

    @returns the unsigned 8 Bit message read from l_address

    Initial Revision - chri.kai.in 2021

   **************************************************************************/
uint8_t P039_data_struct::read8BitRegister(int8_t l_CS_pin_no, uint8_t l_address)
{
  uint8_t l_messageBuffer[2] = { l_address, 0x00 };

  transfer_n_ByteSPI(l_CS_pin_no, 2, l_messageBuffer);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
  {
    addLog(LOG_LEVEL_DEBUG_MORE, strformat(F("P039 : SPI : read8BitRegister : l_address: %s returnvalue: %s"),
                                           formatToHex(l_address).c_str(),
                                           formatToHex_decimal(l_messageBuffer[1]).c_str()));
  }

  # endif // ifndef BUILD_NO_DEBUG

  return l_messageBuffer[1];
}

/**************************************************************************/

/*!
    @brief write 16 bits to adress l_address on the SPI interface, handling a GPIO CS
    @param l_CS_pin_no the GPIO pin number used as CS
    @param l_address the register addess of the connected SPI device

    @returns the unsigned 16 Bit message read from l_address

    Initial Revision - chri.kai.in 2021

   **************************************************************************/
uint16_t P039_data_struct::read16BitRegister(int8_t l_CS_pin_no, uint8_t l_address)
{
  uint8_t  l_messageBuffer[3] = { l_address, 0x00, 0x00 };
  uint16_t l_returnValue;

  transfer_n_ByteSPI(l_CS_pin_no, 3, l_messageBuffer);
  l_returnValue = ((l_messageBuffer[1] << 8) | l_messageBuffer[2]);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
  {
    addLog(LOG_LEVEL_DEBUG_MORE, strformat(F("P039 : SPI : read16BitRegister : l_address: %s l_returnValue: %s"),
                                           formatToHex(l_address).c_str(),
                                           formatToHex_decimal(l_returnValue).c_str()));
  }

  # endif // ifndef BUILD_NO_DEBUG

  return l_returnValue;
}

/**************************************************************************/

/*!
    @brief read from/write to dedicated number of bytes from/to SPI, handling a GPIO CS
    @param l_CS_pin_no the GPIO pin number used as CS
    @param l_noBytesToSend number of bytes to read/write from/to SPI
    @param l_inoutMessageBuffer pointer to the messsage buffer to provide bytes to send
    and provide read bytes from the SPI bus after the call

    @returns

    Initial Revision - chri.kai.in 2021

   **************************************************************************/
void P039_data_struct::transfer_n_ByteSPI(int8_t l_CS_pin_no, uint8_t l_noBytesToSend, uint8_t *l_inoutMessageBuffer)
{
  // activate communication -> CS low
  handle_SPI_CS_Pin(l_CS_pin_no, LOW);

  for (size_t i = 0u; i < l_noBytesToSend; i++)
  {
    l_inoutMessageBuffer[i] = _spi.transfer(l_inoutMessageBuffer[i]);
  }

  // stop communication -> CS high
  handle_SPI_CS_Pin(l_CS_pin_no, HIGH);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
  {
    String log;

    if (log.reserve(120u)) {                         // reserve value derived from example log file
      log = F("P039 : SPI : transfer_n_ByteSPI : "); // 34 char

      for (uint8_t i = 0; i < l_noBytesToSend; ++i)
      {
        log += ' ';                                          // 1 char
        log += formatToHex_decimal(l_inoutMessageBuffer[i]); // 9 char
      }
      addLogMove(LOG_LEVEL_DEBUG_MORE, log);
    }
  }

  # endif // ifndef BUILD_NO_DEBUG
}

/**************************************************************************/

/*!
    @brief read a 16Bit register and change a flag, writing it back, handling a GPIO CS
    @param l_CS_pin_no the GPIO pin number used as CS
    @param l_readaddress SPI read address of the device register
    @param l_writeaddress SPI write address of the device register
    @param l_flagmask mask set to apply on the read register
    @param l_set_reset controls if flag mask will be set (-> true) or reset ( -> false)


    @returns

    Initial Revision - chri.kai.in 2021

   **************************************************************************/
void P039_data_struct::change16BitRegister(int8_t   l_CS_pin_no,
                                           uint8_t  l_readaddress,
                                           uint8_t  l_writeaddress,
                                           uint16_t l_flagmask,
                                           bool     l_set_reset)
{
  uint16_t l_reg = 0u;

  // read in config register
  l_reg = read16BitRegister(l_CS_pin_no, l_readaddress);

  if (l_set_reset) {
    l_reg |= l_flagmask;
  }
  else
  {
    l_reg &= ~(l_flagmask);
  }

  // write to configuration register
  write16BitRegister(l_CS_pin_no, l_writeaddress, l_reg);
}

/**************************************************************************/

/*!
    @brief read a 8 Bit register and change a flag, writing it back, handling a GPIO CS
    @param l_CS_pin_no the GPIO pin number used as CS
    @param l_readaddress SPI read address of the device register
    @param l_writeaddress SPI write address of the device register
    @param l_flagmask mask set to apply on the read register
    @param l_set_reset controls if flag mask will be set (-> true) or reset ( -> false)


    @returns

    Initial Revision - chri.kai.in 2021

   **************************************************************************/
void P039_data_struct::change8BitRegister(int8_t  l_CS_pin_no,
                                          uint8_t l_readaddress,
                                          uint8_t l_writeaddress,
                                          uint8_t l_flagmask,
                                          bool    l_set_reset)
{
  uint8_t l_reg = 0u;

  // read in config register
  l_reg = read8BitRegister(l_CS_pin_no, l_readaddress);


  // TODO: c.k.i.: analyze opportunity to use arduino bitSet/Clear macros instead
  if (l_set_reset) {
    l_reg |= l_flagmask;
  }
  else
  {
    l_reg &= ~(l_flagmask);
  }

  // write to configuration register
  write8BitRegister(l_CS_pin_no, l_writeaddress, l_reg);
}

#endif // ifdef USES_P039
