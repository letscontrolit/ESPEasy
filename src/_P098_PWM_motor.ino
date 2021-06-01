#include "_Plugin_Helper.h"
#ifdef USES_P098

// #######################################################################################################
// ######################## Plugin 098 PWM Motor I2C Barometric Pressure Sensor  ########################
// #######################################################################################################


# include "src/PluginStructs/P098_data_struct.h"

# define PLUGIN_098
# define PLUGIN_ID_098         98
# define PLUGIN_NAME_098       "Motor - PWM Motor"
# define PLUGIN_VALUENAME1_098 "Position"
# define PLUGIN_VALUENAME2_098 "LimitA"
# define PLUGIN_VALUENAME3_098 "LimitB"
# define PLUGIN_VALUENAME4_098 "State"


# define P098_PWM_FREQ       PCONFIG_LONG(0)
# define P098_FLAGS          PCONFIG_LONG(1)
# define P098_LIMIT_SWA_GPIO PCONFIG(0)
# define P098_LIMIT_SWB_GPIO PCONFIG(1)
# define P098_MOTOR_CONTROL  PCONFIG(2)
# ifdef ESP32
#  define P098_ANALOG_GPIO    PCONFIG(3)
# endif // ifdef ESP32
# define P098_LIMIT_SWA_DEBOUNCE PCONFIG(4)
# define P098_LIMIT_SWB_DEBOUNCE PCONFIG(5)


# define P098_FLAGBIT_LIM_A_PULLUP         0
# define P098_FLAGBIT_LIM_B_PULLUP         1
# define P098_FLAGBIT_ENC_IN_PULLUP        2
# define P098_FLAGBIT_LIM_A_INVERTED       3
# define P098_FLAGBIT_LIM_B_INVERTED       4
# define P098_FLAGBIT_MOTOR_FWD_INVERTED   5
# define P098_FLAGBIT_MOTOR_REV_INVERTED   6


boolean Plugin_098(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_098;
      Device[deviceCount].Type               = DEVICE_TYPE_TRIPLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_098);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_098));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_098));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_098));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_098));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("Motor Fwd"));
      event->String2 = formatGpioName_output(F("Motor Rev"));
      event->String3 = formatGpioName_input_optional(F("Encoder"));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P098_LIMIT_SWA_GPIO     = -1;
      P098_LIMIT_SWB_GPIO     = -1;
      P098_LIMIT_SWA_DEBOUNCE = 100;
      P098_LIMIT_SWB_DEBOUNCE = 100;
      P098_MOTOR_CONTROL      = 0; // No PWM
      P098_PWM_FREQ           = 1000;
      # ifdef ESP32
      P098_ANALOG_GPIO = -1;       // Analog feedback
      # endif // ifdef ESP32

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormCheckBox(F("Motor Fwd Inverted"), F("mot_fwd_inv"), bitRead(P098_FLAGS, P098_FLAGBIT_MOTOR_FWD_INVERTED));
      addFormCheckBox(F("Motor Rev Inverted"), F("mot_rev_inv"), bitRead(P098_FLAGS, P098_FLAGBIT_MOTOR_REV_INVERTED));
      addFormCheckBox(F("Encoder Pull-Up"),    F("enc_pu"),      bitRead(P098_FLAGS, P098_FLAGBIT_ENC_IN_PULLUP));

      {
        # define P098_PWM_MODE_TYPES  static_cast<int>(P098_config_struct::PWM_mode_type::MAX_TYPE)
        String options[P098_PWM_MODE_TYPES];
        int optionValues[P098_PWM_MODE_TYPES];

        for (int i = 0; i < P098_PWM_MODE_TYPES; ++i) {
          options[i]      = P098_config_struct::toString(static_cast<P098_config_struct::PWM_mode_type>(i));
          optionValues[i] = i;
        }
        addFormSelector(F("Motor Control"), F("p098_motor_contr"), P098_PWM_MODE_TYPES, options, optionValues, P098_MOTOR_CONTROL);
      }
      addFormNumericBox(F("PWM Frequency"), F("p098_pwm_freq"), P098_PWM_FREQ, 1000, 100000);
      addUnit(F("Hz"));
      # ifdef ESP32
      {
        addRowLabel(formatGpioName_input_optional(F("Analog Feedback")));
        addADC_PinSelect(true, F("analogpin"), P098_ANALOG_GPIO);
      }
      # endif // ifdef ESP32


      addFormSubHeader(F("Limit Switches"));

      addFormPinSelect(formatGpioName_input_optional(F("Limit A")), F("limit_a"), P098_LIMIT_SWA_GPIO);
      addFormNumericBox(F("Limit A Debounce"), F("limit_a_debounce"), P098_LIMIT_SWA_DEBOUNCE, 0, 1000);
      addUnit(F("ms"));
      addFormCheckBox(F("Limit A Pull-Up"),  F("limit_a_pu"),  bitRead(P098_FLAGS, P098_FLAGBIT_LIM_A_PULLUP));
      addFormCheckBox(F("Limit A Inverted"), F("limit_a_inv"), bitRead(P098_FLAGS, P098_FLAGBIT_LIM_A_INVERTED));

      addFormSeparator(2);

      addFormPinSelect(formatGpioName_input_optional(F("Limit B")), F("limit_b"), P098_LIMIT_SWB_GPIO);
      addFormCheckBox(F("Limit B Pull-Up"),  F("limit_b_pu"),  bitRead(P098_FLAGS, P098_FLAGBIT_LIM_B_PULLUP));
      addFormCheckBox(F("Limit B Inverted"), F("limit_b_inv"), bitRead(P098_FLAGS, P098_FLAGBIT_LIM_B_INVERTED));
      addFormNumericBox(F("Limit B Debounce"), F("limit_b_debounce"), P098_LIMIT_SWB_DEBOUNCE, 0, 1000);
      addUnit(F("ms"));


      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P098_LIMIT_SWA_GPIO     = getFormItemInt(F("limit_a"));
      P098_LIMIT_SWB_GPIO     = getFormItemInt(F("limit_b"));
      P098_LIMIT_SWA_DEBOUNCE = getFormItemInt(F("limit_a_debounce"));
      P098_LIMIT_SWB_DEBOUNCE = getFormItemInt(F("limit_b_debounce"));

      P098_MOTOR_CONTROL = getFormItemInt(F("p098_motor_contr"));
      P098_PWM_FREQ      = getFormItemInt(F("p098_pwm_freq"));
      # ifdef ESP32
      P098_ANALOG_GPIO = getFormItemInt(F("analogpin"));
      # endif // ifdef ESP32

      P098_FLAGS = 0;

      if (isFormItemChecked(F("mot_fwd_inv"))) { bitSet(P098_FLAGS, P098_FLAGBIT_MOTOR_FWD_INVERTED); }

      if (isFormItemChecked(F("mot_rev_inv"))) { bitSet(P098_FLAGS, P098_FLAGBIT_MOTOR_REV_INVERTED); }

      if (isFormItemChecked(F("enc_pu"))) { bitSet(P098_FLAGS, P098_FLAGBIT_ENC_IN_PULLUP); }

      if (isFormItemChecked(F("limit_a_pu"))) { bitSet(P098_FLAGS, P098_FLAGBIT_LIM_A_PULLUP); }

      if (isFormItemChecked(F("limit_b_pu"))) { bitSet(P098_FLAGS, P098_FLAGBIT_LIM_B_PULLUP); }

      if (isFormItemChecked(F("limit_a_inv"))) { bitSet(P098_FLAGS, P098_FLAGBIT_LIM_A_INVERTED); }

      if (isFormItemChecked(F("limit_b_inv"))) { bitSet(P098_FLAGS, P098_FLAGBIT_LIM_B_INVERTED); }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      P098_config_struct config;
      config.motorFwd.gpio       = CONFIG_PIN1;
      config.motorRev.gpio       = CONFIG_PIN2;
      config.encoder.gpio        = CONFIG_PIN3;
      config.limitA.gpio         = P098_LIMIT_SWA_GPIO;
      config.limitB.gpio         = P098_LIMIT_SWB_GPIO;
      config.limitA.debounceTime = P098_LIMIT_SWA_DEBOUNCE;
      config.limitB.debounceTime = P098_LIMIT_SWB_DEBOUNCE;
      # ifdef ESP32
      config.gpio_analogIn = P098_ANALOG_GPIO;
      # endif // ifdef ESP32
      config.motorFwd.inverted = bitRead(P098_FLAGS, P098_FLAGBIT_MOTOR_FWD_INVERTED);
      config.motorRev.inverted = bitRead(P098_FLAGS, P098_FLAGBIT_MOTOR_REV_INVERTED);
      config.limitA.inverted   = bitRead(P098_FLAGS, P098_FLAGBIT_LIM_A_INVERTED);
      config.limitB.inverted   = bitRead(P098_FLAGS, P098_FLAGBIT_LIM_B_INVERTED);
      config.limitA.pullUp     = bitRead(P098_FLAGS, P098_FLAGBIT_LIM_A_PULLUP);
      config.limitB.pullUp     = bitRead(P098_FLAGS, P098_FLAGBIT_LIM_B_PULLUP);
      config.encoder.pullUp    = bitRead(P098_FLAGS, P098_FLAGBIT_ENC_IN_PULLUP);

      config.PWM_mode = static_cast<P098_config_struct::PWM_mode_type>(P098_MOTOR_CONTROL);


      initPluginTaskData(event->TaskIndex, new (std::nothrow) P098_data_struct(config));
      P098_data_struct *P098_data =
        static_cast<P098_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P098_data) {
        P098_data->begin();
        success = true;
      }
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P098_data_struct *P098_data =
        static_cast<P098_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P098_data) {
        bool limitA_triggered, limitB_triggered = false;
        P098_data->getLimitSwitchStates(limitA_triggered, limitB_triggered);

        if (!P098_data->loop()) {}

        switch (P098_data->state) {
          case P098_data_struct::State::Idle:
          case P098_data_struct::State::RunFwd:
          case P098_data_struct::State::RunRev:
            break;
          case P098_data_struct::State::StopLimitSw:
          case P098_data_struct::State::StopPosReached:
          {
            if (Settings.UseRules) {
              String RuleEvent = getTaskDeviceName(event->TaskIndex);
              RuleEvent += '#';

              if (limitA_triggered) {
                eventQueue.addMove(String(RuleEvent + F("limitA")));
              }

              if (limitB_triggered) {
                eventQueue.addMove(String(RuleEvent + F("limitB")));
              }

              if (P098_data->state == P098_data_struct::State::StopPosReached) {
                eventQueue.addMove(String(RuleEvent + F("positionReached")));
              }
            }
            P098_data->state = P098_data_struct::State::Idle;
            break;
          }
        }

        // }
        UserVar[event->BaseVarIndex + 0] = P098_data->getPosition();
        UserVar[event->BaseVarIndex + 1] = (limitA_triggered ? 1 : 0);
        UserVar[event->BaseVarIndex + 2] = (limitB_triggered ? 1 : 0);
        UserVar[event->BaseVarIndex + 3] = static_cast<int>(P098_data->state);
      }
      break;
    }

    case PLUGIN_READ:
    {
      P098_data_struct *P098_data =
        static_cast<P098_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P098_data) {
        // What to do here?
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P098_data_struct *P098_data =
        static_cast<P098_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P098_data) {
        const String command = parseString(string, 1);

        if (command.startsWith(F("pwmmotor"))) {
          if (command == F("pwmmotorhome")) {
            // Run the motor in reverse till limit A switch is reached
            P098_data->findHome();
            success = true;
          } else if (command == F("pwmmotorend")) {
            // Run the motor forward till limit B switch is reached
            P098_data->moveForward(-1);
            success = true;
          } else if (command == F("pwmmotorforward")) {
            // Run the motor N steps forward
            // N <= 0: Move till limit B switch is reached
            P098_data->moveForward(event->Par1);
            success = true;
          } else if (command == F("pwmmotorreverse")) {
            // Run the motor N steps in reverse
            P098_data->moveReverse(event->Par1);
            success = true;
          } else if (command == F("pwmmotorstop")) {
            // Run the motor N steps in reverse
            P098_data->stop();
            success = true;
          } else if (command == F("pwmmotormovetopos")) {
            // Run the motor in the required direction to position N
            // What to do when position is unknown?
            if (!P098_data->moveToPos(event->Par1)) {
              if (!P098_data->homePosSet()) {
                addLog(LOG_LEVEL_ERROR, F("PWM motor: Home position unknown"));
              } else {
                addLog(LOG_LEVEL_ERROR, F("PWM motor: Cannot move to position"));
              }
            }
            success = true;
          }
        }
      }

      break;
    }
  }
  return success;
}

#endif // USES_P098
