#include <Arduino.h>

#ifndef __ITG3205_H_
# define __ITG3205_H_


# define ITG3200_Address  0x68
# define ITG3200_WHOAMI   0x00
# define PWR_MGM          0x3E
# define SMPLRT_DIV       0x15
# define DLPF_FS          0x16
# define INT_CFG          0x17

# define LSB_DEG          14.375

# define TEMP_OUT_H       0x1B
# define TEMP_OUT_L       0x1C

# define GYRO_XOUT_H      0x1D
# define GYRO_XOUT_L      0x1E

# define GYRO_YOUT_H      0x1F
# define GYRO_YOUT_L      0x20

# define GYRO_ZOUT_H      0x21
# define GYRO_ZOUT_L      0x22

class ITG3205 {
  uint8_t address;

public:

  ITG3205() : address(ITG3200_Address) {}

  ITG3205(uint8_t _address) : address(_address) {}

  typedef struct gyroVelocity
  {
    int x = 0;
    int y = 0;
    int z = 0;
  } gyroVelocity;

  gyroVelocity g; // gyro angular velocity readings

  int offset[3] = { 0 };

  uint8_t readWhoAmI();
  void    calibrate();
  void    initGyro();
  void    readGyro();
  void    readGyroRaw();

  int8_t  WriteByte(int8_t i2c_address,
                    int8_t address,
                    int8_t data);

  int readShortI2C();

  int readWordI2C();
};


#endif // __ITG3205_H_
