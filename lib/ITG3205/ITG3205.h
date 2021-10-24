#include "Arduino.h"

#ifndef __ITG3205_H_
#define __ITG3205_H_


class ITG3205 {

#define ITG3200_Address 0x68
#define PWR_MGM 0x3E
#define SMPLRT_DIV 0x15
#define DLPF_FS 0x16
#define INT_CFG 0x17

#define LSB_DEG 14.375


#define TEMP_OUT_H 0x1B
#define TEMP_OUT_L 0x1C

#define GYRO_XOUT_H 0x1D
#define GYRO_XOUT_L 0x1E

#define GYRO_YOUT_H 0x1F
#define GYRO_YOUT_L 0x20

#define GYRO_ZOUT_H 0x21
#define GYRO_ZOUT_L 0x22

    byte address;
public:
    ITG3205(): address(ITG3200_Address) {}
    typedef struct vector
    {
        int x, y, z, t;
    } vector;

    vector g; // gyro angular velocity readings

    int offset[3];

    void calibrate();
    void initGyro();
    void GyroRead();


    byte WriteByte(byte i2c_address, byte address, byte data);

    void readGyroRaw(int *x, int *y, int *z);

    int readShortI2C();

    int readWordI2C();
};


#endif //__ITG3205_H_
