#include "_Plugin_Helper.h"

#ifdef USES_P102

// #######################################################################################################
// #################################### Plugin 102: PZEM004T v30 with modbus mgt##########################
// #######################################################################################################
//


# include <ESPeasySerial.h>
# include <PZEM004Tv30.h>


# define PLUGIN_102
# define PLUGIN_ID_102        102
# define PLUGIN_102_DEBUG     true       // activate extra log info in the debug
# define PLUGIN_NAME_102      "PZEM-004Tv30-Multiple [TESTING]"

# define P102_PZEM_mode       PCONFIG(1) // 0=read value ; 1=reset energy; 2=programm address
# define P102_PZEM_ADDR       PCONFIG(2)

# define P102_QUERY1          PCONFIG(3)
# define P102_QUERY2          PCONFIG(4)
# define P102_QUERY3          PCONFIG(5)
# define P102_QUERY4          PCONFIG(6)
# define P102_PZEM_FIRST      PCONFIG(7)
# define P102_PZEM_ATTEMPT    PCONFIG(8)

# define P102_PZEM_mode_DFLT  0 // Read value
# define P102_QUERY1_DFLT     0 // Voltage (V)
# define P102_QUERY2_DFLT     1 // Current (A)
# define P102_QUERY3_DFLT     2 // Power (W)
# define P102_QUERY4_DFLT     3 // Energy (WH)
# define P102_NR_OUTPUT_VALUES   4
# define P102_NR_OUTPUT_OPTIONS  6
# define P102_QUERY1_CONFIG_POS  3

# define P102_PZEM_MAX_ATTEMPT      3 // Number of tentative before declaring NAN value

PZEM004Tv30 *P102_PZEM_sensor = nullptr;

boolean Plugin_102_init    = false;
uint8_t P102_PZEM_ADDR_SET = 0; // Flag for status of programmation/Energy reset: 0=Reading / 1=Prog confirmed / 3=Prog done / 4=Reset
                                // energy done

boolean Plugin_102(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_102;
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = false;
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
      for (byte i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P102_NR_OUTPUT_VALUES) {
          byte choice = PCONFIG(i + P102_QUERY1_CONFIG_POS);
          safe_strncpy(
            ExtraTaskSettings.TaskDeviceValueNames[i],
            p102_getQueryString(choice),
            sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
        } else {
          ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
        }
      }
      break;
    }

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
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      if (P102_PZEM_sensor == nullptr) { P102_PZEM_FIRST = event->TaskIndex; // To detect if first PZEM or not
      }

      if (P102_PZEM_FIRST == event->TaskIndex)                               // If first PZEM, serial config available
      {
        addHtml(F("<br><B>This PZEM is the first. Its configuration of serial Pins will affect next PZEM. </B>"));
        addHtml(F(
                  "<span style=\"color:red\"> <br><B>If several PZEMs foreseen, don't use HW serial (or invert Tx and Rx to configure as SW serial).</B></span>"));
        addFormSubHeader(F("PZEM actions"));
        String options_model[3] = { F("Read_value"), F("Reset_Energy"), F("Program_adress") };
        addFormSelector(F("PZEM Mode"), F("P102_PZEM_mode"), 3, options_model, NULL, P102_PZEM_mode);

        if (P102_PZEM_mode == 2)
        {
          addHtml(F(
                    "<span style=\"color:red\"> <br>When programming an address, only one PZEMv30 must be connected. Otherwise, all connected PZEMv30s will get the same address, which would cause a conflict during reading.</span>"));
          String options_confirm[2] = { F("NO"), F("YES") };
          addFormSelector(F("Confirm address programming ?"), F("P102_PZEM_addr_set"), 2, options_confirm, NULL, P102_PZEM_ADDR_SET);
          addFormNumericBox(F("Address of PZEM"), F("P102_PZEM_addr"), (P102_PZEM_ADDR < 1) ? 1 : P102_PZEM_ADDR, 1, 247);
          addHtml(F("Select the address to set PZEM. Programming address 0 is forbidden."));
        }
        else
        {
          addFormNumericBox(F("Address of PZEM"), F("P102_PZEM_addr"), P102_PZEM_ADDR, 0, 247);
          addHtml(F("  Address 0 allows to communicate with any <B>single</B> PZEMv30 whatever its address"));
        }

        if (P102_PZEM_ADDR_SET == 3) // If address programming done
        {
          addHtml(F("<span style=\"color:green\"> <br><B>Address programming done ! </B></span>"));
          P102_PZEM_ADDR_SET = 0;    // Reset programming confirmation
        }
      }
      else
      {
        addFormSubHeader(F("PZEM actions"));
        String options_model[2] = { F("Read_value"), F("Reset_Energy") };
        addFormSelector(F("PZEM Mode"), F("P102_PZEM_mode"), 2, options_model, NULL, P102_PZEM_mode);
        addHtml(F(" Tx/Rx Pins config disabled: Configuration is available in the first PZEM plugin.<br>"));
        addFormNumericBox(F("Address of PZEM"), F("P102_PZEM_addr"), P102_PZEM_ADDR, 1, 247);
      }

      addHtml(F("<br><br> Reset energy can be done also by: http://*espeasyip*/control?cmd=resetenergy,*PZEM address*"));

      if (P102_PZEM_ADDR_SET == 4)
      {
        addHtml(F("<span style=\"color:blue\"> <br><B>Energy reset on current PZEM ! </B></span>"));
        P102_PZEM_ADDR_SET = 0; // Reset programming confirmation
      }

      // To select the data in the 4 fields. In a separate scope to free memory of String array as soon as possible

      sensorTypeHelper_webformLoad_header();
      String options[P102_NR_OUTPUT_OPTIONS];

      for (uint8_t i = 0; i < P102_NR_OUTPUT_OPTIONS; ++i) {
        options[i] = p102_getQueryString(i);
      }

      for (byte i = 0; i < P102_NR_OUTPUT_VALUES; ++i) {
        const byte pconfigIndex = i + P102_QUERY1_CONFIG_POS;
        sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, P102_NR_OUTPUT_OPTIONS, options);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      serialHelper_webformSave(event);

      // Save output selector parameters.
      for (byte i = 0; i < P102_NR_OUTPUT_VALUES; ++i) {
        const byte pconfigIndex = i + P102_QUERY1_CONFIG_POS;
        const byte choice       = PCONFIG(pconfigIndex);
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, p102_getQueryString(choice));
      }
      P102_PZEM_mode     = getFormItemInt(F("P102_PZEM_mode"));
      P102_PZEM_ADDR     = getFormItemInt(F("P102_PZEM_addr"));
      P102_PZEM_ADDR_SET = getFormItemInt(F("P102_PZEM_addr_set"));
      Plugin_102_init    = false; // Force device setup next time
      success            = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (P102_PZEM_FIRST == event->TaskIndex) // If first PZEM, serial config available
      {
        int rxPin                    = CONFIG_PIN1;
        int txPin                    = CONFIG_PIN2;
        const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);

        if (P102_PZEM_sensor != nullptr) {
          // Regardless the set pins, the software serial must be deleted.
          delete P102_PZEM_sensor;
          P102_PZEM_sensor = nullptr;
        }

        // Hardware serial is RX on 3 and TX on 1
        P102_PZEM_sensor = new PZEM004Tv30(port, rxPin, txPin);

        // Sequence for changing PZEM address
        if (P102_PZEM_ADDR_SET == 1) // if address programming confirmed
        {
          P102_PZEM_sensor->setAddress(P102_PZEM_ADDR);
          P102_PZEM_mode     = 0;    // Back to read mode
          P102_PZEM_ADDR_SET = 3;    // Address programmed
        }
      }
      P102_PZEM_sensor->init(P102_PZEM_ADDR);

      // Sequence for reseting PZEM energy
      if (P102_PZEM_mode == 1)
      {
        P102_PZEM_sensor->resetEnergy();
        P102_PZEM_mode     = 0; // Back to read mode
        P102_PZEM_ADDR_SET = 4; // Energy reset done
      }

      Plugin_102_init = true;
      success         = true;
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

        float PZEM[6];
        PZEM[0] = P102_PZEM_sensor->voltage();
        PZEM[1] = P102_PZEM_sensor->current();
        PZEM[2] = P102_PZEM_sensor->power();
        PZEM[3] = P102_PZEM_sensor->energy();
        PZEM[4] = P102_PZEM_sensor->pf();
        PZEM[5] = P102_PZEM_sensor->frequency();

        for (byte i = 0; i < 6; i++) // Check each PZEM field
        {
          if (PZEM[i] != PZEM[i])    // Check if NAN
          {
            P102_PZEM_ATTEMPT == P102_PZEM_MAX_ATTEMPT ? P102_PZEM_ATTEMPT = 0 : P102_PZEM_ATTEMPT++;
            break;                   // if one is Not A Number, break
          }
          P102_PZEM_ATTEMPT = 0;
        }

        if (P102_PZEM_ATTEMPT == 0)
        {
          UserVar[event->BaseVarIndex]     = PZEM[P102_QUERY1];
          UserVar[event->BaseVarIndex + 1] = PZEM[P102_QUERY2];
          UserVar[event->BaseVarIndex + 2] = PZEM[P102_QUERY3];
          UserVar[event->BaseVarIndex + 3] = PZEM[P102_QUERY4];

          // sendData(event);   //To send externally from the pluggin (to controller or to rules trigger)
        }
        success = true;
      }
      break;
    }

#ifdef USES_PACKED_RAW_DATA
    case PLUGIN_GET_PACKED_RAW_DATA:
    {
      // Matching JS code:
      // return decode(bytes, [header, int16_1e1, int32_1e3, int32_1e1, int32_1e1, uint16_1e2, uint8_1e1],
      //   ['header', 'voltage', 'current', 'power', 'energy', 'powerfactor', 'frequency']);
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
      string += LoRa_addFloat(P102_PZEM_sensor->voltage(),       PackedData_int16_1e1);
      string += LoRa_addFloat(P102_PZEM_sensor->current(),       PackedData_int32_1e3);
      string += LoRa_addFloat(P102_PZEM_sensor->power(),         PackedData_int32_1e1);
      string += LoRa_addFloat(P102_PZEM_sensor->energy(),        PackedData_int32_1e1);
      string += LoRa_addFloat(P102_PZEM_sensor->pf(),            PackedData_uint16_1e2);
      string += LoRa_addFloat(P102_PZEM_sensor->frequency() - 40, PackedData_uint8_1e1);
      event->Par1 = 6; // valuecount 
      
      success = true;
      break;
    }
#endif // USES_PACKED_RAW_DATA



    case PLUGIN_WRITE:
    {
      if (Plugin_102_init)
      {
        String command = parseString(string, 1);

        if ((command == F("resetenergy")) && (P102_PZEM_FIRST == event->TaskIndex))
        {
          if ((event->Par1 >= 0) && (event->Par1 <= 247))
          {
            P102_PZEM_sensor->init(event->Par1);
            P102_PZEM_sensor->resetEnergy();
            success = true;
          }
        }
      }

      break;
    }
  }
  return success;
}

String p102_getQueryString(byte query) {
  switch (query)
  {
    case 0: return F("Voltage_V");
    case 1: return F("Current_A");
    case 2: return F("Power_W");
    case 3: return F("Energy_WH");
    case 4: return F("Power_Factor_cosphi");
    case 5: return F("Frequency Hz");
  }
  return "";
}

#endif // USES_P102
