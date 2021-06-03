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

#include <SPI.h>
// #include <Misc.h>
#include "src/PluginStructs/P039_data_struct.h"


// // plugin-local quick activation of debug messages
// #ifdef BUILD_NO_DEBUG
//   #undef BUILD_NO_DEBUG
// #endif


#define MAX31865_RD_ADDRESS(n)   (MAX31865_READ_ADDR_BASE + n)
#define MAX31865_WR_ADDRESS(n)   (MAX31865_WRITE_ADDR_BASE + n)

# define PLUGIN_039
# define PLUGIN_ID_039         39
# define PLUGIN_NAME_039       "Environment - Thermosensors"
# define PLUGIN_VALUENAME1_039 "Temperature"

#define P039_SET                true
#define P039_RESET              false

// typically 500ns of wating on positive/negative edge of CS should be enough ( -> datasheet); to make sure we cover a lot of devices we spend 1ms
// FIX 2021-05-05: review of all covered device datasheets showed 2µs is more than enough; review with every newly added device
#define P039_CS_Delay()             delayMicroseconds(2u)

#define P039_MAX_TYPE               PCONFIG(0)
#define P039_TC_TYPE                PCONFIG(1)
#define P039_FAM_TYPE               PCONFIG(2)
#define P039_RTD_TYPE               PCONFIG(3)
#define P039_CONFIG_4               PCONFIG(4)
#define P039_RTD_FILT_TYPE          PCONFIG(5)
#define P039_RTD_LM_TYPE            PCONFIG(6)
#define P039_RTD_LM_SHTDWN          PCONFIG(7)
#define P039_RTD_RES                PCONFIG_LONG(0)
#define P039_RTD_OFFSET             PCONFIG_FLOAT(0)

#define P039_TC                     0u
#define P039_RTD                    1u

# define P039_MAX6675               1u
# define P039_MAX31855              2u
# define P039_MAX31856              3u
# define P039_MAX31865              4u
# define P039_LM7x                  5u

// MAX 6675 related defines

// bit masks to identify failures for MAX 6675
#define MAX6675_TC_DEVID            0x0002u
#define MAX6675_TC_OC               0x0004u

// MAX 31855 related defines

// bit masks to identify failures for MAX 31855
#define MAX31855_TC_OC              0x00000001u
#define MAX31855_TC_SC              0x00000002u
#define MAX31855_TC_SCVCC           0x00000004u
#define MAX31855_TC_GENFLT          0x00010000u


// MAX 31856 related defines

// base address for read/write acces to MAX 31856
#define MAX31856_READ_ADDR_BASE       0x00u
#define MAX31856_WRITE_ADDR_BASE      0x80u

// register offset values for MAX 31856
#define MAX31856_CR0                0u
#define MAX31856_CR1                1u
#define MAX31856_MASK               2u
#define MAX31856_CJHF               3u
#define MAX31856_CJLF               4u
#define MAX31856_LTHFTH             5u
#define MAX31856_LTHFTL             6u
#define MAX31856_LTLFTH             7u
#define MAX31856_LTLFTL             8u
#define MAX31856_CJTO               9u
#define MAX31856_CJTH               10u
#define MAX31856_CJTL               11u
#define MAX31856_LTCBH              12u
#define MAX31856_LTCBM              13u
#define MAX31856_LTCBL              14u
#define MAX31856_SR                 15u

#define MAX31856_NO_REG             16u

// bit masks to identify failures for MAX 31856
#define MAX31856_TC_OC              0x01u
#define MAX31856_TC_OVUV            0x02u
#define MAX31856_TC_TCLOW           0x04u
#define MAX31856_TC_TCLHIGH         0x08u
#define MAX31856_TC_CJLOW           0x10u
#define MAX31856_TC_CJHIGH          0x20u
#define MAX31856_TC_TCRANGE         0x40u
#define MAX31856_TC_CJRANGE         0x80u

// bit masks for access of configuration bits
#define MAX31856_SET_50HZ           0x01u
#define MAX31856_CLEAR_FAULTS       0x02u
#define MAX31856_FLT_ISR_MODE       0x04u
#define MAX31856_CJ_SENS_DISABLE    0x08u
#define MAX31856_FAULT_CTRL_MASK    0x30u
#define MAX31856_SET_ONE_SHOT       0x40u
#define MAX31856_SET_CONV_AUTO      0x80u



// RTD related defines

// MAX 31865 related defines

// waiting time until "in sequence" conversion is ready (-> used in case device is set to shutdown in between call cycles)
// typically 70ms should be fine, according to datasheet maximum -> 66ms - give a little adder to "be sure" conversion is done
// alternatively ONE SHOT bit could be polled (system/SPI bus load !)
#define MAX31865_CONVERSION_TIME    70ul
#define MAX31865_BIAS_WAIT_TIME     10ul

// MAX 31865 Main States
#define MAX31865_INIT_STATE         0u
#define MAX31865_BIAS_ON_STATE      1u
#define MAX31865_RD_STATE           2u
#define MAX31865_RDY_STATE          3u

// sensor type
#define MAX31865_PT100              0u
#define MAX31865_PT1000             1u

// base address for read/write acces to MAX 31865
#define MAX31865_READ_ADDR_BASE         0x00u
#define MAX31865_WRITE_ADDR_BASE        0x80u

// register offset values for MAX 31865
#define MAX31865_CONFIG                 0u
#define MAX31865_RTD_MSB                1u
#define MAX31865_RTD_LSB                2u
#define MAX31865_HFT_MSB                3u
#define MAX31865_HFT_LSB                4u
#define MAX31865_LFT_MSB                5u
#define MAX31865_LFT_LSB                6u
#define MAX31865_FAULT                  7u

// total number of registers in MAX 31865
#define MAX31865_NO_REG                 8u

// bit masks to identify failures for MAX 31865
#define MAX31865_FAULT_HIGHTHRESH   0x80u
#define MAX31865_FAULT_LOWTHRESH    0x40u
#define MAX31865_FAULT_REFINLOW     0x20u
#define MAX31865_FAULT_REFINHIGH    0x10u
#define MAX31865_FAULT_RTDINLOW     0x08u
#define MAX31865_FAULT_OVUV         0x04u

// bit masks for access of configuration bits
#define MAX31865_SET_50HZ           0x01u
#define MAX31865_CLEAR_FAULTS       0x02u
#define MAX31865_FAULT_CTRL_MASK    0x0Cu
#define MAX31865_SET_3WIRE          0x10u
#define MAX31865_SET_ONE_SHOT       0x20u
#define MAX31865_SET_CONV_AUTO      0x40u
#define MAX31865_SET_VBIAS_ON       0x80u

//LM7x related defines

// LM7x subtype defines
#define LM7x_SD70                   0x00u
#define LM7x_SD71                   0x01u
#define LM7x_SD74                   0x04u
#define LM7x_SD121                  0x05u
#define LM7x_SD122                  0x06u
#define LM7x_SD123                  0x07u
#define LM7x_SD124                  0x08u
#define LM7x_SD125                  0x09u

// bit masks for access of configuration bits
#define LM7x_CONV_RDY               0x02u


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
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P039_data_struct());
      P039_data_struct *P039_data = static_cast<P039_data_struct *>(getPluginTaskData(event->TaskIndex));

      uint8_t CS_pin_no = get_SPI_CS_Pin(event);
   
      // set the slaveSelectPin as an output:
      init_SPI_CS_Pin(CS_pin_no);

      // initialize SPI:
      SPI.setHwCs(false);
      SPI.begin();

      if (P039_MAX_TYPE == P039_MAX6675) {

        // ensure MODE3 access to SPI device
        SPI.setDataMode(SPI_MODE3);
        // SPI.setBitOrder(MSBFIRST);

      }

      if (P039_MAX_TYPE == P039_MAX31855) {

        // ensure MODE3 access to SPI device
        SPI.setDataMode(SPI_MODE3);
        // SPI.setBitOrder(MSBFIRST);

        if(nullptr != P039_data){
          // FIXED: c.k.i. : moved static fault flag to instance data structure
          P039_data->sensorFault = false;
        }

      }


      if (P039_MAX_TYPE == P039_MAX31856) {


        // ensure MODE3 access to SPI device
        SPI.setDataMode(SPI_MODE3);

        // init string - content accoring to inital implementation of P039 - MAX31856 read function
        // write to Adress 0x80
        // activate 50Hz filter in CR0, choose averaging and TC type from configuration in CR1, activate OV/UV/OC faults, write defaults to CJHF, CJLF, LTHFTH, LTHFTL, LTLFTH, LTLFTL, CJTO
        uint8_t sendBuffer[11] = {0x80, static_cast<uint8_t> (P039_RTD_FILT_TYPE), static_cast<uint8_t> ((P039_CONFIG_4 << 4) | P039_TC_TYPE), 0xFC, 0x7F, 0xC0, 0x7F, 0xFF, 0x80, 0x00, 0x00 };

        transfer_n_ByteSPI(CS_pin_no, 11, &sendBuffer[0] );

        if(nullptr != P039_data){
          // FIXED: c.k.i. : moved static fault flag to instance data structure
          P039_data->sensorFault = false;
        }

        // start on shot conversion for upcoming read cycle
        change8BitRegister(CS_pin_no, (MAX31856_READ_ADDR_BASE + MAX31856_CR0),(MAX31856_WRITE_ADDR_BASE + MAX31856_CR0), MAX31856_SET_ONE_SHOT, P039_SET );

     }



      if(P039_MAX_TYPE == P039_MAX31865){

        // two step initialization buffer 
        uint8_t initSendBufferHFTH[3] = {(MAX31865_WRITE_ADDR_BASE + MAX31865_HFT_MSB), 0xFF, 0xFF};
        uint8_t initSendBufferLFTH[3] = {(MAX31865_WRITE_ADDR_BASE + MAX31865_HFT_MSB), 0xFF, 0xFF};

        // ensure MODE3 access to SPI device
        SPI.setDataMode(SPI_MODE3);

        // write intially 0x00 to CONFIG register
        write8BitRegister(CS_pin_no, (MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG), 0x00u);

        // activate 50Hz filter, clear all faults, no auto conversion, no conversion started
        change8BitRegister(CS_pin_no, MAX31865_RD_ADDRESS(MAX31865_CONFIG),MAX31865_WR_ADDRESS(MAX31865_CONFIG), MAX31865_SET_50HZ, static_cast<bool>(P039_RTD_FILT_TYPE) );

        // configure 2/4-wire sensor connection as default
        MAX31865_setConType(CS_pin_no, P039_CONFIG_4);
        
        // set HighFault Threshold
        transfer_n_ByteSPI(CS_pin_no, 3, &initSendBufferHFTH[0]);
        
        // set LowFault Threshold
        transfer_n_ByteSPI(CS_pin_no, 3, &initSendBufferLFTH[0]);

        // clear all faults
        MAX31865_clearFaults(CS_pin_no);

        //activate BIAS short before read, to reduce power consumption
        change8BitRegister(CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG),(MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG), MAX31865_SET_VBIAS_ON, P039_SET );

        if(nullptr != P039_data){
          // save current timer for next calculation
          P039_data->timer = millis();

          // start time to follow up on BIAS activation before starting the conversion
          // and start conversion sequence via TIMER API

          Scheduler.setPluginTaskTimer(MAX31865_BIAS_WAIT_TIME, event->TaskIndex, MAX31865_BIAS_ON_STATE);
        }

      }

      if (P039_MAX_TYPE == P039_LM7x)
      {
        // ensure MODE3 access to SPI device
        SPI.setDataMode(SPI_MODE3);

        // TODO: c.k.i.: more detailed inits depending on the sub devices expected , e.g. TMP 122/124
      }

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log;
        if((log.reserve(80u))) { // reserve value derived from example log file
          log = F("P039 : ");                            // 7 char
          log += getTaskDeviceName(event->TaskIndex);    // 41 char
          log += F(" : SPI Init - DONE" );               // 18 char
          addLog(LOG_LEVEL_INFO, log);
        }
      }

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
        const __FlashStringHelper * Foptions[2] = {F("Thermocouple"), F("RTD")};
        const int FoptionValues[2] = {P039_TC, P039_RTD};
        addFormSelector(F("Sensor Family Type"), F("P039_famtype"), 2, Foptions, FoptionValues, family, true); // auto reload activated
        addFormNote(F("Set sensor family of connected sensor - thermocouple or RTD."));
      }

      const byte choice = P039_MAX_TYPE;

      if (family == P039_TC){

        {
          addFormSubHeader(F("Device Type Settings"));
        }

        {
          const __FlashStringHelper * options[3]      = {   F("MAX 6675"), F("MAX 31855"), F("MAX 31856") };
          const int    optionValues[3] = { P039_MAX6675, P039_MAX31855, P039_MAX31856 };
          addFormSelector(F("Adapter IC"), F("P039_maxtype"), 3, options, optionValues, choice, true); // auto reload activated
          addFormNote(F("Set adapter IC used."));
        }
    
        if (choice == P039_MAX31856) {
          {
            addFormSubHeader(F("Device Settings"));
          }
          {
            addFormNote(F("Set Thermocouple type for MAX31856"));
            const __FlashStringHelper * Toptions[10]      = { F("B"), F("E"), F("J"), F("K"), F("N"), F("R"), F("S"), F("T"), F("VM8"), F("VM32") };

            // 2021-05-17: c.k.i.: values are directly written to device register for configuration, therefore no linear values are used here
            // MAX 31856 datasheet (page 20):
            //    Thermocouple Type
            //    0000 = B Type
            //    0001 = E Type
            //    0010 = J Type
            //    0011 = K Type (default)
            //    0100 = N Type
            //    0101 = R Type
            //    0110 = S Type
            //    0111 = T Type
            //    10xx = Voltage Mode, Gain = 8. Code = 8 x 1.6 x 217 x VIN
            //    11xx = Voltage Mode, Gain = 32. Code = 32 x 1.6 x 217 x VIN
            //    Where Code is 19 bit signed number from TC registers and VIN is thermocouple input voltage

            const int    ToptionValues[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 12};
            addFormSelector(F("Thermocouple type"), F("P039_tctype"), 10, Toptions, ToptionValues, P039_TC_TYPE);
          }
          {
            const __FlashStringHelper * Coptions[5] = {F("1"), F("2"), F("4"), F("8"), F("16")};
            const int CoptionValues[5] = {0, 1, 2, 3, 4};
            addFormSelector(F("Averaging"), F("P039_contype"), 5, Coptions, CoptionValues, P039_CONFIG_4);
            addUnit(F("sample(s)"));
            addFormNote(F("Set Averaging Type for MAX31856"));
          }
          {
            const __FlashStringHelper * FToptions[2] = {F("60"), F("50")};
            const int FToptionValues[2] = {0, 1};
            addFormSelector(F("Supply Frequency Filter"), F("P039_filttype"), 2, FToptions, FToptionValues, P039_RTD_FILT_TYPE);
            addUnit(F("Hz"));
            addFormNote(F("Set filter frequency for supply voltage. Choose appropriate to your power net frequency (50/60 Hz)"));
          }
        }
      }
      else {
        {
         addFormSubHeader(F("Device Type Settings"));
        }

        {
          const __FlashStringHelper * TPoptions[2] = {F("MAX 31865"), F("LM7x")};
          const int TPoptionValues[2] = {P039_MAX31865, P039_LM7x};
          addFormSelector(F("Adapter IC"), F("P039_maxtype"), 2, TPoptions, TPoptionValues, choice, true); // auto reload activated
          addFormNote(F("Set used RTD Converter Module. Currently only MAX31865 is fully supported. LM7x derivatives are untested and experimental."));
        }

       

        if (choice == P039_MAX31865)
        {
          {
            addFormSubHeader(F("Device Settings"));
          }
          {
            const __FlashStringHelper * PToptions[2] = {F("PT100"), F("PT1000")};
            const int PToptionValues[2] = {MAX31865_PT100, MAX31865_PT1000};
            addFormSelector(F("Resistor Type"), F("P039_rtdtype"), 2, PToptions, PToptionValues, P039_RTD_TYPE);
            addFormNote(F("Set Resistor Type for MAX31865"));
          }
          {
            const __FlashStringHelper * Coptions[2] = {F("2-/4"), F("3")};
            const int CoptionValues[2] = {0, 1};
            addFormSelector(F("Connection Type"), F("P039_contype"), 2, Coptions, CoptionValues, P039_CONFIG_4);
            addUnit(F("wire"));
            addFormNote(F("Set Connection Type for MAX31865"));
          }
          {
            const __FlashStringHelper * FToptions[2] = {F("60"), F("50")};
            const int FToptionValues[2] = {0, 1};
            addFormSelector(F("Supply Frequency Filter"), F("P039_filttype"), 2, FToptions, FToptionValues, P039_RTD_FILT_TYPE);
            addUnit(F("Hz"));
            addFormNote(F("Set filter frequency for supply voltage. Choose appropriate to your power net frequency (50/60 Hz)"));
          }
          {
            addFormNumericBox(F("Reference Resistor"), F("P039_res"), P039_RTD_RES, 0);
            addUnit(F("Ohm"));
            addFormNote(F("Set reference resistor for MAX31865. PT100: typically 430 [OHM]; PT1000: typically 4300 [OHM]"));
          }
          {
            addFormFloatNumberBox(F("Offset"), F("P039_offset"), P039_RTD_OFFSET, -50.0f, 50.0f, 2, 0.01f);
            addUnit(F("K"));            
            addFormNote(F("Set Offset [K] for MAX31865. Valid values: [-50.0...50.0 K], min. stepsize: [0.01]"));
          }
        }

        if (choice == P039_LM7x)
        {
          {
            addFormSubHeader(F("Device Settings"));
          }

          {
            const __FlashStringHelper * PToptions[8] = {F("LM70"), F("LM71"), F("LM74"), F("TMP121"), F("TMP122"), F("TMP123"), F("TMP124"), F("TMP125")};
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
      P039_CONFIG_4 = getFormItemInt(F("P039_contype"));
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

      if (Plugin_039_Celsius != NAN)
      {
        UserVar[event->BaseVarIndex] = Plugin_039_Celsius;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log;
          if((log.reserve(95u))) { // reserve value derived from example log file
            log = F("P039 : ");                                                         // 7 char
            log += getTaskDeviceName(event->TaskIndex);                                 // 41 char
            log += F(" : ");                                                            // 3 char
          
            for (uint8_t i = 0; i < getValueCountForTask(event->TaskIndex);i++)
            {
              log += getTaskValueName(event->TaskIndex, i);                             // 41 char
              log += F(": ");                                                           // 2 char
              log += formatUserVarNoCheck(event->TaskIndex, i);                         //  char 
              log += ' ';                                                               // 1 char
            }
            addLog(LOG_LEVEL_INFO, log);
          }
        }
        success = true;
      }
      else
      {
        UserVar[event->BaseVarIndex]     = NAN;
        UserVar[event->BaseVarIndex + 1] = NAN;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log;
          if((log.reserve(80u))) { // reserve value derived from example log file
            log = F("P039 :  ");                                                      // 7 char
            log += getTaskDeviceName(event->TaskIndex);                               // 41 char
            log += F(" : ");                                                          // 3 char
            log += F("No Sensor attached !");                                         // 20 char
            addLog(LOG_LEVEL_INFO, log);
          }
        }
        success = false;
      }

      break;
    }

    case PLUGIN_TIMER_IN: 
    {
      P039_data_struct *P039_data = static_cast<P039_data_struct *>(getPluginTaskData(event->TaskIndex));

      uint8_t CS_pin_no = get_SPI_CS_Pin(event);

      // Get the MAX Type (6675 / 31855 / 31856)
      byte MaxType = P039_MAX_TYPE;

      switch (MaxType) 
      {
        case P039_MAX31865:
        {
          if((nullptr != P039_data)){
            switch(event->Par1)
              {
                case MAX31865_BIAS_ON_STATE:
                {
                 
                  # ifndef BUILD_NO_DEBUG
                    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {

                      // calc delta since last call
                      long delta = timePassedSince(P039_data->timer);

                      // save current timer for next calculation
                      P039_data->timer = millis();


                      String log;
                      if((log.reserve(120u))) { // reserve value derived from example log file
                        log = F("P039 : ");                                   // 7 char
                        log += getTaskDeviceName(event->TaskIndex);           // 41 char
                        log += F(" : ");                                      // 3 char
                        log += F("current state: MAX31865_BIAS_ON_STATE");    // 37 char
                        log += F("; delta: ");                                // 9 char
                        log += String(delta, DEC);                            // 4 char
                        log += F(" ms");                                      // 3 char
                        addLog(LOG_LEVEL_DEBUG, log);
                      }
                    }
                  # endif // ifndef BUILD_NO_DEBUG

                  //activate one shot conversion
                  change8BitRegister(CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG),(MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG), MAX31865_SET_ONE_SHOT, P039_SET );

                  // set next state in sequence -> READ STATE
                  // start time to follow up on conversion and read the conversion result
                  P039_data->convReady = false;
                  Scheduler.setPluginTaskTimer(MAX31865_CONVERSION_TIME, event->TaskIndex, MAX31865_RD_STATE);
                  
                  # ifndef BUILD_NO_DEBUG
                    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {

                      String log;
                      if((log.reserve(90u))) { // reserve value derived from example log file
                        log = F("P039 : ");                                   // 7 char
                        log += getTaskDeviceName(event->TaskIndex);           // 41 char
                        log += F(" : ");                                      // 3 char
                        log += F("Next State: ");                             // 12 char
                        log += String(event->Par1, DEC);                      // 4 char
                        addLog(LOG_LEVEL_DEBUG, log);
                      }
                    }
                  # endif // ifndef BUILD_NO_DEBUG
                  
                  break;
                }
                case MAX31865_RD_STATE:
                {

                  # ifndef BUILD_NO_DEBUG
                    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
                      // calc delta since last call
                      long delta = timePassedSince(P039_data->timer);

                      // save current timer for next calculation
                      P039_data->timer = millis();

                      String log;
                      if((log.reserve(120u))) { // reserve value derived from example log file
                        log = F("P039 : ");                             // 7 char
                        log += getTaskDeviceName(event->TaskIndex);     // 41 char ( max length of task device name)
                        log += F(" : ");                                // 3 char
                        log += F("current state: MAX31865_RD_STATE");   // 33 char
                        log += F("; delta: ");                          // 9 char
                        log += String(delta, DEC);                      // 4 char - more than 1000ms delta will not occur
                        log += F(" ms");                                // 2 char   
                        addLog(LOG_LEVEL_DEBUG, log);
                      }
                    }
                  # endif // ifndef BUILD_NO_DEBUG

                  // read conversion result
                  P039_data->conversionResult = read16BitRegister(CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_RTD_MSB));

                  // deactivate BIAS short after read, to reduce power consumption
                  change8BitRegister(CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG),(MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG), MAX31865_SET_VBIAS_ON, P039_RESET );
                                  
                  //read fault register to get a full picture
                  P039_data->deviceFaults = read8BitRegister(CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_FAULT));

                  // mark conversion as ready
                  P039_data->convReady = true;

                  # ifndef BUILD_NO_DEBUG
                    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {

                        String log;
                        if((log.reserve(170u))) { // reserve value derived from example log file
                          log = F("P039 : ");                                // 7 char
                          log += getTaskDeviceName(event->TaskIndex);        // 41 char ( max length of task device name + 1)
                          log += F(" : ");                                   // 3 char
                          log += F("P039_data->conversionResult: ");       // 30 char
                          log += formatToHex_decimal(P039_data->conversionResult);   // 11 char
                          log += F("; P039_data->deviceFaults: ");         // 27 char
                          log += formatToHex_decimal(P039_data->deviceFaults);       // 9 char
                          log += F("; Next State: ");                        // 13 char
                          log += String(event->Par1, DEC);                   // 4 char
                          addLog(LOG_LEVEL_DEBUG, log);
                      }

                    }
                  # endif // ifndef BUILD_NO_DEBUG

 
                  break;
                }
                case MAX31865_INIT_STATE:
                default:
                {
                  // clear all faults
                  MAX31865_clearFaults(CS_pin_no);

                  //activate BIAS short before read, to reduce power consumption
                  change8BitRegister(CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG),(MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG), MAX31865_SET_VBIAS_ON, P039_SET );


                  # ifndef BUILD_NO_DEBUG
                    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
                      String log;
                      if((log.reserve(140u))) { // reserve value derived from example log file
                        log = F("P039 : ");                                         // 7 char
                        log += getTaskDeviceName(event->TaskIndex);                 // 41 char
                        log += F(" : ");                                            // 3 char
                        log += F("current state: MAX31865_INIT_STATE, default;");   // many char - 44
                        log += F(" next state: MAX31865_BIAS_ON_STATE");            // a little less char - 35
                        addLog(LOG_LEVEL_DEBUG, log);
                      }
                      
                      // save current timer for next calculation
                      P039_data->timer = millis();

                    }
                  # endif // ifndef BUILD_NO_DEBUG

                  // start time to follow up on BIAS activation before starting the conversion
                  // and start conversion sequence via TIMER API
                  // set next state in sequence -> BIAS ON STATE

                  Scheduler.setPluginTaskTimer(MAX31865_BIAS_WAIT_TIME, event->TaskIndex, MAX31865_BIAS_ON_STATE);


                  break;
                }
              }
            }
          break;
        }
        default:
        {
          break;
        }
       }
      
      success = true;
      break; 

    } 
  }
  return success;
}

float readMax6675(struct EventStruct *event)
{
  uint8_t CS_pin_no = get_SPI_CS_Pin(event);

  uint8_t messageBuffer[2] = {0};
  uint16_t rawvalue = 0u;


  // "transfer" 2 bytes to SPI to get 16 Bit return value
    transfer_n_ByteSPI(CS_pin_no, 2, &messageBuffer[0]);
  
  // merge 16Bit return value from messageBuffer
  rawvalue = ((messageBuffer[0] << 8) | messageBuffer[1]);

  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) 
    {
      String log;
      if((log.reserve(130u))) { // reserve value derived from example log file
        log = F("P039 : MAX6675 : RAW - BIN: ");       // 27 char
        log += String(rawvalue, BIN);                  // 18 char
        log += F(" HEX: ");                          // 5 char
        log += formatToHex(rawvalue);                  // 4 char
        log += F(" DEC: ");                            // 5 char
        log += String(rawvalue);                       // 5 char
        log += F(" MSB: ");                          // 5 char
        log += formatToHex_decimal(messageBuffer[0]);          // 9 char 
        log += F(" LSB: ");                          // 5 char
        log += formatToHex_decimal(messageBuffer[1]);          // 9 char
        addLog(LOG_LEVEL_DEBUG, log);
      }
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
    return (rawvalue * 0.25f);
  }
  else
  {
    return NAN;
  }
}

float readMax31855(struct EventStruct *event)
{
  P039_data_struct *P039_data = static_cast<P039_data_struct *>(getPluginTaskData(event->TaskIndex));

  uint8_t messageBuffer[4] = {0};

  uint8_t CS_pin_no = get_SPI_CS_Pin(event);

  // "transfer" 0x0 and read the 32 Bit conversion register from the Chip
  transfer_n_ByteSPI(CS_pin_no, 4, &messageBuffer[0]);

  // merge rawvalue from 4 bytes of messageBuffer
  uint32_t rawvalue = ((static_cast<uint32_t> (messageBuffer[0]) << 24) | (static_cast<uint32_t> (messageBuffer[1]) << 16) | (static_cast<uint32_t>(messageBuffer[2]) << 8) | static_cast<uint32_t>(messageBuffer[3]));


  
  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) 
    {
      String log;
      if((log.reserve(200u))) { // reserve value derived from example log file
        log = F("P039 : MAX31855 : RAW - BIN: ");      // 35 char
        log += String(rawvalue, BIN);                  // 16 char
        log += F(" rawvalue,HEX: ");                   // 15 char    
        log += formatToHex(rawvalue);                  // 4 char
        log += F(" rawvalue,DEC: ");                   // 15 char
        log += String(rawvalue, DEC);                  // 5 char
        log += F(" messageBuffer[],HEX:");             // 21 char
        for (size_t i = 0u; i < 4; i++)
        {
                log += ' ';                                 // 1 char
                log += formatToHex_decimal(messageBuffer[i]);  // 9 char
        }
        addLog(LOG_LEVEL_DEBUG, log);
      }
    }

  # endif // ifndef BUILD_NO_DEBUG

  if(nullptr != P039_data){

      // FIXED: c.k.i. : moved static fault flag to instance data structure

      // check for fault flags in LSB of 32 Bit messageBuffer
      if (P039_data->sensorFault != ((rawvalue & (MAX31855_TC_SCVCC | MAX31855_TC_SC | MAX31855_TC_OC)) == 0)) {
        // Fault code changed, log them
        P039_data->sensorFault = ((rawvalue & (MAX31855_TC_SCVCC | MAX31855_TC_SC | MAX31855_TC_OC)) == 0);

         if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) 
          {
            String log;
            if((log.reserve(120u))) { // reserve value derived from example log file
              log = F("P039 : MAX31855 : "); 

              if ((P039_data->sensorFault)) {
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
              addLog(LOG_LEVEL_DEBUG_MORE, log);
            }
          } 


      }

      # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) 
        {
          String log;
          if((log.reserve(120u))) { // reserve value derived from example log file
            log = F("P039 : MAX31855 : ");    
            log += F("rawvalue: ");
            log += formatToHex_decimal(rawvalue);
            log += F(" P039_data->sensorFault: ");
            log += formatToHex_decimal(P039_data->sensorFault);
            addLog(LOG_LEVEL_DEBUG, log);
          }
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
    int temperature = Plugin_039_convert_two_complement(rawvalue, 14);

    // Calculate Celsius
    return (temperature * 0.25f);
  }
  else
  {
    // Fault state, thus output no value.
    return NAN;
  }
}

float readMax31856(struct EventStruct *event)
{

  P039_data_struct *P039_data = static_cast<P039_data_struct *>(getPluginTaskData(event->TaskIndex));
  
  uint8_t CS_pin_no = get_SPI_CS_Pin(event);


  uint8_t registers[MAX31856_NO_REG] = { 0 };
  uint8_t messageBuffer[MAX31856_NO_REG + 1] = { 0 };

  messageBuffer[0] = MAX31856_READ_ADDR_BASE;

  // "transfer" 0x0 starting at address 0x00 and read the all registers from the Chip
  transfer_n_ByteSPI(CS_pin_no, (MAX31856_NO_REG + 1), &messageBuffer[0]);

  // transfer data from messageBuffer and get rid of initial address byte
  for (uint8_t i = 0u; i < MAX31856_NO_REG; ++i) {
    registers[i] = messageBuffer[i+1];
  }

  // configure device for next conversion
  // activate frequency filter according to configuration
  change8BitRegister(CS_pin_no, (MAX31856_READ_ADDR_BASE + MAX31856_CR0),(MAX31856_WRITE_ADDR_BASE + MAX31856_CR0), MAX31856_SET_50HZ, static_cast<bool>(P039_RTD_FILT_TYPE) );

  // set averaging and TC type
  write8BitRegister(CS_pin_no, (MAX31856_WRITE_ADDR_BASE + MAX31856_CR1), static_cast<uint8_t> ((P039_CONFIG_4 << 4) | P039_TC_TYPE));
  

  // start on shot conversion for next read cycle
  change8BitRegister(CS_pin_no, (MAX31856_READ_ADDR_BASE + MAX31856_CR0),(MAX31856_WRITE_ADDR_BASE + MAX31856_CR0), MAX31856_SET_ONE_SHOT, P039_SET );


  // now derive raw value from respective registers
  uint32_t rawvalue = static_cast<uint32_t>(registers[MAX31856_LTCBH]);
  rawvalue = (rawvalue << 8) | static_cast<uint32_t>(registers[MAX31856_LTCBM]);
  rawvalue = (rawvalue << 8) | static_cast<uint32_t>(registers[MAX31856_LTCBL]);

  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) 
    {
      String log;
      if((log.reserve(210u))) { // reserve value derived from example log file
        log = F("P039 : MAX31856 :");

        for (uint8_t i = 0; i < MAX31856_NO_REG; ++i) {
          log += ' ';
          log += formatToHex_decimal(registers[i]);
        }
        log+= F(" rawvalue: ");
        log+= formatToHex_decimal(rawvalue);
        addLog(LOG_LEVEL_DEBUG, log);
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
  if((nullptr != P039_data)){

    // P039_data->sensorFault = false;

    P039_data->sensorFault = (sr != 0); // Set new state

    const bool faultResolved = (P039_data->sensorFault) && (sr == 0);

    if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) 
    {
      if ((P039_data->sensorFault) || faultResolved) {
        String log;
        if((log.reserve(140u))) { // reserve value derived from example log file
          log = F("P039 : MAX31856 : ");
          
          if ((P039_data->sensorFault) == 0) {
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
          addLog(LOG_LEVEL_DEBUG_MORE, log);
          }
        }
      }
    }
  }


    
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
          temperature = rawvalue/1677721.6f; // datasheet: rawvalue = 8 x 1.6 x 2^17 x VIN -> VIN = rawvalue / (8 x 1.6 x 2^17)
          break;
        }
      case 12:
        {
          temperature = rawvalue/6710886.4f; // datasheet: rawvalue = 32 x 1.6 x 2^17 x VIN -> VIN = rawvalue / (32 x 1.6 x 2^17)
          break;
        }
      default:
        {
          temperature = Plugin_039_convert_two_complement(rawvalue, 19);

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

float readMax31865(struct EventStruct *event)
{
  P039_data_struct *P039_data = static_cast<P039_data_struct *>(getPluginTaskData(event->TaskIndex));

  uint8_t registers[MAX31865_NO_REG] = {0};
  uint16_t rawValue = 0u;

  uint8_t CS_pin_no = get_SPI_CS_Pin(event);

  # ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG))
      {
        String log;
        if((log.reserve(80u))) { // reserve value derived from example log file
        
          log = F("P039 : MAX31865 :");
          log += F(" P039_data->convReady: ");
          log += boolToString(P039_data->convReady);

          addLog(LOG_LEVEL_DEBUG, log);
        }
      }

  # endif // ifndef BUILD_NO_DEBUG



  // read conversion result and faults from plugin data structure 
  // if pointer exists and conversion has been finished
  if ((nullptr != P039_data) && (true == P039_data->convReady)) {
    rawValue = P039_data->conversionResult;
    registers[MAX31865_FAULT] = P039_data->deviceFaults;
  }


  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
    {
      String log;
      if((log.reserve(160u))) { // reserve value derived from example log file
       
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

        addLog(LOG_LEVEL_DEBUG_MORE, log);
      }
    }

  # endif // ifndef BUILD_NO_DEBUG

  // Prepare and start next conversion, before handling faults and rawValue
  // clear all faults
  MAX31865_clearFaults(CS_pin_no);

  // set frequency filter
  change8BitRegister(CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG),(MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG), MAX31865_SET_50HZ, static_cast<bool>(P039_RTD_FILT_TYPE) );


  // configure read access with configuration from web interface
  MAX31865_setConType(CS_pin_no, P039_CONFIG_4);

  //activate BIAS short before read, to reduce power consumption
  change8BitRegister(CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG),(MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG), MAX31865_SET_VBIAS_ON, P039_SET );

  // save current timer for next calculation
  P039_data->timer = millis();
  
  // start time to follow up on BIAS activation before starting the conversion
  // and start conversion sequence via TIMER API
  if(nullptr != P039_data){
    // set next state to MAX31865_BIAS_ON_STATE

    Scheduler.setPluginTaskTimer(MAX31865_BIAS_WAIT_TIME, event->TaskIndex, MAX31865_BIAS_ON_STATE);
  }
 
    if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
    {
      if (registers[MAX31865_FAULT])
      {
        String log;
        if((log.reserve(210u))) { // reserve value derived from example log file
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
          addLog(LOG_LEVEL_DEBUG_MORE, log);
        }
      }
    }


  bool ValueValid = false;

  if (registers[MAX31865_FAULT] == 0x00u)
    ValueValid = true;

  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG))
    {
      String log;
      if((log.reserve(85u))) { // reserve value derived from example log file
        log = F("P039 : Temperature :");                            // 20 char
        log += F(" registers[MAX31865_FAULT]: ");                   // 33 char
        log += formatToHex_decimal(registers[MAX31865_FAULT]);      // 7 char
        log += F(" ValueValid: ");                                  // 13 char       
        log += boolToString(ValueValid);                            // 5 char
        addLog(LOG_LEVEL_DEBUG, log);
      }
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
        if((log.reserve(110u))) { // reserve value derived from example log file
          log = F("P039 : Temperature :");            // 20 char
          log += F(" rawValue: ");                    // 11 char
          log += formatToHex_decimal(rawValue);       // 9 char
          log += F(" temperature: ");                 // 14 char
          log += String(temperature, DEC);            // 11 char
          log += F(" P039_RTD_TYPE: ");               // 16 char
          log += String(P039_RTD_TYPE, DEC);          // 1 char
          log += F(" P039_RTD_RES: ");                // 15 char
          log += String(P039_RTD_RES, DEC);           // 4 char
          addLog(LOG_LEVEL_DEBUG, log);
        }
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

void MAX31865_clearFaults(uint8_t l_CS_pin_no)
{
  uint8_t l_reg = 0u;

  // read in config register
  l_reg = read8BitRegister(l_CS_pin_no,(MAX31865_READ_ADDR_BASE + MAX31865_CONFIG));

 
  // clear all faults ( write "0" to D2, D3, D5; write "1" to D2)
  l_reg &= ~(MAX31865_SET_ONE_SHOT | MAX31865_FAULT_CTRL_MASK);
  l_reg |= MAX31865_CLEAR_FAULTS;

  // write configuration 
  write8BitRegister(l_CS_pin_no, (MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG), l_reg);

}

void MAX31865_setConType(uint8_t l_CS_pin_no, uint8_t l_conType)
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
  change8BitRegister(l_CS_pin_no, (MAX31865_READ_ADDR_BASE + MAX31865_CONFIG),(MAX31865_WRITE_ADDR_BASE + MAX31865_CONFIG), MAX31865_SET_3WIRE, l_set_reset );

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
      if((log.reserve(130u))) { // reserve value derived from example log file
        log = F("P039 : LM7x : readLM7x : ");
        log += F(" rawValue: ");
        log += formatToHex_decimal(rawValue);
        log += F(" device_id: ");
        log += formatToHex(device_id);
        log += F(" temperature: ");
        log += String(temperature, DEC);
        addLog(LOG_LEVEL_DEBUG, log);
      }
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

    if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
    {
      String log;
      if((log.reserve(185u))) { // reserve value derived from example log file
        log = F("P039 : LM7x : convertLM7xTemp : ");
        log += F(" l_returnValue: ");
        log += formatToHex_decimal(l_returnValue);
        log += F(" l_LM7xsubtype: ");
        log += formatToHex_decimal(l_LM7xsubtype);
        log += F(" l_rawValue: ");
        log += formatToHex_decimal(l_rawValue);
        log += F(" l_noBits: ");
        log += String(l_noBits, DEC);
        log += F(" l_lsbvalue: ");
        log += String(l_lsbvalue, DEC);
        addLog(LOG_LEVEL_DEBUG_MORE, log);
      }
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

    if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
    {
      String log;
      if((log.reserve(115u))) { // reserve value derived from example log file
        log = F("P039 : LM7x : readLM7xRegisters : ");
        log += F(" l_returnValue: ");
        log += formatToHex_decimal(l_returnValue);
        log += F(" l_device_id: ");
        log += formatToHex(*(l_device_id));
        addLog(LOG_LEVEL_DEBUG_MORE, log);
      }
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

    if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
    {
      String log;
      if((log.reserve(100u))) { // reserve value derived from example log file
        log = F("P039 : SPI : write8BitRegister : ");
        log += F("l_address: ");
        log += formatToHex(l_address);
        log += F(" value: ");
        log += formatToHex_decimal(value);
        addLog(LOG_LEVEL_DEBUG_MORE, log);
      }
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

void write16BitRegister(uint8_t l_CS_pin_no, uint8_t l_address, uint16_t value)
{
  uint8_t l_messageBuffer[3] = {l_address, static_cast<uint8_t> ((value >> 8) & 0xFF), static_cast<uint8_t> (value & 0xFF)};

  transfer_n_ByteSPI(l_CS_pin_no, 3, l_messageBuffer);

  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
    {
      String log;
      if((log.reserve(110u))) { // reserve value derived from example log file
        log = F("P039 : SPI : write16BitRegister : ");
        log += F("l_address: ");
        log += formatToHex(l_address);
        log += F(" value: ");
        log += formatToHex_decimal(value);
        addLog(LOG_LEVEL_DEBUG_MORE, log);
      }
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

    if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
    {
      String log;
      if((log.reserve(100u))) { // reserve value derived from example log file
        log = F("P039 : SPI : read8BitRegister : ");
        log += F("l_address: ");
        log += formatToHex(l_address);
        log += F(" returnvalue: ");
        log += formatToHex_decimal(l_messageBuffer[1]);
        addLog(LOG_LEVEL_DEBUG_MORE, log);
      }
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

  transfer_n_ByteSPI(l_CS_pin_no, 3, l_messageBuffer );
  l_returnValue = ((l_messageBuffer[1]<<8) | l_messageBuffer[2] );

  # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
    {
      String log;
      if((log.reserve(110u))) { // reserve value derived from example log file
        log = F("P039 : SPI : read16BitRegister : ");
        log += F("l_address: ");
        log += formatToHex(l_address);
        log += F(" l_returnValue: ");
        log += formatToHex_decimal(l_returnValue);
        addLog(LOG_LEVEL_DEBUG_MORE, log);
      }
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

    if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE))
    {
      String log;
      if((log.reserve(120u))) { // reserve value derived from example log file
        log = F("P039 : SPI : transfer_n_ByteSPI : ");      // 34 char
        for (uint8_t i = 0; i < l_noBytesToSend; ++i)
        {
          log += ' ';                                               // 1 char
          log += formatToHex_decimal(l_inoutMessageBuffer[i]);      // 9 char
        }
        addLog(LOG_LEVEL_DEBUG_MORE, log);
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

/**************************************************************************/

void change16BitRegister(uint8_t l_CS_pin_no, uint8_t l_readaddress, uint8_t l_writeaddress, uint16_t l_flagmask, bool l_set_reset )
{
  uint16_t l_reg = 0u;

  // read in config register
  l_reg = read16BitRegister(l_CS_pin_no, l_readaddress);

  if(l_set_reset){
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

/**************************************************************************/

void change8BitRegister(uint8_t l_CS_pin_no, uint8_t l_readaddress, uint8_t l_writeaddress, uint8_t l_flagmask, bool l_set_reset )
{
  uint8_t l_reg = 0u;

  // read in config register
  l_reg = read8BitRegister(l_CS_pin_no, l_readaddress);


  //TODO: c.k.i.: analyze opportunity to use arduino bitSet/Clear macros instead
  if(l_set_reset){
    l_reg |= l_flagmask;
  }
  else
  {
    l_reg &= ~(l_flagmask);
  }

  // write to configuration register
  write8BitRegister(l_CS_pin_no, l_writeaddress, l_reg);


}

#endif // USES_P039
