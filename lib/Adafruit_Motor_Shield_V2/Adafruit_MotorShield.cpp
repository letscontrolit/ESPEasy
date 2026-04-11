/*!
 * @file Adafruit_MotorShield.cpp
 *
 * @mainpage Adafruit FXOS8700 accel/mag sensor driver
 *
 * @section intro_sec Introduction
 *
 * This is the library for the Adafruit Motor Shield V2 for Arduino.
 * It supports DC motors & Stepper motors with microstepping as well
 * as stacking-support. It is *not* compatible with the V1 library!
 * For use with the Motor Shield https://www.adafruit.com/products/1483
 * and Motor FeatherWing https://www.adafruit.com/product/2927
 *
 * This shield/wing uses I2C to communicate, 2 pins (SCL+SDA) are required
 * to interface.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section author Author
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries.
 *
 * @section license License
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#include "Adafruit_MotorShield.h"
#include "Arduino.h"
#include <Adafruit_MS_PWMServoDriver.h>

#if (MICROSTEPS == 8)
///! A sinusoial microstepping curve for the PWM output (8-bit range) with 9
/// points - last one is start of next step.
static uint8_t microstepcurve[] = {0, 50, 98, 142, 180, 212, 236, 250, 255};
#elif (MICROSTEPS == 16)
///! A sinusoial microstepping curve for the PWM output (8-bit range) with 17
/// points - last one is start of next step.
static uint8_t microstepcurve[] = {0,   25,  50,  74,  98,  120, 141, 162, 180,
                                   197, 212, 225, 236, 244, 250, 253, 255};
#endif

/**************************************************************************/
/*!
    @brief  Create the Motor Shield object at an I2C address, default is 0x60
    @param  addr Optional I2C address if you've changed it
*/
/**************************************************************************/
Adafruit_MotorShield::Adafruit_MotorShield(uint8_t addr) { _addr = addr; }

/**************************************************************************/
/*!
    @brief  Initialize the I2C hardware and PWM driver, then turn off all pins.
    @param    freq
    The PWM frequency for the driver, used for speed control and microstepping.
    By default we use 1600 Hz which is a little audible but efficient.
    @param    theWire
    A pointer to an optional I2C interface. If not provided, we use Wire or
   Wire1 (on Due)
    @returns true if successful, false otherwise
*/
/**************************************************************************/
bool Adafruit_MotorShield::begin(uint16_t freq, TwoWire *theWire) {
  // init PWM w/_freq
  _pwm = Adafruit_MS_PWMServoDriver(_addr);
  if (!_pwm.begin(theWire))
    return false;
  _freq = freq;
  _pwm.setPWMFreq(_freq); // This is the maximum PWM frequency
  for (uint8_t i = 0; i < 16; i++)
    _pwm.setPWM(i, 0, 0);
  return true;
}

/**************************************************************************/
/*!
    @brief  Helper that sets the PWM output on a pin and manages 'all on or off'
    @param  pin The PWM output on the driver that we want to control (0-15)
    @param  value The 12-bit PWM value we want to set (0-4095) - 4096 is a
   special 'all on' value
*/
/**************************************************************************/
void Adafruit_MotorShield::setPWM(uint8_t pin, uint16_t value) {
  if (value > 4095) {
    _pwm.setPWM(pin, 4096, 0);
  } else
    _pwm.setPWM(pin, 0, value);
}

/**************************************************************************/
/*!
    @brief  Helper that sets the PWM output on a pin as if it were a GPIO
    @param  pin The PWM output on the driver that we want to control (0-15)
    @param  value HIGH or LOW depending on the value you want!
*/
/**************************************************************************/
void Adafruit_MotorShield::setPin(uint8_t pin, boolean value) {
  if (value == LOW)
    _pwm.setPWM(pin, 0, 0);
  else
    _pwm.setPWM(pin, 4096, 0);
}

/**************************************************************************/
/*!
    @brief  Mini factory that will return a pointer to an already-allocated
    Adafruit_DCMotor object. Initializes the DC motor and turns off all pins
    @param  num The DC motor port we want to use: 1 thru 4 are valid
    @returns NULL if something went wrong, or a pointer to a Adafruit_DCMotor
*/
/**************************************************************************/
Adafruit_DCMotor *Adafruit_MotorShield::getMotor(uint8_t num) {
  if (num > 4)
    return NULL;

  num--;

  if (dcmotors[num].motornum == 0) {
    // not init'd yet!
    dcmotors[num].motornum = num;
    dcmotors[num].MC = this;
    uint8_t pwm, in1, in2;
    switch (num) {
    case 0:
      pwm = 8;
      in2 = 9;
      in1 = 10;
      break;
    case 1:
      pwm = 13;
      in2 = 12;
      in1 = 11;
      break;
    case 2:
      pwm = 2;
      in2 = 3;
      in1 = 4;
      break;
    default:
      pwm = 7;
      in2 = 6;
      in1 = 5;
      break;
    }
    dcmotors[num].PWMpin = pwm;
    dcmotors[num].IN1pin = in1;
    dcmotors[num].IN2pin = in2;
  }
  return &dcmotors[num];
}

/**************************************************************************/
/*!
    @brief  Mini factory that will return a pointer to an already-allocated
    Adafruit_StepperMotor object with a given 'steps per rotation.
    Then initializes the stepper motor and turns off all pins.
    @param  steps How many steps per revolution (used for RPM calculation)
    @param  num The stepper motor port we want to use: only 1 or 2 are valid
    @returns NULL if something went wrong, or a pointer to a
   Adafruit_StepperMotor
*/
/**************************************************************************/
Adafruit_StepperMotor *Adafruit_MotorShield::getStepper(uint16_t steps,
                                                        uint8_t num) {
  if (num > 2)
    return NULL;

  num--;

  if (steppers[num].steppernum == 0) {
    // not init'd yet!
    steppers[num].steppernum = num;
    steppers[num].revsteps = steps;
    steppers[num].MC = this;
    uint8_t pwma, pwmb, ain1, ain2, bin1, bin2;
    if (num == 0) {
      pwma = 8;
      ain2 = 9;
      ain1 = 10;
      pwmb = 13;
      bin2 = 12;
      bin1 = 11;
    } else {
      pwma = 2;
      ain2 = 3;
      ain1 = 4;
      pwmb = 7;
      bin2 = 6;
      bin1 = 5;
    }
    steppers[num].PWMApin = pwma;
    steppers[num].PWMBpin = pwmb;
    steppers[num].AIN1pin = ain1;
    steppers[num].AIN2pin = ain2;
    steppers[num].BIN1pin = bin1;
    steppers[num].BIN2pin = bin2;
  }
  return &steppers[num];
}

/******************************************
               MOTORS
******************************************/

/**************************************************************************/
/*!
    @brief  Create a DCMotor object, un-initialized!
    You should never call this, instead have the {@link Adafruit_MotorShield}
    give you a DCMotor object with {@link Adafruit_MotorShield.getMotor}
*/
/**************************************************************************/
Adafruit_DCMotor::Adafruit_DCMotor(void) {
  MC = NULL;
  motornum = 0;
  PWMpin = IN1pin = IN2pin = 0;
}

/**************************************************************************/
/*!
    @brief  Control the DC Motor direction and action
    @param  cmd The action to perform, can be FORWARD, BACKWARD or RELEASE
*/
/**************************************************************************/
void Adafruit_DCMotor::run(uint8_t cmd) {
  switch (cmd) {
  case FORWARD:
    MC->setPin(IN2pin, LOW); // take low first to avoid 'break'
    MC->setPin(IN1pin, HIGH);
    break;
  case BACKWARD:
    MC->setPin(IN1pin, LOW); // take low first to avoid 'break'
    MC->setPin(IN2pin, HIGH);
    break;
  case RELEASE:
    MC->setPin(IN1pin, LOW);
    MC->setPin(IN2pin, LOW);
    break;
  }
}

/**************************************************************************/
/*!
    @brief  Control the DC Motor speed/throttle
    @param  speed The 8-bit PWM value, 0 is off, 255 is on
*/
/**************************************************************************/
void Adafruit_DCMotor::setSpeed(uint8_t speed) {
  MC->setPWM(PWMpin, speed * 16);
}

/**************************************************************************/
/*!
    @brief  Control the DC Motor speed/throttle with 12 bit resolution.
    @param  speed The 12-bit PWM value, 0 (full off) to 4095 (full on)
*/
/**************************************************************************/
void Adafruit_DCMotor::setSpeedFine(uint16_t speed) {
  MC->setPWM(PWMpin, speed > 4095 ? 4095 : speed);
}

/**************************************************************************/
/*!
    @brief  Set DC motor to full on.
*/
/**************************************************************************/
void Adafruit_DCMotor::fullOn() { MC->_pwm.setPWM(PWMpin, 4096, 0); }

/**************************************************************************/
/*!
    @brief  Set DC motor to full off.
*/
/**************************************************************************/
void Adafruit_DCMotor::fullOff() { MC->_pwm.setPWM(PWMpin, 0, 4096); }

/******************************************
               STEPPERS
******************************************/

/**************************************************************************/
/*!
    @brief  Create a StepperMotor object, un-initialized!
    You should never call this, instead have the {@link Adafruit_MotorShield}
    give you a StepperMotor object with {@link Adafruit_MotorShield.getStepper}
*/
/**************************************************************************/
Adafruit_StepperMotor::Adafruit_StepperMotor(void) {
  revsteps = steppernum = currentstep = 0;
}

/**************************************************************************/
/*!
    @brief  Set the delay for the Stepper Motor speed in RPM
    @param  rpm The desired RPM, we will do our best to reach it!
*/
/**************************************************************************/
void Adafruit_StepperMotor::setSpeed(uint16_t rpm) {
  // Serial.println("steps per rev: "); Serial.println(revsteps);
  // Serial.println("RPM: "); Serial.println(rpm);

  usperstep = 60000000 / ((uint32_t)revsteps * (uint32_t)rpm);
}

/**************************************************************************/
/*!
    @brief  Release all pins of the stepper motor so it free-spins
*/
/**************************************************************************/
void Adafruit_StepperMotor::release(void) {
  MC->setPin(AIN1pin, LOW);
  MC->setPin(AIN2pin, LOW);
  MC->setPin(BIN1pin, LOW);
  MC->setPin(BIN2pin, LOW);
  MC->setPWM(PWMApin, 0);
  MC->setPWM(PWMBpin, 0);
}

/**************************************************************************/
/*!
    @brief  Move the stepper motor with the given RPM speed, don't forget to
   call
    {@link Adafruit_StepperMotor.setSpeed} to set the speed!
    @param  steps The number of steps we want to move
    @param  dir The direction to go, can be FORWARD or BACKWARD
    @param  style How to perform each step, can be SINGLE, DOUBLE, INTERLEAVE or
   MICROSTEP
*/
/**************************************************************************/
void Adafruit_StepperMotor::step(uint16_t steps, uint8_t dir, uint8_t style) {
  uint32_t uspers = usperstep;

  if (style == INTERLEAVE) {
    uspers /= 2;
  } else if (style == MICROSTEP) {
    uspers /= MICROSTEPS;
    steps *= MICROSTEPS;
#ifdef MOTORDEBUG
    Serial.print("steps = ");
    Serial.println(steps, DEC);
#endif
  }

  while (steps--) {
    // Serial.println("step!"); Serial.println(uspers);
    onestep(dir, style);
    delayMicroseconds(uspers);
#ifdef ESP8266
    yield(); // required for ESP8266
#endif
  }
}

/**************************************************************************/
/*!
    @brief  Move the stepper motor one step only, with no delays
    @param  dir The direction to go, can be FORWARD or BACKWARD
    @param  style How to perform each step, can be SINGLE, DOUBLE, INTERLEAVE or
   MICROSTEP
    @returns The current step/microstep index, useful for
   Adafruit_StepperMotor.step to keep track of the current location, especially
   when microstepping
*/
/**************************************************************************/
uint8_t Adafruit_StepperMotor::onestep(uint8_t dir, uint8_t style) {
  uint8_t ocrb, ocra;

  ocra = ocrb = 255;

  // next determine what sort of stepping procedure we're up to
  if (style == SINGLE) {
    if ((currentstep / (MICROSTEPS / 2)) % 2) { // we're at an odd step, weird
      if (dir == FORWARD) {
        currentstep += MICROSTEPS / 2;
      } else {
        currentstep -= MICROSTEPS / 2;
      }
    } else { // go to the next even step
      if (dir == FORWARD) {
        currentstep += MICROSTEPS;
      } else {
        currentstep -= MICROSTEPS;
      }
    }
  } else if (style == DOUBLE) {
    if (!(currentstep / (MICROSTEPS / 2) % 2)) { // we're at an even step, weird
      if (dir == FORWARD) {
        currentstep += MICROSTEPS / 2;
      } else {
        currentstep -= MICROSTEPS / 2;
      }
    } else { // go to the next odd step
      if (dir == FORWARD) {
        currentstep += MICROSTEPS;
      } else {
        currentstep -= MICROSTEPS;
      }
    }
  } else if (style == INTERLEAVE) {
    if (dir == FORWARD) {
      currentstep += MICROSTEPS / 2;
    } else {
      currentstep -= MICROSTEPS / 2;
    }
  }

  if (style == MICROSTEP) {
    if (dir == FORWARD) {
      currentstep++;
    } else {
      // BACKWARDS
      currentstep--;
    }

    currentstep += MICROSTEPS * 4;
    currentstep %= MICROSTEPS * 4;

    ocra = ocrb = 0;
    if (currentstep < MICROSTEPS) {
      ocra = microstepcurve[MICROSTEPS - currentstep];
      ocrb = microstepcurve[currentstep];
    } else if ((currentstep >= MICROSTEPS) && (currentstep < MICROSTEPS * 2)) {
      ocra = microstepcurve[currentstep - MICROSTEPS];
      ocrb = microstepcurve[MICROSTEPS * 2 - currentstep];
    } else if ((currentstep >= MICROSTEPS * 2) &&
               (currentstep < MICROSTEPS * 3)) {
      ocra = microstepcurve[MICROSTEPS * 3 - currentstep];
      ocrb = microstepcurve[currentstep - MICROSTEPS * 2];
    } else if ((currentstep >= MICROSTEPS * 3) &&
               (currentstep < MICROSTEPS * 4)) {
      ocra = microstepcurve[currentstep - MICROSTEPS * 3];
      ocrb = microstepcurve[MICROSTEPS * 4 - currentstep];
    }
  }

  currentstep += MICROSTEPS * 4;
  currentstep %= MICROSTEPS * 4;

#ifdef MOTORDEBUG
  Serial.print("current step: ");
  Serial.println(currentstep, DEC);
  Serial.print(" pwmA = ");
  Serial.print(ocra, DEC);
  Serial.print(" pwmB = ");
  Serial.println(ocrb, DEC);
#endif
  MC->setPWM(PWMApin, ocra * 16);
  MC->setPWM(PWMBpin, ocrb * 16);

  // release all
  uint8_t latch_state = 0; // all motor pins to 0

  // Serial.println(step, DEC);
  if (style == MICROSTEP) {
    if (currentstep < MICROSTEPS)
      latch_state |= 0x03;
    if ((currentstep >= MICROSTEPS) && (currentstep < MICROSTEPS * 2))
      latch_state |= 0x06;
    if ((currentstep >= MICROSTEPS * 2) && (currentstep < MICROSTEPS * 3))
      latch_state |= 0x0C;
    if ((currentstep >= MICROSTEPS * 3) && (currentstep < MICROSTEPS * 4))
      latch_state |= 0x09;
  } else {
    switch (currentstep / (MICROSTEPS / 2)) {
    case 0:
      latch_state |= 0x1; // energize coil 1 only
      break;
    case 1:
      latch_state |= 0x3; // energize coil 1+2
      break;
    case 2:
      latch_state |= 0x2; // energize coil 2 only
      break;
    case 3:
      latch_state |= 0x6; // energize coil 2+3
      break;
    case 4:
      latch_state |= 0x4; // energize coil 3 only
      break;
    case 5:
      latch_state |= 0xC; // energize coil 3+4
      break;
    case 6:
      latch_state |= 0x8; // energize coil 4 only
      break;
    case 7:
      latch_state |= 0x9; // energize coil 1+4
      break;
    }
  }
#ifdef MOTORDEBUG
  Serial.print("Latch: 0x");
  Serial.println(latch_state, HEX);
#endif

  if (latch_state & 0x1) {
    // Serial.println(AIN2pin);
    MC->setPin(AIN2pin, HIGH);
  } else {
    MC->setPin(AIN2pin, LOW);
  }
  if (latch_state & 0x2) {
    MC->setPin(BIN1pin, HIGH);
    // Serial.println(BIN1pin);
  } else {
    MC->setPin(BIN1pin, LOW);
  }
  if (latch_state & 0x4) {
    MC->setPin(AIN1pin, HIGH);
    // Serial.println(AIN1pin);
  } else {
    MC->setPin(AIN1pin, LOW);
  }
  if (latch_state & 0x8) {
    MC->setPin(BIN2pin, HIGH);
    // Serial.println(BIN2pin);
  } else {
    MC->setPin(BIN2pin, LOW);
  }

  return currentstep;
}
