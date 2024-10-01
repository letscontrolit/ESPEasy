#include "CG_RadSens.h"

// gcc -o test test.cpp radSens1v2.cpp -lwiringPi

CG_RadSens::CG_RadSens(uint8_t sensor_address)
{
    _sensor_address = sensor_address;
}
CG_RadSens::~CG_RadSens()
{
}

/*Initialization function and sensor connection. Returns false if the sensor is not connected to the I2C bus.*/
bool CG_RadSens::init()
{
#if defined(ARDUINO)
    Wire.beginTransmission(_sensor_address); // safety check, make sure the sensor is connected
    Wire.write(0x0);
    if (Wire.endTransmission(true) != 0)
        return false;
#elif defined(__arm__)
    _fd = wiringPiI2CSetup(_sensor_address);
    if (_fd == -1)
    {
        return false;
    }
#endif
    updatePulses();
    uint8_t res[2];
    if (i2c_read(RS_DEVICE_ID_RG, res, 2))
    {
        _chip_id = res[0];
        _firmware_ver = res[1];
    }
    return true;
}

/*Get chip id, default value: 0x7D.*/
uint8_t CG_RadSens::getChipId()
{
    return _chip_id;
}

/*Get firmware version.*/
uint8_t CG_RadSens::getFirmwareVersion()
{
    return _firmware_ver;
}

/*Get radiation intensity (dynamic period T < 123 sec).*/
float CG_RadSens::getRadIntensyDynamic()
{
    updatePulses();
    uint8_t res[3];
    if (i2c_read(RS_RAD_INTENSY_DYNAMIC_RG, res, 3))
    {
        float temp = (((uint32_t)res[0] << 16) | ((uint16_t)res[1] << 8) | res[2]) / 10.0;
        return temp;
    }
    else
    {
        return 0;
    }
}

/*Get radiation intensity (static period T = 500 sec).*/
float CG_RadSens::getRadIntensyStatic()
{
    updatePulses();
    uint8_t res[3];
    if (i2c_read(RS_RAD_INTENSY_STATIC_RG, res, 3))
    {
        return (((uint32_t)res[0] << 16) | ((uint16_t)res[1] << 8) | res[2]) / 10.0;
    }
    else
    {
        return 0;
    }
}

void CG_RadSens::updatePulses()
{
    uint8_t res[2];
    if (i2c_read(RS_PULSE_COUNTER_RG, res, 2))
    {
        _new_cnt = (res[0] << 8) | res[1];
        _pulse_cnt += _new_cnt;
    }
}

/*Get the accumulated number of pulses registered by the module
since the last I2C data reading.*/
uint32_t CG_RadSens::getNumberOfPulses()
{
    updatePulses();
    return _pulse_cnt;
}

/*Get current number of pulses*/
uint32_t CG_RadSens::getNumberOfNewPulses()
{
    updatePulses();
    return _new_cnt;
}

/*Reset accumulated count*/
void CG_RadSens::resetPulses()
{
    _pulse_cnt = 0;
}

/*Get sensor address.*/
uint8_t CG_RadSens::getSensorAddress()
{
    uint8_t res;
    if (i2c_read(RS_DEVICE_ADDRESS_RG, &res, 1))
    {
        _sensor_address = res;
        return _sensor_address;
    }
    return 0;
}

/*Get state of high-voltage voltage Converter.*/
bool CG_RadSens::getHVGeneratorState()
{
    uint8_t res;
    if (i2c_read(RS_HV_GENERATOR_RG, &res, 1))
    {
        if (res == 1)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

/*Get the value coefficient used for calculating the radiation intensity.*/
uint16_t CG_RadSens::getSensitivity()
{
    uint8_t res[2];
    if (i2c_read(RS_SENSITIVITY_RG, res, 2))
    {
        return res[1] * 256 + res[0];
    }
    return 0;
}

/*Control register for a high-voltage voltage Converter. By
default, it is in the enabled state. To enable the HV generator,
write 1 to the register, and 0 to disable it. If you try to write other
values, the command is ignored.
 * @param state  true - generator on / false - generator off
 */
bool CG_RadSens::setHVGeneratorState(bool state)
{
#if defined(ARDUINO)
    Wire.beginTransmission(_sensor_address);
#if (ARDUINO >= 100)
    Wire.write(RS_HV_GENERATOR_RG);
    if (state)
    {
        Wire.write(1);
    }
    else
    {
        Wire.write(0);
    }
#else
    Wire.send(RS_HV_GENERATOR_RG);
    if (state)
    {
        Wire.send(1);
    }
    else
    {
        Wire.send(0);
    }
#endif
    if (Wire.endTransmission(true) == 0)
        return true; //"true" sends stop message after transmission & releases I2C bus
#elif defined(__arm__)
    if (state)
    {
        if (wiringPiI2CWriteReg8(_fd, RS_HV_GENERATOR_RG, 1) > 0)
            return true;
    }
    else
    {
        if (wiringPiI2CWriteReg8(_fd, RS_HV_GENERATOR_RG, 0) > 0)
            return true;
    }
#endif
    return false;
}
/*Control register for a low power mode. By
default, it is in the disabled? state. To enable the LP mode,
write 1 to the register, and 0 to disable it. If you try to write other
values, the command is ignored.
 * @param state  true - LP on / false - LP off
 */
bool CG_RadSens::setLPmode(bool state)
{
#if defined(ARDUINO)
    Wire.beginTransmission(_sensor_address);
#if (ARDUINO >= 100)
    Wire.write(RS_LMP_MODE_RG);
    if (state)
    {
        Wire.write(1);
    }
    else
    {
        Wire.write(0);
    }
#else
    Wire.send(RS_LMP_MODE_RG);
    if (state)
    {
        Wire.send(1);
    }
    else
    {
        Wire.send(0);
    }
#endif
    if (Wire.endTransmission(true) == 0)
        return true; //"true" sends stop message after transmission & releases I2C bus
#elif defined(__arm__)
    if (state)
    {
        if (wiringPiI2CWriteReg8(_fd, RS_LMP_MODE_RG, 1) > 0)
            return true;
    }
    else
    {
        if (wiringPiI2CWriteReg8(_fd, RS_LMP_MODE_RG, 0) > 0)
            return true;
    }
#endif
    return false;
}
/*Contains the value coefficient used for calculating
the radiation intensity. If necessary (for example, when installing a different
type of counter), the necessary sensitivity value in
Imp / uR is entered in the register. The default value is 105 Imp / uR. At the end of
recording, the new value is stored in the non-volatile memory of the
microcontroller.
*@param sens sensitivity coefficient in Impulse / uR
*/
bool CG_RadSens::setSensitivity(uint16_t sens)
{
#if defined(ARDUINO)
    Wire.beginTransmission(_sensor_address);
#if (ARDUINO >= 100)
    Wire.write(RS_SENSITIVITY_RG);
    Wire.write((uint8_t)(sens & 0xFF));
    Wire.endTransmission(true);
    delay(15);
    Wire.beginTransmission(_sensor_address);
    Wire.write(RS_SENSITIVITY_RG + 0x01);
    Wire.write((uint8_t)(sens >> 8));
#else
    Wire.send(RS_SENSITIVITY_RG);
    Wire.send((uint8_t)(sens & 0xFF));
    Wire.endTransmission(true);
    delay(15);
    Wire.beginTransmission(_sensor_address);
    Wire.send(RS_SENSITIVITY_RG + 0x01);
    Wire.send((uint8_t)(sens >> 8));
#endif
    bool err = Wire.endTransmission(true);
    delay(15);
    if (!err)
        return true;
#elif defined(__arm__)
    if (wiringPiI2CWriteReg16(_fd, RS_SENSITIVITY_RG, sens) > 0)
        return true;
#endif
    return false;
}

/*Control register for a indication diode. By
default, it is in the enabled state. To enable the indication,
write 1 to the register, and 0 to disable it. If you try to write other
values, the command is ignored.
 * @param state  true - diode on / false - diode off
*/
bool CG_RadSens::setLedState(bool state)
{
#if defined(ARDUINO)
    Wire.beginTransmission(_sensor_address);
#if (ARDUINO >= 100)
    Wire.write(RS_LED_CONTROL_RG);
    if (state)
    {
        Wire.write(1);
    }
    else
    {
        Wire.write(0);
    }
#else
    Wire.send(RS_LED_CONTROL_RG);
    if (state)
    {
        Wire.send(1);
    }
    else
    {
        Wire.send(0);
    }
#endif
    bool err = Wire.endTransmission(true);
    delay(15);
    if (!err)
        return true;
#elif defined(__arm__)
    if (state)
    {
        if (wiringPiI2CWriteReg8(_fd, RS_LED_CONTROL_RG, 1) > 0)
            return true;
    }
    else
    {
        if (wiringPiI2CWriteReg8(_fd, RS_LED_CONTROL_RG, 0) > 0)
            return true;
    }
#endif
    return false;
}

/*Get state of led indication.*/
bool CG_RadSens::getLedState()
{
    uint8_t res;
    if (i2c_read(RS_LED_CONTROL_RG, &res, 1))
    {
        if (res == 1)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

/**
 * Read block of data
 * @param regAddr - address of starting register
 * @param dest -destination array
 * @param num - number of bytes to read
 */
bool CG_RadSens::i2c_read(uint8_t RegAddr, uint8_t *dest, uint8_t num)
{
#if defined(ARDUINO)
    Wire.beginTransmission(_sensor_address);
    Wire.write(RegAddr);
    if (Wire.endTransmission() != 0)
        return false;
    if (Wire.requestFrom(_sensor_address, num) == num)
    {
        for (int i = 0; i < num; i++)
            dest[i] = Wire.read();
        return true;
    }
    return false;
#elif defined(__arm__)
    int buf = 0;
    for (int i = 0; i < num; i++)
    {
        buf = wiringPiI2CReadReg8(_fd, RegAddr);
        if (buf < 0)
            return false;
        dest[i] = buf;
    }
    return true;
#endif
}
