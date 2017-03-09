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
byte Plugin_048_MotorType = 0;
byte Plugin_048_MotorNumber = 1;
byte Plugin_048_MotorSpeed = 255;


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

		byte choice2 = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
		String options2[3];
		options2[0] = F("DCMotor");
		options2[1] = F("Stepper [NOT IMPLEMETED]");
		options2[2] = F("Servo [NOT IMPLEMETED]");
		int optionValues2[3];
		optionValues2[0] = 0;
		optionValues2[1] = 1;
		optionValues2[2] = 2;
		string += F("<TR><TD>Motor Type:<TD><select name='plugin_048_motortype'>");
		for (byte x = 0; x < 3; x++) {
			string += F("<option value='");
			string += optionValues2[x];
			string += "'";
			if (choice2 == optionValues2[x])
				string += F(" selected");
			string += ">";
			string += options2[x];
			string += F("</option>");
		}
		string += F("</select>");

		byte choice3 = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
		String options3[4];
		options3[0] = F("1");
		options3[1] = F("2");
		options3[2] = F("3");
		options3[3] = F("4");
		int optionValues3[4];
		optionValues3[0] = 1;
		optionValues3[1] = 2;
		optionValues3[2] = 3;
		optionValues3[3] = 4;
		string += F("<TR><TD>Motor Number:<TD><select name='plugin_048_motornumber'>");
		for (byte x = 0; x < 4; x++) {
			string += F("<option value='");
			string += optionValues3[x];
			string += "'";
			if (choice3 == optionValues3[x])
				string += F(" selected");
			string += ">";
			string += options3[x];
			string += F("</option>");
		}
		string += F("</select>");

		success = true;
		break;
	}

	case PLUGIN_WEBFORM_SAVE: {
		String plugin1 = WebServer.arg(F("plugin_048_adr"));
		Settings.TaskDevicePluginConfig[event->TaskIndex][0] = (int) strtol(plugin1.c_str(), 0, 16);
		String plugin2 = WebServer.arg(F("plugin_048_motortype"));
		Settings.TaskDevicePluginConfig[event->TaskIndex][1] = plugin2.toInt();
		String plugin4 = WebServer.arg(F("plugin_048_motornumber"));
		Settings.TaskDevicePluginConfig[event->TaskIndex][3] = plugin4.toInt();
		success = true;
		break;
	}

	case PLUGIN_INIT: {
		Plugin_048_MotorShield_address = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
		Plugin_048_MotorType = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
		Plugin_048_MotorNumber = Settings.TaskDevicePluginConfig[event->TaskIndex][3];

		success = true;
		break;
	}


	case PLUGIN_READ: {

		success = false;
		break;
	}

	case PLUGIN_WRITE: {

		String tmpString = string;
		int argIndex = tmpString.indexOf(',');
		if (argIndex)
			tmpString = tmpString.substring(0, argIndex);

		if (tmpString.equalsIgnoreCase(F("MotorShieldCMD"))) {

			// Create the motor shield object with the default I2C address
			AFMS = Adafruit_MotorShield(Plugin_048_MotorShield_address);

			if (Plugin_048_MotorType == 0) {
				myMotor = AFMS.getMotor(Plugin_048_MotorNumber);
			}
			if (Plugin_048_MotorType == 1) {
				// TODO remove hardcoded 200 steps per revolution (1.8 degree)
				myStepper = AFMS.getStepper(200,Plugin_048_MotorNumber);
			}
			if (Plugin_048_MotorType == 2) {
				// TODO Servo
			}

			String log = F("MotorShield: Address: 0x");
			log += String(Plugin_048_MotorShield_address,HEX);
			log += F(" Motortype: ");
			log += String(Plugin_048_MotorNumber);
			log += F(" Motor#: ");
			log += String(Plugin_048_MotorNumber);
			addLog(LOG_LEVEL_DEBUG, log);

			AFMS.begin();  // create with the default frequency 1.6KHz

			success = true;
			argIndex = string.indexOf(',');
			tmpString = string.substring(argIndex + 1);

			String cmdString = tmpString.substring(0,tmpString.indexOf(','));

			if (cmdString.equalsIgnoreCase(F("Forward"))) {
				if (event->Par2 >= 0 && event->Par2 <= 255) {
					Plugin_048_MotorSpeed = event->Par2;
					addLog(LOG_LEVEL_INFO, "DCMotor->Forward Speed: " + String(Plugin_048_MotorSpeed));
					myMotor->setSpeed(Plugin_048_MotorSpeed);
					myMotor->run(FORWARD);
				}
			}
			else if (cmdString.equalsIgnoreCase(F("Backward"))) {
				if (event->Par2 >= 0 && event->Par2 <= 255) {
					Plugin_048_MotorSpeed = event->Par2;
					addLog(LOG_LEVEL_INFO, "DCMotor->Backward Speed: " + String(Plugin_048_MotorSpeed));
					myMotor->setSpeed(Plugin_048_MotorSpeed);
					myMotor->run(BACKWARD);
				}
			}
			else if (tmpString.equalsIgnoreCase(F("Release"))) {
				addLog(LOG_LEVEL_INFO, "DCMotor->Release");
				myMotor->run(RELEASE);
			}
		}
		break;
	}

	}
	return success;
}

#endif
