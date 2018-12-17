#ifdef USES_P079
//#######################################################################################################
//#################################### Plugin 079: Wemos Motorshield ##############################
//#######################################################################################################

// Wemos Motorshield
// like this one: https://wiki.wemos.cc/products:d1_mini_shields:motor_shield
// see also: https://hackaday.io/project/18439-motor-shield-reprogramming
// based on this library: https://github.com/wemos/WEMOS_Motor_Shield_Arduino_Library
// Plugin part written by Susanne Jaeckel + TungstenE2

// N.B. this shield does only accept data transmissions in sets of 4 bytes.
// This means a simple I2C scan may mess up the module (and all other I2C communications)
// until a full reset of the node.
// See: https://github.com/wemos/Motor_Shield_Firmware/issues/1#issuecomment-394475451

#define PLUGIN_079
#define PLUGIN_ID_079         79
#define PLUGIN_NAME_079       "Motor - Wemos Motorshield [TESTING]"
#define PLUGIN_VALUENAME1_079 "Wemos Motorshield"

// copied from <WEMOS_Motor.h>
#ifndef __WEMOS_MOTOR_H
#define __WEMOS_MOTOR_H

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include "Wire.h"

#define _MOTOR_A 0
#define _MOTOR_B 1

#define _SHORT_BRAKE 0
#define _CCW  1
#define _CW     2
#define _STOP 3
#define _STANDBY 4

class WemosMotor {
public:
WemosMotor(uint8_t address, uint8_t motor, uint32_t freq);
WemosMotor(uint8_t address, uint8_t motor, uint32_t freq, uint8_t STBY_IO);
void setfreq(uint32_t freq);
void setmotor(uint8_t dir, float pwm_val);
void setmotor(uint8_t dir);

private:
uint8_t _address;
uint8_t _motor;
bool _use_STBY_IO = false;
uint8_t _STBY_IO;
};

#endif
// end copied from <WEMOS_Motor.h>

uint8_t Plugin_079_MotorShield_address = 0x30;


boolean Plugin_079(byte function, struct EventStruct *event, String& string)
{
	boolean success = false;

	//WemosMotor WMS;

	switch (function) {
	case PLUGIN_DEVICE_ADD: {
		Device[++deviceCount].Number = PLUGIN_ID_079;
		Device[deviceCount].Type = DEVICE_TYPE_I2C;
		Device[deviceCount].VType = SENSOR_TYPE_NONE;
		Device[deviceCount].Ports = 0;
		Device[deviceCount].PullUpOption = false;
		Device[deviceCount].InverseLogicOption = false;
		Device[deviceCount].FormulaOption = false;
		Device[deviceCount].ValueCount = 0;
		Device[deviceCount].SendDataOption = false;
		Device[deviceCount].TimerOption = false;
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

	case PLUGIN_WEBFORM_LOAD: {
    String i2c_addres_string = formatToHex(Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
		addFormTextBox(F("I2C Address (Hex)"), F("p079_adr"), i2c_addres_string, 4);

		success = true;
		break;
	}

	case PLUGIN_WEBFORM_SAVE: {
		String i2c_address = WebServer.arg(F("p079_adr"));
		Settings.TaskDevicePluginConfig[event->TaskIndex][0] = (int)strtol(i2c_address.c_str(), 0, 16);

		success = true;
		break;
	}

	case PLUGIN_INIT: {
		Plugin_079_MotorShield_address = Settings.TaskDevicePluginConfig[event->TaskIndex][0];

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
			String paramMotor = parseString(tmpString, 2); // Motor 0 or 1
			String paramDirection = parseString(tmpString, 3); // Direction
			String paramSpeed = parseString(tmpString, 4); // Speed

			WemosMotor WMS(Plugin_079_MotorShield_address, paramMotor.toInt(), 1000);
			addLog(LOG_LEVEL_DEBUG, String(F("WemosMotorShield: Address = ")) + Plugin_079_MotorShield_address + String(F(" Motor = ")) + paramMotor);

			if (paramDirection.equalsIgnoreCase(F("Forward"))) {
				WMS.setmotor(_CW, paramSpeed.toInt());
				addLog(LOG_LEVEL_INFO, String(F("WemosMotor: Motor = ")) + paramMotor + String(F(" Direction = ")) + paramDirection + String(F(" Speed = ")) + paramSpeed);
			}
			if (paramDirection.equalsIgnoreCase(F("Backward"))) {
				WMS.setmotor(_CCW, paramSpeed.toInt());
				addLog(LOG_LEVEL_INFO, String(F("WemosMotor: Motor = ")) + paramMotor + String(F(" Direction = ")) + paramDirection + String(F(" Speed = ")) + paramSpeed);
			}
			if (paramDirection.equalsIgnoreCase(F("Stop"))) {
				WMS.setmotor(_STOP);
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

	if (motor == _MOTOR_A)
		_motor = _MOTOR_A;
	else
		_motor = _MOTOR_B;

	//Wire.begin();   called in ESPEasy framework

	_address = address;

	setfreq(freq);
}


WemosMotor::WemosMotor(uint8_t address, uint8_t motor, uint32_t freq, uint8_t STBY_IO)
{
	_use_STBY_IO = true;
	_STBY_IO = STBY_IO;

	if (motor == _MOTOR_A)
		_motor = _MOTOR_A;
	else
		_motor = _MOTOR_B;

	//Wire.begin();   called in ESPEasy framework

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
     |0x0X  set freq  	| uint32  freq|

 */
void WemosMotor::setfreq(uint32_t freq)
{
	Wire.beginTransmission(_address);
	Wire.write(((byte)(freq >> 24)) & (byte)0x0f);
	Wire.write((byte)(freq >> 16));
	Wire.write((byte)(freq >> 8));
	Wire.write((byte)freq);
	Wire.endTransmission();     // stop transmitting
	delay(0);
}

/* setmotor() -- set motor

   motor:
        _MOTOR_A	0	Motor A
        _MOTOR_B	1	Motor B

   dir:
        _SHORT_BRAKE  0
        _CCW		      1
        _CW           2
        _STOP         3
        _STANDBY      4

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
		if (dir == _STANDBY) {
			digitalWrite(_STBY_IO, LOW);
			return;
		}else
			digitalWrite(_STBY_IO, HIGH);
	}

	Wire.beginTransmission(_address);
	Wire.write(_motor | (byte)0x10);  // CMD either 0x10 or 0x11
	Wire.write(dir);

  // PWM in %
	_pwm_val = uint16_t(pwm_val * 100);

	if (_pwm_val > 10000) // _pwm_val > 100.00
		_pwm_val = 10000;

	Wire.write((byte)(_pwm_val >> 8));
	Wire.write((byte)_pwm_val);
	Wire.endTransmission();     // stop transmitting

	delay(0);
}

void WemosMotor::setmotor(uint8_t dir)
{
	setmotor(dir, 100);
}

// end copied from <WEMOS_Motor.cpp>

#endif // USES_P079
