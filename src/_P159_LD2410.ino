#include "_Plugin_Helper.h"

// #######################################################################################################
// ###########################  Plugin 159: Radar Presence detection LD2410(/LD2420)  ####################
// #######################################################################################################

/** Changelog:
 * 2024-12-09 tonhuisman: Fix: Reduced max sensitivity to configure to 101, as the max value that vill be reported by the sensor is 100,
 *                        so checking up to 100 was an off-by-one error.
 * 2024-10-09 tonhuisman: Extend sensitivity max. value to 110 (experimental, was 100)
 * 2023-10-29 tonhuisman: Rework processing, allow Interval = 0, as now the events will be generated when a value changes,
 *                        but at most once per 100 msec, to not overload the ESP. Fixed the LD2410 library to work correctly
 *                        with the event-driven scheduler model of ESPEasy, instead of the continuous loop() run of Arduino
 *                        Add getconfig values for configured sensitivity level per gate, see below
 * 2023-10-28 tonhuisman: !! Breaking change (again): Insert Gate 0 value before Gate 1 value, both for Static and Moving
 *                        energy, as that was missing until now. This is because of poor documentation, talking about 8 values
 *                        where there are in fact 9 values, 0..8 !
 *                        Add support for getting all values (function PLUGIN_GET_CONFIG_VALUE), also when not configured
 *                        like [<TaskName>#<ValueName>]. Available valuenames: see below.
 *                        Add command ld2410,logall that writes all available values to the log (depending on Engineering mode)
 * 2023-10-23 tonhuisman: !! Breaking change: Insert OutputPinState (undocumented) value after AmbientLight value
 *                        shifting all Sensitivity value offsets by 1 !!
 * 2023-10-22 tonhuisman: Add command handling: ld2410,factoryreset
 * 2023-10-21 tonhuisman: Read data at 50/sec instead of 10/sec to catch up with the high speed of output
 *                        Add/update settings for Sensitivity and nr. of active gates, idle seconds
 *                        Shorten default value names, (breaking) change setting for Engineering mode
 *                        Add setting for generating events only when a value has changed
 * 2023-10-14 tonhuisman: Using ld2410 library from https://github.com/ncmreynolds/ld2410, but with PR #3 applied
 *                        including a fix for index/index + 1, mentioned in the comments of that PR and timeout
 *                        extended to 2500 msec
 * 2023-10-14 tonhuisman: Initial plugin setup
 */

/** Info:
 * This plugin reads the presence and distance of stationary and moving objects detected by the HiLink LD2410 and LD2420 24 GHz Radar
 * human presence detectors
 */

/** Commands:
 * ld2410,factoryreset  : Reset sensor to factory defaults, also restarts the sensor like the task is just started.
 * ld2410,logall        : Send all available values to the log (INFO level) like: "LD2410: <Display name>: <value>"
 */

/** ValueNames: (for PLUGIN_GET_CONFIG_VALUE support)
 * Presence                         : Presence (0/1)
 * StatPres                         : Stationary presence (0/1)
 * MovPres                          : Moving presence (0/1)
 * Distance                         : Distance in cm
 * StatDist                         : Stationary distance in cm
 * MovDist                          : Moving distance in cm
 * StatEnergy                       : Stationary object energy level
 * MovEnergy                        : Moving object energy level
 * AmbLight                         : Ambient light level (0..255) (unknown unit)
 * OutputPin                        : State of the output pin (0/1)
 * StatEnergyGate0..StatEnergyGate8 : Stationary object energy level for gate 0..8, 75 cm/gate
 * MovEnergyGate0..MovEnergyGate8   : Moving object energy for gate 0..8, 75 cm/gate
 * StatSensGate0..StatSensGate8     : Stationary sensitivity setting gate 0..8
 * MovSensGate0..MovSensGate8       : Moving sensitivity setting gate 0..8
 */

#ifdef USES_P159
# include "src/PluginStructs/P159_data_struct.h" // Sensor abstraction for P159

// Standard plugin defines
# define PLUGIN_159
# define PLUGIN_ID_159          159
# define PLUGIN_NAME_159        "Presence - LD2410"


boolean Plugin_159(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      // This case defines the device characteristics

      auto& dev = Device[++deviceCount];
      dev.Number             = PLUGIN_ID_159;
      dev.Type               = DEVICE_TYPE_SERIAL;
      dev.VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.ValueCount         = 4;
      dev.OutputDataType     = Output_Data_type_t::Simple;
      dev.FormulaOption      = true;
      dev.SendDataOption     = true;
      dev.TimerOption        = true;
      dev.TimerOptional      = true;
      dev.PluginStats        = true;
      dev.ExitTaskBeforeSave = false; // Enable calling PLUGIN_WEBFORM_SAVE on the instantiated object

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_159);

      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      const int valueCount = P159_NR_OUTPUT_VALUES;

      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < valueCount) {
          const uint8_t pconfigIndex = i + P159_QUERY1_CONFIG_POS;
          const uint8_t option       = PCONFIG(pconfigIndex);

          if ((option >= P159_OUTPUT_STATIC_DISTANCE_ENERGY_GATE0) && (option <= P159_OUTPUT_STATIC_DISTANCE_ENERGY_GATE8)) {
            ExtraTaskSettings.setTaskDeviceValueName(i, concat(Plugin_159_valuename(P159_OUTPUT_STATIC_DISTANCE_GATE_index, false),
                                                               option - P159_OUTPUT_STATIC_DISTANCE_ENERGY_GATE0));
          } else
          if ((option >= P159_OUTPUT_MOVING_DISTANCE_ENERGY_GATE0) && (option <= P159_OUTPUT_MOVING_DISTANCE_ENERGY_GATE8)) {
            ExtraTaskSettings.setTaskDeviceValueName(i, concat(Plugin_159_valuename(P159_OUTPUT_MOVING_DISTANCE_GATE_index, false),
                                                               option - P159_OUTPUT_MOVING_DISTANCE_ENERGY_GATE0));
          } else {
            ExtraTaskSettings.setTaskDeviceValueName(i, Plugin_159_valuename(option, false));
          }
          ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0; // No values have decimals
        } else {
          ExtraTaskSettings.clearTaskDeviceValueName(i);
        }
      }

      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      serialHelper_getGpioNames(event);

      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);

      success = true;

      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P159_NR_OUTPUT_VALUES;

      success = true;

      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(P159_SENSOR_TYPE_INDEX));
      event->idx        = P159_SENSOR_TYPE_INDEX;

      success = true;

      break;
    }


    case PLUGIN_SET_DEFAULTS:
    {
      PCONFIG(0) = P159_OUTPUT_PRESENCE;
      PCONFIG(1) = P159_OUTPUT_DISTANCE;
      PCONFIG(2) = P159_OUTPUT_STATIONARY_ENERGY;
      PCONFIG(3) = P159_OUTPUT_MOVING_ENERGY;

      PCONFIG(P159_SENSOR_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_QUAD);

      success = true;

      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      const uint8_t optionCount = P159_GET_ENGINEERING_MODE == 1 ? P159_NR_ENGINEERING_OUTPUT_OPTIONS : P159_NR_OUTPUT_OPTIONS;
      String options[optionCount];

      for (uint8_t option = 0; option < optionCount; ++option) {
        if ((option >= P159_OUTPUT_STATIC_DISTANCE_ENERGY_GATE0) && (option <= P159_OUTPUT_STATIC_DISTANCE_ENERGY_GATE8)) {
          options[option] = concat(Plugin_159_valuename(P159_OUTPUT_STATIC_DISTANCE_GATE_index, true),
                                   option - P159_OUTPUT_STATIC_DISTANCE_ENERGY_GATE0);
        } else
        if ((option >= P159_OUTPUT_MOVING_DISTANCE_ENERGY_GATE0) && (option <= P159_OUTPUT_MOVING_DISTANCE_ENERGY_GATE8)) {
          options[option] = concat(Plugin_159_valuename(P159_OUTPUT_MOVING_DISTANCE_GATE_index, true),
                                   option - P159_OUTPUT_MOVING_DISTANCE_ENERGY_GATE0);
        } else {
          options[option] = Plugin_159_valuename(option, true);
        }
      }

      const int valueCount = P159_NR_OUTPUT_VALUES;

      for (uint8_t i = 0; i < valueCount; ++i) {
        const uint8_t pconfigIndex = i + P159_QUERY1_CONFIG_POS;
        sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, optionCount, options);
      }

      success = true;

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSelector_YesNo(F("Engineering mode"), F("eng"), P159_GET_ENGINEERING_MODE, true);
      addFormNote(F("When changing this setting the page will be reloaded"));

      addFormCheckBox(F("Generate Events only when changed"), F("diff"), P159_GET_UPDATE_DIFF_ONLY);

      success = true;

      P159_data_struct *P159_data = static_cast<P159_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P159_data) {
        addFormSubHeader(F("Sensitivity settings"));
        success = P159_data->plugin_webform_load(event);
      }

      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P159_SET_ENGINEERING_MODE(getFormItemInt(F("eng")));
      P159_SET_UPDATE_DIFF_ONLY(isFormItemChecked(F("diff")));

      P159_data_struct *P159_data = static_cast<P159_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P159_data) {
        P159_data->plugin_webform_save(event);
      }

      success = true;

      break;
    }
    case PLUGIN_INIT:
    {
      const int8_t rxPin               = serialHelper_getRxPin(event);
      const int8_t txPin               = serialHelper_getTxPin(event);
      const ESPEasySerialPort portType = serialHelper_getSerialType(event);

      // Create the P159_data_struct object that will do all the sensor interaction
      success = initPluginTaskData(event->TaskIndex, new (std::nothrow) P159_data_struct(portType,
                                                                                         rxPin,
                                                                                         txPin,
                                                                                         P159_GET_ENGINEERING_MODE == 1));
      addLog(LOG_LEVEL_INFO, concat(F("P159 : INIT, success: "), success ? 1 : 0));

      break;
    }

    case PLUGIN_READ:
    {
      P159_data_struct *P159_data = static_cast<P159_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P159_data) {
        success = P159_data->plugin_read(event);
      }

      break;
    }

    case PLUGIN_WRITE:
    {
      P159_data_struct *P159_data = static_cast<P159_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P159_data) {
        success = P159_data->plugin_write(event, string);
      }

      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P159_data_struct *P159_data = static_cast<P159_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P159_data) {
        success = P159_data->plugin_get_config_value(event, string);
      }

      break;
    }

    case PLUGIN_EXIT:
    {
      P159_data_struct *P159_data = static_cast<P159_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P159_data) {
        P159_data->disconnectSerial();
      }

      success = true;

      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P159_data_struct *P159_data = static_cast<P159_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P159_data) && !P159_data->isRunning()) {
        success = P159_data->processSensor(event); // Not Running can have 50 msec delays included, so NOT in PLUGIN_FIFTY_PER_SECOND !
      }

      break;
    }
    case PLUGIN_FIFTY_PER_SECOND:
    {
      P159_data_struct *P159_data = static_cast<P159_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P159_data) && P159_data->isRunning()) {
        success = P159_data->processSensor(event); // When running no delays are inserted, so can go in PLUGIN_FIFTY_PER_SECOND
      }

      break;
    }
  } // switch

  return success;
}   // function

#endif // ifdef USES_P159
