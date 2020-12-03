#ifndef PLUGINSTRUCTS_P079_DATA_STRUCT_H
#define PLUGINSTRUCTS_P079_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P079

# define DEF_I2C_ADDRESS_079  0x30
# define MOTOR_FREQ_P079      1000
# define PRODUCT_ID_I2C_LOLIN 0x02


# if (ARDUINO >= 100)
 #  include "Arduino.h"
# else // if (ARDUINO >= 100)
 #  include "WProgram.h"
# endif  // if (ARDUINO >= 100)

# include "Wire.h"

# define P079_MOTOR_A     0
# define P079_MOTOR_B     1
# define P079_SHORT_BRAKE 0
# define P079_CCW         1
# define P079_CW          2
# define P079_STOP        3
# define P079_STANDBY     4

enum class P079_BoardType
{
  WemosMotorshield = 0x01,
  LolinMotorshield
};

enum class MOTOR_STATES
{
  MOTOR_STOP = 0x00,
  MOTOR_FWD,
  MOTOR_REV,
  MOTOR_STBY,
  MOTOR_BRAKE
};


enum LOLIN_I2C_CMD
{
  GET_SLAVE_STATUS = 0x01,
  RESET_SLAVE,
  CHANGE_I2C_ADDRESS,
  CHANGE_STATUS,
  CHANGE_FREQ,
  CHANGE_DUTY
};

enum LOLIN_MOTOR_STATUS
{
  MOTOR_STATUS_STOP = 0x00,
  MOTOR_STATUS_CCW,
  MOTOR_STATUS_CW,
  MOTOR_STATUS_SHORT_BRAKE,
  MOTOR_STATUS_STANDBY
};

enum LOLIN_MOTOR_CHANNEL
{
  MOTOR_CH_A = 0x00,
  MOTOR_CH_B,
  MOTOR_CH_BOTH
};


class WemosMotor {
public:

  WemosMotor(uint8_t  address,
             uint8_t  motor,
             uint32_t freq);
  WemosMotor(uint8_t  address,
             uint8_t  motor,
             uint32_t freq,
             uint8_t  STBY_IO);
  void setfreq(uint32_t freq);
  void setmotor(uint8_t dir,
                float   pwm_val);
  void setmotor(uint8_t dir);

private:

  uint8_t _address;
  uint8_t _motor;
  bool _use_STBY_IO = false;
  uint8_t _STBY_IO  = 0;
};


class LOLIN_I2C_MOTOR {
public:

  LOLIN_I2C_MOTOR(unsigned char address);
  unsigned char reset(void);
  unsigned char getInfo(void);
  unsigned char changeStatus(unsigned char ch,
                             unsigned char sta);
  unsigned char changeFreq(unsigned char ch,
                           uint32_t      freq);
  unsigned char changeDuty(unsigned char ch,
                           float         duty);
  unsigned char changeAddress(unsigned char address);

  unsigned char VERSION_ID = 0;
  unsigned char PRODUCT_ID = 0;

private:

  unsigned char _address;
  unsigned char send_data[5] = { 0 };
  unsigned char get_data[2]  = { 0 };
  unsigned char sendData(unsigned char *data,
                         unsigned char  len);
};


#endif // ifdef USES_P079
#endif // ifndef PLUGINSTRUCTS_P079_DATA_STRUCT_H
