#include "P032_data_struct.h"

#ifdef USES_P032

#include "../Globals/I2Cdev.h"


enum
{
  MS5xxx_CMD_RESET    = 0x1E, // perform reset
  MS5xxx_CMD_ADC_READ = 0x00, // initiate read sequence
  MS5xxx_CMD_ADC_CONV = 0x40, // start conversion
  MS5xxx_CMD_ADC_D1   = 0x00, // read ADC 1
  MS5xxx_CMD_ADC_D2   = 0x10, // read ADC 2
  MS5xxx_CMD_ADC_256  = 0x00, // set ADC oversampling ratio to 256
  MS5xxx_CMD_ADC_512  = 0x02, // set ADC oversampling ratio to 512
  MS5xxx_CMD_ADC_1024 = 0x04, // set ADC oversampling ratio to 1024
  MS5xxx_CMD_ADC_2048 = 0x06, // set ADC oversampling ratio to 2048
  MS5xxx_CMD_ADC_4096 = 0x08, // set ADC oversampling ratio to 4096
  MS5xxx_CMD_PROM_RD  = 0xA0  // initiate readout of PROM registers
};


P032_data_struct::P032_data_struct(uint8_t i2c_addr) : i2cAddress(i2c_addr) {}


// **************************************************************************/
// Initialize MS5611
// **************************************************************************/
bool P032_data_struct::begin() {
  Wire.beginTransmission(i2cAddress);
  uint8_t ret = Wire.endTransmission(true);

  return ret == 0;
}

// **************************************************************************/
// Reads the PROM of MS5611
// There are in total 8 addresses resulting in a total memory of 128 bit.
// Address 0 contains factory data and the setup, addresses 1-6 calibration
// coefficients and address 7 contains the serial code and CRC.
// The command sequence is 8 bits long with a 16 bit result which is
// clocked with the MSB first.
// **************************************************************************/
void P032_data_struct::read_prom() {
  I2C_write8(i2cAddress, MS5xxx_CMD_RESET);
  delay(3);

  for (uint8_t i = 0; i < 8; i++)
  {
    ms5611_prom[i] = I2C_read16_reg(i2cAddress, MS5xxx_CMD_PROM_RD + 2 * i);
  }
}

// **************************************************************************/
// Read analog/digital converter
// **************************************************************************/
unsigned long P032_data_struct::read_adc(unsigned char aCMD)
{
  I2C_write8(i2cAddress, MS5xxx_CMD_ADC_CONV + aCMD); // start DAQ and conversion of ADC data

  switch (aCMD & 0x0f)
  {
    case MS5xxx_CMD_ADC_256: delayMicroseconds(900);
      break;
    case MS5xxx_CMD_ADC_512: delay(3);
      break;
    case MS5xxx_CMD_ADC_1024: delay(4);
      break;
    case MS5xxx_CMD_ADC_2048: delay(6);
      break;
    case MS5xxx_CMD_ADC_4096: delay(10);
      break;
  }

  // read out values
  return I2C_read24_reg(i2cAddress, MS5xxx_CMD_ADC_READ);
}

// **************************************************************************/
// Readout
// **************************************************************************/
void P032_data_struct::readout() {
  unsigned long D1 = 0, D2 = 0;

  double dT;
  double OFF;
  double SENS;

  D2 = read_adc(MS5xxx_CMD_ADC_D2 + MS5xxx_CMD_ADC_4096);
  D1 = read_adc(MS5xxx_CMD_ADC_D1 + MS5xxx_CMD_ADC_4096);

  // calculate 1st order pressure and temperature (MS5611 1st order algorithm)
  dT                 = D2 - ms5611_prom[5] * pow(2, 8);
  OFF                = ms5611_prom[2] * pow(2, 16) + dT * ms5611_prom[4] / pow(2, 7);
  SENS               = ms5611_prom[1] * pow(2, 15) + dT * ms5611_prom[3] / pow(2, 8);
  ms5611_temperature = (2000 + (dT * ms5611_prom[6]) / pow(2, 23));
  ms5611_pressure    = (((D1 * SENS) / pow(2, 21) - OFF) / pow(2, 15));

  // perform higher order corrections
  double T2 = 0., OFF2 = 0., SENS2 = 0.;

  if (ms5611_temperature < 2000) {
    T2    = dT * dT / pow(2, 31);
    OFF2  = 5 * (ms5611_temperature - 2000) * (ms5611_temperature - 2000) / pow(2, 1);
    SENS2 = 5 * (ms5611_temperature - 2000) * (ms5611_temperature - 2000) / pow(2, 2);

    if (ms5611_temperature < -1500) {
      OFF2  += 7 * (ms5611_temperature + 1500) * (ms5611_temperature + 1500);
      SENS2 += 11 * (ms5611_temperature + 1500) * (ms5611_temperature + 1500) / pow(2, 1);
    }
  }

  ms5611_temperature -= T2;
  OFF                -= OFF2;
  SENS               -= SENS2;
  ms5611_pressure     = (((D1 * SENS) / pow(2, 21) - OFF) / pow(2, 15)); // FIXME TD-er: This is computed twice, is that correct?
}

// **************************************************************************/
// MSL pressure formula
// **************************************************************************/
double P032_data_struct::pressureElevation(double atmospheric, int altitude) {
  return atmospheric / pow(1.0f - (altitude / 44330.0f), 5.255f);
}

#endif // ifdef USES_P032
