#include "_Plugin_Helper.h"
#ifdef USES_P019

# include "src/DataStructs/PinMode.h"
# include "src/ESPEasyCore/ESPEasyGPIO.h"
# include "src/Helpers/_Plugin_Helper_webform.h"

# include "src/PluginStructs/P019_data_struct.h"

// #######################################################################################################
// #################################### Plugin 019: PCF8574 ##############################################
// #######################################################################################################


# define PLUGIN_019
# define PLUGIN_ID_019         19
# define PLUGIN_NAME_019       "Switch input - PCF8574"
# define PLUGIN_VALUENAME1_019 "State"


boolean Plugin_019(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number             = PLUGIN_ID_019;
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
      string = F(PLUGIN_NAME_019);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_019));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        String portNames[8];
        int    portValues[8];
        const uint8_t unit = (CONFIG_PORT - 1) / 8;
        const uint8_t port = CONFIG_PORT - (unit * 8);
        uint8_t address    = 0x20 + unit;

        if (unit > 7) { address += 0x10; }

        for (uint8_t x = 0; x < 8; ++x) {
          portValues[x] = x + 1;
          portNames[x]  = 'P';
          portNames[x] += x;
        }
        addFormSelectorI2C(F("pi2c"), 16, i2cAddressValues, address);
        addFormSelector(F("Port"), F("pport"), 8, portNames, portValues, port);
        addFormNote(F("PCF8574 uses addresses 0x20..0x27, PCF8574<b>A</b> uses addresses 0x38..0x3F."));
      } else {
        success = intArrayContains(16, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P019_data_struct::getI2C_address(event);
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      // @giig1967g: set current task value for taking actions after changes
      const uint32_t key = createKey(PLUGIN_PCF, CONFIG_PORT);

      auto it = globalMapPortStatus.find(key);

      if (it != globalMapPortStatus.end()) {
        it->second.previousTask = event->TaskIndex;
      }
      SwitchWebformLoad(
        P019_BOOTSTATE,
        P019_DEBOUNCE,
        P019_DOUBLECLICK,
        P019_DC_MAX_INT,
        P019_LONGPRESS,
        P019_LP_MIN_INT,
        P019_SAFE_BTN);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      uint8_t i2c = getFormItemInt(F("pi2c"));

      if (i2c > 0x27) { i2c -= 0x10; }

      uint8_t port = getFormItemInt(F("pport"));
      CONFIG_PORT = (((i2c - 0x20) << 3) + port);

      SwitchWebformSave(
        event->TaskIndex,
        PLUGIN_PCF,
        P019_BOOTSTATE,
        P019_DEBOUNCE,
        P019_DOUBLECLICK,
        P019_DC_MAX_INT,
        P019_LONGPRESS,
        P019_LP_MIN_INT,
        P019_SAFE_BTN);

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // apply INIT only if PIN is in range. Do not start INIT if pin not set in the device page.
      if (CONFIG_PORT >= 0)
      {
        success = initPluginTaskData(event->TaskIndex, new (std::nothrow) P019_data_struct(event));
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P019_data_struct *P019_data =
        static_cast<P019_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P019_data)
      {
        P019_data->tenPerSecond(event);
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
               strformat(F("PCF  : Port= %d State=%d"), CONFIG_PORT, UserVar[event->BaseVarIndex]));
      }
      success = true;
      break;
    }

    case PLUGIN_TASKTIMER_IN:
    case PLUGIN_DEVICETIMER_IN:
    {
      Scheduler.clearGPIOTimer(PLUGIN_PCF, event->Par1);
      GPIO_PCF_Write(event->Par1, event->Par2);

      // WARNING: operator [] creates an entry in the map if key does not exist
      const uint32_t   key        = createKey(PLUGIN_PCF, event->Par1);
      portStatusStruct tempStatus = globalMapPortStatus[key];

      tempStatus.state = event->Par2;
      tempStatus.mode  = PIN_MODE_OUTPUT;

      // FIXME TD-er: Why is this different from the MCP code in P009?
      if (function == PLUGIN_TASKTIMER_IN) {
        // sp      tempStatus.forceMonitor = (tempStatus.monitor) ?  1 :  0; // added to send event for longpulse command
        tempStatus.forceMonitor = 1;
      } else {
        tempStatus.forceMonitor = (tempStatus.monitor) ?  1 :  0; // added to send event for longpulse command
      }
      savePortStatus(key, tempStatus);
      break;
    }
  }
  return success;
}

#endif // USES_P019
