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


#include <Adafruit_MotorShield.h>


#define PLUGIN_048
#define PLUGIN_ID_048         48
#define PLUGIN_NAME_048       "Motor - Adafruit Motorshield v2 [TESTING]"
#define PLUGIN_VALUENAME1_048 "MotorShield v2"

#define Plugin_048_MotorShield_address     PCONFIG(0)
#define Plugin_048_MotorStepsPerRevolution PCONFIG(1)
#define Plugin_048_StepperSpeed            PCONFIG(2)

boolean Plugin_048(byte function, struct EventStruct *event, String& string) {
  boolean success = false;

  Adafruit_MotorShield AFMS;


  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_048;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 0;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = false;
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
      addFormTextBox(F("I2C Address (Hex)"), F("p048_adr"),
                     formatToHex_decimal(Plugin_048_MotorShield_address), 4);

      // FIXME TD-er: Why not using addFormSelectorI2C here?
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      Plugin_048_MotorShield_address     = 0x60;
      Plugin_048_MotorStepsPerRevolution = 200;
      Plugin_048_StepperSpeed            = 10;

      break;
    }


    case PLUGIN_WEBFORM_LOAD: {
      addFormNumericBox(F("Stepper: steps per revolution"), F("p048_MotorStepsPerRevolution")
                        , Plugin_048_MotorStepsPerRevolution);

      addFormNumericBox(F("Stepper speed (rpm)"),           F("p048_StepperSpeed")
                        , Plugin_048_StepperSpeed);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      String plugin1 = web_server.arg(F("p048_adr"));
      Plugin_048_MotorShield_address = (int)strtol(plugin1.c_str(), 0, 16);

      Plugin_048_MotorStepsPerRevolution = getFormItemInt(F("p048_MotorStepsPerRevolution"));

      Plugin_048_StepperSpeed = getFormItemInt(F("p048_StepperSpeed"));
      success                 = true;
      break;
    }

    case PLUGIN_INIT: {
      success = true;
      break;
    }

    case PLUGIN_READ: {
      success = false;
      break;
    }

    case PLUGIN_WRITE: {
      String cmd = parseString(string, 1);

      // Commands:
      // MotorShieldCMD,<DCMotor>,<Motornumber>,<Forward/Backward/Release>,<Speed>

      if (cmd.equalsIgnoreCase(F("MotorShieldCMD")))
      {
        String param1 = parseString(string, 2);
        String param2 = parseString(string, 3);
        String param3 = parseString(string, 4);
        String param4 = parseString(string, 5);
        String param5 = parseString(string, 6);

        int p2_int;
        int p4_int;
        const bool param2_is_int = validIntFromString(param2, p2_int);
        const bool param4_is_int = validIntFromString(param4, p4_int);

        // Create the motor shield object with the default I2C address
        AFMS = Adafruit_MotorShield(Plugin_048_MotorShield_address);
        String log = F("MotorShield: Address: 0x");
        log += String(Plugin_048_MotorShield_address, HEX);
        addLog(LOG_LEVEL_DEBUG, log);

        if (param1.equalsIgnoreCase(F("DCMotor"))) {
          if (param2_is_int && (p2_int > 0) && (p2_int < 5))
          {
            Adafruit_DCMotor *myMotor;
            myMotor = AFMS.getMotor(p2_int);

            if (param3.equalsIgnoreCase(F("Forward")))
            {
              byte speed = 255;

              if (param4_is_int && (p4_int >= 0) && (p4_int <= 255)) {
                speed = p4_int;
              }
              AFMS.begin();
              addLog(LOG_LEVEL_INFO, String(F("DCMotor")) + param2 + String(F("->Forward Speed: ")) + String(speed));
              myMotor->setSpeed(speed);
              myMotor->run(FORWARD);
              success = true;
            }

            if (param3.equalsIgnoreCase(F("Backward")))
            {
              byte speed = 255;

              if (param4_is_int && (p4_int >= 0) && (p4_int <= 255)) {
                speed = p4_int;
              }
              AFMS.begin();
              addLog(LOG_LEVEL_INFO, String(F("DCMotor")) + param2 + String(F("->Backward Speed: ")) + String(speed));
              myMotor->setSpeed(speed);
              myMotor->run(BACKWARD);
              success = true;
            }

            if (param3.equalsIgnoreCase(F("Release")))
            {
              AFMS.begin();
              addLog(LOG_LEVEL_INFO, String(F("DCMotor")) + param2 + String(F("->Release")));
              myMotor->run(RELEASE);
              success = true;
            }
          }
        }

        // MotorShieldCMD,<Stepper>,<Motornumber>,<Forward/Backward/Release>,<Steps>,<SINGLE/DOUBLE/INTERLEAVE/MICROSTEP>
        if (param1.equalsIgnoreCase(F("Stepper")))
        {
          // Stepper# is which port it is connected to. If you're using M1 and M2, its port 1.
          // If you're using M3 and M4 indicate port 2
          if (param2_is_int && (p2_int > 0) && (p2_int < 3))
          {
            Adafruit_StepperMotor *myStepper;
            myStepper = AFMS.getStepper(Plugin_048_MotorStepsPerRevolution, p2_int);
            myStepper->setSpeed(Plugin_048_StepperSpeed);

            if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
              String log = F("MotorShield: StepsPerRevolution: ");
              log += String(Plugin_048_MotorStepsPerRevolution);
              log += F(" Stepperspeed: ");
              log += String(Plugin_048_StepperSpeed);
              addLog(LOG_LEVEL_DEBUG_MORE, log);
            }

            if (param3.equalsIgnoreCase(F("Forward")))
            {
              if (param4_is_int && (p4_int != 0))
              {
                int steps = p4_int;

                if (param5.equalsIgnoreCase(F("SINGLE")))
                {
                  AFMS.begin();
                  addLog(LOG_LEVEL_INFO, String(F("Stepper")) + param2 + String(F("->Forward Steps: ")) +
                         steps + String(F(" SINGLE")));
                  myStepper->step(steps, FORWARD, SINGLE);
                  success = true;
                }

                if (param5.equalsIgnoreCase(F("DOUBLE")))
                {
                  AFMS.begin();
                  addLog(LOG_LEVEL_INFO, String(F("Stepper")) + param2 + String(F("->Forward Steps: ")) +
                         steps + String(F(" DOUBLE")));
                  myStepper->step(steps, FORWARD, DOUBLE);
                  success = true;
                }

                if (param5.equalsIgnoreCase(F("INTERLEAVE")))
                {
                  AFMS.begin();
                  addLog(LOG_LEVEL_INFO, String(F("Stepper")) + param2 + String(F("->Forward Steps: ")) +
                         steps + String(F(" INTERLEAVE")));
                  myStepper->step(steps, FORWARD, INTERLEAVE);
                  success = true;
                }

                if (param5.equalsIgnoreCase(F("MICROSTEP")))
                {
                  AFMS.begin();
                  addLog(LOG_LEVEL_INFO, String(F("Stepper")) + param2 + String(F("->Forward Steps: ")) +
                         steps + String(F(" MICROSTEP")));
                  myStepper->step(steps, FORWARD, MICROSTEP);
                  success = true;
                }
              }
            }

            if (param3.equalsIgnoreCase(F("Backward")))
            {
              if (param4_is_int && (p4_int != 0))
              {
                int steps = p4_int;

                if (param5.equalsIgnoreCase(F("SINGLE")))
                {
                  AFMS.begin();
                  addLog(LOG_LEVEL_INFO, String(F("Stepper")) + param2 + String(F("->Backward Steps: ")) +
                         steps + String(F(" SINGLE")));
                  myStepper->step(steps, BACKWARD, SINGLE);
                  success = true;
                }

                if (param5.equalsIgnoreCase(F("DOUBLE")))
                {
                  AFMS.begin();
                  addLog(LOG_LEVEL_INFO, String(F("Stepper")) + param2 + String(F("->Backward Steps: ")) +
                         steps + String(F(" DOUBLE")));
                  myStepper->step(steps, BACKWARD, DOUBLE);
                  success = true;
                }

                if (param5.equalsIgnoreCase(F("INTERLEAVE")))
                {
                  AFMS.begin();
                  addLog(LOG_LEVEL_INFO, String(F("Stepper")) + param2 + String(F("->Backward Steps: ")) +
                         steps + String(F(" INTERLEAVE")));
                  myStepper->step(steps, BACKWARD, INTERLEAVE);
                  success = true;
                }

                if (param5.equalsIgnoreCase(F("MICROSTEP")))
                {
                  AFMS.begin();
                  addLog(LOG_LEVEL_INFO, String(F("Stepper")) + param2 + String(F("->Backward Steps: ")) +
                         steps + String(F(" MICROSTEP")));
                  myStepper->step(steps, BACKWARD, MICROSTEP);
                  success = true;
                }
              }
            }

            if (param3.equalsIgnoreCase(F("Release")))
            {
              AFMS.begin();
              addLog(LOG_LEVEL_INFO, String(F("Stepper")) + param2 + String(F("->Release.")));
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
