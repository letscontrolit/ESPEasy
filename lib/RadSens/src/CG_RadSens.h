#ifndef _RADSENS1V2_H_
#define _RADSENS1V2_H_

#include <stdint.h>

#if defined(ARDUINO)
#include <Arduino.h>
#include <Wire.h>
#elif defined(__arm__)
#include <wiringPiI2C.h>
#include <stdio.h>
#endif

#define RS_REG_COUNT 21

// Default radSens i2c device address
#define RS_DEFAULT_I2C_ADDRESS 0x66

// Device id, default value: 0x7D
// Size: 8 bit
#define RS_DEVICE_ID_RG 0x00

// Firmware version
// Size: 8 bit
#define RS_FIRMWARE_VER_RG 0x01

// Radiation intensity (dynamic period T < 123 sec)
// Size: 24 bit
#define RS_RAD_INTENSY_DYNAMIC_RG 0x03

// Radiation intensity (static period T = 500 sec)
// Size: 24 bit
#define RS_RAD_INTENSY_STATIC_RG 0x06

/*Contains the accumulated number of pulses registered by the module
since the last I2C data reading. The value is reset each
time it is read. Allows you to process directly the pulses
from the Geiger counter and implement other algorithms. The value is updated
when each pulse is registered.
Size: 16 bit */
#define RS_PULSE_COUNTER_RG 0x09

/*This register is used to change the device address when multiple
devices need to be connected to the same line at the same
time. By default, it contains the value 0x66. At the end of recording, the new
value is stored in the non-volatile memory of the microcontroller.
Size: 8 bit
Access: R/W*/
#define RS_DEVICE_ADDRESS_RG 0x10

/*Control register for a high-voltage voltage Converter. By
default, it is in the enabled state. To enable the HV generator,
write 1 to the register, and 0 to disable it. If you try to write other
values, the command is ignored.
Size: 8 bit
Access: R/W*/
#define RS_HV_GENERATOR_RG 0x11

/*Contains the value coefficient used for calculating
the radiation intensity. If necessary (for example, when installing a different
type of counter), the necessary sensitivity value in
imp/MKR is entered in the register. The default value is 105 imp/MKR. At the end of
recording, the new value is stored in the non-volatile memory of the
microcontroller.
Size: 16 bit
Access: R/W*/
#define RS_SENSITIVITY_RG 0x12

/*Control register for a indication diode. By
default, it is in the enabled state. To enable the indication,
write 1 to the register, and 0 to disable it. If you try to write other
values, the command is ignored.
Size: 8 bit
Access: R/W*/
#define RS_LED_CONTROL_RG 0x14
/*Control register for a low power mode. to enable send 1 to the register, and 0 to disable)
Size: 8 bit
Access: R/W*/
#define RS_LMP_MODE_RG 0x0C

class CG_RadSens
{
private:
#if defined(__arm__)
    int _fd = 0;
#endif
    uint8_t _sensor_address;
    uint8_t _chip_id = 0;
    uint8_t _firmware_ver = 0;
    uint32_t _pulse_cnt = 0;
    uint32_t _new_cnt = 0;
    bool i2c_read(uint8_t RegAddr, uint8_t *dest, uint8_t num);
    void updatePulses();

public:
    CG_RadSens(uint8_t sensorAddress);
    ~CG_RadSens();
    bool init();
    uint8_t getChipId();
    uint8_t getFirmwareVersion();
    float getRadIntensyDynamic();
    float getRadIntensyStatic();
    uint32_t getNumberOfPulses();
    uint32_t getNumberOfNewPulses();
    void resetPulses();
    uint8_t getSensorAddress();
    bool getHVGeneratorState();
    bool getLedState();
    uint16_t getSensitivity();
    bool setHVGeneratorState(bool state);
    bool setLPmode(bool state);
    bool setSensitivity(uint16_t sens);
    bool setLedState(bool state);
};

#endif // _RADSENS1V2_H_
