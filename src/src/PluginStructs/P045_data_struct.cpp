#include "../PluginStructs/P045_data_struct.h"

#ifdef USES_P045

# define MPU6050_RA_GYRO_CONFIG              0x1B
# define MPU6050_RA_ACCEL_CONFIG             0x1C
# define MPU6050_RA_ACCEL_XOUT_H             0x3B
# define MPU6050_RA_PWR_MGMT_1               0x6B
# define MPU6050_ACONFIG_AFS_SEL_BIT         4
# define MPU6050_ACONFIG_AFS_SEL_LENGTH      2
# define MPU6050_GCONFIG_FS_SEL_BIT          4
# define MPU6050_GCONFIG_FS_SEL_LENGTH       2
# define MPU6050_CLOCK_PLL_XGYRO             0x01
# define MPU6050_GYRO_FS_250                 0x00
# define MPU6050_ACCEL_FS_2                  0x00
# define MPU6050_PWR1_SLEEP_BIT              6
# define MPU6050_PWR1_CLKSEL_BIT             2
# define MPU6050_PWR1_CLKSEL_LENGTH          3

P045_data_struct::P045_data_struct(uint8_t i2c_addr) : i2cAddress(i2c_addr)
{
  // Initialize the MPU6050, for details look at the MPU6050 library: MPU6050::Initialize
  writeBits(MPU6050_RA_PWR_MGMT_1,   MPU6050_PWR1_CLKSEL_BIT,     MPU6050_PWR1_CLKSEL_LENGTH,     MPU6050_CLOCK_PLL_XGYRO);
  writeBits(MPU6050_RA_GYRO_CONFIG,  MPU6050_GCONFIG_FS_SEL_BIT,  MPU6050_GCONFIG_FS_SEL_LENGTH,  MPU6050_GYRO_FS_250);
  writeBits(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, MPU6050_ACCEL_FS_2);
  writeBits(MPU6050_RA_PWR_MGMT_1,   MPU6050_PWR1_SLEEP_BIT,      1,                              0);

  // Read the MPU6050 once to clear out zeros (1st time reading MPU6050 returns all 0s)
  int16_t ax, ay, az, gx, gy, gz;

  getRaw6AxisMotion(&ax, &ay, &az, &gx, &gy, &gz);

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 5; ++j) {
      _axis[i][j] = 0;
    }
  }
}

void P045_data_struct::loop()
{
  // Read the sensorvalues, we run this bit every 1/10th of a second
  getRaw6AxisMotion(&_axis[0][3],
                    &_axis[1][3],
                    &_axis[2][3],
                    &_axis[0][4],
                    &_axis[1][4],
                    &_axis[2][4]);

  // Set the minimum and maximum value for each axis a-value, overwrite previous values if smaller/larger
  trackMinMax(_axis[0][3], &_axis[0][0], &_axis[0][1]);
  trackMinMax(_axis[1][3], &_axis[1][0], &_axis[1][1]);
  trackMinMax(_axis[2][3], &_axis[2][0], &_axis[2][1]);

  //          ^ current value @ 3   ^ min val @ 0         ^ max val @ 1

  /*
     // Uncomment this block if you want to debug your MPU6050, but be prepared for a log overload
     String log = F("MPU6050 : axis values: ");

     log += _axis[0][3];
     log += F(", ");
     log += _axis[1][3];
     log += F(", ");
     log += _axis[2][3];
     log += F(", g values: ");
     log += _axis[0][4];
     log += F(", ");
     log += _axis[1][4];
     log += F(", ");
     log += _axis[2][4];
     addLog(LOG_LEVEL_INFO, log);
   */

  // Run this bit every 5 seconds per deviceaddress (not per instance)
  if (timeOutReached(_timer + 5000))
  {
    _timer = millis();

    // Determine the maximum measured range of each axis
    for (uint8_t i = 0; i < 3; i++) {
      _axis[i][2] = abs(_axis[i][1] - _axis[i][0]);
      _axis[i][0] = _axis[i][3];
      _axis[i][1] = _axis[i][3];
    }
  }
}

void P045_data_struct::trackMinMax(int16_t current, int16_t *min, int16_t *max)
{
  // From nodemcu-laundry.ino by Nolan Gilley
  if (current > *max)
  {
    *max = current;
  }
  else if (current < *min)
  {
    *min = current;
  }
}

void P045_data_struct::getRaw6AxisMotion(int16_t *ax, int16_t *ay, int16_t *az, int16_t *gx, int16_t *gy, int16_t *gz) {
  // From I2Cdev::readBytes and MPU6050::getMotion6, both by Jeff Rowberg
  uint8_t buffer[14];
  uint8_t count = 0;

  I2C_write8(i2cAddress, MPU6050_RA_ACCEL_XOUT_H);
  Wire.requestFrom(i2cAddress, (uint8_t)14);

  for (; Wire.available(); count++) {
    buffer[count] = Wire.read();
  }
  *ax = (((int16_t)buffer[0]) << 8) | buffer[1];
  *ay = (((int16_t)buffer[2]) << 8) | buffer[3];
  *az = (((int16_t)buffer[4]) << 8) | buffer[5];
  *gx = (((int16_t)buffer[8]) << 8) | buffer[9];
  *gy = (((int16_t)buffer[10]) << 8) | buffer[11];
  *gz = (((int16_t)buffer[12]) << 8) | buffer[13];
}

void P045_data_struct::writeBits(uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data) {
  // From I2Cdev::writeBits by Jeff Rowberg
  //      010 value to write
  // 76543210 bit numbers
  //    xxx   args: bitStart=4, length=3
  // 00011100 mask byte
  // 10101111 original value (sample)
  // 10100011 original & ~mask
  // 10101011 masked | value
  bool is_ok = true;
  uint8_t b  = I2C_read8_reg(i2cAddress, regAddr, &is_ok);

  if (is_ok) {
    uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
    data <<= (bitStart - length + 1); // shift data into correct position
    data  &= mask;                    // zero all non-important bits in data
    b     &= ~(mask);                 // zero all important bits in existing byte
    b     |= data;                    // combine data with existing byte
    I2C_write8_reg(i2cAddress, regAddr, b);
  }
}

#endif // ifdef USES_P045
