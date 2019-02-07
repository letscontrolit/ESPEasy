/*
OpenTherm.cpp - OpenTherm Communication Library For Arduino, ESP8266
Copyright 2018, Ihor Melnyk
*/

#include "OpenTherm.h"

OpenTherm::OpenTherm(int inPin, int outPin):
	inPin(inPin),
	outPin(outPin),	
	status(OpenThermStatus::NOT_INITIALIZED),	
	response(0),
	responseStatus(OpenThermResponseStatus::NONE),
	responseTimestamp(0),
	handleInterruptCallback(NULL),
	processResponseCallback(NULL)
{
}

void OpenTherm::begin(void(*handleInterruptCallback)(void), void(*processResponseCallback)(unsigned long, OpenThermResponseStatus))
{
	pinMode(inPin, INPUT);
	pinMode(outPin, OUTPUT);
	if (handleInterruptCallback != NULL) {
		this->handleInterruptCallback = handleInterruptCallback;
		attachInterrupt(digitalPinToInterrupt(inPin), handleInterruptCallback, CHANGE);		
	}
	activateBoiler();
	status = OpenThermStatus::READY;
	this->processResponseCallback = processResponseCallback;	
}

void OpenTherm::begin(void(*handleInterruptCallback)(void))
{
	begin(handleInterruptCallback, NULL);	
}

bool OpenTherm::isReady()
{
	return status == OpenThermStatus::READY;
}

int OpenTherm::readState() {
	return digitalRead(inPin);
}

void OpenTherm::setActiveState() {
	digitalWrite(outPin, LOW);
}

void OpenTherm::setIdleState() {
	digitalWrite(outPin, HIGH);
}

void OpenTherm::activateBoiler() {
	setIdleState();
	delay(1000);
}

void OpenTherm::sendBit(bool high) {
	if (high) setActiveState(); else setIdleState();
	delayMicroseconds(500);
	if (high) setIdleState(); else setActiveState();
	delayMicroseconds(500);
}

bool OpenTherm::sendRequestAync(unsigned long request)
{	
	//Serial.println("Request: " + String(request, HEX));
	noInterrupts();
	const bool ready = isReady();
	interrupts();

	if (!ready)
	  return false;

	status = OpenThermStatus::REQUEST_SENDING;
	response = 0;
	responseStatus = OpenThermResponseStatus::NONE;

	sendBit(HIGH); //start bit
	for (int i = 31; i >= 0; i--) {
		sendBit(bitRead(request, i));
	}
	sendBit(HIGH); //stop bit  
	setIdleState();

	status = OpenThermStatus::RESPONSE_WAITING;
	responseTimestamp = micros();	
	return true;
}

unsigned long OpenTherm::sendRequest(unsigned long request)
{	
	if (!sendRequestAync(request)) return 0;
	while (!isReady()) {
		process();
		yield();
	}	
	return response;
}

OpenThermResponseStatus OpenTherm::getLastResponseStatus()
{
	return responseStatus;
}

void OpenTherm::handleInterrupt()
{	
	if (isReady()) return;	

	unsigned long newTs = micros();
	if (status == OpenThermStatus::RESPONSE_WAITING) {
		if (readState() == HIGH) {
			status = OpenThermStatus::RESPONSE_START_BIT;
			responseTimestamp = newTs;
		}
		else {
			status = OpenThermStatus::RESPONSE_INVALID;
			responseTimestamp = newTs;
		}
	}
	else if (status == OpenThermStatus::RESPONSE_START_BIT) {
		if ((newTs - responseTimestamp < 750) && readState() == LOW) {
			status = OpenThermStatus::RESPONSE_RECEIVING;
			responseTimestamp = newTs;
			responseBitIndex = 0;
		}
		else {
			status = OpenThermStatus::RESPONSE_INVALID;
			responseTimestamp = newTs;
		}
	}
	else if (status == OpenThermStatus::RESPONSE_RECEIVING) {
		if ((newTs - responseTimestamp) > 750) {
			if (responseBitIndex < 32) {
				response = (response << 1) | !readState();
				responseTimestamp = newTs;
				responseBitIndex++;
			}
			else { //stop bit
				status = OpenThermStatus::RESPONSE_READY;
				responseTimestamp = newTs;
			}
		}
	}
}

void OpenTherm::process()
{
	noInterrupts();
	OpenThermStatus st = status;
	unsigned long ts = responseTimestamp;
	interrupts();	

	if (st == OpenThermStatus::READY) return;
	unsigned long newTs = micros();
	if (st != OpenThermStatus::NOT_INITIALIZED && (newTs - ts) > 1000000) {
		responseStatus = OpenThermResponseStatus::TIMEOUT;
		if (processResponseCallback != NULL) {
			processResponseCallback(response, responseStatus);
		}
		status = OpenThermStatus::READY;		
	}	
	else if (st == OpenThermStatus::RESPONSE_INVALID) {		
		responseStatus = OpenThermResponseStatus::INVALID;
		if (processResponseCallback != NULL) {
			processResponseCallback(response, responseStatus);
		}
		status = OpenThermStatus::DELAY;		
	}
	else if (st == OpenThermStatus::RESPONSE_READY) {		
		responseStatus = isValidResponse(response) ? OpenThermResponseStatus::SUCCESS : OpenThermResponseStatus::INVALID;
		if (processResponseCallback != NULL) {
			processResponseCallback(response, responseStatus);
		}
		status = OpenThermStatus::DELAY;		
	}
	else if (st == OpenThermStatus::DELAY) {
		if ((newTs - ts) > 100000) {
			status = OpenThermStatus::READY;
		}
	}	
}

bool OpenTherm::parity(unsigned long frame) //odd parity
{
	byte p = 0;
	while (frame > 0)
	{
		if (frame & 1) p++;
		frame = frame >> 1;
	}
	return (p & 1);
}

unsigned long OpenTherm::buildRequest(OpenThermRequestType type, OpenThermMessageID id, unsigned int data)
{
	unsigned long request = data;
	if (type == OpenThermRequestType::WRITE) {
		request |= 1ul << 28;
	}
	request |= ((unsigned long)id) << 16;
	if (parity(request)) request |= (1ul << 31);
	return request;
}

bool OpenTherm::isValidResponse(unsigned long response)
{
	if (parity(response)) return false;
	byte msgType = (response << 1) >> 29;
	return msgType == 4 || msgType == 5; //4 - read, 5 - write
}

void OpenTherm::end() {
	if (this->handleInterruptCallback != NULL) {		
		detachInterrupt(digitalPinToInterrupt(inPin));
	}
}

//building requests

unsigned long OpenTherm::buildSetBoilerStatusRequest(bool enableCentralHeating, bool enableHotWater, bool enableCooling, bool enableOutsideTemperatureCompensation, bool enableCentralHeating2) {
	unsigned int data = enableCentralHeating | (enableHotWater << 1) | (enableCooling << 2) | (enableOutsideTemperatureCompensation << 3) | (enableCentralHeating2 << 4);
	data <<= 8;	
	return buildRequest(OpenThermRequestType::READ, OpenThermMessageID::Status, data);
}

unsigned long OpenTherm::buildSetBoilerTemperatureRequest(float temperature) {
	unsigned int data = temperatureToData(temperature);
	return buildRequest(OpenThermRequestType::WRITE, OpenThermMessageID::TSet, data);
}

unsigned long OpenTherm::buildGetBoilerTemperatureRequest() {
	return buildRequest(OpenThermRequestType::READ, OpenThermMessageID::Tboiler, 0);
}

//parsing responses
bool OpenTherm::isFault(unsigned long response) {
	return response & 0x1;
}

bool OpenTherm::isCentralHeatingEnabled(unsigned long response) {
	return response & 0x2;
}

bool OpenTherm::isHotWaterEnabled(unsigned long response) {
	return response & 0x4;
}

bool OpenTherm::isFlameOn(unsigned long response) {
	return response & 0x8;
}

bool OpenTherm::isCoolingEnabled(unsigned long response) {
	return response & 0x10;
}

bool OpenTherm::isDiagnostic(unsigned long response) {
	return response & 0x40;
}

float OpenTherm::getTemperature(unsigned long response) {
	float temperature = isValidResponse(response) ? (response & 0xFFFF) / 256.0 : 0;
	return temperature;
}

unsigned int OpenTherm::temperatureToData(float temperature) {
	if (temperature < 0) temperature = 0;
	if (temperature > 100) temperature = 100;
	unsigned int data = (unsigned int)(temperature * 256);
	return data;
}

//basic requests

unsigned long OpenTherm::setBoilerStatus(bool enableCentralHeating, bool enableHotWater, bool enableCooling, bool enableOutsideTemperatureCompensation, bool enableCentralHeating2) {	
	return sendRequest(buildSetBoilerStatusRequest(enableCentralHeating, enableHotWater, enableCooling, enableOutsideTemperatureCompensation, enableCentralHeating2));	
}

bool OpenTherm::setBoilerTemperature(float temperature) {
	unsigned long response = sendRequest(buildSetBoilerTemperatureRequest(temperature));
	return isValidResponse(response);
}

float OpenTherm::getBoilerTemperature() {
	unsigned long response = sendRequest(buildGetBoilerTemperatureRequest());
	return getTemperature(response);
}
