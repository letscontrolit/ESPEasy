#ifdef USES_P079

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

#define PLUGIN_079
#define PLUGIN_ID_079         79
#define PLUGIN_NAME_079       "Motor - Wemos/Lolin Motorshield"
#define PLUGIN_VALUENAME1_079 "Motorshield"
#define PLUGIN_DEF_NAME1_079  "Wemos_DC_Motor"
#define PLUGIN_DEF_NAME2_079  "Lolin_DC_Motor"

#define DEF_I2C_ADDRESS_079  0x30
#define I2C_ADDR_PCFG_P079   PCONFIG(0)
#define SHIELD_VER_PCFG_P079 PCONFIG(1)
#define MOTOR_FREQ_P079      1000
#define PRODUCT_ID_I2C_LOLIN 0x02

// Command keywords: The two Command Names are interchangeable.
#define CMD_NAME_LOLIN "LolinMotorShieldCMD"
#define CMD_NAME_WEMOS "WemosMotorShieldCMD"

// Compiler Options
// #define VERBOSE_P079                 // Uncomment to enable Verbose info log status messages.

// ************************************************************************************************

enum BOARD_TYPE
{
  WEMOS_P079 = 0x01,
  LOLIN_P079
};

enum MOTOR_STATES
{
  MOTOR_STOP = 0x00,
  MOTOR_FWD,
  MOTOR_REV,
  MOTOR_STBY,
  MOTOR_BRAKE
};

static uint8_t Plugin_079_MotorShield_address = DEF_I2C_ADDRESS_079;
static uint8_t Plugin_079_MotorShield_type    = WEMOS_P079;

// copied from <WEMOS_Motor.h>
#ifndef __WEMOS_MOTOR_H
# define __WEMOS_MOTOR_H

# if (ARDUINO >= 100)
 #  include "Arduino.h"
# else // if (ARDUINO >= 100)
 #  include "WProgram.h"
# endif  // if (ARDUINO >= 100)

# include "Wire.h"
# include "_Plugin_Helper.h"

# define P079_MOTOR_A     0
# define P079_MOTOR_B     1
# define P079_SHORT_BRAKE 0
# define P079_CCW         1
# define P079_CW          2
# define P079_STOP        3
# define P079_STANDBY     4

class WemosMotor {
public:

  WemosMotor(uint8_t  address,
             uint8_t  motor,
             uint32_t freq);
  WemosMotor(uint8_t  address,
             uint8_t  motor,
             uint32_t freq,
             uint8_t  STBY_IO);
  void setfreq(uint32_t freq);
  void setmotor(uint8_t dir,
                float   pwm_val);
  void setmotor(uint8_t dir);

private:

  uint8_t _address;
  uint8_t _motor;
  bool _use_STBY_IO = false;
  uint8_t _STBY_IO  = 0;
};

#endif // ifndef __WEMOS_MOTOR_H

// end copied from <WEMOS_Motor.h>

// ************************************************************************************************

// Copied from <LOLIN_I2C_MOTOR.h>
#ifndef __LOLIN_I2C_MOTOR_H
# define __LOLIN_I2C_MOTOR_H

enum LOLIN_I2C_CMD
{
  GET_SLAVE_STATUS = 0x01,
  RESET_SLAVE,
  CHANGE_I2C_ADDRESS,
  CHANGE_STATUS,
  CHANGE_FREQ,
  CHANGE_DUTY
};

enum LOLIN_MOTOR_STATUS
{
  MOTOR_STATUS_STOP = 0x00,
  MOTOR_STATUS_CCW,
  MOTOR_STATUS_CW,
  MOTOR_STATUS_SHORT_BRAKE,
  MOTOR_STATUS_STANDBY
};

enum LOLIN_MOTOR_CHANNEL
{
  MOTOR_CH_A = 0x00,
  MOTOR_CH_B,
  MOTOR_CH_BOTH
};

class LOLIN_I2C_MOTOR {
public:

  LOLIN_I2C_MOTOR(unsigned char address = Plugin_079_MotorShield_address);
  unsigned char reset(void);
  unsigned char getInfo(void);
  unsigned char changeStatus(unsigned char ch,
                             unsigned char sta);
  unsigned char changeFreq(unsigned char ch,
                           uint32_t      freq);
  unsigned char changeDuty(unsigned char ch,
                           float         duty);
  unsigned char changeAddress(unsigned char address);

  unsigned char VERSION_ID = 0;
  unsigned char PRODUCT_ID = 0;

private:

  unsigned char _address;
  unsigned char send_data[5] = { 0 };
  unsigned char get_data[2]  = { 0 };
  unsigned char sendData(unsigned char *data,
                         unsigned char  len);
};

#endif // ifndef __LOLIN_I2C_MOTOR_H

// end copied from <LOLIN_I2C_MOTOR.h>

// ************************************************************************************************

boolean Plugin_079(byte function, struct EventStruct *event, String& string)
{
  boolean success      = false;
  uint8_t motor_dir    = P079_CCW;
  uint8_t motor_number = P079_MOTOR_A;
  int16_t motor_speed  = 100;
  String p079_adr_str  = String(F("p079_adr"));
  String p079_shield_type_str = String(F("p079_shield_type"));

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
      Plugin_079_MotorShield_address = (I2C_ADDR_PCFG_P079);
      String i2c_addres_string = formatToHex(Plugin_079_MotorShield_address);
      addFormTextBox(F("I2C Address (Hex)"), p079_adr_str, i2c_addres_string, 4);

      if ((SHIELD_VER_PCFG_P079 != WEMOS_P079) && (SHIELD_VER_PCFG_P079 != LOLIN_P079)) { // Validate Shield Type.
        SHIELD_VER_PCFG_P079 = WEMOS_P079;
      }
      Plugin_079_MotorShield_type = SHIELD_VER_PCFG_P079;

      const String options[] = { F("WEMOS V1.0"), F("LOLIN V2.0") };
      int indices[]          = { WEMOS_P079, LOLIN_P079 };
      addFormSelector(F("Motor Shield Type"), p079_shield_type_str, 2, options, indices, SHIELD_VER_PCFG_P079);

      if (SHIELD_VER_PCFG_P079 == WEMOS_P079) {
        addFormNote(F(
                      "WEMOS V1.0 Motor Shield requires updated firmware, see <a href='https://www.letscontrolit.com/wiki/index.php?title=WemosMotorshield'>wiki</a>"));
      }

      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      String i2c_address = web_server.arg(p079_adr_str);
      I2C_ADDR_PCFG_P079   = (int)strtol(i2c_address.c_str(), 0, 16);
      SHIELD_VER_PCFG_P079 = getFormItemInt(p079_shield_type_str);

      if ((SHIELD_VER_PCFG_P079 != WEMOS_P079) && (SHIELD_VER_PCFG_P079 != LOLIN_P079)) { // Invalid Motor Shield Type.
        SHIELD_VER_PCFG_P079 = WEMOS_P079;
      }
      Plugin_079_MotorShield_type = SHIELD_VER_PCFG_P079;

      if (getTaskDeviceName(event->TaskIndex) == "") {                    // Check to see if user entered device name.
        if (Plugin_079_MotorShield_type == WEMOS_P079) {
          strcpy(ExtraTaskSettings.TaskDeviceName, PLUGIN_DEF_NAME1_079); // Name missing, populate default name.
        }
        else {
          strcpy(ExtraTaskSettings.TaskDeviceName, PLUGIN_DEF_NAME2_079); // Name missing, populate default name.
        }
      }

      success = true;
      break;
    }

    case PLUGIN_INIT: {
      Plugin_079_MotorShield_address = I2C_ADDR_PCFG_P079;

      if ((SHIELD_VER_PCFG_P079 != WEMOS_P079) && (SHIELD_VER_PCFG_P079 != LOLIN_P079)) { // Validate Shield Type, for newly upgraded
                                                                                          // devices.
        SHIELD_VER_PCFG_P079 = WEMOS_P079;
      }
      Plugin_079_MotorShield_type = SHIELD_VER_PCFG_P079;

      success = true;
      break;
    }

    case PLUGIN_READ: {
      success = false;
      break;
    }

    case PLUGIN_WRITE: {
      Plugin_079_MotorShield_type = SHIELD_VER_PCFG_P079;
      byte   parse_error = false;
      String tmpString   = string;
      String ModeStr;

      String cmd = parseString(tmpString, 1);

      if (cmd.equalsIgnoreCase(F(CMD_NAME_WEMOS)) || cmd.equalsIgnoreCase(F(CMD_NAME_LOLIN))) {
        if (Plugin_079_MotorShield_type == WEMOS_P079) {
          ModeStr = F("WemosV1");
        }
        else {
          ModeStr = F("LolinV2");
        }
        ModeStr += F(" MotorShield");

        String paramMotor     = parseString(tmpString, 2); // Motor 0 or 1
        String paramDirection = parseString(tmpString, 3); // Direction, Forward/Backward/Stop
        String paramSpeed     = parseString(tmpString, 4); // Speed, 0-100

        if ((paramMotor == "") && (paramDirection == "") && (paramSpeed == "")) {
          if (Plugin_079_MotorShield_type == WEMOS_P079) {
            #ifdef VERBOSE_P079
            ModeStr += String(F(": Unknown CMD"));
            #else // ifdef VERBOSE_P079
            ModeStr += String(F(": ?"));
            #endif  // ifdef VERBOSE_P079
          }
          else {
            LOLIN_I2C_MOTOR lolin;
            lolin.getInfo();

            if (lolin.PRODUCT_ID != PRODUCT_ID_I2C_LOLIN) {
              ModeStr += String(F(": Fail"));
            }
            else {
              ModeStr += String(F(": Pass, ID=")) + lolin.PRODUCT_ID + String(F(", Ver=")) + lolin.VERSION_ID;
            }
          }
          addLog(LOG_LEVEL_INFO, ModeStr);
          SendStatus(event->Source, ModeStr + String(F(" <br>"))); // Reply (echo) to sender. This will print message on browser.
          return true;                                             // Exit now. Info Log shows Lolin Info.
        }
        else {
          if ((paramMotor == "0") || (paramMotor == "1")) {
            motor_number = paramMotor.toInt();
          }
          else {
            if (paramMotor == "") {
              paramMotor = F("?");
            }
            motor_number = 0;
            parse_error  = true;
          }

          if (paramDirection.equalsIgnoreCase(F("Stop"))) {
            motor_dir = MOTOR_STOP;
          }
          else if (paramDirection.equalsIgnoreCase(F("Forward"))) {
            motor_dir = MOTOR_FWD;
          }
          else if ((paramDirection.equalsIgnoreCase(F("Backward")))) {
            motor_dir = MOTOR_REV;
          }
          else if (paramDirection.equalsIgnoreCase(F("Standby"))) {
            motor_dir = MOTOR_STBY;
          }
          else if (paramDirection.equalsIgnoreCase(F("Brake"))) {
            motor_dir = MOTOR_BRAKE;
          }
          else {
            paramDirection = F("?");
            motor_dir      = MOTOR_STOP;
            parse_error    = true;
          }

          if (paramSpeed == "") {
            if ((motor_dir == MOTOR_STOP) || (motor_dir == MOTOR_STBY || (motor_dir == MOTOR_BRAKE))) {
              paramSpeed  = F("0");
              motor_speed = 0;
            }
            else {
              parse_error = true;
              paramSpeed  = F("?");
            }
          }
          else {
            motor_speed = paramSpeed.toInt();

            if ((motor_speed < 0) || (motor_speed > 100)) {
              motor_speed = 100;
              #ifdef VERBOSE_P079
              addLog(LOG_LEVEL_INFO, ModeStr + String(F(": Warning, invalid speed: Now using 100")));
              #endif // ifdef VERBOSE_P079
            }
          }
        }

        if (parse_error == true) {
          String ErrorStr = ModeStr + String(F(": CMD Syntax Error"));
          addLog(LOG_LEVEL_INFO, ErrorStr);
          SendStatus(event->Source, ErrorStr + String(F(" <br>"))); // Reply (echo) to sender. This will print message on browser.
        }
        else {
          WemosMotor Wemos(Plugin_079_MotorShield_address, motor_number, MOTOR_FREQ_P079);
          LOLIN_I2C_MOTOR lolin;
          lolin.changeFreq(MOTOR_CH_BOTH, MOTOR_FREQ_P079);

          if (motor_dir == MOTOR_FWD) {
            Wemos.setmotor(P079_CW, motor_speed);
            lolin.changeStatus(motor_number, MOTOR_STATUS_CW);
            lolin.changeDuty(motor_number, motor_speed);
          }
          else if (motor_dir == MOTOR_REV) {
            Wemos.setmotor(P079_CCW, motor_speed);
            lolin.changeStatus(motor_number, MOTOR_STATUS_CCW);
            lolin.changeDuty(motor_number, motor_speed);
          }
          else if (motor_dir == MOTOR_STOP) {
            paramSpeed = F("0");
            Wemos.setmotor(P079_STOP);
            lolin.changeStatus(motor_number, MOTOR_STATUS_STOP);
          }
          else if (motor_dir == MOTOR_STBY) {
            paramSpeed = F("0");
            Wemos.setmotor(P079_STANDBY);
            lolin.changeStatus(motor_number, MOTOR_STATUS_STANDBY);
          }
          else if (motor_dir == MOTOR_BRAKE) {
            paramSpeed = F("0");
            Wemos.setmotor(P079_SHORT_BRAKE);
            lolin.changeStatus(motor_number, MOTOR_STATUS_SHORT_BRAKE);
          }
        }
        addLog(LOG_LEVEL_INFO,
               ModeStr + String(F(": Addr=")) + formatToHex(Plugin_079_MotorShield_address) + String(F(": Mtr=")) + paramMotor +
               String(F(", Dir=")) + paramDirection + String(F(", Spd=")) + paramSpeed);

        success = true;
      }
      break;
    }
  }
  return success;
}

// ************************************************************************************************

// copied from <WEMOS_Motor.cpp>

WemosMotor::WemosMotor(uint8_t address, uint8_t motor, uint32_t freq)
{
  if (Plugin_079_MotorShield_type != WEMOS_P079) {
    return;
  }

  _use_STBY_IO = false;

  if (motor == P079_MOTOR_A) {
    _motor = P079_MOTOR_A;
  }
  else {
    _motor = P079_MOTOR_B;
  }

  // Wire.begin();   Called in ESPEasy framework

  _address = address;

  setfreq(freq);
}

WemosMotor::WemosMotor(uint8_t address, uint8_t motor, uint32_t freq, uint8_t STBY_IO)
{
  if (Plugin_079_MotorShield_type != WEMOS_P079) {
    return;
  }

  _use_STBY_IO = true;
  _STBY_IO     = STBY_IO;

  if (motor == P079_MOTOR_A) {
    _motor = P079_MOTOR_A;
  }
  else {
    _motor = P079_MOTOR_B;
  }

  // Wire.begin();   Called in ESPEasy framework

  _address = address;

  setfreq(freq);

  pinMode(_STBY_IO, OUTPUT);
  digitalWrite(_STBY_IO, LOW);
}

/* setfreq() -- set PWM's frequency
   freq: PWM's frequency

   total 4bytes
 |0.5byte CMD     | 3.5byte Parm|
 |CMD             | parm        |
 |0x0X  set freq  | uint32  freq|
 */
void WemosMotor::setfreq(uint32_t freq)
{
  if (Plugin_079_MotorShield_type != WEMOS_P079) {
    return;
  }
  Wire.beginTransmission(_address);
  Wire.write(((byte)(freq >> 24)) & (byte)0x0f);
  Wire.write((byte)(freq >> 16));
  Wire.write((byte)(freq >> 8));
  Wire.write((byte)freq);
  Wire.endTransmission(); // stop transmitting
  delay(0);
}

/* setmotor() -- set motor
   motor:
        P079_MOTOR_A    0   Motor A
        P079_MOTOR_B    1   Motor B

   dir:
        P079_SHORT_BRAKE  0
        P079_CCW          1
        P079_CW               2
        P079_STOP         3
        P079_STANDBY      4

   pwm_val:
        0.00 - 100.00  (%)

   total 4bytes
 |0.5byte CMD      | 3.5byte Parm         |
 |CMD              | parm                 |
 |0x10  set motorA | uint8 dir  uint16 pwm|
 |0x11  set motorB | uint8 dir  uint16 pwm|
 */
void WemosMotor::setmotor(uint8_t dir, float pwm_val)
{
  uint16_t _pwm_val;

  if (Plugin_079_MotorShield_type != WEMOS_P079) {
    return;
  }

  if (_use_STBY_IO == true) {
    if (dir == P079_STANDBY) {
      digitalWrite(_STBY_IO, LOW);
      return;
    } else {
      digitalWrite(_STBY_IO, HIGH);
    }
  }

  Wire.beginTransmission(_address);
  Wire.write(_motor | (byte)0x10); // CMD either 0x10 or 0x11
  Wire.write(dir);

  // PWM in %
  _pwm_val = uint16_t(pwm_val * 100);

  if (_pwm_val > 10000) { // _pwm_val > 100.00
    _pwm_val = 10000;
  }

  Wire.write((byte)(_pwm_val >> 8));
  Wire.write((byte)_pwm_val);
  Wire.endTransmission(); // stop transmitting

  delay(0);
}

void WemosMotor::setmotor(uint8_t dir)
{
  if (Plugin_079_MotorShield_type != WEMOS_P079) {
    return;
  }
  setmotor(dir, 100);
}

// end copy from <WEMOS_Motor.cpp>

// ************************************************************************************************

// copied from <LOLIN_I2C_MOTOR.cpp>

/*
        Init
 */
LOLIN_I2C_MOTOR::LOLIN_I2C_MOTOR(uint8_t address)
{
  if (Plugin_079_MotorShield_type != LOLIN_P079) {
    return;
  }

  // Wire.begin();   Called in ESPEasy framework
  _address = address;
}

/*
    Change Motor Status.
    ch: Motor Channel
        MOTOR_CH_A
        MOTOR_CH_B
        MOTOR_CH_BOTH

    sta: Motor Status
        MOTOR_STATUS_STOP
        MOTOR_STATUS_CCW
        MOTOR_STATUS_CW
        MOTOR_STATUS_SHORT_BRAKE
        MOTOR_STATUS_STANDBY
 */
unsigned char LOLIN_I2C_MOTOR::changeStatus(unsigned char ch, unsigned char sta)
{
  send_data[0] = CHANGE_STATUS;
  send_data[1] = ch;
  send_data[2] = sta;
  unsigned char result = sendData(send_data, 3);

  return result;
}

/*
    Change Motor Frequency
        ch: Motor Channel
            MOTOR_CH_A
            MOTOR_CH_B
            MOTOR_CH_BOTH

        freq: PWM frequency (Hz)
            1 - 80KHz, typically 1000Hz
 */
unsigned char LOLIN_I2C_MOTOR::changeFreq(unsigned char ch, uint32_t freq)
{
  if (Plugin_079_MotorShield_type != LOLIN_P079) {
    return 0;
  }

  send_data[0] = CHANGE_FREQ;
  send_data[1] = ch;

  send_data[2] = (uint8_t)(freq & 0xff);
  send_data[3] = (uint8_t)((freq >> 8) & 0xff);
  send_data[4] = (uint8_t)((freq >> 16) & 0xff);
  unsigned char result = sendData(send_data, 5);

  return result;
}

/*
    Change Motor Duty.
    ch: Motor Channel
        MOTOR_CH_A
        MOTOR_CH_B
        MOTOR_CH_BOTH

    duty: PWM Duty (%)
        0.01 - 100.00 (%)
 */
unsigned char LOLIN_I2C_MOTOR::changeDuty(unsigned char ch, float duty)
{
  if (Plugin_079_MotorShield_type != LOLIN_P079) {
    return 0;
  }

  uint16_t _duty;
  _duty = (uint16_t)(duty * 100);

  send_data[0] = CHANGE_DUTY;
  send_data[1] = ch;

  send_data[2] = (uint8_t)(_duty & 0xff);
  send_data[3] = (uint8_t)((_duty >> 8) & 0xff);
  unsigned char result = sendData(send_data, 4);

  return result;
}

/*
    Reset Device.
 */
unsigned char LOLIN_I2C_MOTOR::reset()
{
  if (Plugin_079_MotorShield_type != LOLIN_P079) {
    return 0;
  }

  send_data[0] = RESET_SLAVE;
  unsigned char result = sendData(send_data, 1);

  return result;
}

/*
    Change Device I2C address
    address: when address=0, address>=127, will change address to default I2C address 0x31
 */
unsigned char LOLIN_I2C_MOTOR::changeAddress(unsigned char address)
   {
        if (Plugin_079_MotorShield_type != LOLIN_P079) {
            return 0;
        }

        send_data[0] = CHANGE_I2C_ADDRESS;
        send_data[1] = address;
        unsigned char result = sendData(send_data, 2);

        return result;
   }

/*
    Get PRODUCT_ID and Firmwave VERSION
 */
unsigned char LOLIN_I2C_MOTOR::getInfo(void)
{
  if (Plugin_079_MotorShield_type != LOLIN_P079) {
    return 0;
  }

  send_data[0] = GET_SLAVE_STATUS;
  unsigned char result = sendData(send_data, 1);

  if (result == 0)
  {
    PRODUCT_ID = get_data[0];
    VERSION_ID = get_data[1];
  }
  else
  {
    PRODUCT_ID = 0;
    VERSION_ID = 0;
  }

  return result;
}

/*
    Send and Get I2C Data
 */
unsigned char LOLIN_I2C_MOTOR::sendData(unsigned char *data, unsigned char len)
{
  unsigned char i;

  if (Plugin_079_MotorShield_type != LOLIN_P079) {
    return 0;
  }

  if ((_address == 0) || (_address >= 127))
  {
    return 1;
  }
  else
  {
    Wire.beginTransmission(_address);

    for (i = 0; i < len; i++) {
      Wire.write(data[i]);
    }
    Wire.endTransmission();
    delay(50);

    if (data[0] == GET_SLAVE_STATUS) {
      Wire.requestFrom((int)_address, 2);
    }
    else {
      Wire.requestFrom((int)_address, 1);
    }

    i = 0;

    while (Wire.available())
    {
      get_data[i] = Wire.read();
      i++;
    }

    return 0;
  }
}

// ************************************************************************************************
// end copy from <LOLIN_I2C_MOTOR.cpp>

#endif // USES_P079
