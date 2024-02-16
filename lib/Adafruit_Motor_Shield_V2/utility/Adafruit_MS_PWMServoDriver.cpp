/***************************************************
  This is a library for our Adafruit 16-channel PWM & Servo driver

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/815

  These displays use I2C to communicate, 2 pins are required to
  interface. For Arduino UNOs, thats SCL -> Analog 5, SDA -> Analog 4

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Adafruit_MS_PWMServoDriver.h>

Adafruit_MS_PWMServoDriver::Adafruit_MS_PWMServoDriver(uint8_t addr) {
  _i2caddr = addr;
}

bool Adafruit_MS_PWMServoDriver::begin(TwoWire *theWire) {
  if (i2c_dev)
    delete i2c_dev;
  i2c_dev = new Adafruit_I2CDevice(_i2caddr, theWire);
  if (!i2c_dev->begin())
    return false;
  reset();
  return true;
}

void Adafruit_MS_PWMServoDriver::reset(void) { write8(PCA9685_MODE1, 0x0); }

void Adafruit_MS_PWMServoDriver::setPWMFreq(float freq) {
  // Serial.print("Attempting to set freq ");
  // Serial.println(freq);

  freq *=
      0.9; // Correct for overshoot in the frequency setting (see issue #11).

  float prescaleval = 25000000;
  prescaleval /= 4096;
  prescaleval /= freq;
  prescaleval -= 1;
  // Serial.print("Estimated pre-scale: "); Serial.println(prescaleval);
  uint8_t prescale = floor(prescaleval + 0.5);
  // Serial.print("Final pre-scale: "); Serial.println(prescale);

  uint8_t oldmode = read8(PCA9685_MODE1);
  uint8_t newmode = (oldmode & 0x7F) | 0x10; // sleep
  write8(PCA9685_MODE1, newmode);            // go to sleep
  write8(PCA9685_PRESCALE, prescale);        // set the prescaler
  write8(PCA9685_MODE1, oldmode);
  delay(5);
  write8(PCA9685_MODE1,
         oldmode |
             0xa1); //  This sets the MODE1 register to turn on auto increment.
                    // This is why the beginTransmission below was not working.
  //  Serial.print("Mode now 0x"); Serial.println(read8(PCA9685_MODE1), HEX);
}

void Adafruit_MS_PWMServoDriver::setPWM(uint8_t num, uint16_t on,
                                        uint16_t off) {
  // Serial.print("Setting PWM "); Serial.print(num); Serial.print(": ");
  // Serial.print(on); Serial.print("->"); Serial.println(off);
  uint8_t buffer[5];
  buffer[0] = LED0_ON_L + 4 * num;
  buffer[1] = on;
  buffer[2] = on >> 8;
  buffer[3] = off;
  buffer[4] = off >> 8;
  i2c_dev->write(buffer, 5);
}

uint8_t Adafruit_MS_PWMServoDriver::read8(uint8_t addr) {
  uint8_t buffer[1] = {addr};
  i2c_dev->write_then_read(buffer, 1, buffer, 1);
  return buffer[0];
}

void Adafruit_MS_PWMServoDriver::write8(uint8_t addr, uint8_t d) {
  uint8_t buffer[2] = {addr, d};
  i2c_dev->write(buffer, 2);
}
