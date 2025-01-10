#include "_Plugin_Helper.h"
#ifdef USES_P048

// #######################################################################################################
// #################################### Plugin 048: Adafruit Motorshield v2 ##############################
// #######################################################################################################

// Adafruit Motorshield v2
// like this one: https://www.adafruit.com/products/1438
// based on this library: https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library
// written by https://github.com/krikk
// Currently DC Motors and Steppers are implemented, Servos are in default firmware!!!


# include <Adafruit_MotorShield.h>


# define PLUGIN_048
# define PLUGIN_ID_048         48
# define PLUGIN_NAME_048       "Motor - Adafruit Motorshield v2"
# define PLUGIN_VALUENAME1_048 "MotorShield v2"

# define Plugin_048_MotorShield_address     PCONFIG(0)
# define Plugin_048_MotorStepsPerRevolution PCONFIG(1)
# define Plugin_048_StepperSpeed            PCONFIG(2)

boolean Plugin_048(uint8_t function, struct EventStruct *event, String& string) {
  boolean success = false;

  Adafruit_MotorShield AFMS;


  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      auto& dev = Device[++deviceCount];
      dev.Number           = PLUGIN_ID_048;
      dev.Type             = DEVICE_TYPE_I2C;
      dev.VType            = Sensor_VType::SENSOR_TYPE_NONE;
      dev.I2CNoDeviceCheck = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_048);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0],
               PSTR(PLUGIN_VALUENAME1_048));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      addFormTextBox(F("I2C Address (Hex)"), F("i2c_addr"),
                     formatToHex_decimal(Plugin_048_MotorShield_address), 4);

      // FIXME TD-er: Why not using addFormSelectorI2C here?
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = Plugin_048_MotorShield_address;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      Plugin_048_MotorShield_address     = 0x60;
      Plugin_048_MotorStepsPerRevolution = 200;
      Plugin_048_StepperSpeed            = 10;

      break;
    }


    case PLUGIN_WEBFORM_LOAD: {
      addFormNumericBox(F("Stepper: steps per revolution"), F("steps")
                        , Plugin_048_MotorStepsPerRevolution);

      addFormNumericBox(F("Stepper speed (rpm)"),           F("speed")
                        , Plugin_048_StepperSpeed);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      String plugin1 = webArg(F("i2c_addr"));
      Plugin_048_MotorShield_address = (int)strtol(plugin1.c_str(), 0, 16);

      Plugin_048_MotorStepsPerRevolution = getFormItemInt(F("steps"));

      Plugin_048_StepperSpeed = getFormItemInt(F("speed"));
      success                 = true;
      break;
    }

    case PLUGIN_INIT: {
      success = true;
      break;
    }

    case PLUGIN_WRITE: {
      # if FEATURE_I2C_DEVICE_CHECK

      if (!I2C_deviceCheck(Plugin_048_MotorShield_address, event->TaskIndex, 10, PLUGIN_I2C_GET_ADDRESS)) {
        break; // Will return the default false for success
      }
      # endif // if FEATURE_I2C_DEVICE_CHECK
      String cmd = parseString(string, 1);

      // Commands:
      // MotorShieldCMD,<DCMotor>,<Motornumber>,<Forward/Backward/Release>,<Speed>

      if (equals(cmd, F("motorshieldcmd")))
      {
        const String param1 = parseString(string, 2);
        const String param2 = parseString(string, 3);
        const String param3 = parseString(string, 4);
        const String param4 = parseString(string, 5);
        const String param5 = parseString(string, 6);

        int32_t p2_int;
        int32_t p4_int;
        const bool param2_is_int = validIntFromString(param2, p2_int);
        const bool param4_is_int = validIntFromString(param4, p4_int);

        // Create the motor shield object with the default I2C address
        AFMS = Adafruit_MotorShield(Plugin_048_MotorShield_address);
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          addLog(LOG_LEVEL_DEBUG, strformat(F("MotorShield: Address: 0x%x"), Plugin_048_MotorShield_address));
        }
        # endif // ifndef BUILD_NO_DEBUG

        if (equals(param1, F("dcmotor"))) {
          if (param2_is_int && (p2_int > 0) && (p2_int < 5))
          {
            Adafruit_DCMotor *myMotor;
            myMotor = AFMS.getMotor(p2_int);

            if (equals(param3, F("forward")))
            {
              uint8_t speed = 255;

              if (param4_is_int && (p4_int >= 0) && (p4_int <= 255)) {
                speed = p4_int;
              }
              AFMS.begin();

              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                addLog(LOG_LEVEL_INFO, strformat(F("DCMotor%s->Forward Speed: %d"), param2.c_str(), speed));
              }
              myMotor->setSpeed(speed);
              myMotor->run(FORWARD);
              success = true;
            }

            if (equals(param3, F("backward")))
            {
              uint8_t speed = 255;

              if (param4_is_int && (p4_int >= 0) && (p4_int <= 255)) {
                speed = p4_int;
              }
              AFMS.begin();

              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                addLog(LOG_LEVEL_INFO, strformat(F("DCMotor%s->Backward Speed: %d"), param2.c_str(), speed));
              }

              myMotor->setSpeed(speed);
              myMotor->run(BACKWARD);
              success = true;
            }

            if (equals(param3, F("release")))
            {
              AFMS.begin();

              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                addLog(LOG_LEVEL_INFO, strformat(F("DCMotor%s->Release"), param2.c_str()));
              }
              myMotor->run(RELEASE);
              success = true;
            }
          }
        }

        // MotorShieldCMD,<Stepper>,<Motornumber>,<Forward/Backward/Release>,<Steps>,<SINGLE/DOUBLE/INTERLEAVE/MICROSTEP>
        if (equals(param1, F("stepper")))
        {
          // Stepper# is which port it is connected to. If you're using M1 and M2, its port 1.
          // If you're using M3 and M4 indicate port 2
          if (param2_is_int && (p2_int > 0) && (p2_int < 3))
          {
            Adafruit_StepperMotor *myStepper;
            myStepper = AFMS.getStepper(Plugin_048_MotorStepsPerRevolution, p2_int);
            myStepper->setSpeed(Plugin_048_StepperSpeed);

            # ifndef BUILD_NO_DEBUG

            if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
              addLog(LOG_LEVEL_DEBUG_MORE, strformat(F("MotorShield: StepsPerRevolution: %d Stepperspeed: %d"),
                                                     Plugin_048_MotorStepsPerRevolution,
                                                     Plugin_048_StepperSpeed));
            }
            # endif // ifndef BUILD_NO_DEBUG

            if (equals(param3, F("forward")))
            {
              if (param4_is_int && (p4_int != 0))
              {
                int steps = p4_int;

                if (equals(param5, F("single")))
                {
                  AFMS.begin();
                  myStepper->step(steps, FORWARD, SINGLE);
                  success = true;
                }

                if (equals(param5, F("double")))
                {
                  AFMS.begin();
                  myStepper->step(steps, FORWARD, DOUBLE);
                  success = true;
                }

                if (equals(param5, F("interleave")))
                {
                  AFMS.begin();
                  myStepper->step(steps, FORWARD, INTERLEAVE);
                  success = true;
                }

                if (equals(param5, F("microstep")))
                {
                  AFMS.begin();
                  myStepper->step(steps, FORWARD, MICROSTEP);
                  success = true;
                }

                if (success && loglevelActiveFor(LOG_LEVEL_INFO)) {
                  addLog(LOG_LEVEL_INFO, strformat(F("Stepper%s->Forward Steps: %d %s"), param2.c_str(), steps, param5.c_str()));
                }
              }
            }

            if (equals(param3, F("backward")))
            {
              if (param4_is_int && (p4_int != 0))
              {
                int steps = p4_int;

                if (equals(param5, F("single")))
                {
                  AFMS.begin();
                  myStepper->step(steps, BACKWARD, SINGLE);
                  success = true;
                }

                if (equals(param5, F("double")))
                {
                  AFMS.begin();
                  myStepper->step(steps, BACKWARD, DOUBLE);
                  success = true;
                }

                if (equals(param5, F("interleave")))
                {
                  AFMS.begin();
                  myStepper->step(steps, BACKWARD, INTERLEAVE);
                  success = true;
                }

                if (equals(param5, F("microstep")))
                {
                  AFMS.begin();
                  myStepper->step(steps, BACKWARD, MICROSTEP);
                  success = true;
                }

                if (success && loglevelActiveFor(LOG_LEVEL_INFO)) {
                  addLog(LOG_LEVEL_INFO, strformat(F("Stepper%s->Backward Steps: %d %s"), param2.c_str(), steps, param5.c_str()));
                }
              }
            }

            if (equals(param3, F("release")))
            {
              AFMS.begin();

              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                addLog(LOG_LEVEL_INFO, strformat(F("Stepper%s->Release."), param2.c_str()));
              }
              myStepper->release();
              success = true;
            }
          }
        }
      }

      break;
    }
  }
  return success;
}

#endif // USES_P048
