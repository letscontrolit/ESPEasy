#include "_Plugin_Helper.h"

#ifdef USES_P102

// #######################################################################################################
// ######################## Plugin 102: PZEM004Tv30 / PZEM-017v1  with modbus mgt ########################
// #######################################################################################################
//

/** Changelog:
 * 2025-09-14 tonhuisman: Add extra instruction to submit page once more after 'Reset energy' is used, to save 'Read energy' default Mode.
 * 2025-08-06 tonhuisman: Change plugin name to "Energy - PZEM-004T (AC) / PZEM-017 (DC)"
 * 2025-08-05 tonhuisman: Add support for PZEM-017v1 from a forum suggestion:
 *                        https://www.letscontrolit.com/forum/viewtopic.php?p=74069#p74064
 * 2025-01-17 tonhuisman: Implement support for MQTT AutoDiscovery (partially)
 * 2025-01-12 tonhuisman: Add support for MQTT AutoDiscovery (not supported yet for PZEM00x)
 */

# include <ESPeasySerial.h>
# include <PZEM004Tv30.h>

# include "src/DataStructs/ESPEasy_packed_raw_data.h"


# define PLUGIN_102
# define PLUGIN_ID_102        102
# define PLUGIN_102_DEBUG     true       // activate extra log info in the debug
# define PLUGIN_NAME_102      "Energy - PZEM-004T (AC) / PZEM-017 (DC)"

# define P102_PZEM_mode       PCONFIG(1) // 0=read value ; 1=reset energy; 2=programm address
# define P102_PZEM_ADDR       PCONFIG(2)

# define P102_QUERY1          PCONFIG(3)
# define P102_QUERY2          PCONFIG(4)
# define P102_QUERY3          PCONFIG(5)
# define P102_QUERY4          PCONFIG(6)
# define P102_PZEM_FIRST      PCONFIG(7)
# define P102_PZEM_ATTEMPT    PCONFIG_LONG(1)
# define P102_PZEM_TYPE       PCONFIG_LONG(0)

# define P102_PZEM_mode_DFLT  0 // Read value
# define P102_QUERY1_DFLT     0 // Voltage (V)
# define P102_QUERY2_DFLT     1 // Current (A)
# define P102_QUERY3_DFLT     2 // Power (W)
# define P102_QUERY4_DFLT     3 // Energy (kWH)
# define P102_NR_OUTPUT_VALUES   4
# define P102_NR_OUTPUT_OPTIONS  6
# define P102_QUERY1_CONFIG_POS  3

# define P102_PZEM_MAX_ATTEMPT   3 // Number of tentative before declaring NAN value

# define P102_PZEM004_VALUE_COUNT 6
# define P102_PZEM017_VALUE_COUNT 4


PZEM004Tv30 * P102_PZEM_sensor = nullptr;

boolean Plugin_102_init    = false;
uint8_t P102_PZEM_ADDR_SET = 0; // Flag for status of programmation/Energy reset: 0=Reading / 1=Prog confirmed / 3=Prog done / 4=Reset
                                // energy done

// Forward declaration helper function
const __FlashStringHelper* p102_getQueryString(uint8_t query);


boolean                    Plugin_102(uint8_t function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_102;
      dev.Type           = DEVICE_TYPE_SERIAL;
      dev.VType          = Sensor_VType::SENSOR_TYPE_QUAD;
      dev.FormulaOption  = true;
      dev.ValueCount     = 4;
      dev.OutputDataType = Output_Data_type_t::Simple;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.MqttStateClass = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string  = F(PLUGIN_NAME_102);
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P102_NR_OUTPUT_VALUES) {
          uint8_t choice = PCONFIG(i + P102_QUERY1_CONFIG_POS);
          ExtraTaskSettings.setTaskDeviceValueName(i, p102_getQueryString(choice));
        } else {
          ExtraTaskSettings.clearTaskDeviceValueName(i);
        }
      }
      break;
    }

    # if FEATURE_MQTT_DISCOVER
    case PLUGIN_GET_DISCOVERY_VTYPES:
    {
      success = getDiscoveryVType(event, Plugin_102_QueryVType, P102_QUERY1_CONFIG_POS, event->Par5);
      break;
    }
    # endif // if FEATURE_MQTT_DISCOVER

    case PLUGIN_SET_DEFAULTS:
    {
      // Load some defaults
      P102_PZEM_mode = P102_PZEM_mode_DFLT;
      P102_QUERY1    = P102_QUERY1_DFLT;
      P102_QUERY2    = P102_QUERY2_DFLT;
      P102_QUERY3    = P102_QUERY3_DFLT;
      P102_QUERY4    = P102_QUERY4_DFLT;

      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      serialHelper_getGpioNames(event);

      // event->String3 = formatGpioName_output(F("Reset"));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      string += strformat(F("<BR>addr: 0x%02x"), P102_PZEM_ADDR); // Show modbus address
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      // To select the data in the 4 fields.
      const __FlashStringHelper *options[P102_NR_OUTPUT_OPTIONS];
      const uint8_t nrOptions = PZEM_model::PZEM004Tv30 == static_cast<PZEM_model>(P102_PZEM_TYPE)
                                ? P102_PZEM004_VALUE_COUNT
                                : P102_PZEM017_VALUE_COUNT;

      for (uint8_t i = 0; i < nrOptions; ++i) {
        options[i] = p102_getQueryString(i);
      }

      for (uint8_t i = 0; i < P102_NR_OUTPUT_VALUES; ++i) {
        const uint8_t pconfigIndex = i + P102_QUERY1_CONFIG_POS;
        sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, nrOptions, options);
      }

      success = true;

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper*pzemModels[] = {
          F("PZEM-004Tv30 (AC)"),
          F("PZEM-017v1 (DC)"),
        };
        constexpr int pzemCount = NR_ELEMENTS(pzemModels);
        FormSelectorOptions selector(pzemCount, pzemModels);
        selector.reloadonchange = true;
        selector.addFormSelector(F("PZEM Model"), F("pztype"), P102_PZEM_TYPE);
      }

      if (P102_PZEM_sensor == nullptr) { P102_PZEM_FIRST = event->TaskIndex; // To detect if first PZEM or not
      }

      if (P102_PZEM_FIRST == event->TaskIndex)                               // If first PZEM, serial config available
      {
        addHtml(F("<br><B>This PZEM is the first. Its configuration of serial Pins will affect next PZEM. </B>"));
        addHtml(F("<span style=\"color:red\"> <br><B>If several PZEMs foreseen, "
                  "don't use HW serial (or invert Tx and Rx to configure as SW serial).</B></span>"));
        addFormSubHeader(F("PZEM actions"));
        {
          const __FlashStringHelper *options_model[] = { F("Read value"), F("Reset Energy"), F("Program address") };
          constexpr size_t optionCount               = NR_ELEMENTS(options_model);
          const FormSelectorOptions selector(optionCount, options_model);
          selector.addFormSelector(F("PZEM Mode"), F("PZEM_mode"), P102_PZEM_mode);
        }

        if (P102_PZEM_mode == 2)
        {
          addHtml(F("<span style=\"color:red\"> <br>When programming an address, only one PZEM must be connected. "
                    "Otherwise, all connected PZEMs will get the same address, which would cause a conflict during reading.</span>"));
          {
            const __FlashStringHelper *options_confirm[] = { F("NO"), F("YES") };
            constexpr size_t optionCount                 = NR_ELEMENTS(options_confirm);
            const FormSelectorOptions selector(optionCount, options_confirm);
            selector.addFormSelector(F("Confirm address programming ?"), F("PZEM_addr_set"), P102_PZEM_ADDR_SET);
          }
          addFormNumericBox(F("Address of PZEM"), F("PZEM_addr"), (P102_PZEM_ADDR < 1) ? 1 : P102_PZEM_ADDR, 1, 247);
          addHtml(F("Select the address to set PZEM. Programming address 0 is forbidden."));
        }
        else
        {
          addFormNumericBox(F("Address of PZEM"), F("PZEM_addr"), P102_PZEM_ADDR, 0x00, 0xF7);
          addHtml(F("&nbsp;Address 0 allows to communicate with any <B>single</B> PZEM whatever its address"));
        }

        if (P102_PZEM_ADDR_SET == 3) // If address programming done
        {
          addHtml(F("<span style=\"color:green\"><br><B>Address programming done ! </B></span>"));
          P102_PZEM_ADDR_SET = 0;    // Reset programming confirmation
        }
      }
      else
      {
        addFormSubHeader(F("PZEM actions"));
        {
          const __FlashStringHelper *options_model[] = { F("Read value"), F("Reset Energy") };
          constexpr size_t optionCount               = NR_ELEMENTS(options_model);
          const FormSelectorOptions selector(optionCount, options_model);
          selector.addFormSelector(F("PZEM Mode"), F("PZEM_mode"), P102_PZEM_mode);
        }
        addHtml(F("&nbsp;Tx/Rx Pins config ignored: Configuration is available in the first PZEM plugin.<br>"));
        addFormNumericBox(F("Address of PZEM"), F("PZEM_addr"), P102_PZEM_ADDR, 0x01, 0xF7);
      }

      if (P102_PZEM_ADDR_SET == 4)
      {
        addHtml(F("<span style=\"color:blue\"> <br><B>Energy reset on current PZEM ! </B></span>"));
        addHtml(F("<span style=\"color:red\"> <br>Click <B>Submit</B> button once more to save 'Read value' PZEM Mode !</span>"));
        P102_PZEM_ADDR_SET = 0; // Reset programming confirmation
      }

      addFormNote(F("Reset Energy can also be done by: http://*espeasyip*/control?cmd=resetenergy,*PZEM address*"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      serialHelper_webformSave(event);

      // Save output selector parameters.
      for (uint8_t i = 0; i < P102_NR_OUTPUT_VALUES; ++i) {
        const uint8_t pconfigIndex = i + P102_QUERY1_CONFIG_POS;
        const uint8_t choice       = PCONFIG(pconfigIndex);
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, p102_getQueryString(choice));
      }
      P102_PZEM_TYPE     = getFormItemInt(F("pztype"));
      P102_PZEM_mode     = getFormItemInt(F("PZEM_mode"));
      P102_PZEM_ADDR     = getFormItemInt(F("PZEM_addr"));
      P102_PZEM_ADDR_SET = getFormItemInt(F("PZEM_addr_set"));
      Plugin_102_init    = false; // Force device setup next time
      success            = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // FIXME TD-er: This will fail if the set to be first taskindex is no longer enabled
      if (P102_PZEM_FIRST == event->TaskIndex) // If first PZEM, serial config available
      {
        const int rxPin              = CONFIG_PIN1;
        const int txPin              = CONFIG_PIN2;
        const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);

        if (P102_PZEM_sensor != nullptr) {
          // Regardless the set pins, the software serial must be deleted.
          delete P102_PZEM_sensor;
          P102_PZEM_sensor = nullptr;
        }

        // Hardware serial is RX on 3 and TX on 1
        P102_PZEM_sensor = new (std::nothrow) PZEM004Tv30(port, rxPin, txPin);

        // Sequence for changing PZEM address
        if ((P102_PZEM_ADDR_SET == 1) && (P102_PZEM_sensor != nullptr)) // if address programming confirmed
        {
          P102_PZEM_sensor->setAddress(P102_PZEM_ADDR);
          P102_PZEM_mode     = 0;                                       // Back to read mode
          P102_PZEM_ADDR_SET = 3;                                       // Address programmed
        }
      }

      if (P102_PZEM_sensor != nullptr) {
        P102_PZEM_sensor->init(P102_PZEM_ADDR);
        P102_PZEM_sensor->setModel(static_cast<PZEM_model>(P102_PZEM_TYPE));

        // Sequence for reseting PZEM energy
        if (P102_PZEM_mode == 1)
        {
          P102_PZEM_sensor->resetEnergy();
          P102_PZEM_mode     = 0; // Back to read mode
          P102_PZEM_ADDR_SET = 4; // Energy reset done
        }

        Plugin_102_init = true;
        success         = true;
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      if (P102_PZEM_FIRST == event->TaskIndex) // If first PZEM, serial config available
      {
        if (P102_PZEM_sensor)
        {
          delete P102_PZEM_sensor;
          P102_PZEM_sensor = nullptr;
        }
      }
      break;
    }

    case PLUGIN_READ:
    {
      if (Plugin_102_init && (P102_PZEM_mode == 0)) // Read sensor
      {
        // When new data is available, return true
        P102_PZEM_sensor->init(P102_PZEM_ADDR);
        P102_PZEM_sensor->setModel(static_cast<PZEM_model>(P102_PZEM_TYPE));

        float PZEM[6];
        PZEM[0] = P102_PZEM_sensor->voltage();
        PZEM[1] = P102_PZEM_sensor->current();
        PZEM[2] = P102_PZEM_sensor->power();
        PZEM[3] = P102_PZEM_sensor->energy();
        PZEM[4] = P102_PZEM_sensor->pf();
        PZEM[5] = P102_PZEM_sensor->frequency();

        // Skip non-available values
        const uint8_t toCheckMax = PZEM_model::PZEM004Tv30 == static_cast<PZEM_model>(P102_PZEM_TYPE)
                                    ? P102_PZEM004_VALUE_COUNT
                                    : P102_PZEM017_VALUE_COUNT;

        for (uint8_t i = 0; i < toCheckMax; i++) // Check each PZEM field
        {
          if (PZEM[i] != PZEM[i])                // Check if NAN
          {
            P102_PZEM_ATTEMPT == P102_PZEM_MAX_ATTEMPT ? P102_PZEM_ATTEMPT = 0 : P102_PZEM_ATTEMPT++;
            break;                               // if one is Not A Number, break
          }
          P102_PZEM_ATTEMPT = 0;
        }

        if (P102_PZEM_ATTEMPT == 0)
        {
          UserVar.setFloat(event->TaskIndex, 0, PZEM[P102_QUERY1]);
          UserVar.setFloat(event->TaskIndex, 1, PZEM[P102_QUERY2]);
          UserVar.setFloat(event->TaskIndex, 2, PZEM[P102_QUERY3]);
          UserVar.setFloat(event->TaskIndex, 3, PZEM[P102_QUERY4]);

          // sendData(event);   //To send externally from the pluggin (to controller or to rules trigger)
        }
        success = true;
      }
      break;
    }

# if FEATURE_PACKED_RAW_DATA
    case PLUGIN_GET_PACKED_RAW_DATA:
    {
      // Matching JS code for PZEM-004T:
      // return decode(bytes, [header, int16_1e1, int32_1e3, int32_1e1, int32_1e1, uint16_1e2, uint8_1e1],
      //   ['header', 'voltage', 'current', 'power', 'energy', 'powerfactor', 'frequency']);
      //
      // Matching JS code for PZEM-017:
      // return decode(bytes, [header, int16_1e1, int32_1e3, int32_1e1, int32_1e1],
      //   ['header', 'voltage', 'current', 'power', 'energy']);
      //
      // Resolutions:
      //  Voltage:     0.1V     => int16_1e1  (range 80-260V)
      //  Current:     0.001A   => int32_1e3
      //  Power:       0.1W     => int32_1e1
      //  Energy:      1Wh      => int32_1e1
      //  PowerFactor: 0.01     => uint16_1e2
      //  Frequency:   0.1Hz    => uint8_1e1  (range 45Hz - 65Hz), offset 40Hz

      // FIXME TD-er: Calling these functions is probably done within the 200 msec timeout used in the library.
      // If not, this should be cached in a task data struct.
      string += LoRa_addFloat(P102_PZEM_sensor->voltage(), PackedData_int16_1e1);
      string += LoRa_addFloat(P102_PZEM_sensor->current(), PackedData_int32_1e3);
      string += LoRa_addFloat(P102_PZEM_sensor->power(),   PackedData_int32_1e1);
      string += LoRa_addFloat(P102_PZEM_sensor->energy(),  PackedData_int32_1e1);

      if (PZEM_model::PZEM004Tv30 == static_cast<PZEM_model>(P102_PZEM_TYPE)) {
        string     += LoRa_addFloat(P102_PZEM_sensor->pf(),             PackedData_uint16_1e2);
        string     += LoRa_addFloat(P102_PZEM_sensor->frequency() - 40, PackedData_uint8_1e1);
        event->Par1 = P102_PZEM004_VALUE_COUNT; // valuecount
      } else {
        event->Par1 = P102_PZEM017_VALUE_COUNT; // valuecount
      }

      success = true;
      break;
    }
# endif // if FEATURE_PACKED_RAW_DATA


    case PLUGIN_WRITE:
    {
      if (Plugin_102_init)
      {
        const String command = parseString(string, 1);

        if (equals(command, F("resetenergy")) && (event->Par1 >= 0) && (event->Par1 <= 247)) {
          P102_PZEM_sensor->init(event->Par1);
          P102_PZEM_sensor->resetEnergy();
          success = true;
        }
      }

      break;
    }
  }
  return success;
}

const __FlashStringHelper* p102_getQueryString(uint8_t query) {
  switch (query)
  {
    case 0: return F("Voltage_V");
    case 1: return F("Current_A");
    case 2: return F("Power_W");
    case 3: return F("Energy_kWh");
    case 4: return F("Power_Factor_cosphi");
    case 5: return F("Frequency_Hz");
  }
  return F("");
}

# if FEATURE_MQTT_DISCOVER
int Plugin_102_QueryVType(uint8_t query) {
  Sensor_VType result = Sensor_VType::SENSOR_TYPE_NONE;

  switch (query)
  {
    case 0: result = Sensor_VType::SENSOR_TYPE_VOLTAGE_ONLY; break;
    case 1: result = Sensor_VType::SENSOR_TYPE_CURRENT_ONLY; break;
    case 2: result = Sensor_VType::SENSOR_TYPE_POWER_USG_ONLY; break;
    case 3: result = Sensor_VType::SENSOR_TYPE_ENERGY; break;
    case 4: result = Sensor_VType::SENSOR_TYPE_POWER_FACT_ONLY; break;
    case 5: result = Sensor_VType::SENSOR_TYPE_FREQUENCY; break;
  }
  return static_cast<int>(result);
}

# endif // if FEATURE_MQTT_DISCOVER

#endif // USES_P102
