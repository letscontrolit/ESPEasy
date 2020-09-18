#ifdef USES_P079

// #######################################################################################################
// #################################### Plugin 079: Wemos Motorshield ##############################
// #######################################################################################################

// Wemos Motorshield
// like this one: https://wiki.wemos.cc/products:d1_mini_shields:motor_shield
// see also: https://hackaday.io/project/18439-motor-shield-reprogramming
// based on this library: https://github.com/wemos/WEMOS_Motor_Shield_Arduino_Library
// Plugin part written by Susanne Jaeckel + TungstenE2

// Note: see wiki for setup. motor_shield.bin needs to be flashed first!!!
// see wiki: https://www.letscontrolit.com/wiki/index.php?title=WemosMotorshield


#define PLUGIN_079
#define PLUGIN_ID_079         79
#define PLUGIN_NAME_079       "Motor - Wemos Motorshield"
#define PLUGIN_VALUENAME1_079 "Wemos Motorshield"

// copied from <WEMOS_Motor.h>
#ifndef __WEMOS_MOTOR_H
# define __WEMOS_MOTOR_H

# if ARDUINO >= 100
 #  include "Arduino.h"
# else // if ARDUINO >= 100
 #  include "WProgram.h"
# endif // if ARDUINO >= 100

# include "Wire.h"
# include "_Plugin_Helper.h"

# define P079_MOTOR_A 0
# define P079_MOTOR_B 1

# define P079_SHORT_BRAKE 0
# define P079_CCW         1
# define P079_CW          2
# define P079_STOP        3
# define P079_STANDBY     4

# define P079_I2C_ADDR PCONFIG(0)

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


boolean Plugin_079(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  // WemosMotor WMS;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_079;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = SENSOR_TYPE_NONE;
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
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0],
               PSTR(PLUGIN_VALUENAME1_079));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P079_I2C_ADDR = 0x30;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      String i2c_addres_string = formatToHex(P079_I2C_ADDR);
      addFormTextBox(F("I2C Address (Hex)"), F("p079_adr"), i2c_addres_string, 4);
      addFormNote(F(
                    "Make sure to update the Wemos Motorshield firmware, see <a href='https://www.letscontrolit.com/wiki/index.php?title=WemosMotorshield'>wiki</a>"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      String i2c_address = web_server.arg(F("p079_adr"));
      P079_I2C_ADDR = (int)strtol(i2c_address.c_str(), 0, 16);

      success = true;
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
      String tmpString = string;

      String cmd = parseString(tmpString, 1);

      // Commands:
      // MotorShieldCMD,<DCMotor>,<Motornumber>,<Forward/Backward/Stop>,<Speed>

      if (cmd.equalsIgnoreCase(F("WemosMotorShieldCMD"))) {
        String paramMotor     = parseString(tmpString, 2); // Motor 0 or 1
        String paramDirection = parseString(tmpString, 3); // Direction
        String paramSpeed     = parseString(tmpString, 4); // Speed

        WemosMotor WMS(P079_I2C_ADDR, paramMotor.toInt(), 1000);
        addLog(LOG_LEVEL_DEBUG,
               String(F("WemosMotorShield: Address = ")) + P079_I2C_ADDR + String(F(" Motor = ")) + paramMotor);

        if (paramDirection.equalsIgnoreCase(F("Forward"))) {
          WMS.setmotor(P079_CW, paramSpeed.toInt());
          addLog(LOG_LEVEL_INFO,
                 String(F("WemosMotor: Motor = ")) + paramMotor + String(F(" Direction = ")) + paramDirection + String(F(
                                                                                                                         " Speed = ")) +
                 paramSpeed);
        }

        if (paramDirection.equalsIgnoreCase(F("Backward"))) {
          WMS.setmotor(P079_CCW, paramSpeed.toInt());
          addLog(LOG_LEVEL_INFO,
                 String(F("WemosMotor: Motor = ")) + paramMotor + String(F(" Direction = ")) + paramDirection + String(F(
                                                                                                                         " Speed = ")) +
                 paramSpeed);
        }

        if (paramDirection.equalsIgnoreCase(F("Stop"))) {
          WMS.setmotor(P079_STOP);
          addLog(LOG_LEVEL_INFO, String(F("WemosMotor: Motor = ")) + paramMotor + String(F(" Direction = ")) + paramDirection);
        }

        success = true;
      }

      break;
    }
  }
  return success;
}

// copied from <WEMOS_Motor.cpp>

WemosMotor::WemosMotor(uint8_t address, uint8_t motor, uint32_t freq)
{
  _use_STBY_IO = false;

  if (motor == P079_MOTOR_A) {
    _motor = P079_MOTOR_A;
  }
  else {
    _motor = P079_MOTOR_B;
  }

  // Wire.begin();   called in ESPEasy framework

  _address = address;

  setfreq(freq);
}

WemosMotor::WemosMotor(uint8_t address, uint8_t motor, uint32_t freq, uint8_t STBY_IO)
{
  _use_STBY_IO = true;
  _STBY_IO     = STBY_IO;

  if (motor == P079_MOTOR_A) {
    _motor = P079_MOTOR_A;
  }
  else {
    _motor = P079_MOTOR_B;
  }

  // Wire.begin();   called in ESPEasy framework

  _address = address;

  setfreq(freq);

  pinMode(_STBY_IO, OUTPUT);
  digitalWrite(_STBY_IO, LOW);
}

/* setfreq() -- set PWM's frequency

   freq:
        PWM's frequency

   total 4bytes
 |0.5byte CMD       | 3.5byte Parm|
 |CMD								| parm        |
 |0x0X  set freq        | uint32  freq|

 */
void WemosMotor::setfreq(uint32_t freq)
{
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
        P079_MOTOR_A	0	Motor A
        P079_MOTOR_B	1	Motor B

   dir:
        P079_SHORT_BRAKE  0
        P079_CCW		      1
        P079_CW           2
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
  setmotor(dir, 100);
}

// end copied from <WEMOS_Motor.cpp>

#endif // USES_P079
