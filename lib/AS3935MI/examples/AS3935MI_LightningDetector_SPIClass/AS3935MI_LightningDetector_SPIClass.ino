// AS3935_LightningDetector_SPIClass.ino
//
// shows how to use the AS3935 library with the lightning sensor connected using an interface that inherits from SPIClass.
// in this example, the SPI object is used, but other objects that inherit from SPIClass may be used instead.
//
// Copyright (c) 2018-2019 Gregor Christandl
//
// connect the AS3935 to the Arduino like this:
//
// Arduino - AS3935
// 5V ------ VCC
// GND ----- GND
// D2 ------ IRQ		must be a pin supporting external interrupts, e.g. D2 or D3 on an Arduino Uno.
// MOSI ---- MOSI
// MISO ---- MISO
// SCK ----- SCK
// GND ----- SI		(activates SPI for the AS3935)
// D3 ------ CS		chip select pin for AS3935
// 5V ------ EN_VREG !IMPORTANT when using 5V Arduinos (Uno, Mega2560, ...)
// other pins can be left unconnected.

#include <Arduino.h>
#include <SPI.h>

#include <AS3935SPIClass.h>

#define PIN_IRQ 3
#define PIN_CS 4

//create an AS3935 object using the SPI interface, chip select pin 4 and IRQ pin number 3
//the SPIClass object is passed by reference
AS3935SPIClass as3935(&SPI, PIN_CS, PIN_IRQ);

//this value will be set to true by the AS3935 interrupt service routine.
volatile bool interrupt_ = false;

constexpr uint32_t SENSE_INCREASE_INTERVAL = 15000;	//15 s sensitivity increase interval
uint32_t sense_adj_last_ = 0L;						//time of last sensitivity adjustment

void setup() {
	// put your setup code here, to run once:
	Serial.begin(9600);

	//wait for serial connection to open (only necessary on some boards)
	while (!Serial);

	//set the IRQ pin as an input pin. do not use INPUT_PULLUP - the AS3935 will pull the pin 
	//high if an event is registered.
	pinMode(PIN_IRQ, INPUT);

	SPI.begin();

	//begin() checks the Interface passed to the constructor and resets the AS3935 to 
	//default values.
	if (!as3935.begin())
	{
		Serial.println("begin() failed. check your AS3935 Interface setting.");
		while (1);
	}

	//check SPI connection.
	if (!as3935.checkConnection())
	{
		Serial.println("checkConnection() failed. check your SPI connection and SPI chip select pin. ");
		while (1);
	}
	else
		Serial.println("SPI connection check passed. ");

	//check the IRQ pin connection.
	if (!as3935.checkIRQ())
	{
		Serial.println("checkIRQ() failed. check if the correct IRQ pin was passed to the AS3935SPI constructor. ");
		while (1);
	}
	else
		Serial.println("IRQ pin connection check passed. ");

	//calibrate the resonance frequency. failing the resonance frequency could indicate an issue 
	//of the sensor. resonance frequency calibration will take about 1.7 seconds to complete.
	int32_t frequency = 0;
	if (!as3935.calibrateResonanceFrequency(frequency))
	{
		Serial.print("Resonance Frequency Calibration failed: is ");
		Serial.print(frequency);
		Serial.println(" Hz, should be 482500 Hz - 517500 Hz");
		//while (1);
	}
	else
		Serial.println("Resonance Frequency Calibration passed. ");

	Serial.print("Resonance Frequency is "); Serial.print(frequency); Serial.println(" Hz");


	//calibrate the RCO.
	if (!as3935.calibrateRCO())
	{
		Serial.println("RCP Calibration failed. ");
		while (1);
	}
	else
		Serial.println("RCO Calibration passed. ");

	//set the analog front end to 'indoors'
	as3935.writeAFE(AS3935MI::AS3935_INDOORS);

	//set default value for noise floor threshold
	as3935.writeNoiseFloorThreshold(AS3935MI::AS3935_NFL_2);

	//set the default Watchdog Threshold
	as3935.writeWatchdogThreshold(AS3935MI::AS3935_WDTH_2);

	//set the default Spike Rejection 
	as3935.writeSpikeRejection(AS3935MI::AS3935_SREJ_2);

	//write default value for minimum lightnings (1)
	as3935.writeMinLightnings(AS3935MI::AS3935_MNL_1);

	//do not mask disturbers
	as3935.writeMaskDisturbers(false);

	//the AS3935 will pull the interrupt pin HIGH when an event is registered and will keep it 
	//pulled high until the event register is read.
	attachInterrupt(digitalPinToInterrupt(PIN_IRQ), AS3935ISR, RISING);

	Serial.println("Initialization complete, waiting for events...");
}

void loop() {
	// put your main code here, to run repeatedly:

	if (interrupt_)
	{
		//the Arduino should wait at least 2ms after the IRQ pin has been pulled high
		delay(2);

		//reset the interrupt variable
		interrupt_ = false;

		//query the interrupt source from the AS3935
		uint8_t event = as3935.readInterruptSource();

		//send a report if the noise floor is too high. 
		if (event == AS3935MI::AS3935_INT_NH)
		{
			Serial.println("Noise floor too high. attempting to increase noise floor threshold. ");

			//if the noise floor threshold setting is not yet maxed out, increase the setting.
			//note that noise floor threshold events can also be triggered by an incorrect
			//analog front end setting.
			if (as3935.increaseNoiseFloorThreshold() == AS3935MI::AS3935_NFL_0)
				Serial.println("noise floor threshold already at maximum");
			else
				Serial.println("increased noise floor threshold");
		}

		//send a report if a disturber was detected. if disturbers are masked with as3935.writeMaskDisturbers(true);
		//this event will never be reported.
		else if (event == AS3935MI::AS3935_INT_D)
		{
			Serial.println("Disturber detected, attempting to increase noise floor threshold. ");

			//increasing the Watchdog Threshold and / or Spike Rejection setting improves the AS3935s resistance 
			//against disturbers but also decrease the lightning detection efficiency (see AS3935 datasheet)
			uint8_t wdth = as3935.readWatchdogThreshold();
			uint8_t srej = as3935.readSpikeRejection();

			if ((wdth < AS3935MI::AS3935_WDTH_10) || (srej < AS3935MI::AS3935_SREJ_10))
			{
				sense_adj_last_ = millis();

				//alternatively increase spike rejection and watchdog threshold 
				if (srej < wdth)
				{
					if (as3935.increaseSpikeRejection() == AS3935MI::AS3935_SREJ_0)
						Serial.println("spike rejection ratio already at maximum");
					else
						Serial.println("increased spike rejection ratio");
				}
				else
				{
					if (as3935.increaseWatchdogThreshold() == AS3935MI::AS3935_WDTH_0)
						Serial.println("watchdog threshold already at maximum");
					else
						Serial.println("increased watchdog threshold");
				}
			}
			else
			{
				Serial.println("error: Watchdog Threshold and Spike Rejection settings are already maxed out.");
			}
		}

		else if (event == AS3935MI::AS3935_INT_L)
		{
			Serial.print("Lightning detected! Storm Front is ");
			Serial.print(as3935.readStormDistance());
			Serial.println("km away.");
		}
	}

	//increase sensor sensitivity every once in a while. SENSE_INCREASE_INTERVAL controls how quickly the code 
	//attempts to increase sensitivity. 
	if (millis() - sense_adj_last_ > SENSE_INCREASE_INTERVAL)
	{
		sense_adj_last_ = millis();

		Serial.println("No disturber detected, attempting to decrease noise floor threshold. ");

		uint8_t wdth = as3935.readWatchdogThreshold();
		uint8_t srej = as3935.readSpikeRejection();

		if ((wdth > AS3935MI::AS3935_WDTH_0) || (srej > AS3935MI::AS3935_SREJ_0))
		{

			//alternatively derease spike rejection and watchdog threshold 
			if (srej > wdth)
			{
				if (as3935.decreaseSpikeRejection())
					Serial.println("decreased spike rejection ratio");
				else
					Serial.println("spike rejection ratio already at minimum");
			}
			else
			{
				if (as3935.decreaseWatchdogThreshold())
					Serial.println("decreased watchdog threshold");
				else
					Serial.println("watchdog threshold already at minimum");
			}
		}
	}
}

//interrupt service routine. this function is called each time the AS3935 reports an event by pulling 
//the IRQ pin high.
#if defined(ESP32)
ICACHE_RAM_ATTR void AS3935ISR()
{
  interrupt_ = true;
}
#elif defined(ESP8266)
ICACHE_RAM_ATTR void AS3935ISR()
{
  interrupt_ = true;
}
#else
void AS3935ISR()
{
  interrupt_ = true;
}
#endif