#include "_Plugin_Helper.h"

#ifdef USES_P001

# include "src/DataStructs/PinMode.h"
# include "src/ESPEasyCore/Controller.h"
# include "src/ESPEasyCore/ESPEasyGPIO.h"
# include "src/Helpers/PortStatus.h"
# include "src/Helpers/Scheduler.h"
# include "src/Helpers/_Plugin_Helper_webform.h"

# include "src/PluginStructs/P001_data_struct.h"

// #######################################################################################################
// #################################### Plugin 001: Input Switch #########################################
// #######################################################################################################


# define PLUGIN_001
# define PLUGIN_ID_001 1
# define PLUGIN_NAME_001 "Switch input - Switch"
# define PLUGIN_VALUENAME1_001 "State"

boolean Plugin_001(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  // static uint8_t switchstate[TASKS_MAX];
  // static uint8_t outputstate[TASKS_MAX];
  // static int8_t PinMonitor[GPIO_MAX];
  // static int8_t PinMonitorState[GPIO_MAX];

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number             = PLUGIN_ID_001;
      dev.Type               = DEVICE_TYPE_SINGLE;
      dev.VType              = Sensor_VType::SENSOR_TYPE_SWITCH;
      dev.PullUpOption       = true;
      dev.InverseLogicOption = true;
      dev.ValueCount         = 1;
      dev.SendDataOption     = true;
      dev.TimerOption        = true;
      dev.TimerOptional      = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_001);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_001));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      // FIXME TD-er: Split functionality of this plugin into 2 new ones:
      // - switch/dimmer input
      // - switch output (relays) and pwm output (led) - show the duty cycle as a value
      // [ https://github.com/letscontrolit/ESPEasy/issues/4400 ]
      event->String1 = formatGpioName_bidirectional(F(""));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // @giig1967g: set current task value for taking actions after changes in the task gpio
      const uint32_t key = createKey(PLUGIN_GPIO, CONFIG_PIN1);

      auto it = globalMapPortStatus.find(key);

      if (it != globalMapPortStatus.end())
      {
        it->second.previousTask = event->TaskIndex;
      }

      {
        const __FlashStringHelper *options[] = { F("Switch"), F("Dimmer") };
        const int optionValues[]             = { PLUGIN_001_TYPE_SWITCH, PLUGIN_001_TYPE_DIMMER };
        const uint8_t switchtype             = P001_data_struct::P001_getSwitchType(event);
        constexpr size_t optionCount         = NR_ELEMENTS(optionValues);
        addFormSelector(F("Switch Type"), F("type"), optionCount, options, optionValues, switchtype);

        if (switchtype == PLUGIN_001_TYPE_DIMMER)
        {
          addFormNumericBox(F("Dim value"), F("dimvalue"), P001_DIMMER_VALUE, 0, 255);
        }
      }

      {
        const __FlashStringHelper *buttonOptions[] = {
          F("Normal Switch"),
          F("Push Button Active Low"),
          F("Push Button Active High") };
        const int buttonOptionValues[] = {
          SWITCH_TYPE_NORMAL_SWITCH,
          SWITCH_TYPE_PUSH_ACTIVE_LOW,
          SWITCH_TYPE_PUSH_ACTIVE_HIGH };
        addFormSelector(
          F("Switch Button Type"),
          F("button"),
          NR_ELEMENTS(buttonOptionValues),
          buttonOptions,
          buttonOptionValues,
          P001_BUTTON_TYPE);
      }

      SwitchWebformLoad(
        P001_BOOTSTATE,
        P001_DEBOUNCE,
        P001_DOUBLECLICK,
        P001_DC_MAX_INT,
        P001_LONGPRESS,
        P001_LP_MIN_INT,
        P001_SAFE_BTN);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P001_SWITCH_OR_DIMMER = getFormItemInt(F("type"));

      if (P001_SWITCH_OR_DIMMER == PLUGIN_001_TYPE_DIMMER)
      {
        P001_DIMMER_VALUE = getFormItemInt(F("dimvalue"));
      }

      P001_BUTTON_TYPE = getFormItemInt(F("button"));

      SwitchWebformSave(
        event->TaskIndex,
        PLUGIN_GPIO,
        P001_BOOTSTATE,
        P001_DEBOUNCE,
        P001_DOUBLECLICK,
        P001_DC_MAX_INT,
        P001_LONGPRESS,
        P001_LP_MIN_INT,
        P001_SAFE_BTN);

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // apply INIT only if PORT is in range. Do not start INIT if port not set in the device page.
      if (validGpio(CONFIG_PIN1))
      {
        success = initPluginTaskData(event->TaskIndex, new (std::nothrow) P001_data_struct(event));
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P001_data_struct *P001_data =
        static_cast<P001_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P001_data)
      {
        P001_data->tenPerSecond(event);
        success = true;
      }
      break;
    }

    case PLUGIN_READ:
    {
      // We do not actually read the pin state as this is already done 10x/second
      // Instead we just send the last known state stored in Uservar
# ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO))
      {
        addLogMove(LOG_LEVEL_INFO, concat(F("SW   : State "), static_cast<int>(UserVar[event->BaseVarIndex])));
      }
# endif // ifndef BUILD_NO_DEBUG
      success = true;
      break;
    }

    case PLUGIN_TASKTIMER_IN:
    {
      digitalWrite(event->Par1, event->Par2);

      // setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);

      // WARNING: operator [] creates an entry in the map if key does not exist
      const uint32_t   key        = createKey(PLUGIN_GPIO, event->Par1);
      portStatusStruct tempStatus = globalMapPortStatus[key];

      tempStatus.state = event->Par2;
      tempStatus.mode  = PIN_MODE_OUTPUT;

      // sp    (tempStatus.monitor) ? tempStatus.forceMonitor = 1 : tempStatus.forceMonitor = 0;
      tempStatus.forceMonitor = 1; // added to send event for longpulse command
      savePortStatus(key, tempStatus);
      break;
    }
  }
  return success;
}

#endif // USES_P001
