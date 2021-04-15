/**************************************************************************/
/*!
    @file     Adafruit_TCS34725.cpp
    @author   KTOWN (Adafruit Industries)
    @license  BSD (see license.txt)

    Driver for the TCS34725 digital color sensors.

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section  HISTORY

    v1.0 - First release
*/
/**************************************************************************/
#ifdef __AVR
  #include <avr/pgmspace.h>
#elif defined(ESP8266)
  #include <pgmspace.h>
#endif
#include <stdlib.h>
#include <math.h>

#include "Adafruit_TCS34725.h"

/*========================================================================*/
/*                          PRIVATE FUNCTIONS                             */
/*========================================================================*/

/**************************************************************************/
/*!
 *  @brief  Implements missing powf function
 *  @param  x
 *          Base number
 *  @param  y
 *          Exponent
 *  @return x raised to the power of y
*/
/**************************************************************************/
float powf(const float x, const float y)
{
  return (float)(pow((double)x, (double)y));
}

/**************************************************************************/
/*!
 *  @brief  Writes a register and an 8 bit value over I2C
 *  @param  reg
 *  @param  value
*/
/**************************************************************************/
void Adafruit_TCS34725::write8 (uint8_t reg, uint32_t value)
{
  Wire.beginTransmission(TCS34725_ADDRESS);
  #if ARDUINO >= 100
  Wire.write(TCS34725_COMMAND_BIT | reg);
  Wire.write(value & 0xFF);
  #else
  Wire.send(TCS34725_COMMAND_BIT | reg);
  Wire.send(value & 0xFF);
  #endif
  Wire.endTransmission();
}

/**************************************************************************/
/*!
    @brief  Reads an 8 bit value over I2C
*/
/**************************************************************************/
uint8_t Adafruit_TCS34725::read8(uint8_t reg)
{
  Wire.beginTransmission(TCS34725_ADDRESS);
  #if ARDUINO >= 100
  Wire.write(TCS34725_COMMAND_BIT | reg);
  #else
  Wire.send(TCS34725_COMMAND_BIT | reg);
  #endif
  Wire.endTransmission();

  Wire.requestFrom(TCS34725_ADDRESS, 1);
  #if ARDUINO >= 100
  return Wire.read();
  #else
  return Wire.receive();
  #endif
}

/**************************************************************************/
/*!
    @brief  Reads a 16 bit values over I2C
*/
/**************************************************************************/
uint16_t Adafruit_TCS34725::read16(uint8_t reg)
{
  uint16_t x; uint16_t t;

  Wire.beginTransmission(TCS34725_ADDRESS);
  #if ARDUINO >= 100
  Wire.write(TCS34725_COMMAND_BIT | reg);
  #else
  Wire.send(TCS34725_COMMAND_BIT | reg);
  #endif
  Wire.endTransmission();

  Wire.requestFrom(TCS34725_ADDRESS, 2);
  #if ARDUINO >= 100
  t = Wire.read();
  x = Wire.read();
  #else
  t = Wire.receive();
  x = Wire.receive();
  #endif
  x <<= 8;
  x |= t;
  return x;
}

/**************************************************************************/
/*!
    Enables the device
*/
/**************************************************************************/
void Adafruit_TCS34725::enable(void)
{
  write8(TCS34725_ENABLE, TCS34725_ENABLE_PON);
  delay(3);
  write8(TCS34725_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
}

/*!
 *  @brief  Briefly disable integration to activate new settings
 */
void Adafruit_TCS34725::reset() {
  uint8_t reg = 0;
  reg = read8(TCS34725_ENABLE);
  write8(TCS34725_ENABLE, reg & ~(TCS34725_ENABLE_AEN));
  delay(3);
  write8(TCS34725_ENABLE, reg);
}

/**************************************************************************/
/*!
    Disables the device (putting it in lower power sleep mode)
*/
/**************************************************************************/
void Adafruit_TCS34725::disable(void)
{
  /* Turn the device off to save power */
  uint8_t reg = 0;
  reg = read8(TCS34725_ENABLE);
  write8(TCS34725_ENABLE, reg & ~(TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN));
}

/*========================================================================*/
/*                            CONSTRUCTORS                                */
/*========================================================================*/

/**************************************************************************/
/*!
    Constructor
*/
/**************************************************************************/
Adafruit_TCS34725::Adafruit_TCS34725(tcs34725IntegrationTime_t it, tcs34725Gain_t gain)
{
  _tcs34725Initialised = false;
  _tcs34725IntegrationTime = it;
  _tcs34725Gain = gain;
}

/*========================================================================*/
/*                           PUBLIC FUNCTIONS                             */
/*========================================================================*/

/**************************************************************************/
/*!
    Initializes I2C and configures the sensor (call this function before
    doing anything else)
*/
/**************************************************************************/
boolean Adafruit_TCS34725::begin(void)
{
  //Wire.begin();   called in ESPEasy framework

  /* Make sure we're actually connected */
  uint8_t x = read8(TCS34725_ID);
  if ((x != 0x44) && (x != 0x10))
  {
    return false;
  }
  _tcs34725Initialised = true;

  /* Set default integration time and gain */
  setIntegrationTime(_tcs34725IntegrationTime);
  setGain(_tcs34725Gain);

  /* Note: by default, the device is in power down mode on bootup */
  enable();

  return true;
}

/*!
 *  @brief  Sets the integration time for the TC34725
 *  @param  it_msec
 *          Desired integration time in milliseconds
 *  @return Actual integration time in milliseconds
 */
float Adafruit_TCS34725::setIntegrationTimeMsec(float it_msec) {
  if (!_tcs34725Initialised)
    begin();

  uint8_t atime = calculateIntegrationConstant(it_msec);
  setIntegrationTime(atime);
  float actual_it_msec = calculateIntegrationTime(atime);
  return actual_it_msec;
}

/**************************************************************************/
/*!
 *  @brief  Sets the integration time for the TC34725
 *  @param  it
 *          Integration time constant (0-255; see ATIME in the datasheet)
*/
/**************************************************************************/
void Adafruit_TCS34725::setIntegrationTime(uint8_t it)
{
  if (!_tcs34725Initialised) begin();

  /* Update the timing register */
  write8(TCS34725_ATIME, it);

  /* Restart integration so we don't have to wait for the previous
   * integration to finish.
   */
  reset();

  /* Update value placeholders */
  _tcs34725IntegrationTime = it;
}

/*!
 *  @brief  (deprecated) Sets the integration time for the TC34725
 *  @param  it
 *          Integration Time
 */
void Adafruit_TCS34725::setIntegrationTime(tcs34725IntegrationTime_t it) {
    setIntegrationTime((uint8_t)it);
}

/**************************************************************************/
/*!
 *  @brief  Adjusts the gain on the TCS34725
 *  @param  gain
 *          Gain (sensitivity to light)
*/
/**************************************************************************/
void Adafruit_TCS34725::setGain(tcs34725Gain_t gain)
{
  if (!_tcs34725Initialised) begin();

  /* Update the timing register */
  write8(TCS34725_CONTROL, gain);

  /* Restart integration so we don't have to wait for the previous
   * integration to finish.
   */
  reset();

  /* Update value placeholders */
  _tcs34725Gain = gain;
}

/**************************************************************************/
/*!
 *  @brief  Reads the raw red, green, blue and clear channel values immediately
 *  @param  *r
 *          Red value
 *  @param  *g
 *          Green value
 *  @param  *b
 *          Blue value
 *  @param  *c
 *          Clear channel value
 *  @param  wait
 *          If true, delay before reading to ensure integration has completed
*/
/**************************************************************************/
void Adafruit_TCS34725::getRawData (uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c, bool wait)
{
  if (!_tcs34725Initialised) begin();

  if( wait ) {
    float int_time_ms = calculateIntegrationTime(_tcs34725IntegrationTime);
    delay(int_time_ms + 4);
  }

  *c = read16(TCS34725_CDATAL);
  *r = read16(TCS34725_RDATAL);
  *g = read16(TCS34725_GDATAL);
  *b = read16(TCS34725_BDATAL);

}

/*!
 *  @brief  Returns the maximum value that could be reported by getRawData,
 *          given the currently set integration time
 */
uint16_t Adafruit_TCS34725::getRawDataMax() {
    long mx = ((long)256 - (long)_tcs34725IntegrationTime) * 1024;
    mx = mx > 65535 ? 65535 : mx;
    return (uint16_t) mx;
}

/**************************************************************************/
/*!
 *  @brief  Converts the raw R/G/B values to color temperature in degrees Kelvin
 *  @param  r
 *          Red value
 *  @param  g
 *          Green value
 *  @param  b
 *          Blue value
 *  @return Color temperature in degrees Kelvin

*/
/**************************************************************************/
uint16_t Adafruit_TCS34725::calculateColorTemperature(uint16_t r, uint16_t g, uint16_t b)
{
  float X, Y, Z;      /* RGB to XYZ correlation      */
  float xc, yc;       /* Chromaticity co-ordinates   */
  float n;            /* McCamy's formula            */
  float cct;

  if (r == 0 && g == 0 && b == 0) {
    return 0;
  }

  /* 1. Map RGB values to their XYZ counterparts.    */
  /* Based on 6500K fluorescent, 3000K fluorescent   */
  /* and 60W incandescent values for a wide range.   */
  /* Note: Y = Illuminance or lux                    */
  X = (-0.14282F * r) + (1.54924F * g) + (-0.95641F * b);
  Y = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);
  Z = (-0.68202F * r) + (0.77073F * g) + ( 0.56332F * b);

  /* 2. Calculate the chromaticity co-ordinates      */
  xc = (X) / (X + Y + Z);
  yc = (Y) / (X + Y + Z);

  /* 3. Use McCamy's formula to determine the CCT    */
  n = (xc - 0.3320F) / (0.1858F - yc);

  /* Calculate the final CCT */
  cct = (449.0F * powf(n, 3)) + (3525.0F * powf(n, 2)) + (6823.3F * n) + 5520.33F;

  /* Return the results in degrees Kelvin */
  return (uint16_t)cct;
}

/*!
 *  @brief  Converts the raw R/G/B values to color temperature in degrees
 *          Kelvin using the algorithm described in DN40 from Taos (now AMS).
 *  @param  r
 *          Red value
 *  @param  g
 *          Green value
 *  @param  b
 *          Blue value
 *  @param  c
 *          Clear channel value
 *  @return Color temperature in degrees Kelvin
 */
uint16_t Adafruit_TCS34725::calculateColorTemperature_dn40(uint16_t r,
                                                           uint16_t g,
                                                           uint16_t b,
                                                           uint16_t c) {
  uint16_t r2, b2; /* RGB values minus IR component */
  uint16_t sat;    /* Digital saturation level */
  uint16_t ir;     /* Inferred IR content */

  if (c == 0) {
    return 0;
  }

  /* Analog/Digital saturation:
   *
   * (a) As light becomes brighter, the clear channel will tend to
   *     saturate first since R+G+B is approximately equal to C.
   * (b) The TCS34725 accumulates 1024 counts per 2.4ms of integration
   *     time, up to a maximum values of 65535. This means analog
   *     saturation can occur up to an integration time of 153.6ms
   *     (64*2.4ms=153.6ms).
   * (c) If the integration time is > 153.6ms, digital saturation will
   *     occur before analog saturation. Digital saturation occurs when
   *     the count reaches 65535.
   */
  if ((256 - _tcs34725IntegrationTime) > 63) {
    /* Track digital saturation */
    sat = 65535;
  } else {
    /* Track analog saturation */
    sat = 1024 * (256 - _tcs34725IntegrationTime);
  }

  /* Ripple rejection:
   *
   * (a) An integration time of 50ms or multiples of 50ms are required to
   *     reject both 50Hz and 60Hz ripple.
   * (b) If an integration time faster than 50ms is required, you may need
   *     to average a number of samples over a 50ms period to reject ripple
   *     from fluorescent and incandescent light sources.
   *
   * Ripple saturation notes:
   *
   * (a) If there is ripple in the received signal, the value read from C
   *     will be less than the max, but still have some effects of being
   *     saturated. This means that you can be below the 'sat' value, but
   *     still be saturating. At integration times >150ms this can be
   *     ignored, but <= 150ms you should calculate the 75% saturation
   *     level to avoid this problem.
   */
  if ((256 - _tcs34725IntegrationTime) <= 63) {
    /* Adjust sat to 75% to avoid analog saturation if atime < 153.6ms */
    sat -= sat / 4;
  }

  /* Check for saturation and mark the sample as invalid if true */
  if (c >= sat) {
    return 0;
  }

  /* AMS RGB sensors have no IR channel, so the IR content must be */
  /* calculated indirectly. */
  ir = (r + g + b > c) ? (r + g + b - c) / 2 : 0;

  /* Remove the IR component from the raw RGB values */
  r2 = r - ir;
  b2 = b - ir;

  if (r2 == 0) {
    return 0;
  }

  /* A simple method of measuring color temp is to use the ratio of blue */
  /* to red light, taking IR cancellation into account. */
  uint16_t cct = (3810 * (uint32_t)b2) / /** Color temp coefficient. */
                     (uint32_t)r2 +
                 1391; /** Color temp offset. */

  return cct;
}

/**************************************************************************/
/*!
 *  @brief  Converts the raw R/G/B values to lux
 *  @param  r
 *          Red value
 *  @param  g
 *          Green value
 *  @param  b
 *          Blue value
 *  @return Lux value

*/
/**************************************************************************/
uint16_t Adafruit_TCS34725::calculateLux(uint16_t r, uint16_t g, uint16_t b)
{
  float illuminance;

  /* This only uses RGB ... how can we integrate clear or calculate lux */
  /* based exclusively on clear since this might be more reliable?      */
  illuminance = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);
  if (illuminance < 0) { // patched to avoid spurious 65535 results
    illuminance = 0.0f;
  }

  return (uint16_t)illuminance;
}

/*!
 *  @brief  Calculate the ATIME value (argument to setIntegrationTime) needed to achieve a certain integration time
 *  @param  it_msec
 *          Desired integration time in milliseconds
 *  @return ATIME value
 */
uint8_t Adafruit_TCS34725::calculateIntegrationConstant(float it_msec) {
  int atime = 258.59 - it_msec / 2.475;
  atime = atime < 0 ? 0 : atime;
  atime = atime > 255 ? 255 : atime;
  return (uint8_t) atime;
}

/*!
 *  @brief  Calculate the integration time in seconds corresponding to sn ATIME value
 *  @param  it
 *          ATIME value
 *  @return Integration time in milliseconds
 */
float Adafruit_TCS34725::calculateIntegrationTime(uint8_t atime) {
  /* equation according to datasheet is 2.4 * (256 - atime), but that seems to be inaccurate
   */
  return 2.475 * (258.59 - atime);
}

/*!
 *  @brief  Sets inerrupt for TCS34725
 *  @param  i
 *          Interrupt (True/False)
 */
void Adafruit_TCS34725::setInterrupt(boolean i) {
  uint8_t r = read8(TCS34725_ENABLE);
  if (i) {
    r |= TCS34725_ENABLE_AIEN;
  } else {
    r &= ~TCS34725_ENABLE_AIEN;
  }
  write8(TCS34725_ENABLE, r);
}

/*!
 *  @brief  Clears inerrupt for TCS34725
 */
void Adafruit_TCS34725::clearInterrupt(void) {
  Wire.beginTransmission(TCS34725_ADDRESS);
  #if ARDUINO >= 100
  Wire.write(TCS34725_COMMAND_BIT | 0x66);
  #else
  Wire.send(TCS34725_COMMAND_BIT | 0x66);
  #endif
  Wire.endTransmission();
}

/*!
 *  @brief  Sets inerrupt limits
 *  @param  low
 *          Low limit
 *  @param  high
 *          High limit
 */
void Adafruit_TCS34725::setIntLimits(uint16_t low, uint16_t high) {
   write8(0x04, low & 0xFF);
   write8(0x05, low >> 8);
   write8(0x06, high & 0xFF);
   write8(0x07, high >> 8);
}
