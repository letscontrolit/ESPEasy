#include "_Plugin_Helper.h"
#ifdef USES_P039

// #######################################################################################################
// ######################## Plugin 039: Thermocouple (MAX6675 / MAX31855) ################################
// #######################################################################################################

// Original work by Dominik

// Plugin Description
// This Plugin reads the data from Thermocouples. You have to use an Adapter Board with a
// MAX6675 or MAX31855 in order to read the values. Take a look at ebay to find such boards :-)
// You can only use ESP8266 boards which expose the SPI Interface. This Plugin uses only the Hardware
// SPI Interface - no software SPI at the moment.
// But nevertheless you need at least 3 Pins to use SPI. So using an very simple ESP-01 is no option - Sorry.
// The Wiring is straight forward ...
//
// If you like to send suggestions feel free to send me an email : dominik@logview.info
// Have fun ... Dominik

// Wiring
// https://de.wikipedia.org/wiki/Serial_Peripheral_Interface
// You need an ESP8266 device with accessible SPI Pins. These are:
// Name   Description     GPIO      NodeMCU   Notes
// MOSI   Master Output   GPIO13    D7        Not used (No Data sending to MAX)
// MISO   Master Input    GPIO12    D6        Hardware SPI
// SCK    Clock Output    GPIO14    D5        Hardware SPI
// CS     Chip Select     GPIO15    D8        Hardware SPI (CS is configurable through the web interface)

// Thermocouple Infos
// http://www.bristolwatch.com/ele2/therc.htm

// Resistor Temperature Detector Infos
// https://en.wikipedia.org/wiki/Resistance_thermometer

// Chips
// MAX6675  - Cold-Junction-Compensated K-Thermocouple-to-Digital Converter (   0°C to +1024°C)
//            https://cdn-shop.adafruit.com/datasheets/MAX6675.pdf (only
// MAX31855 - Cold-Junction Compensated Thermocouple-to-Digital Converter   (-270°C to +1800°C)
//            https://cdn-shop.adafruit.com/datasheets/MAX31855.pdf
// MAX31856 - Precision Thermocouple to Digital Converter with Linearization   (-210°C to +1800°C)
//            https://datasheets.maximintegrated.com/en/ds/MAX31856.pdf
// MAX31865 - Precision Resistor Temperature Detector to Digital Converter with Linearization   (PT100 / PT1000)
//            https://datasheets.maximintegrated.com/en/ds/MAX31865.pdf
// TI Digital Temperature sensors with SPI interface
//            https://www.ti.com/sensors/temperature-sensors/digital/products.html#p1918=SPI,%20Microwire
// TI LM7x -  Digital temperature sensor with SPI interface 
//            https://www.ti.com/lit/gpn/LM70
//            https://www.ti.com/lit/gpn/LM71
//            https://www.ti.com/lit/gpn/LM70
//            https://www.ti.com/lit/gpn/LM74
// TI TMP12x  Digital temperature sensor with SPI interface 
//            https://www.ti.com/lit/gpn/TMP121
//            https://www.ti.com/lit/gpn/TMP122
//            https://www.ti.com/lit/gpn/TMP123
//            https://www.ti.com/lit/gpn/TMP124

# include <SPI.h>

// // plugin-local quick activation of debug messages
// #ifdef BUILD_NO_DEBUG
//   #undef BUILD_NO_DEBUG
// #endif

# define PLUGIN_039
# define PLUGIN_ID_039         39
# define PLUGIN_NAME_039       "Environment - Thermosensors"
# define PLUGIN_VALUENAME1_039 "Temperature"

// typically 500ns of wating on positive/negative edge of CS should be enough ( -> datasheet); to make sure we cover a lot of devices we spend 5ms ( factor 10 !)
// TODO: c.k.i: analyze if less wating could be sufficient
#define P039_CS_Delay()             delayMicroseconds(5)

#define P039_MAX_TYPE               PCONFIG(0)
#define P039_TC_TYPE                PCONFIG(1)
#define P039_FAM_TYPE               PCONFIG(2)
#define P039_RTD_TYPE               PCONFIG(3)
#define P039_RTD_CON_TYPE           PCONFIG(4)
#define P039_RTD_FILT_TYPE          PCONFIG(5)
#define P039_RTD_LM_TYPE            PCONFIG(6)
#define P039_RTD_LM_SHTDWN          PCONFIG(7)
#define P039_RTD_RES                PCONFIG_LONG(0)
#define P039_RTD_OFFSET             PCONFIG_FLOAT(0)

#define P039_TC                     0
#define P039_RTD                    1

# define P039_MAX_6675              1
# define P039_MAX_31855             2
# define P039_MAX_31856             3
# define P039_MAX31865              4
# define P039_LM7x                  5

// register offset values for MAX 31856
# define MAX31856_RAWVALUE           0
# define MAX31856_CR0                1
# define MAX31856_CR1                2
# define MAX31856_MASK               3
# define MAX31856_CJHF               4
# define MAX31856_CJLF               5
# define MAX31856_LTHFTH             6
# define MAX31856_LTHFTL             7
# define MAX31856_LTLFTH             8
# define MAX31856_LTLFTL             9
# define MAX31856_CJTO              10
# define MAX31856_CJTH              11
# define MAX31856_CJTL              12
# define MAX31856_LTCBH             13
# define MAX31856_LTCBM             14
# define MAX31856_LTCBL             15
# define MAX31856_SR                16

#define MAX31856_NO_REG             17


// RTD related defines

// MAX 31865 related defines

// waiting time until "in sequence" conversion is ready (-> used in case device is set to shutdown in between call cycles)
// typically 75ms should be OK - give a little adder to "be sure" conversion is done; alternatively ONE SHOT bit could be polled (system/SPI bus load !)
// TODO: c.k.i: reduce to balanced minimum
#define MAX31865_CONVERSION_BREAK   100

// sensor type
#define MAX31865_PT100              0
#define MAX31865_PT1000             1

// base address for read/write acces to MAX 31865
#define MAX31865_READ_ADDR_BASE         0x00
#define MAX31865_WRITE_ADDR_BASE        0x80

// register offset values for MAX 31865
#define MAX31865_CONFIG                 0
#define MAX31865_RTD_MSB                1
#define MAX31865_RTD_LSB                2
#define MAX31865_HFT_MSB                3
#define MAX31865_HFT_LSB                4
#define MAX31865_LFT_MSB                5
#define MAX31865_LFT_LSB                6
#define MAX31865_FAULT                  7

// total number of registers in MAX 31865
#define MAX31865_NO_REG                 8

// bit masks to identify failures for MAX 31865
#define MAX31865_FAULT_HIGHTHRESH   0x80
#define MAX31865_FAULT_LOWTHRESH    0x40
#define MAX31865_FAULT_REFINLOW     0x20
#define MAX31865_FAULT_REFINHIGH    0x10
#define MAX31865_FAULT_RTDINLOW     0x08
#define MAX31865_FAULT_OVUV         0x04

// bit masks for access of configuration bits
#define MAX31865_SET_50HZ           0x01
#define MAX31865_CLEAR_FAULTS       0x02
#define MAX31865_FAULT_CTRL_MASK    0x0C
#define MAX31865_SET_3WIRE          0x10
#define MAX31865_SET_ONE_SHOT       0x20
#define MAX31865_SET_CONV_AUTO      0x40
#define MAX31865_SET_VBIAS_ON       0x80

//LM7x related defines

// LM7x subtype defines
#define LM7x_SD70                   0x00
#define LM7x_SD71                   0x01
#define LM7x_SD74                   0x04
#define LM7x_SD121                  0x05
#define LM7x_SD122                  0x06
#define LM7x_SD123                  0x07
#define LM7x_SD124                  0x08
#define LM7x_SD125                  0x09

// bit masks for access of configuration bits
#define LM7x_CONV_RDY               0x02


boolean Plugin_039(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_039;
      Device[deviceCount].Type               = DEVICE_TYPE_SPI;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_039);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_039));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("CS"));
      break;
    }

    case PLUGIN_INIT:
    {
      uint8_t CS_pin_no = get_SPI_CS_Pin(event);
   
      // set the slaveSelectPin as an output:
      init_SPI_CS_Pin(CS_pin_no);

      // initialize SPI:
      SPI.setHwCs(false);
      SPI.begin();

      if (P039_MAX_TYPE == P039_MAX_31856) {

        // init string - content accoring to inital implemetnation of P039 - MAX31856 read function
        uint8_t sendBuffer[11] = {0x80, 0x01, (uint8_t) P039_TC_TYPE, 0xFF, 0x7F, 0xC0, 0x7F, 0xFF, 0x80, 0x00, 0x00 };

        transfer_n_ByteSPI(CS_pin_no, 11, &sendBuffer[0] );

        //    // activate communication -> CS low
        //   handle_SPI_CS_Pin(CS_pin_no, LOW);

        //   // delay(650);
          
        //   SPI.transfer(0x80);
        //   SPI.transfer(0x01);         // noisefilter 50Hz (set this to 0x00 if You live in a 60Hz country)
        //   SPI.transfer(P039_TC_TYPE); // thermocouple type
        //   SPI.transfer(0xFF);
        //   SPI.transfer(0x7F);
        //   SPI.transfer(0xC0);
        //   SPI.transfer(0x7F);
        //   SPI.transfer(0xFF);
        //   SPI.transfer(0x80);
        //   SPI.transfer(0x00);
        //   SPI.transfer(0x00);

        //   // stop communication -> CS high
        //   handle_SPI_CS_Pin(CS_pin_no, HIGH);
      }

      if(P039_MAX_TYPE == P039_MAX31865){

        // two step initialization buffer 
        uint8_t initSendBufferHFTH[3] = {(MAX31865_WRITE_ADDR_BASE + MAX31865_HFT_MSB), 0xFF, 0xFF};
        uint8_t initSendBufferLFTH[3] = {(MAX31865_WRITE_ADDR_BASE + MAX31865_HFT_MSB), 0xFF, 0xFF};

        // ensure MODE3 access to SPI device
        SPI.setDataMode(SPI_MODE3);

        // activate 50Hz filter, clear all faults, no auto conversion, no conversion started
        chooseFilterType(CS_pin_no, 0x01);

        // configure 2/4-wire sensor connection as default
        setConType(CS_pin_no, 0x00);
        
        // set HighFault Threshold
        transfer_n_ByteSPI(CS_pin_no, 3, &initSendBufferHFTH[0]);
        
        // set LowFault Threshold
        transfer_n_ByteSPI(CS_pin_no, 3, &initSendBufferLFTH[0]);

        // finally clear all faults again - playing safe
        clearFaults(CS_pin_no);

      }
      if (P039_MAX_TYPE == P039_LM7x)
      {
        // ensure MODE3 access to SPI device
        SPI.setDataMode(SPI_MODE3);

        // TODO: c.k.i.: more detailed inits depending on the sub devices expected , e.g. TMP 122/124
      }

      addLog(LOG_LEVEL_INFO, F("P039 : SPI Init - DONE"));
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // FIXME TD-er: Why is this list needed? GPIO selector should provide this info.
        # ifdef ESP8266
          {
            addFormNote(F("<b>1st GPIO</b> = CS (Usable GPIOs : 0, 2, 4, 5, 15)"));
          }
        # endif // ifdef ESP8266
        # ifdef ESP32
          {
            addFormNote(F("<b>1st GPIO</b> = CS (Usable GPIOs : 0, 2, 4, 5, 15..19, 21..23, 25..27, 32, 33)"));
          }
        # endif // ifdef ESP32

      // addHtml(F("<TR><TD>Info GPIO:<TD><b>1st GPIO</b> = CS (Usable GPIOs : 0, 2, 4, 5, 15)"));
      {
        addFormSubHeader(F("Sensor Family Selection"));
      }

      const byte family = P039_FAM_TYPE;
      {
        const String Foptions[2] = {F("Thermocouple"), F("RTD")};
        const int FoptionValues[2] = {P039_TC, P039_RTD};
        addFormSelector(F("Sensor Family Type"), F("P039_famtype"), 2, Foptions, FoptionValues, family);
        addFormNote(F("Set sensor family of connected sensor - thermocouple or RTD. Submit the form after choice to allow update of sections below accordingly !"));
      }

      const byte choice = P039_MAX_TYPE;

      if (family == P039_TC){

        {
          addFormSubHeader(F("Device Type Settings"));
        }

        {
          const String options[3]      = {   F("MAX 6675"), F("MAX 31855"), F("MAX 31856") };
          const int    optionValues[3] = { P039_MAX_6675, P039_MAX_31855, P039_MAX_31856 };
          addFormSelector(F("Adapter IC"), F("P039_maxtype"), 3, options, optionValues, choice);
          addFormNote(F("Set adapter IC used. Submit the form after choice to allow update of sections below accordingly !"));
        }
    
        if (choice == P039_MAX_31856) {
          {
            addFormSubHeader(F("Device Settings"));
          }
          {
            addFormNote(F("Set Thermocouple type for MAX31856"));
            const String Toptions[8]      = { F("B"), F("E"), F("J"), F("K"), F("N"), F("R"), F("S"), F("T") };
            const int    ToptionValues[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
            addFormSelector(F("Thermocouple type"), F("P039_tctype"), 8, Toptions, ToptionValues, P039_TC_TYPE);
          }
        }
      }
      else {
        {
         addFormSubHeader(F("Device Type Settings"));
        }

        {
          const String TPoptions[2] = {F("MAX 31865"), F("LM7x")};
          const int TPoptionValues[2] = {P039_MAX31865, P039_LM7x};
          addFormSelector(F("Adapter IC"), F("P039_maxtype"), 2, TPoptions, TPoptionValues, choice);
          addFormNote(F("Set used RTD Converter Module. Currently only MAX31865 is fully supported. LM7x derivatives are untested and experimental.\nSubmit the form after choice to allow update of sections below accordingly !"));
        }

       

        if (choice == P039_MAX31865)
        {
          {
            addFormSubHeader(F("Device Settings"));
          }
          {
            const String PToptions[2] = {F("PT100"), F("PT1000")};
            const int PToptionValues[2] = {MAX31865_PT100, MAX31865_PT1000};
            addFormSelector(F("Resistor Type"), F("P039_rtdtype"), 2, PToptions, PToptionValues, P039_RTD_TYPE);
            addFormNote(F("Set Resistor Type for MAX31865"));
          }
          {
            const String Coptions[2] = {F("2-/4-wire"), F("3-wire")};
            const int CoptionValues[2] = {0, 1};
            addFormSelector(F("Connection Type"), F("P039_contype"), 2, Coptions, CoptionValues, P039_RTD_CON_TYPE);
            addFormNote(F("Set Connection Type for MAX31865"));
          }
          {
            const String FToptions[2] = {F("60 Hz"), F("50 Hz")};
            const int FToptionValues[2] = {0, 1};
            addFormSelector(F("Supply Frequency Filter"), F("P039_filttype"), 2, FToptions, FToptionValues, P039_RTD_FILT_TYPE);
            addFormNote(F("Set filter frequency for supply voltage. Choose appropriate to your power net frequency (50/60 Hz)"));
          }
          {
            addFormNumericBox(F("Reference Resistor [OHM]"), F("P039_res"), P039_RTD_RES, 0);
            addFormNote(F("Set reference resistor for MAX31865. PT100: typically 430 [OHM]; PT1000: typically 4300 [OHM]"));
          }
          {
            addFormFloatNumberBox(F("Offset [K]"), F("P039_offset"), P039_RTD_OFFSET, -50.0f, 50.0f, 2, 0.01f);
            addFormNote(F("Set Offset [K] for MAX31865. Valid values: [-50.0...50.0 K], min. stepsize: [0.01]"));
          }
        }

        if (choice == P039_LM7x)
        {
          {
            addFormSubHeader(F("Device Settings"));
          }

          {
            const String PToptions[8] = {F("LM70"), F("LM71"), F("LM74"), F("TMP121"), F("TMP122"), F("TMP123"), F("TMP124"), F("TMP125")};
            const int PToptionValues[8] = {LM7x_SD70, LM7x_SD71, LM7x_SD74, LM7x_SD121, LM7x_SD122, LM7x_SD123, LM7x_SD124, LM7x_SD125};
            addFormSelector(F("LM7x device details"), F("P039_rtd_lm_type"), 8, PToptions, PToptionValues, P039_RTD_LM_TYPE);
            addFormNote(F("Choose LM7x device details to allow handling of device specifics,TMP122/124 not yet supported with all options -> fixed 12 Bit resolution, no advanced options active"));
          }
          {
            addFormCheckBox(F("Enable Shutdown Mode"), F("P039_rtd_lm_shtdwn"), P039_RTD_LM_SHTDWN);
            addFormNote(F("Enable shutdown mode for LM7x devices. Device is set to shutdown between sample cycles. Useful for very long call cycles, to save power.\nWithout LM7x device conversion happens in between call cycles. Call Cylces should therefore not become lower than 350ms."));
          }
        }  
      }
      
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P039_FAM_TYPE = getFormItemInt(F("P039_famtype"));
      P039_MAX_TYPE = getFormItemInt(F("P039_maxtype"));
      P039_TC_TYPE  = getFormItemInt(F("P039_tctype"));
      P039_RTD_TYPE = getFormItemInt(F("P039_rtdtype"));
      P039_RTD_CON_TYPE = getFormItemInt(F("P039_contype"));
      P039_RTD_FILT_TYPE = getFormItemInt(F("P039_filttype"));
      P039_RTD_RES = getFormItemInt(F("P039_res"));
      P039_RTD_OFFSET = getFormItemFloat(F("P039_offset"));
      P039_RTD_LM_TYPE = getFormItemInt(F("P039_rtd_lm_type"));
      P039_RTD_LM_SHTDWN = isFormItemChecked(F("P039_rtd_lm_shtdwn"));

      success       = true;
      break;
    }

    case PLUGIN_READ:
    {
      // Get the MAX Type (6675 / 31855 / 31856)
      byte MaxType = P039_MAX_TYPE;

      float Plugin_039_Celsius = NAN;

      switch (MaxType) {
        case P039_MAX_6675:
          Plugin_039_Celsius = readMax6675(event);
          break;
        case P039_MAX_31855:
          Plugin_039_Celsius = readMax31855(event);
          break;
        case P039_MAX_31856:
          Plugin_039_Celsius = readMax31856(event);
          break;
        case P039_MAX31865:
          Plugin_039_Celsius = readMax31865(event);
          break;
        case P039_LM7x:
          Plugin_039_Celsius = readLM7x(event);
          break;
      }

      if (Plugin_039_Celsius != NAN)
      {
        UserVar[event->BaseVarIndex] = Plugin_039_Celsius;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log;
          log.reserve(66u);
          log = F("P039 : Temperature ");
          log += formatUserVarNoCheck(event->TaskIndex, 0);
          addLog(LOG_LEVEL_INFO, log);
        }
        success = true;
      }
      else
      {
        UserVar[event->BaseVarIndex]     = NAN;
        UserVar[event->BaseVarIndex + 1] = NAN;
        addLog(LOG_LEVEL_INFO, F("P039 : No Sensor attached !"));
        success = false;
      }

      break;
    }
  }
  return success;
}

float readMax6675(struct EventStruct *event)
{
  uint8_t CS_pin_no = get_SPI_CS_Pin(event);

  // "transfer" 0x0 and read the Data from the Chip
  // uint16_t rawvalue = SPI.transfer16(0x0);
  uint16_t rawvalue = read16BitRegister(CS_pin_no, 0x0000);

  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) 
    {
      String log;
      log.reserve(66u);
      log = F("P039 : MAX6675 : RAW - BIN:");
      log += String(rawvalue, BIN);
      log += F(" HEX:");
      log += String(rawvalue, HEX);
      log += F(" DEC:");
      log += String(rawvalue);
      addLog(LOG_LEVEL_DEBUG, log);
    }

  # endif // ifndef BUILD_NO_DEBUG

  // Open Thermocouple
  // Bit D2 is normally low and goes high if the thermocouple input is open. In order to allow the operation of the
  // open  thermocouple  detector,  T-  must  be  grounded. Make  the  ground  connection  as  close  to  the  GND  pin
  // as possible.
  const bool Plugin_039_SensorAttached = !(rawvalue & 0x0004);

  if (Plugin_039_SensorAttached)
  {
    // Shift RAW value 3 Bits to the right to get the data
    rawvalue >>= 3;

    // Calculate Celsius
    return rawvalue * 0.25f;
  }
  else
  {
    return NAN;
  }
}

float readMax31855(struct EventStruct *event)
{
  union {
      uint8_t messageBuffer[4] = {0x00, 0x00, 0x00, 0x00};
      uint32_t value;
  } u_mB;

  uint8_t CS_pin_no = get_SPI_CS_Pin(event);

  // u_mB.messageBuffer[4] = {0x00, 0x00, 0x00, 0x00};

  // "transfer" 0x0 and read the 32 Bit conversion register from the Chip

  transfer_n_ByteSPI(CS_pin_no, 4, &u_mB.messageBuffer[0]);
  uint16_t rawvalue = u_mB.value;

  // uint32_t rawvalue = read_n_ByteitRegister(CS_pin_no, 0x0000);

  // // Shift MSB 16 Bits to the left
  // rawvalue <<= 16;

  // // "transfer" 0x0 and read the LSB Data from the Chip
  // rawvalue |= read16BitRegister(CS_pin_no, 0x0000);

  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) 
    {
      String log;
      log.reserve(66u);
      log = F("P039 : MAX31855 : RAW - BIN:");
      log += String(rawvalue, BIN);
      log += F(" rawvalue,HEX: ");
      log += String(rawvalue, HEX);
      log += F(" rawvalue,DEC: ");
      log += String(rawvalue);
      log += F(" u_mB.value,DEC: ");
      log += String(u_mB.value, DEC);
      log += F(" u_mB.messageBuffer[],DEC:");
      for (size_t i = 0u; i < 4; i++)
      {
              log += ' ';
              log += String(u_mB.messageBuffer[i], DEC);
      }
      
      addLog(LOG_LEVEL_DEBUG, log);
    }

  # endif // ifndef BUILD_NO_DEBUG


  // FIXME TD-er: This static flag is shared among all instances of this plugin
  static bool sensorFault = false;

  if (sensorFault != ((rawvalue & 0x7) == 0)) {
    // Fault code changed, log them
    sensorFault = ((rawvalue & 0x7) == 0);

    # ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) 
      {
        String log;
        log.reserve(66u);
        log = F("P039 : MAX31855");

        if (!sensorFault) {
          log += F("Fault resolved");
        } else {
          log += F("Fault code:");

          if (rawvalue & 0x01) {
            log += F(" Open (no connection)");
          }

          if (rawvalue & 0x02) {
            log += F(" Short-circuit to GND");
          }

          if (rawvalue & 0x04) {
            log += F(" Short-circuit to Vcc");
          }
        }
        addLog(LOG_LEVEL_DEBUG, log);
      } 

    # endif // ifndef BUILD_NO_DEBUG

  }

  // D16 - This bit reads at 1 when any of the SCV, SCG, or OC faults are active. Default value is 0.
  const bool Plugin_039_SensorAttached = !(rawvalue & 0x00010000);

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
    int temperature = Plugin_039_convert_two_complement(rawvalue, 14);

    // Calculate Celsius
    return temperature * 0.25f;
  }
  else
  {
    // Fault state, thus output no value.
    return NAN;
  }
}

float readMax31856(struct EventStruct *event)
{
  uint8_t CS_pin_no = get_SPI_CS_Pin(event);


  uint32_t registers[MAX31856_NO_REG] = { 0 };

  for (int i = 0u; i < MAX31856_NO_REG; ++i) {
    registers[i] = read8BitRegister(CS_pin_no, i);
  }

  uint32_t rawvalue = registers[MAX31856_LTCBH];
  rawvalue = (rawvalue << 8) | registers[MAX31856_LTCBM];
  rawvalue = (rawvalue << 8) | registers[MAX31856_LTCBL];

  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) 
    {
      String log;
      log.reserve(66u);
      log = F("P039 : MAX31856 :");

      for (int i = 1; i < MAX31856_NO_REG; ++i) {
        log += ' ';
        log += String(registers[i], HEX);
      }
      addLog(LOG_LEVEL_DEBUG, log);
    }

  # endif // ifndef BUILD_NO_DEBUG

  const uint32_t sr = registers[MAX31856_SR];

  # ifndef BUILD_NO_DEBUG

    // FIXME TD-er: This static flag is shared among all instances of this plugin
    static bool sensorFault = false;
 
    sensorFault = (sr != 0); // Set new state

    const bool faultResolved = sensorFault && (sr == 0);

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) 
    {
      if (sensorFault || faultResolved) {
        String log;
        log.reserve(66u);
        log = F("P039 : MAX31856");
        
        if (sensorFault == 0) {
          log += F("Fault resolved");
        } else {
          log += F("Fault :");

          if (sr & 0x01) {
            log += F(" Open (no connection)");
          }

          if (sr & 0x02) {
            log += F(" Over/Under Voltage");
          }

          if (sr & 0x04) {
            log += F(" TC Low");
          }

          if (sr & 0x08) {
            log += F(" TC High");
          }

          if (sr & 0x10) {
            log += F(" CJ Low");
          }

          if (sr & 0x20) {
            log += F(" CJ High");
          }

          if (sr & 0x40) {
            log += F(" TC Range");
          }

          if (sr & 0x80) {
            log += F(" CJ Range");
          }
        }
        addLog(LOG_LEVEL_DEBUG, log);
      }
    }

  # endif // ifndef BUILD_NO_DEBUG

  const bool Plugin_039_SensorAttached = (sr == 0);

  if (Plugin_039_SensorAttached)
  {
    registers[MAX31856_RAWVALUE] >>= 5; // bottom 5 bits are unused
    // We're left with (24 - 5 =) 19 bits
    float temperature = Plugin_039_convert_two_complement(registers[MAX31856_RAWVALUE], 19);

    // Calculate Celsius
    return temperature / 128.0f;
  }
  else
  {
    // Fault state, thus output no value.
    return NAN;
  }
}

int Plugin_039_convert_two_complement(uint32_t value, int nr_bits) {
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

float readMax31865(struct EventStruct *event)
{

  uint8_t registers[MAX31865_NO_REG] = {0};
  uint16_t rawValue = 0u;

  uint8_t CS_pin_no = get_SPI_CS_Pin(event);

  // clear all faults
  clearFaults(CS_pin_no);

  // set frequency filter
  chooseFilterType(CS_pin_no, P039_RTD_FILT_TYPE);

  //activate BIAS short before read, to reduce power consumption
  handleBias(CS_pin_no, true);

  // wait for external capacities to load (min. 10ms -> give 50% adder to "be sure")
  delay(15);

  // configure read access with configuration from web interface
  setConType(CS_pin_no, P039_RTD_CON_TYPE);

  //activate one shot conversion
  startOneShotConversion(CS_pin_no);

  // wait for 100ms -> conversion to be ready
  delay(100);

  // read conversion result
  rawValue = read16BitRegister(CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_RTD_MSB));

  //deactivate BIAS short after read, to reduce power consumption
  handleBias(CS_pin_no, false);


  registers[MAX31865_FAULT] = read8BitRegister(CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_FAULT));


  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG))
    {
      String log;
      log.reserve(66u);

      for (int i = 0u; i < MAX31865_NO_REG; ++i)
      {
        registers[i] = read8BitRegister(CS_pin_no, (MAX31865_READ_ADDR_BASE + i));
      }

      log = F("P039 : MAX31865 :");

      for (int i = 0u; i < MAX31865_NO_REG; ++i)
      {
        log += F(" 0x");
        log += String(registers[i], HEX);
      }

      addLog(LOG_LEVEL_DEBUG, log);
    }
  # endif // ifndef BUILD_NO_DEBUG

  // clear all faults
  clearFaults(CS_pin_no);

  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG))
    {
      if (registers[MAX31865_FAULT])
      {
        String log;
        log.reserve(66u);
        log = F("P039 : MAX31865 : ");

        log += F("Fault : 0x");
        log += String(registers[MAX31865_FAULT], HEX);
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
        addLog(LOG_LEVEL_DEBUG, log);
      }
    }

  # endif // ifndef BUILD_NO_DEBUG

  bool ValueValid = false;

  if (registers[MAX31865_FAULT] == 0x00)
    ValueValid = true;

  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG))
    {
      String log;
      log.reserve(66u);
      log = F("P039 : Temperature :");
      log += F(" registers[MAX31865_FAULT]: ");
      log += String(registers[MAX31865_FAULT], HEX);
      log += F(" ValueValid: ");
      log += String(ValueValid, BIN);
      addLog(LOG_LEVEL_DEBUG, log);
    }

  # endif // ifndef BUILD_NO_DEBUG

  if (ValueValid)
  {
    rawValue >>= 1; // bottom fault bits is unused

    float temperature = Plugin_039_convert_to_temperature(rawValue, getNomResistor(P039_RTD_TYPE), P039_RTD_RES);

    # ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG))
      {
        String log;
        log.reserve(66u);
        log = F("P039 : Temperature :");
        log += F(" rawValue: ");
        log += String(rawValue, DEC);
        log += F(" temperature: ");
        log += String(temperature, DEC);
        log += F(" P039_RTD_TYPE: ");
        log += String(P039_RTD_TYPE, DEC);
        log += F(" P039_RTD_RES: ");
        log += String(P039_RTD_RES, DEC);
        addLog(LOG_LEVEL_DEBUG, log);
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

void clearFaults(uint8_t l_CS_pin_no)
{
  uint8_t l_reg = 0u;

  // read in config register
  l_reg = read8BitRegister(l_CS_pin_no,(MAX31865_READ_ADDR_BASE + MAX31865_CONFIG));

 
  // clear all faults ( write "0" to D2, D3, D5; write "1" to D2)
  l_reg &= ~(MAX31865_SET_ONE_SHOT | MAX31865_FAULT_CTRL_MASK);
  l_reg |= MAX31865_CLEAR_FAULTS;

  // write configuration to MAX31865 to enable VBIAS
  write8BitRegister(l_CS_pin_no, (MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG), l_reg);

}

void handleBias(uint8_t l_CS_pin_no, bool l_active)
{
  uint8_t l_reg = 0u;

  // read in config register
  l_reg = read8BitRegister(l_CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG));

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
  write8BitRegister(l_CS_pin_no, (MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG), l_reg);

}

void chooseFilterType(uint8_t l_CS_pin_no, uint8_t l_filtType)
{
  uint8_t l_reg = 0u;

  // read in config register
  l_reg = read8BitRegister(l_CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG));

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
  write8BitRegister(l_CS_pin_no, (MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG), l_reg);

}

void setConType(uint8_t l_CS_pin_no, uint8_t l_conType)
{
  uint8_t l_reg = 0u;

  // read in config register
  l_reg = read8BitRegister(l_CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG));
 
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
  write8BitRegister(l_CS_pin_no, (MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG), l_reg);

}

void startOneShotConversion(uint8_t l_CS_pin_no)
{
  uint8_t l_reg = 0u;

  // read in config register
  l_reg = read8BitRegister(l_CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG));

  //activate one shot conversion
  l_reg |= MAX31865_SET_ONE_SHOT;

  // write to configuration register
  write8BitRegister(l_CS_pin_no, (MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG), l_reg);

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
float Plugin_039_convert_to_temperature(uint32_t l_rawvalue, float RTDnominal, float refResistor)
{

  #define RTD_A 3.9083e-3f
  #define RTD_B -5.775e-7f

  float Z1, Z2, Z3, Z4, Rt, temp;

  Rt = l_rawvalue;
  Rt /= 32768u;
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

  temp = -242.02f;
  temp += 2.2228f * rpoly;
  rpoly *= Rt; // square
  temp += 2.5859e-3f * rpoly;
  rpoly *= Rt; // ^3
  temp -= 4.8260e-6f * rpoly;
  rpoly *= Rt; // ^4
  temp -= 2.8183e-8f * rpoly;
  rpoly *= Rt; // ^5
  temp += 1.5243e-10f * rpoly;

  return temp;
}

uint16_t getNomResistor(uint8_t l_RType)
{
  uint16_t l_returnValue = 100u;

  switch (l_RType)
  {
    case MAX31865_PT100:
                          l_returnValue = 100u;
                          break;
    case MAX31865_PT1000:
                          l_returnValue = 1000u;
                          break;
    default:
                          l_returnValue = 100u;
                          break;
  }
  return (l_returnValue);
}

float readLM7x(struct EventStruct *event)
{

  float temperature = 0.0f;
  uint16_t device_id = 0u;
  uint16_t rawValue = 0u;

  uint8_t CS_pin_no = get_SPI_CS_Pin(event);

  // operate LM7x devices in polling mode, assuming conversion is ready with every call of this read function ( >=210ms call cycle)
  // this allows usage of multiples generations of LM7x devices, that doe not provde conversion ready information in temperature register

  rawValue = readLM7xRegisters(CS_pin_no, P039_RTD_LM_TYPE, P039_RTD_LM_SHTDWN, &device_id);

  temperature = convertLM7xTemp(rawValue, P039_RTD_LM_TYPE);

  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG))
    {
      String log;
      log.reserve(66u);
      log = F("P039 : LM7x : readLM7x : ");
      log += F(" rawValue: ");
      log += String(rawValue, DEC);
      log += F(" device_id: 0x");
      log += String(device_id, HEX);
      log += F(" temperature: ");
      log += String(temperature, DEC);
      addLog(LOG_LEVEL_DEBUG, log);
    }

  # endif // ifndef BUILD_NO_DEBUG

  return (temperature);
}

float convertLM7xTemp(uint16_t l_rawValue, uint16_t l_LM7xsubtype)
{
  float l_returnValue = 0.0f;
  float l_lsbvalue = 0.0f;
  uint8_t l_noBits = 0u;
  int l_intTemperature = 0;

  switch (l_LM7xsubtype)
  {
    case LM7x_SD70:
      l_rawValue >>= 5;
      l_lsbvalue = 0.25f;
      l_noBits = 11u;
      break;
    case LM7x_SD71:
      l_rawValue >>= 2;
      l_lsbvalue = 0.03125f;
      l_noBits = 14u;
      break;
    case LM7x_SD74:
      l_rawValue >>= 3;
      l_lsbvalue = 0.0625f;
      l_noBits = 13u;
      break;
    case LM7x_SD121:
    case LM7x_SD122:
    case LM7x_SD123:
    case LM7x_SD124:
      l_rawValue >>= 4;
      l_lsbvalue = 0.0625f;
      l_noBits = 12u;
      break;
    case LM7x_SD125:
      l_rawValue >>= 5;
      l_lsbvalue = 0.25f;
      l_noBits = 10u;
      break;
    default: // use lowest resolution as fallback if no device has been configured
      l_rawValue >>= 5;
      l_lsbvalue = 0.25f;
      l_noBits = 11u;
      break;
  }

  l_intTemperature = Plugin_039_convert_two_complement(l_rawValue, l_noBits);

  l_returnValue = l_intTemperature * l_lsbvalue;

  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG))
    {
      String log;
      log.reserve(66u);
      log = F("P039 : LM7x : convertLM7xTemp : ");
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
      addLog(LOG_LEVEL_DEBUG, log);
    }

  # endif // ifndef BUILD_NO_DEBUG

  return (l_returnValue);
}

uint16_t readLM7xRegisters(uint8_t l_CS_pin_no, uint8_t l_LM7xsubType, uint8_t l_runMode, uint16_t *l_device_id)
{
  uint16_t l_returnValue = 0u;
  uint16_t l_mswaitTime = 0u;
 

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
    uint8_t messageBuffer[12] = {0xFF, 0xFF, 0xFF, 0X00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF};

    // send inital 4 bytes to wake the device and start the conversion
    transfer_n_ByteSPI(l_CS_pin_no, 4, &messageBuffer[0] );

    //wait specific ms for conversion to be ready (TI datasheet per devices)
    delay(l_mswaitTime);

    // send remaining 8 bytes to read the device ID and shutdown the device
    transfer_n_ByteSPI(l_CS_pin_no, 8, &messageBuffer[4] );

    //read temperature value (16 Bit)
    l_returnValue = ((messageBuffer[4]<<8) | messageBuffer[5]);

    // read Manufatures/Device ID (16 Bit)
    *(l_device_id) = ((messageBuffer[8]<<8) | messageBuffer[9]);

    // // wakeup device and start conversion
    // // initial read of conversion result is obsolete
    // SPI.transfer16(0xFFFF);

    // // (wakeup device with "all zero2 message in the last 8 bits
    // SPI.transfer16(0xFF00);
 
    // //wait specific ms for conversion to be ready (TI datasheet per devices)
    // delay(l_mswaitTime);

    // //read temperature value (16 Bit)
    // l_returnValue = SPI.transfer16(0x0000);
    // // l_returnValue <<= 8;
    // // l_returnValue = SPI.transfer(0x00);

    // // set device to shutdown with "all one" message in the last 8 bits
    // SPI.transfer16(0xFFFF);
   
    // // read Manufatures/Device ID (16 Bit)
    // *(l_device_id) = SPI.transfer16(0x0000);
    // // *(l_device_id) <<= 8;
    // // *(l_device_id) = SPI.transfer(0x00);

    // // set device to shutdown with "all one" message in the last 8 bits ( maybe redundant, check with test)
    // SPI.transfer16(0xFFFF);
  
  }
  else
  {
    // shutdown mode inactive -> normal background conversion during call cycle
    uint8_t messageBuffer[8] = {0x00, 0x00, 0xFF, 0XFF, 0x00, 0x00, 0x00, 0x00};

    transfer_n_ByteSPI(l_CS_pin_no, 8, &messageBuffer[0] );
    
    //read temperature value (16 Bit)
    l_returnValue = ((messageBuffer[0]<<8) | messageBuffer[1]);
    
    // read Manufatures/Device ID (16 Bit)
    *(l_device_id) = ((messageBuffer[4]<<8) | messageBuffer[5]);


    // l_returnValue = SPI.transfer16(0x0000); //read temperature value (16 Bit)
    // // l_returnValue <<= 8;
    // // l_returnValue = SPI.transfer(0x00);

    // // set device to shutdown
    // SPI.transfer16(0xFFFF);

    // // read Manufatures/Device ID (16 Bit)
    // *(l_device_id) = SPI.transfer16(0x0000);
    // // *(l_device_id) <<= 8;
    // // *(l_device_id) = SPI.transfer(0x00);

    // // start conversion until next read  (8 Bit sufficient)
    // // 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F allowed - else device goes to test mode (not desirable here)
    // SPI.transfer(0x00);
    // // SPI.transfer16(0x0000);
  }

  // // stop communication -> CS high
  // handle_SPI_CS_Pin(l_CS_pin_no, HIGH);

  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG))
    {
      String log;
      log.reserve(66u);
      log = F("P039 : LM7x : readLM7xRegisters : ");
      log += F(" l_returnValue: 0x");
      log += String(l_returnValue, HEX);
      log += F(" l_device_id: 0x");
      log += String(*(l_device_id), HEX);
      addLog(LOG_LEVEL_DEBUG, log);
    }

  # endif // ifndef BUILD_NO_DEBUG

  return (l_returnValue);
}

// POSSIBLE START OF GENERIC SPI HIGH LEVEL FUNCTIONS WITH POTENTIAL OF SYSTEM WIDE RE-USE

/**************************************************************************/
/*!
    @brief generic high level library to access SPI interface from plugins
    with GPIO pin handled as CS - chri.kai.in 2021

    Initial Revision - chri.kai.in 2021

    TODO: c.k.i.: make it generic and carve out to generic _SPI_helper.c library 


/**************************************************************************/



/**************************************************************************/
/*!

    @brief Identifying the CS pin from the event basic data structure
    @param event pointer to the event structure; default GPIO is chosen as GPIO 15

    @returns 

    Initial Revision - chri.kai.in 2021
    
/**************************************************************************/
int get_SPI_CS_Pin(struct EventStruct *event) {  // If no Pin is in Config we use 15 as default -> Hardware Chip Select on ESP8266
  if (CONFIG_PIN1 != 0) {
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

/**************************************************************************/
void init_SPI_CS_Pin (uint8_t l_CS_pin_no) {
  
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

/**************************************************************************/
void handle_SPI_CS_Pin (uint8_t l_CS_pin_no, bool l_state) {
  
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

/**************************************************************************/

void write8BitRegister(uint8_t l_CS_pin_no, uint8_t l_address, uint8_t value)
{
  uint8_t l_messageBuffer[2] = {l_address, value};

  transfer_n_ByteSPI(l_CS_pin_no, 2, l_messageBuffer);

   # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG))
    {
      String log;
      log.reserve(66u);
      log = F("P039 : SPI : write8BitRegister : ");
      log += F("l_address: 0x");
      log += String(l_address, HEX);
      log += F(" value: 0x");
      log += String(value, HEX);
      addLog(LOG_LEVEL_DEBUG, log);
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

/**************************************************************************/

void write16BitRegisters(uint8_t l_CS_pin_no, uint8_t l_address, uint16_t value)
{
  uint8_t l_messageBuffer[3] = {l_address, (uint8_t) (value >> 8), (uint8_t) (value)};

  transfer_n_ByteSPI(l_CS_pin_no, 3, l_messageBuffer);

  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG))
    {
      String log;
      log.reserve(66u);
      log = F("P039 : SPI : write16BitRegister : ");
      log += F("l_address: 0x");
      log += String(l_address, HEX);
      log += F(" value: 0x");
      log += String(value, HEX);
      addLog(LOG_LEVEL_DEBUG, log);
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

/**************************************************************************/

uint8_t read8BitRegister(uint8_t l_CS_pin_no, uint8_t l_address)
{
  uint8_t l_messageBuffer[2] = {l_address, 0x00};

  transfer_n_ByteSPI(l_CS_pin_no, 2, l_messageBuffer );

  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG))
    {
      String log;
      log.reserve(66u);
      log = F("P039 : SPI : read8BitRegister : ");
      log += F("l_address: 0x");
      log += String(l_address, HEX);
      log += F(" returnvalue: 0x");
      log += String(l_messageBuffer[1], HEX);
      addLog(LOG_LEVEL_DEBUG, log);
    }

  # endif // ifndef BUILD_NO_DEBUG

  return (l_messageBuffer[1]);
}

/**************************************************************************/
/*!
    @brief write 16 bits to adress l_address on the SPI interface, handling a GPIO CS
    @param l_CS_pin_no the GPIO pin number used as CS
    @param l_address the register addess of the connected SPI device

    @returns the unsigned 16 Bit message read from l_address
    
    Initial Revision - chri.kai.in 2021

/**************************************************************************/

uint16_t read16BitRegister(uint8_t l_CS_pin_no, uint8_t l_address)
{
  uint8_t l_messageBuffer[3] = {l_address, 0x00 , 0x00};
  uint16_t l_returnValue;

  transfer_n_ByteSPI(l_CS_pin_no, 2, l_messageBuffer );
  l_returnValue = ((l_messageBuffer[1]<<8) | l_messageBuffer[2] );

  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG))
    {
      String log;
      log.reserve(66u);
      log = F("P039 : SPI : read16BitRegister : ");
      log += F("l_address: 0x");
      log += String(l_address, HEX);
      log += F(" l_returnValue: 0x");
      log += String(l_returnValue, HEX);
      addLog(LOG_LEVEL_DEBUG, log);
    }

  # endif // ifndef BUILD_NO_DEBUG

  return (l_returnValue);
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

/**************************************************************************/

void transfer_n_ByteSPI(uint8_t l_CS_pin_no, uint8_t l_noBytesToSend, uint8_t* l_inoutMessageBuffer )
{
 

   // activate communication -> CS low
  handle_SPI_CS_Pin(l_CS_pin_no, LOW);

  for (size_t i = 0u; i < l_noBytesToSend; i++)
  {
    l_inoutMessageBuffer[i] = SPI.transfer(l_inoutMessageBuffer[i]);
  }

  // stop communication -> CS high
  handle_SPI_CS_Pin(l_CS_pin_no, HIGH);

  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG))
    {
      String log;
      log.reserve(66u);
      log = F("P039 : SPI : transfer_n_ByteSPI : ");
      for (int i = 0; i < l_noBytesToSend; ++i)
      {
        log += F(" 0x");
        log += String(l_inoutMessageBuffer[i], HEX);
      }
      addLog(LOG_LEVEL_DEBUG, log);
    }

  # endif // ifndef BUILD_NO_DEBUG

}

#endif // USES_P039
