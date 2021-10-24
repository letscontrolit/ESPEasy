#include "config.h"

#include <Wire.h>
#include "ITG3205.h"


void ITG3205::initGyro() {
    WriteByte(address, PWR_MGM, 0x00);
    WriteByte(address, SMPLRT_DIV, 0x07); //Fsample = 1kHz / (7 + 1) = 125Hz, or 8ms per sample
    //WriteByte(address, DLPF_FS, 0x1E); // +/- 2000 dgrs/sec, 1KHz, Low Pass Filter Bandwidth: 5Hz
    WriteByte(address, DLPF_FS, 0x1B); // +/- 2000 dgrs/sec, 1KHz, Low Pass Filter Bandwidth: 98Hzh
    //WriteByte(address, DLPF_FS, 0x18); // +/- 2000 dgrs/sec, 8KHz, Low Pass Filter Bandwidth: 256Hz
    WriteByte(address, INT_CFG, 0x00);
}

void ITG3205::calibrate() {
    int x, y, z, i, count = 128;
    int xSum = 0, ySum = 0, zSum = 0;

    for(i = 0; i < count; i++) {
        delay(5);
        readGyroRaw(&x, &y, &z);
        xSum += x;
        ySum += y;
        zSum += z;
    }

    offset[0] = -xSum / count;
    offset[1] = -ySum / count;
    offset[2] = -zSum / count;
}

void ITG3205::readGyroRaw(int *x, int *y, int *z) {
    Wire.beginTransmission(address);
    Wire.write(GYRO_XOUT_H);
    Wire.endTransmission();

    Wire.requestFrom(address, (byte)6);

    *x = readShortI2C();
    *y = readShortI2C();
    *z = readShortI2C();
}

void ITG3205::GyroRead() {
    Wire.beginTransmission(address);
    Wire.write(GYRO_XOUT_H);
    Wire.endTransmission();

    Wire.requestFrom(address, (byte)6);    // request 8 bytes from ITG3200

    //g.t = (buff[0] << 8) | buff[1]; // temperature
    g.x = readShortI2C() + offset[0];
    g.y = readShortI2C() + offset[1];
    g.z = readShortI2C() + offset[2];

    Wire.endTransmission();

    g.x /= LSB_DEG;
    g.y /= LSB_DEG;
    g.z /= LSB_DEG;

}

int ITG3205::readShortI2C() {
    return (signed short)readWordI2C();
}


int ITG3205::readWordI2C() {
    return (Wire.read() << 8) | Wire.read();
}

byte ITG3205::WriteByte(byte i2c_address, byte address, byte data) {
    Wire.beginTransmission(i2c_address);
    Wire.write(address);
    Wire.write(data);
    byte result = Wire.endTransmission();

    #if DEBUG_SENSORS_ENABLE
    //do some error checking
    if (result > 0) {
        debug((char *)result, "I2C Write PROBLEM:");
    }
    #endif
    return result;
}
