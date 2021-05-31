#include "_Plugin_Helper.h"
#ifdef USES_P079

# include "src/PluginStructs/P079_data_struct.h"

// #######################################################################################################
// ################################## Plugin 079: Wemos / Lolin Motorshield ##############################
// #######################################################################################################

// Wemos/Lolin Motorshield:
// I2C control for two Brushed DC motors. Supports Wemos V1 and Lolin V2 Motor Shields.
// Plugin written by Susanne Jaeckel + TungstenE2, Merged Aug-2018 ESPEasy Mega.
// Plugin Updated by ThomasB (ThomasTech), Sep-18-2020: Added support for Lolin V2.0 Motor Shield.
//   Change Log, Sep-22-2020:
//      Added:  Support for Lolin V2.0 Motor Shield. Plugin's Web GUI uses form selector to choose board type.
//      Added:  Command syntax checking. Basic Error msg is reported in browser. Expanded msg in info log.
//      Added:  LolinMotorShieldCMD command keyword is alias to WemosMotorShieldCMD.
//      Added:  Standby command (put board into low power idle).
//      Added:  Brake command (short brake).
//      Added:  I2C Communication test for Lolin V2; Use command "LolinMotorShieldCMD" without parameters.
//      Added:  Use default device name if user forgets to populate the name field on device creation.
//      Added:  New device now defaults I2C address field to "0x30" rather than "0x".
//      Fixed:  Eliminated "Invalid character in names" error message that appears in new plugin.
//      Change: Renamed plugin's title to "Motor - Wemos/Lolin Motorshield" (was "Motor - Wemos Motorshield").
//
// This ESPEasy plugin has incorporated source code from the official motorshield libraries:
//   -> https://github.com/wemos/WEMOS_Motor_Shield_Arduino_Library
//   -> https://github.com/wemos/LOLIN_I2C_MOTOR_Library
//
// Wemos Motor Shield V1.0 requires updated firmware to prevent i2c bus lockups.
// See: https://hackaday.io/project/18439-motor-shield-reprogramming
//
// Note: see wiki for Wemos V1 setup. motor_shield.bin needs to be flashed first!!!
// see wiki: https://www.letscontrolit.com/wiki/index.php?title=WemosMotorshield
//
// Board Docs:
//   Wemos V1.0 Motor Shield: https://diyprojects.io/test-shield-motor-i2c-wemos-d1-mini-pro-drive-2-motors-15vdc
//   Lolin V2.0 Motor Shield: https://docs.wemos.cc/en/latest/d1_mini_shiled/motor.html
//   TB6612FNG DC Motor Driver: https://toshiba.semicon-storage.com/info/docget.jsp?did=10660&prodName=TB6612FNG
//
// Command Syntax (WemosMotorShieldCMD and LolinMotorShieldCMD keywords are interchangeable):
//   WemosMotorShieldCMD,<Motornumber [0,1]>,<Forward/Backward/Stop/Standby/Brake>,<Speed [0-100]>
//   LolinMotorShieldCMD,<Motornumber [0,1]>,<Forward/Backward/Stop/Standby/Brake>,<Speed [0-100]>
// Command Examples:
//   WemosMotorShieldCMD,0,Forward,100
//   WemosMotorShieldCMD,1,Stop
//   LolinMotorShieldCMD,0,Reverse,25
//   LolinMotorShieldCMD,1,Standby
//   LolinMotorShieldCMD,1,Brake
// Command Example for i2c Communication test (Lolin V2 board only)
//   LolinMotorShieldCMD
//
// ************************************************************************************************

# define PLUGIN_079
# define PLUGIN_ID_079         79
# define PLUGIN_NAME_079       "Motor - Wemos/Lolin Motorshield"
# define PLUGIN_VALUENAME1_079 "Motorshield"
# define PLUGIN_DEF_NAME1_079  "Wemos_DC_Motor"
# define PLUGIN_DEF_NAME2_079  "Lolin_DC_Motor"

# define I2C_ADDR_PCFG_P079   PCONFIG(0)
# define SHIELD_VER_PCFG_P079 PCONFIG(1)
# define Plugin_079_MotorShield_type  static_cast<P079_BoardType>(SHIELD_VER_PCFG_P079)


// Command keywords: The two Command Names are interchangeable.
# define CMD_NAME_LOLIN "LolinMotorShieldCMD"
# define CMD_NAME_WEMOS "WemosMotorShieldCMD"


// Compiler Options
// #define VERBOSE_P079                 // Uncomment to enable Verbose info log status messages.

// ************************************************************************************************

boolean Plugin_079(byte function, struct EventStruct *event, String& string)
{
  boolean success        = false;
  MOTOR_STATES motor_dir = MOTOR_STATES::MOTOR_FWD;
  uint8_t motor_number   = P079_MOTOR_A;
  int16_t motor_speed    = 100;


  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_079;
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
      string = F(PLUGIN_NAME_079);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_079));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      I2C_ADDR_PCFG_P079 = DEF_I2C_ADDRESS_079;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      if ((I2C_ADDR_PCFG_P079 < 0x01) || (I2C_ADDR_PCFG_P079 > 0x7f)) { // Validate I2C Addr.
        I2C_ADDR_PCFG_P079 = DEF_I2C_ADDRESS_079;
      }
      String i2c_addres_string = formatToHex(I2C_ADDR_PCFG_P079);
      addFormTextBox(F("I2C Address (Hex)"), F("i2c_addr"), i2c_addres_string, 4);

      // Validate Shield Type.
      bool valid = false;

      switch (Plugin_079_MotorShield_type) {
        case P079_BoardType::WemosMotorshield:
        case P079_BoardType::LolinMotorshield:
          valid = true;
          break;

          // TD-er: Do not use defaul: here, as the compiler can now warn if we're missing a newly added option
      }

      if (!valid) {
        SHIELD_VER_PCFG_P079 = static_cast<int>(P079_BoardType::WemosMotorshield);
      }


      {
        const __FlashStringHelper * options[] = { F("WEMOS V1.0"), F("LOLIN V2.0") };
        int indices[]          = { static_cast<int>(P079_BoardType::WemosMotorshield), static_cast<int>(P079_BoardType::LolinMotorshield) };
        addFormSelector(F("Motor Shield Type"), F("p079_shield_type"), 2, options, indices, SHIELD_VER_PCFG_P079);
      }

      if (Plugin_079_MotorShield_type == P079_BoardType::WemosMotorshield) {
        addFormNote(F("WEMOS V1.0 Motor Shield requires updated firmware, see <a href='https://www.letscontrolit.com/wiki/index.php?title=WemosMotorshield'>wiki</a>"));
      }

      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      String i2c_address = webArg(F("i2c_addr"));
      I2C_ADDR_PCFG_P079   = (int)strtol(i2c_address.c_str(), 0, 16);
      SHIELD_VER_PCFG_P079 = getFormItemInt(F("p079_shield_type"));

      // Validate Shield Type.
      bool valid = false;

      switch (Plugin_079_MotorShield_type) {
        case P079_BoardType::WemosMotorshield:
        case P079_BoardType::LolinMotorshield:
          valid = true;
          break;

          // TD-er: Do not use defaul: here, as the compiler can now warn if we're missing a newly added option
      }

      if (!valid) {
        SHIELD_VER_PCFG_P079 = static_cast<int>(P079_BoardType::WemosMotorshield);
      }


      if (getTaskDeviceName(event->TaskIndex).isEmpty()) {                    // Check to see if user entered device name.
        switch (Plugin_079_MotorShield_type) {
          case P079_BoardType::WemosMotorshield:
            safe_strncpy(ExtraTaskSettings.TaskDeviceName, F(PLUGIN_DEF_NAME1_079), sizeof(ExtraTaskSettings.TaskDeviceName)); // Name missing, populate default name.
            break;
          case P079_BoardType::LolinMotorshield:
            safe_strncpy(ExtraTaskSettings.TaskDeviceName, F(PLUGIN_DEF_NAME2_079), sizeof(ExtraTaskSettings.TaskDeviceName)); // Name missing, populate default name.
            break;
        }
      }

      success = true;
      break;
    }

    case PLUGIN_INIT: {
      // Validate Shield Type.
      bool valid = false;

      switch (Plugin_079_MotorShield_type) {
        case P079_BoardType::WemosMotorshield:
        case P079_BoardType::LolinMotorshield:
          valid = true;
          break;

          // TD-er: Do not use defaul: here, as the compiler can now warn if we're missing a newly added option
      }

      if (!valid) {
        SHIELD_VER_PCFG_P079 = static_cast<int>(P079_BoardType::WemosMotorshield);
      }

      success = true;
      break;
    }

    case PLUGIN_READ: {
      success = false;
      break;
    }

    case PLUGIN_WRITE: {

      byte   parse_error = false;
      String tmpString   = string;
      String ModeStr;

      String cmd = parseString(tmpString, 1);

      if (cmd.equalsIgnoreCase(F(CMD_NAME_WEMOS)) || cmd.equalsIgnoreCase(F(CMD_NAME_LOLIN))) {
        switch (Plugin_079_MotorShield_type) {
          case P079_BoardType::WemosMotorshield:
            ModeStr = F("WemosV1");
            break;
          case P079_BoardType::LolinMotorshield:
            ModeStr = F("LolinV2");
            break;
        }
        ModeStr += F(" MotorShield");

        String paramMotor     = parseString(tmpString, 2); // Motor 0 or 1
        String paramDirection = parseString(tmpString, 3); // Direction, Forward/Backward/Stop
        String paramSpeed     = parseString(tmpString, 4); // Speed, 0-100

        if ((paramMotor.isEmpty()) && (paramDirection.isEmpty()) && (paramSpeed.isEmpty())) {
          switch (Plugin_079_MotorShield_type) {
            case P079_BoardType::WemosMotorshield:
            # ifdef VERBOSE_P079
              ModeStr += F(": Unknown CMD");
            # else // ifdef VERBOSE_P079
              ModeStr += F(": ?");
            # endif // ifdef VERBOSE_P079
              break;
            case P079_BoardType::LolinMotorshield:
            {
              LOLIN_I2C_MOTOR lolin(I2C_ADDR_PCFG_P079);
              lolin.getInfo();

              if (lolin.PRODUCT_ID != PRODUCT_ID_I2C_LOLIN) {
                ModeStr += F(": Fail");
              }
              else {
                ModeStr += F(": Pass, ID=");
                ModeStr += lolin.PRODUCT_ID;
                ModeStr += F(", Ver=");
                ModeStr += lolin.VERSION_ID;
              }
              break;
            }
          }
          addLog(LOG_LEVEL_INFO, ModeStr);
          SendStatus(event, ModeStr + F(" <br>")); // Reply (echo) to sender. This will print message on browser.
          return true;                                             // Exit now. Info Log shows Lolin Info.
        }
        else {
          if ((paramMotor == F("0")) || (paramMotor == F("1"))) {
            motor_number = paramMotor.toInt();
          }
          else {
            if (paramMotor.isEmpty()) {
              paramMotor = '?';
            }
            motor_number = 0;
            parse_error  = true;
          }

          if (paramDirection.equalsIgnoreCase(F("Stop"))) {
            motor_dir = MOTOR_STATES::MOTOR_STOP;
          }
          else if (paramDirection.equalsIgnoreCase(F("Forward"))) {
            motor_dir = MOTOR_STATES::MOTOR_FWD;
          }
          else if ((paramDirection.equalsIgnoreCase(F("Backward")))) {
            motor_dir = MOTOR_STATES::MOTOR_REV;
          }
          else if (paramDirection.equalsIgnoreCase(F("Standby"))) {
            motor_dir = MOTOR_STATES::MOTOR_STBY;
          }
          else if (paramDirection.equalsIgnoreCase(F("Brake"))) {
            motor_dir = MOTOR_STATES::MOTOR_BRAKE;
          }
          else {
            paramDirection = '?';
            motor_dir      = MOTOR_STATES::MOTOR_STOP;
            parse_error    = true;
          }

          if (paramSpeed.isEmpty()) {
            switch (motor_dir) {
              case MOTOR_STATES::MOTOR_STOP:
              case MOTOR_STATES::MOTOR_STBY:
              case MOTOR_STATES::MOTOR_BRAKE:
                paramSpeed  = '0';
                motor_speed = 0;
                break;
              default:
                parse_error = true;
                paramSpeed  = '?';
                break;
            }
          }
          else {
            motor_speed = paramSpeed.toInt();

            if ((motor_speed < 0) || (motor_speed > 100)) {
              motor_speed = 100;
              # ifdef VERBOSE_P079
              addLog(LOG_LEVEL_INFO, ModeStr + F(": Warning, invalid speed: Now using 100"));
              # endif // ifdef VERBOSE_P079
            }
          }
        }

        if (parse_error == true) {
          String ErrorStr = ModeStr;
          ErrorStr += F(": CMD Syntax Error");
          addLog(LOG_LEVEL_INFO, ErrorStr);
          SendStatus(event, ErrorStr + F(" <br>")); // Reply (echo) to sender. This will print message on browser.
        }
        else {
          switch (motor_dir) {
            case MOTOR_STATES::MOTOR_STOP:
            case MOTOR_STATES::MOTOR_STBY:
            case MOTOR_STATES::MOTOR_BRAKE:
              paramSpeed = '0';
              break;
            default:
              break;
          }

          switch (Plugin_079_MotorShield_type) {
            case P079_BoardType::WemosMotorshield: {
              WemosMotor Wemos(I2C_ADDR_PCFG_P079, motor_number, MOTOR_FREQ_P079);

              switch (motor_dir) {
                case MOTOR_STATES::MOTOR_FWD:
                  Wemos.setmotor(P079_CW,  motor_speed);
                  break;
                case MOTOR_STATES::MOTOR_REV:
                  Wemos.setmotor(P079_CCW, motor_speed);
                  break;
                case MOTOR_STATES::MOTOR_STOP:
                  Wemos.setmotor(P079_STOP);
                  break;
                case MOTOR_STATES::MOTOR_STBY:
                  Wemos.setmotor(P079_STANDBY);
                  break;
                case MOTOR_STATES::MOTOR_BRAKE:
                  Wemos.setmotor(P079_SHORT_BRAKE);
                  break;
              }

              break;
            }
            case P079_BoardType::LolinMotorshield: {
              LOLIN_I2C_MOTOR lolin(I2C_ADDR_PCFG_P079);
              lolin.changeFreq(MOTOR_CH_BOTH, MOTOR_FREQ_P079);

              switch (motor_dir) {
                case MOTOR_STATES::MOTOR_FWD:
                  lolin.changeStatus(motor_number, MOTOR_STATUS_CW);
                  lolin.changeDuty(motor_number, motor_speed);
                  break;
                case MOTOR_STATES::MOTOR_REV:
                  lolin.changeStatus(motor_number, MOTOR_STATUS_CCW);
                  lolin.changeDuty(motor_number, motor_speed);
                  break;
                case MOTOR_STATES::MOTOR_STOP:
                  lolin.changeStatus(motor_number, MOTOR_STATUS_STOP);
                  break;
                case MOTOR_STATES::MOTOR_STBY:
                  lolin.changeStatus(motor_number, MOTOR_STATUS_STANDBY);
                  break;
                case MOTOR_STATES::MOTOR_BRAKE:
                  lolin.changeStatus(motor_number, MOTOR_STATUS_SHORT_BRAKE);
                  break;
              }

              break;
            }
          }
        }
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          ModeStr += F(": Addr=");
          ModeStr += formatToHex(I2C_ADDR_PCFG_P079);
          ModeStr += F(": Mtr=");
          ModeStr += paramMotor;
          ModeStr += F(", Dir=");
          ModeStr += paramDirection;
          ModeStr += F(", Spd=");
          ModeStr += paramSpeed;
          addLog(LOG_LEVEL_INFO, ModeStr);          
        }

        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P079
