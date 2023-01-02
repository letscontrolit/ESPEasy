#include "seesaw_spectrum.h"

/**************************************************************************/
/*!
  @brief  Pull latest audio spectrum data from device.
*/
/**************************************************************************/
void seesaw_Audio_Spectrum::getData(void) {
  read(SEESAW_SPECTRUM_BASE, SEESAW_SPECTRUM_RESULTS_LOWER, bins, 32, 0);
  read(SEESAW_SPECTRUM_BASE, SEESAW_SPECTRUM_RESULTS_UPPER, &bins[32], 32, 0);
}

/**************************************************************************/
/*!
  @brief  Set the audio sampling rate.
  @param  value  Sampling rate index, 0-31. Values outside this range
                 will be clipped on the Seesaw device side.
*/
/**************************************************************************/
void seesaw_Audio_Spectrum::setRate(uint8_t value) {
  write8(SEESAW_SPECTRUM_BASE, SEESAW_SPECTRUM_RATE, value);
}

/**************************************************************************/
/*!
  @brief  Set the analog input channel.
  @param  value  Channel index, 0-TBD (probably 1). Values outside the
                 valid range will be clipped on the Seesaw device side.
*/
/**************************************************************************/
void seesaw_Audio_Spectrum::setChannel(uint8_t value) {
  write8(SEESAW_SPECTRUM_BASE, SEESAW_SPECTRUM_CHANNEL, value);
}

/**************************************************************************/
/*!
  @brief   Query the current audio sampling rate.
  @return  Sampling rate index, 0-31.
*/
/**************************************************************************/
uint8_t seesaw_Audio_Spectrum::getRate(void) {
  return read8(SEESAW_SPECTRUM_BASE, SEESAW_SPECTRUM_RATE);
}

/**************************************************************************/
/*!
  @brief   Query the current analog input channel.
  @return  Active ADC channel, 0-TBD (probably 1).
*/
/**************************************************************************/
uint8_t seesaw_Audio_Spectrum::getChannel(void) {
  return read8(SEESAW_SPECTRUM_BASE, SEESAW_SPECTRUM_CHANNEL);
}
