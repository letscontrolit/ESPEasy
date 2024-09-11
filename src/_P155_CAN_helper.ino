#include "_Plugin_Helper.h"
#ifdef USES_P155

// #######################################################################################################
// ################################### Plugin 155: CAN SJA1000 Importer ##################################
// #######################################################################################################


# include "src/Helpers/_Plugin_Helper_serial.h"
# include <CAN.h>


# define PLUGIN_155
# define PLUGIN_ID_155         155
# define PLUGIN_NAME_155       "Communication - CAN Importer"

# define P155_MAX_NODES      255
# define P155_TOPIC_MAX_SIZE 16
# define P155_TIMEOUT        50

# define P155_VAL1_FLAG 0x01
# define P155_VAL2_FLAG 0x02
# define P155_VAL3_FLAG 0x04
# define P155_VAL4_FLAG 0x10

#define PLUGIN_VALUENAME1_155 "Status"

void P155_check_timeout(EventStruct *event, int idx)
{
  int16_t idx_flag = (0x01 << idx);

  if (PCONFIG_ULONG(idx) > PCONFIG(2))
  {
    if (!(PCONFIG(3) & idx_flag))
    {
      String log_debug = F("Disconnected");
      addLogMove(LOG_LEVEL_INFO, log_debug);
    }
    PCONFIG(3) |= idx_flag;
  }
  else if (!(PCONFIG(3) & idx_flag))
  {
    PCONFIG_ULONG(idx) += 100;
  }
}

boolean Plugin_155(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_155;
      Device[deviceCount].Type               = DEVICE_TYPE_DUMMY;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 0;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].GlobalSyncOption   = false;
      Device[deviceCount].TimerOptional      = true;
      Device[deviceCount].Custom             = true;
      Device[deviceCount].DecimalsOnly       = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_155);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      const uint8_t valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(1)));
      if (valueCount > 0) {
        String lines[valueCount];
        LoadCustomTaskSettings(event->TaskIndex, lines, valueCount, 0);

        for (int i = 0; i < valueCount; ++i) {
        if (lines[i].length() == 0) {
          lines[i] = concat(F("val"), i + 1);
        }
          ExtraTaskSettings.setTaskDeviceValueName(i, lines[i]);
        }
      }
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(1)));
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(1));
      event->idx        = 1;
      success           = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      PCONFIG(0) = 0;
      PCONFIG(1) = static_cast<int16_t>(Sensor_VType::SENSOR_TYPE_NONE);
      PCONFIG(2) = 0;
      break;
    }

    case PLUGIN_FORMAT_USERVAR:
    {
      string = UserVar.getAsString(event->TaskIndex, event->idx, static_cast<Sensor_VType>(PCONFIG(1)), 2);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      String lines[VARS_PER_TASK];

      addRowLabel(F("Node"));
      addNumericBox(F("p155_node"), PCONFIG(0), 0, 0xFFFF);

      addRowLabel(F("Task Id"));
      addNumericBox(F("p155_task_id"), PCONFIG(2) + 1, 0, 0xFFFF);

      addRowLabel(F("Sensor Type"));
      addTextBox(F("p155_sensor_type"),
                getSensorTypeLabel(static_cast<Sensor_VType>(PCONFIG(1))),
                32,
                true);

      LoadCustomTaskSettings(event->TaskIndex, lines, VARS_PER_TASK, 0);

      for (int i = 0; i < VARS_PER_TASK; ++i) {
        addRowLabel(concat(F("Label "), i + 1));
        if (lines[i].length() == 0) {
          lines[i] = concat(F("val"), i + 1);
        }
        addTextBox(concat(F("p155_value"), i), lines[i], 0);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      String lines[VARS_PER_TASK];

      addRowLabel(F("Node"));
      PCONFIG(0) = getFormItemInt(F("p155_node"));
      PCONFIG(2) = getFormItemInt(F("p155_task_id"))-1;

      for (int i = 0; i < VARS_PER_TASK; ++i) {
        lines[i] = webArg(concat(F("p155_value"), i));
        if (lines[i].length() == 0) {
          lines[i] = concat(F("val"), i + 1);
        }
      }

      SaveCustomTaskSettings(event->TaskIndex, lines, VARS_PER_TASK, 0);

      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      String log = "CAN Import :" + String(event->TaskIndex) + " got READ event with values: " +
          String(event->Par1) + String(" ") + String(event->Par2);
      addLogMove(LOG_LEVEL_DEBUG, log);
      if (event->Par1 == PCONFIG(0) && event->Par2 == PCONFIG(2))
      {
        const int16_t sensorTypeVal = static_cast<int16_t>(event->sensorType);
        if (PCONFIG(1) != sensorTypeVal) {
          PCONFIG(1) = sensorTypeVal;

          //Zero all other values if sensor type changed
          for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
            if (i != event->idx) {
              UserVar.setUint32(event->TaskIndex, i, 0);
            }
          }
        }

        sendData(event);
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      // P155_check_timeout(event, 0);
      // P155_check_timeout(event, 1);
      // P155_check_timeout(event, 2);
      // P155_check_timeout(event, 3);
      break;
    }

    case PLUGIN_INIT:
    {
      Settings.TaskDeviceTimer[event->TaskIndex] = 0;
      //Clear timeout flags
      PCONFIG(3) = 0;
      PCONFIG_ULONG(0) = 0;
      PCONFIG_ULONG(1) = 0;
      PCONFIG_ULONG(2) = 0;
      PCONFIG_ULONG(3) = 0;
      success = true;
      break;
    }

    case PLUGIN_EXIT:
    {
      success = true;
      break;
    }
  }
  return success;
}

#endif // USES_P155
