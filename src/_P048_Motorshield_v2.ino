//#######################################################################################################
//#################################### Plugin 048: Adafruit Motorshield v2 ##############################
//#######################################################################################################

// Adafruit Motorshield v2
// like this one: https://www.adafruit.com/products/1438
// based on this library: https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library
// Currently only DC Motors Part is implemented, Steppers and Servos are missing!!!

#ifdef PLUGIN_BUILD_DEV

#include <Adafruit_MotorShield.h>

#define PLUGIN_048
#define PLUGIN_ID_048         48
#define PLUGIN_NAME_048       "Motor - Adafruit Motorshield v2 [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_048 "MotorShield v2"

uint8_t Plugin_048_MotorShield_address = 0x60;

int Plugin_048_MotorStepsPerRevolution = 200;

boolean Plugin_048(byte function, struct EventStruct *event, String& string) {
	boolean success = false;

	Adafruit_MotorShield AFMS;
	Adafruit_DCMotor *myMotor;
	Adafruit_StepperMotor *myStepper;

	switch (function) {

		case PLUGIN_DEVICE_ADD: {
			Device[++deviceCount].Number = PLUGIN_ID_048;
			Device[deviceCount].Type = DEVICE_TYPE_I2C;
			Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
			Device[deviceCount].Ports = 0;
			Device[deviceCount].PullUpOption = false;
			Device[deviceCount].InverseLogicOption = false;
			Device[deviceCount].FormulaOption = false;
			Device[deviceCount].ValueCount = 0;
			Device[deviceCount].SendDataOption = false;
			Device[deviceCount].TimerOption = true;
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

		case PLUGIN_WEBFORM_LOAD: {

			string +=
			F("<TR><TD>I2C Address (Hex): <TD><input type='text' title='Set i2c Address of sensor' name='");
			string += F("plugin_048_adr' value='0x");
			string += String(Settings.TaskDevicePluginConfig[event->TaskIndex][0],HEX);
			string += F("'>");

			string +=
			F("<TR><TD>Stepper: steps per revolution: <TD><input type='text' title='Set steps per revolution for steppers' name='");
			string += F("plugin_048_MotorStepsPerRevolution' value='");
			string += String(Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
			string += F("'>");




			success = true;
			break;
		}

		case PLUGIN_WEBFORM_SAVE: {
			String plugin1 = WebServer.arg(F("plugin_048_adr"));
			Settings.TaskDevicePluginConfig[event->TaskIndex][0] = (int) strtol(plugin1.c_str(), 0, 16);
			String plugin2 = WebServer.arg(F("plugin_048_MotorStepsPerRevolution"));
			Settings.TaskDevicePluginConfig[event->TaskIndex][1] = plugin2.toInt();
			success = true;
			break;
		}

		case PLUGIN_INIT: {
			Plugin_048_MotorShield_address = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
			Plugin_048_MotorStepsPerRevolution = Settings.TaskDevicePluginConfig[event->TaskIndex][1];

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
			String param1 = parseString(tmpString, 2);
			String param2 = parseString(tmpString, 3);
			String param3 = parseString(tmpString, 4);
			String param4 = parseString(tmpString, 5);
			String param5 = parseString(tmpString, 6);
			String log = "Debug parsesting: ";
			log += tmpString;
			log += "cmd: ";
			log += cmd;
			log += "param1: ";
			log += param1;
			log += "param2: ";
			log += param2;
			log += "param3: ";
			log += param3;
			log += "param4: ";
			log += param4;
			log += "param5: ";
			log += param5;
			addLog(LOG_LEVEL_DEBUG_MORE, log);

			// Commands:
			// MotorShieldCMD,<DCMotor>,<Motornumber>,<Forward/Backward/Release>,<Speed>


			if (cmd.equalsIgnoreCase(F("MotorShieldCMD")))
			{

				// Create the motor shield object with the default I2C address
				AFMS = Adafruit_MotorShield(Plugin_048_MotorShield_address);
				String log = F("MotorShield: Address: 0x");
				log += String(Plugin_048_MotorShield_address,HEX);
				addLog(LOG_LEVEL_DEBUG, log);

				if (param1.equalsIgnoreCase(F("DCMotor"))) {
					if (param2.toInt() > 0 && param2.toInt() < 5)
					{
						myMotor = AFMS.getMotor(param2.toInt());
						if (param3.equalsIgnoreCase(F("Forward")))
						{
							byte speed = 255;
							if (param4.toInt() <= 0 && param4.toInt() <= 255)
								speed = param4.toInt();
							AFMS.begin();
							addLog(LOG_LEVEL_INFO, "DCMotor" + param2 + "->Forward Speed: " + String(speed));
							myMotor->setSpeed(speed);
							myMotor->run(FORWARD);
							success = true;
						}
						if (param3.equalsIgnoreCase(F("Backward")))
						{
							byte speed = 255;
							if (param4.toInt() <= 0 && param4.toInt() <= 255)
								speed = param4.toInt();
							AFMS.begin();
							addLog(LOG_LEVEL_INFO, "DCMotor" + param2 + "->Backward Speed: " + String(speed));
							myMotor->setSpeed(speed);
							myMotor->run(BACKWARD);
							success = true;
						}
						if (param3.equalsIgnoreCase(F("Release")))
						{
							AFMS.begin();
							addLog(LOG_LEVEL_INFO, "DCMotor" + param2 + "->Release");
							myMotor->run(RELEASE);
							success = true;
						}
					}
				}

				// MotorShieldCMD,<Stepper>,<Motornumber>,<Forward/Backward/Release>,<Steps>,<SINGLE/DOUBLE/INTERLEAVE/MICROSTEP>
				// TODO crashes ?? must test more...
				if (param1.equalsIgnoreCase(F("Stepper")))
				{
					// Stepper# is which port it is connected to. If you're using M1 and M2, its port 1.
					// If you're using M3 and M4 indicate port 2
					if (param2.toInt() > 0 && param2.toInt() < 3)
					{
						myStepper = AFMS.getStepper(Plugin_048_MotorStepsPerRevolution, param2.toInt());
						if (param3.equalsIgnoreCase(F("Forward")))
						{
							int steps = 0;
							if (param4.toInt())
							{
								steps = param4.toInt();
								if (param5.equalsIgnoreCase(F("SINGLE")))
								{
									AFMS.begin();
									addLog(LOG_LEVEL_INFO, "Stepper" + param2 + "->Forward Steps: " + steps + " SINGLE");
									myMotor->setSpeed(10);
									myStepper->step(steps, FORWARD, SINGLE);
									success = true;
								}
							}
						}
						if (param3.equalsIgnoreCase(F("Backward")))
						{
							int steps = 0;
							if (param4.toInt())
							{
								steps = param4.toInt();
								if (param5.equalsIgnoreCase(F("SINGLE")))
								{
									AFMS.begin();
									addLog(LOG_LEVEL_INFO, "Stepper" + param2 + "->Backward Steps: " + steps + " SINGLE");
									myMotor->setSpeed(10);
									myStepper->step(steps, BACKWARD, SINGLE);
									success = true;
								}
							}
						}

					}
				}





			}

			break;
		}

	}
	return success;
}


#endif
