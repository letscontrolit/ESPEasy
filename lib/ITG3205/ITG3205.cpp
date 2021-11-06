
#include <Wire.h>
#include "ITG3205.h"


void ITG3205::initGyro() {
  WriteByte(address, PWR_MGM,    0x00);
  WriteByte(address, SMPLRT_DIV, 0x07); // Fsample = 1kHz / (7 + 1) = 125Hz, or 8ms per sample
  // WriteByte(address, DLPF_FS, 0x1E); // +/- 2000 dgrs/sec, 1KHz, Low Pass Filter Bandwidth: 5Hz
  WriteByte(address, DLPF_FS,    0x1B); // +/- 2000 dgrs/sec, 1KHz, Low Pass Filter Bandwidth: 98Hzh
  // WriteByte(address, DLPF_FS, 0x18); // +/- 2000 dgrs/sec, 8KHz, Low Pass Filter Bandwidth: 256Hz
  WriteByte(address, INT_CFG,    0x00); // No interrupts used
}

uint8_t ITG3205::readWhoAmI() {
  Wire.beginTransmission(address);
  Wire.write(TEMP_OUT_H);
  Wire.endTransmission();

  Wire.requestFrom(address, static_cast<uint8_t>(1));

  return Wire.read() & 0xFF;
}

void ITG3205::calibrate() {
  int i, count = 128;
  int xSum = 0, ySum = 0, zSum = 0;

  for (i = 0; i < count; i++) {
    delay(5);
    readGyroRaw();
    xSum += g.x;
    ySum += g.y;
    zSum += g.z;
  }

  offset[0] = -xSum / count;
  offset[1] = -ySum / count;
  offset[2] = -zSum / count;
}

void ITG3205::readGyroRaw() {
  Wire.beginTransmission(address);
  Wire.write(GYRO_XOUT_H);
  Wire.endTransmission();

  Wire.requestFrom(address, static_cast<uint8_t>(6)); // request 6 bytes from ITG3200

  g.x = readShortI2C();
  g.y = readShortI2C();
  g.z = readShortI2C();
}

void ITG3205::readGyro() {
  Wire.beginTransmission(address);
  Wire.write(GYRO_XOUT_H);
  Wire.endTransmission();

  Wire.requestFrom(address, static_cast<uint8_t>(6)); // request 6 bytes from ITG3200

  g.x = readShortI2C() + offset[0];                   // Apply calibration data
  g.y = readShortI2C() + offset[1];
  g.z = readShortI2C() + offset[2];

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

int8_t ITG3205::WriteByte(int8_t i2c_address, int8_t address, int8_t data) {
  Wire.beginTransmission(i2c_address);
  Wire.write(address);
  Wire.write(data);

  return Wire.endTransmission();
}
