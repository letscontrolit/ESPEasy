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

/** Changelog:
 * 2025-08-13 tonhuisman: Enable use of secondary SPI bus
 * 2025-08-13 tonhuisman: Move most code to P039_PluginStruct
 * 2025-01-12 tonhuisman: Add support for MQTT AutoDiscovery
 * 2025-01-03 tonhuisman: Small code size reductions, cleanup of DEBUG level logging
 * 2024-01-04 tonhuisman: Minor corrections, formatted source using Uncrustify
 * 2023-01-08 tonhuisman: Add Low temperature threshold setting (default 0 K/-273.15 C) to ignore temperatures below that value
 * 2023-01-02 tonhuisman: Cleanup and uncrustify source
 * 2022-10-22 tonhuisman: Correct CS pin check to allow GPIO0
 * 2022-10: Older changelog not recorded
 */

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

// #include <Misc.h>
# include "src/PluginStructs/P039_data_struct.h"


// // plugin-local quick activation of debug messages
// #ifdef BUILD_NO_DEBUG
//   #undef BUILD_NO_DEBUG
// #endif


# define PLUGIN_039
# define PLUGIN_ID_039         39
# define PLUGIN_NAME_039       "Environment - Thermosensors"
# define PLUGIN_VALUENAME1_039 "Temperature"


boolean Plugin_039(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_039;
      dev.Type           = DEVICE_TYPE_SPI;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.FormulaOption  = true;
      dev.ValueCount     = 1;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;
      dev.SpiBusSelect   = true;
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

    # if FEATURE_MQTT_DISCOVER
    case PLUGIN_GET_DISCOVERY_VTYPES:
    {
      success = getDiscoveryVType(event, Plugin_QueryVType_Temperature, 255, event->Par5);
      break;
    }
    # endif // if FEATURE_MQTT_DISCOVER

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("CS"));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P039_TEMP_THRESHOLD = P039_TEMP_THRESHOLD_DEFAULT; // 0 K
      bitSet(P039_FLAGS, P039_TEMP_THRESHOLD_FLAG);
      break;
    }

    case PLUGIN_INIT:
    {
      if (!bitRead(P039_FLAGS, P039_TEMP_THRESHOLD_FLAG)) {
        P039_TEMP_THRESHOLD = P039_TEMP_THRESHOLD_DEFAULT; // 0 K
      }

      if ((P039_MAX_TYPE < P039_MAX6675) || (P039_MAX_TYPE > P039_LM7x)) {
        break;
      }


      initPluginTaskData(event->TaskIndex, new (std::nothrow) P039_data_struct(event));
      P039_data_struct *P039_data = static_cast<P039_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P039_data) {
        success = P039_data->begin(event);
      }
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSubHeader(F("Sensor Family Selection"));

      const uint8_t family = P039_FAM_TYPE;
      {
        const __FlashStringHelper *Foptions[] = { F("Thermocouple"), F("RTD") };
        const int FoptionValues[]             = { P039_TC, P039_RTD };
        constexpr size_t optionCount          = NR_ELEMENTS(FoptionValues);
        FormSelectorOptions selector(optionCount, Foptions, FoptionValues);
        selector.reloadonchange = true;
        selector.addFormSelector(F("Sensor Family Type"), F("famtype"), family);
      }

      const uint8_t choice = P039_MAX_TYPE;

      addFormSubHeader(F("Device Type Settings"));

      if (family == P039_TC) {
        {
          const __FlashStringHelper *options[] = {   F("MAX 6675"), F("MAX 31855"), F("MAX 31856") };
          const int optionValues[]             = { P039_MAX6675, P039_MAX31855, P039_MAX31856 };
          constexpr size_t optionCount         = NR_ELEMENTS(optionValues);
          FormSelectorOptions selector(optionCount, options, optionValues);
          selector.reloadonchange = true;
          selector.addFormSelector(F("Adapter IC"), F("maxtype"), choice);
        }

        if (choice == P039_MAX31856) {
          addFormSubHeader(F("Device Settings"));
          {
            const __FlashStringHelper *Toptions[] = { F("B"), F("E"), F("J"), F("K"), F("N"), F("R"), F("S"), F("T"), F("VM8"), F("VM32") };

            // 2021-05-17: c.k.i.: values are directly written to device register for configuration, therefore no linear values are used
            // here
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

            const int ToptionValues[]    = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 12 };
            constexpr size_t optionCount = NR_ELEMENTS(ToptionValues);
            const FormSelectorOptions selector(optionCount, Toptions, ToptionValues);
            selector.addFormSelector(F("Thermocouple type"), F("tctype"), P039_TC_TYPE);
          }
          {
            const __FlashStringHelper *Coptions[] = { F("1"), F("2"), F("4"), F("8"), F("16") };
            const int CoptionValues[]             = { 0, 1, 2, 3, 4 };
            constexpr size_t optionCount          = NR_ELEMENTS(CoptionValues);
            const FormSelectorOptions selector(optionCount, Coptions, CoptionValues);
            selector.addFormSelector(F("Averaging"), F("contype"), P039_CONFIG_4);
            addUnit(F("sample(s)"));
          }
          P039_data_struct::AddMainsFrequencyFilterSelection(event);
        }
      }
      else {
        {
          const __FlashStringHelper *TPoptions[] = { F("MAX 31865"), F("LM7x") };
          const int TPoptionValues[]             = { P039_MAX31865, P039_LM7x };
          constexpr size_t optionCount           = NR_ELEMENTS(TPoptionValues);
          FormSelectorOptions selector(optionCount, TPoptions, TPoptionValues);
          selector.reloadonchange = true;
          selector.addFormSelector(F("Adapter IC"), F("maxtype"), choice);
          addFormNote(F("LM7x support is experimental."));
        }


        if (choice == P039_MAX31865)
        {
          {
            addFormSubHeader(F("Device Settings"));
          }
          {
            const __FlashStringHelper *PToptions[] = { F("PT100"), F("PT1000") };
            const int PToptionValues[]             = { MAX31865_PT100, MAX31865_PT1000 };
            constexpr size_t optionCount           = NR_ELEMENTS(PToptionValues);
            const FormSelectorOptions selector(optionCount, PToptions, PToptionValues);
            selector.addFormSelector(F("Resistor Type"), F("rtdtype"), P039_RTD_TYPE);
          }
          {
            const __FlashStringHelper *Coptions[] = { F("2-/4"), F("3") };
            constexpr size_t optionCount          = NR_ELEMENTS(Coptions);
            const FormSelectorOptions selector(optionCount, Coptions);
            selector.addFormSelector(F("Connection Type"), F("contype"), P039_CONFIG_4);
            addUnit(F("wire"));
          }

          P039_data_struct::AddMainsFrequencyFilterSelection(event);

          {
            addFormNumericBox(F("Reference Resistor"), F("res"), P039_RTD_RES, 0);
            addUnit(F("Ohm"));
            addFormNote(F("PT100: typically 430 [OHM]; PT1000: typically 4300 [OHM]"));
          }
          {
            addFormFloatNumberBox(F("Temperature Offset"), F("offset"), P039_RTD_OFFSET, -50.0f, 50.0f, 2, 0.01f);
            addUnit('K');
            # ifndef BUILD_NO_DEBUG
            addFormNote(F("Valid values: [-50.0...50.0 K], min. stepsize: [0.01]"));
            # endif // ifndef BUILD_NO_DEBUG
          }
        }

        if (choice == P039_LM7x)
        {
          {
            addFormSubHeader(F("Device Settings"));
          }

          {
            const __FlashStringHelper *PToptions[] =
            { F("LM70"), F("LM71"), F("LM74"), F("TMP121"), F("TMP122"), F("TMP123"), F("TMP124"), F("TMP125") };
            const int PToptionValues[]   = { LM7x_SD70, LM7x_SD71, LM7x_SD74, LM7x_SD121, LM7x_SD122, LM7x_SD123, LM7x_SD124, LM7x_SD125 };
            constexpr size_t optionCount = NR_ELEMENTS(PToptionValues);
            const FormSelectorOptions selector(optionCount, PToptions, PToptionValues);
            selector.addFormSelector(F("LM7x device details"), F("rtd_lm_type"), P039_RTD_LM_TYPE);
            addFormNote(F("TMP122/124 Limited support -> fixed 12 Bit res, no advanced options"));
          }
          {
            addFormCheckBox(F("Enable Shutdown Mode"), F("rtd_lm_shtdwn"), P039_RTD_LM_SHTDWN);
            # ifndef BUILD_NO_DEBUG
            addFormNote(F("Device is set to shutdown between sample cycles. Useful for very long call cycles, to save power.<BR>"
                          "Without LM7x device conversion happens in between call cycles. Call Cylces should therefore not become lower than 350ms."));
            # endif // ifndef BUILD_NO_DEBUG
          }
        }
      }

      addFormSubHeader(F("Value validation"));

      if (!bitRead(P039_FLAGS, P039_TEMP_THRESHOLD_FLAG)) {
        P039_TEMP_THRESHOLD = P039_TEMP_THRESHOLD_DEFAULT; // 0 K
      }
      addFormFloatNumberBox(F("Low temperature threshold"),
                            F("temp_thres"),
                            P039_TEMP_THRESHOLD,
                            P039_TEMP_THRESHOLD_MIN,
                            P039_TEMP_THRESHOLD_MAX,
                            2u);
      addUnit(F("&deg;C"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P039_FAM_TYPE       = getFormItemInt(F("famtype"));
      P039_MAX_TYPE       = getFormItemInt(F("maxtype"));
      P039_TC_TYPE        = getFormItemInt(F("tctype"));
      P039_RTD_TYPE       = getFormItemInt(F("rtdtype"));
      P039_CONFIG_4       = getFormItemInt(F("contype"));
      P039_RTD_FILT_TYPE  = getFormItemInt(F("filttype"));
      P039_RTD_RES        = getFormItemInt(F("res"));
      P039_RTD_OFFSET     = getFormItemFloat(F("offset"));
      P039_RTD_LM_TYPE    = getFormItemInt(F("rtd_lm_type"));
      P039_RTD_LM_SHTDWN  = isFormItemChecked(F("rtd_lm_shtdwn"));
      P039_TEMP_THRESHOLD = getFormItemFloat(F("temp_thres"));
      bitSet(P039_FLAGS, P039_TEMP_THRESHOLD_FLAG); // We've set a value, don't replace by default

      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      P039_data_struct *P039_data = static_cast<P039_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P039_data) {
        success = P039_data->read(event);
      }

      break;
    }

    case PLUGIN_TASKTIMER_IN:
    {
      P039_data_struct *P039_data = static_cast<P039_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P039_data) {
        success = P039_data->plugin_tasktimer_in(event);
      }

      break;
    }
  }
  return success;
}

#endif // USES_P039
