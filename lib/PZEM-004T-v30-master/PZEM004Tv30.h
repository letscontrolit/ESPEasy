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


#define PZEM_DEFAULT_ADDR    0xF8


class PZEM004Tv30
{
public:
#if defined(PZEM004_SOFTSERIAL)
    PZEM004Tv30(const ESPEasySerialPort port, uint8_t receivePin, uint8_t transmitPin, uint8_t addr=PZEM_DEFAULT_ADDR);
#endif
    ~PZEM004Tv30();


    float voltage();
    float current();
    float power();
    float energy();
    float frequency();
    float pf();


    bool setAddress(uint8_t addr);
    uint8_t getAddress();

    bool setPowerAlarm(uint16_t watts);
    bool getPowerAlarm();

    bool resetEnergy();
    void init(uint8_t addr); // Init common to all constructors

private:

    Stream* _serial; // Serial interface
    bool _isSoft;    // Is serial interface software

    uint8_t _addr;   // Device address

    struct {
        float voltage;
        float current;
        float power;
        float energy;
        float frequeny;
        float pf;
        uint16_t alarms;
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
