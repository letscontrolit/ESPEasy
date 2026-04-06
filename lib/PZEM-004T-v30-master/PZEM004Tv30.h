/*
 * PZEM-004Tv30.h
 *
 * Interface library for the upgraded version of PZEM-004T v3.0
 * Based on the PZEM004T library by @olehs https://github.com/olehs/PZEM004T
 *
 * Author: Jakub Mandula https://github.com/mandulaj
 *
 *
*/


#ifndef PZEM004TV30_H
#define PZEM004TV30_H



#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define PZEM004_SOFTSERIAL

#if defined(PZEM004_SOFTSERIAL)
//#include <SoftwareSerial.h>
#include <ESPeasySerial.h>
#endif

#define PZEM004Tv30_EXPECTED_RESPONSE 25
#define PZEM004Tv30_REQUEST_REGISTERS 10

#define PZEM017v1_EXPECTED_RESPONSE 21
#define PZEM017v1_REQUEST_REGISTERS 8

#define PZEM_DEFAULT_ADDR    0xF8

#define PZEM_MAX_EXPECTED_RESPONSE 25 // PZEM004v30 needs this buffer size, adjust for models that need a bigger buffer

enum class PZEM_model : uint8_t {
    PZEM004Tv30 = 0, // Buffer size 25, request 10 registers
    PZEM017Tv1  = 1, // Buffer size 21, request 8 registers
};

class PZEM004Tv30
{
public:
#if defined(PZEM004_SOFTSERIAL)
    PZEM004Tv30(const ESPEasySerialPort port, uint8_t receivePin, uint8_t transmitPin, uint8_t addr=PZEM_DEFAULT_ADDR);
#endif
    ~PZEM004Tv30();

    void setModel(PZEM_model model); // Sets some communication buffer params

    float voltage();
    float current();
    float power();
    float energy();
    float frequency();
    float pf();


    bool setAddress(uint8_t addr);
    uint8_t getAddress();

    // Unused methods:
    // bool setPowerAlarm(uint16_t watts);
    // bool getPowerAlarm();

    // bool setHighvoltAlarm(uint16_t volts); //moja uprava
    // bool setLowvoltAlarm(uint16_t volts); //moja uprava
    // bool isHighvoltAlarmOn(); //upravil som
    // bool isLowvoltAlarmOn(); //upravil som

    bool resetEnergy();
    void init(uint8_t addr); // Init common to all constructors

private:

    Stream* _serial; // Serial interface
    bool _isSoft;    // Is serial interface software

    uint8_t _addr;   // Device address
    PZEM_model _model = PZEM_model::PZEM004Tv30; // Current default

    uint8_t _response[PZEM_MAX_EXPECTED_RESPONSE];
    uint8_t _expectedResponse = PZEM004Tv30_EXPECTED_RESPONSE;
    uint8_t _requestRegisters = PZEM004Tv30_REQUEST_REGISTERS;

    struct {
        float voltage;
        float current;
        float power;
        float energy;
        float frequeny; // PZEM004
        float pf; // PZEM004
        uint16_t alarms; // PZEM004
        uint16_t HVAlarms; //upravil som PZEM017
        uint16_t LVAlarms; //upravil sOM PZEM017
    }  _currentValues; // Measured values

    uint64_t _lastRead; // Last time values were updated

    

    //void init(uint8_t addr); // Init common to all constructors

    bool updateValues();    // Get most up to date values from device registers and cache them
    uint16_t recieve(uint8_t *resp, uint16_t len); // Receive len bytes into a buffer

    bool sendCmd8(uint8_t cmd, uint16_t rAddr, uint16_t val, bool check=false); // Send 8 byte command

    void setCRC(uint8_t *buf, uint16_t len);           // Set the CRC for a buffer
    bool checkCRC(const uint8_t *buf, uint16_t len);   // Check CRC of buffer

    uint16_t CRC16(const uint8_t *data, uint16_t len); // Calculate CRC of buffer
};

#endif // PZEM004T_H
