#include "../PluginStructs/P079_data_struct.h"

#ifdef USES_P079



WemosMotor::WemosMotor(uint8_t address, uint8_t motor, uint32_t freq)
  : _address(address)
{
  _use_STBY_IO = false;

  if (motor == P079_MOTOR_A) {
    _motor = P079_MOTOR_A;
  }
  else {
    _motor = P079_MOTOR_B;
  }

  setfreq(freq);
}

WemosMotor::WemosMotor(uint8_t address, uint8_t motor, uint32_t freq, uint8_t STBY_IO)
  : _address(address)
{
  _use_STBY_IO = true;
  _STBY_IO     = STBY_IO;

  if (motor == P079_MOTOR_A) {
    _motor = P079_MOTOR_A;
  }
  else {
    _motor = P079_MOTOR_B;
  }

  setfreq(freq);

  pinMode(_STBY_IO, OUTPUT);
  digitalWrite(_STBY_IO, LOW);
}

/* setfreq() -- set PWM's frequency
   freq: PWM's frequency

   total 4bytes
 |0.5byte CMD     | 3.5byte Parm|
 |CMD             | parm        |
 |0x0X  set freq  | uint32  freq|
 */
void WemosMotor::setfreq(uint32_t freq)
{
  Wire.beginTransmission(_address);
  Wire.write(((byte)(freq >> 24)) & (byte)0x0f);
  Wire.write((byte)(freq >> 16));
  Wire.write((byte)(freq >> 8));
  Wire.write((byte)freq);
  Wire.endTransmission(); // stop transmitting
  delay(0);
}

/* setmotor() -- set motor
   motor:
        P079_MOTOR_A    0   Motor A
        P079_MOTOR_B    1   Motor B

   dir:
        P079_SHORT_BRAKE  0
        P079_CCW          1
        P079_CW               2
        P079_STOP         3
        P079_STANDBY      4

   pwm_val:
        0.00 - 100.00  (%)

   total 4bytes
 |0.5byte CMD      | 3.5byte Parm         |
 |CMD              | parm                 |
 |0x10  set motorA | uint8 dir  uint16 pwm|
 |0x11  set motorB | uint8 dir  uint16 pwm|
 */
void WemosMotor::setmotor(uint8_t dir, float pwm_val)
{
  uint16_t _pwm_val;

  if (_use_STBY_IO == true) {
    if (dir == P079_STANDBY) {
      digitalWrite(_STBY_IO, LOW);
      return;
    } else {
      digitalWrite(_STBY_IO, HIGH);
    }
  }

  Wire.beginTransmission(_address);
  Wire.write(_motor | (byte)0x10); // CMD either 0x10 or 0x11
  Wire.write(dir);

  // PWM in %
  _pwm_val = uint16_t(pwm_val * 100);

  if (_pwm_val > 10000) { // _pwm_val > 100.00
    _pwm_val = 10000;
  }

  Wire.write((byte)(_pwm_val >> 8));
  Wire.write((byte)_pwm_val);
  Wire.endTransmission(); // stop transmitting

  delay(0);
}

void WemosMotor::setmotor(uint8_t dir)
{
  setmotor(dir, 100);
}

LOLIN_I2C_MOTOR::LOLIN_I2C_MOTOR(uint8_t address) : _address(address) {}

/*
    Change Motor Status.
    ch: Motor Channel
        MOTOR_CH_A
        MOTOR_CH_B
        MOTOR_CH_BOTH

    sta: Motor Status
        MOTOR_STATUS_STOP
        MOTOR_STATUS_CCW
        MOTOR_STATUS_CW
        MOTOR_STATUS_SHORT_BRAKE
        MOTOR_STATUS_STANDBY
 */
unsigned char LOLIN_I2C_MOTOR::changeStatus(unsigned char ch, unsigned char sta)
{
  send_data[0] = CHANGE_STATUS;
  send_data[1] = ch;
  send_data[2] = sta;
  unsigned char result = sendData(send_data, 3);

  return result;
}

/*
    Change Motor Frequency
        ch: Motor Channel
            MOTOR_CH_A
            MOTOR_CH_B
            MOTOR_CH_BOTH

        freq: PWM frequency (Hz)
            1 - 80KHz, typically 1000Hz
 */
unsigned char LOLIN_I2C_MOTOR::changeFreq(unsigned char ch, uint32_t freq)
{
  send_data[0] = CHANGE_FREQ;
  send_data[1] = ch;

  send_data[2] = (uint8_t)(freq & 0xff);
  send_data[3] = (uint8_t)((freq >> 8) & 0xff);
  send_data[4] = (uint8_t)((freq >> 16) & 0xff);
  unsigned char result = sendData(send_data, 5);

  return result;
}

/*
    Change Motor Duty.
    ch: Motor Channel
        MOTOR_CH_A
        MOTOR_CH_B
        MOTOR_CH_BOTH

    duty: PWM Duty (%)
        0.01 - 100.00 (%)
 */
unsigned char LOLIN_I2C_MOTOR::changeDuty(unsigned char ch, float duty)
{
  uint16_t _duty;

  _duty = (uint16_t)(duty * 100);

  send_data[0] = CHANGE_DUTY;
  send_data[1] = ch;

  send_data[2] = (uint8_t)(_duty & 0xff);
  send_data[3] = (uint8_t)((_duty >> 8) & 0xff);
  unsigned char result = sendData(send_data, 4);

  return result;
}

/*
    Reset Device.
 */
unsigned char LOLIN_I2C_MOTOR::reset()
{
  send_data[0] = RESET_SLAVE;
  unsigned char result = sendData(send_data, 1);

  return result;
}

/*
    Change Device I2C address
    address: when address=0, address>=127, will change address to default I2C address 0x31
 */
unsigned char LOLIN_I2C_MOTOR::changeAddress(unsigned char address)
{
  send_data[0] = CHANGE_I2C_ADDRESS;
  send_data[1] = address;
  unsigned char result = sendData(send_data, 2);

  return result;
}

/*
    Get PRODUCT_ID and Firmwave VERSION
 */
unsigned char LOLIN_I2C_MOTOR::getInfo(void)
{
  send_data[0] = GET_SLAVE_STATUS;
  unsigned char result = sendData(send_data, 1);

  if (result == 0)
  {
    PRODUCT_ID = get_data[0];
    VERSION_ID = get_data[1];
  }
  else
  {
    PRODUCT_ID = 0;
    VERSION_ID = 0;
  }

  return result;
}

/*
    Send and Get I2C Data
 */
unsigned char LOLIN_I2C_MOTOR::sendData(unsigned char *data, unsigned char len)
{
  unsigned char i;

  if ((_address == 0) || (_address >= 127))
  {
    return 1;
  }
  else
  {
    Wire.beginTransmission(_address);

    for (i = 0; i < len; i++) {
      Wire.write(data[i]);
    }
    Wire.endTransmission();
    delay(50);

    if (data[0] == GET_SLAVE_STATUS) {
      Wire.requestFrom((int)_address, 2);
    }
    else {
      Wire.requestFrom((int)_address, 1);
    }

    i = 0;

    while (Wire.available())
    {
      get_data[i] = Wire.read();
      i++;
    }

    return 0;
  }
}

#endif // ifdef USES_P079
