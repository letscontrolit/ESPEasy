#include "_Plugin_Helper.h"
#ifdef USES_P009

# include "src/DataStructs/PinMode.h"
# include "src/ESPEasyCore/ESPEasyGPIO.h"
# include "src/Helpers/_Plugin_Helper_webform.h"

# include "src/PluginStructs/P009_data_struct.h"

// #######################################################################################################
// #################################### Plugin 009: MCP23017 input #######################################
// #######################################################################################################


# define PLUGIN_009
# define PLUGIN_ID_009         9
# define PLUGIN_NAME_009       "Switch input - MCP23017"
# define PLUGIN_VALUENAME1_009 "State"


boolean Plugin_009(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  // static int8_t switchstate[TASKS_MAX];

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number             = PLUGIN_ID_009;
      dev.Type               = DEVICE_TYPE_I2C;
      dev.VType              = Sensor_VType::SENSOR_TYPE_SWITCH;
      dev.InverseLogicOption = true;
      dev.ValueCount         = 1;
      dev.SendDataOption     = true;
      dev.TimerOption        = true;
      dev.TimerOptional      = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_009);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_009));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        String portNames[16];
        int    portValues[16];
        const uint8_t unit    = (CONFIG_PORT - 1) / 16;
        const uint8_t port    = CONFIG_PORT - (unit * 16);
        const uint8_t address = 0x20 + unit;

        for (uint8_t x = 0; x < 16; ++x) {
          portValues[x] = x + 1;
          portNames[x]  = 'P';
          portNames[x] += (x < 8 ? 'A' : 'B');
          portNames[x] += (x < 8 ? x : x - 8);
        }
        addFormSelectorI2C(F("pi2c"), 8, i2cAddressValues, address);
        addFormSelector(F("Port"), F("pport"), 16, portNames, portValues, port);
      } else {
        success = intArrayContains(8, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P009_data_struct::getI2C_address(event);
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      // @giig1967g: set current task value for taking actions after changes
      const uint32_t key = createKey(PLUGIN_MCP, CONFIG_PORT);

      auto it = globalMapPortStatus.find(key);

      if (it != globalMapPortStatus.end()) {
        it->second.previousTask = event->TaskIndex;
      }

      SwitchWebformLoad(
        P009_BOOTSTATE,
        P009_DEBOUNCE,
        P009_DOUBLECLICK,
        P009_DC_MAX_INT,
        P009_LONGPRESS,
        P009_LP_MIN_INT,
        P009_SAFE_BTN);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      uint8_t i2c  = getFormItemInt(F("pi2c"));
      uint8_t port = getFormItemInt(F("pport"));
      CONFIG_PORT = (((i2c - 0x20) << 4) + port);

      SwitchWebformSave(
        event->TaskIndex,
        PLUGIN_MCP,
        P009_BOOTSTATE,
        P009_DEBOUNCE,
        P009_DOUBLECLICK,
        P009_DC_MAX_INT,
        P009_LONGPRESS,
        P009_LP_MIN_INT,
        P009_SAFE_BTN);

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // apply INIT only if PIN is in range. Do not start INIT if pin not set in the device page.
      if (CONFIG_PORT >= 0)
      {
        success = initPluginTaskData(event->TaskIndex, new (std::nothrow) P009_data_struct(event));
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P009_data_struct *P009_data =
        static_cast<P009_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P009_data)
      {
        P009_data->tenPerSecond(event);
        success = true;
      }
      break;
    }

    case PLUGIN_READ:
    {
      // We do not actually read the pin state as this is already done 10x/second
      // Instead we just send the last known state stored in Uservar
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO,
               strformat(F("MCP   : Port=%d State=%d"), CONFIG_PORT, UserVar[event->BaseVarIndex]));
      }
      success = true;
      break;
    }

    case PLUGIN_REQUEST:
    {
      // parseString(string, 1) = device
      // parseString(string, 2) = command
      // parseString(string, 3) = gpio number

      // returns pin value using syntax: [plugin#mcpgpio#pinstate#xx]
      if ((string.length() >= 16) && string.substring(0, 16).equalsIgnoreCase(F("mcpgpio,pinstate")))
      {
        int32_t par1;

        if (validIntFromString(parseString(string, 3), par1)) {
          string = GPIO_MCP_Read(par1);
        }
        success = true;
      }
      break;
    }

    case PLUGIN_TASKTIMER_IN:
    case PLUGIN_DEVICETIMER_IN:
    {
      Scheduler.clearGPIOTimer(PLUGIN_MCP, event->Par1);
      GPIO_MCP_Write(event->Par1, event->Par2);

      // WARNING: operator [] creates an entry in the map if key does not exist
      const uint32_t   key        = createKey(PLUGIN_MCP, event->Par1);
      portStatusStruct tempStatus = globalMapPortStatus[key];

      tempStatus.state        = event->Par2;
      tempStatus.mode         = PIN_MODE_OUTPUT;
      tempStatus.forceMonitor = (tempStatus.monitor) ?  1 :  0; // added to send event for longpulse command
      savePortStatus(key, tempStatus);
      break;
    }
  }
  return success;
}

#endif // USES_P009
