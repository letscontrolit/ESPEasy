#ifdef USES_P032
//#######################################################################################################
//################ Plugin 032 MS5611 (GY-63) I2C Temp/Barometric Pressure Sensor  #######################
//#######################################################################################################
// This sketch is based on https://github.com/Schm1tz1/arduino-ms5xxx

#define PLUGIN_032
#define PLUGIN_ID_032        32
#define PLUGIN_NAME_032       "Environment - MS5611 (GY-63)"
#define PLUGIN_VALUENAME1_032 "Temperature"
#define PLUGIN_VALUENAME2_032 "Pressure"

enum
{
  MS5xxx_CMD_RESET                    = 0x1E,    // perform reset
  MS5xxx_CMD_ADC_READ                 = 0x00,    // initiate read sequence
  MS5xxx_CMD_ADC_CONV                 = 0x40,    // start conversion
  MS5xxx_CMD_ADC_D1                   = 0x00,    // read ADC 1
  MS5xxx_CMD_ADC_D2                   = 0x10,    // read ADC 2
  MS5xxx_CMD_ADC_256                  = 0x00,    // set ADC oversampling ratio to 256
  MS5xxx_CMD_ADC_512                  = 0x02,    // set ADC oversampling ratio to 512
  MS5xxx_CMD_ADC_1024                 = 0x04,    // set ADC oversampling ratio to 1024
  MS5xxx_CMD_ADC_2048                 = 0x06,    // set ADC oversampling ratio to 2048
  MS5xxx_CMD_ADC_4096                 = 0x08,    // set ADC oversampling ratio to 4096
  MS5xxx_CMD_PROM_RD                  = 0xA0     // initiate readout of PROM registers
};



uint8_t ms5611_i2caddr;
unsigned int ms5611_prom[8];
double  ms5611_pressure;
double  ms5611_temperature;

boolean Plugin_032_init = false;

boolean Plugin_032(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_032;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_TEMP_BARO;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_032);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_032));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_032));
        break;
      }
    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        /*String options[2] = { F("0x77 - default I2C address"), F("0x76 - alternate I2C address") };*/
        int optionValues[2] = { 0x77, 0x76 };
        addFormSelectorI2C(F("p032_ms5611_i2c"), 2, optionValues, choice);

        addFormNumericBox(F("Altitude [m]"), F("p032_ms5611_elev"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("p032_ms5611_i2c"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("p032_ms5611_elev"));
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        if (!Plugin_032_init)
        {
          Plugin_032_init = Plugin_032_begin(Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
        }

        if (Plugin_032_init) {
          Plugin_032_read_prom();
          Plugin_032_readout();

          UserVar[event->BaseVarIndex] = ms5611_temperature / 100;
          int elev = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
          if (elev)
          {
             UserVar[event->BaseVarIndex + 1] = Plugin_032_pressureElevation(ms5611_pressure, elev);
          } else {
             UserVar[event->BaseVarIndex + 1] = ms5611_pressure;
          }

          String log = F("MS5611  : Temperature: ");
          log += UserVar[event->BaseVarIndex];
          addLog(LOG_LEVEL_INFO, log);
          log = F("MS5611  : Barometric Pressure: ");
          log += UserVar[event->BaseVarIndex + 1];
          addLog(LOG_LEVEL_INFO, log);
          success = true;
        }
        break;
      }

  }
  return success;
}

//**************************************************************************/
// Initialize MS5611
//**************************************************************************/
bool Plugin_032_begin(uint8_t a) {
  ms5611_i2caddr = a;

  Wire.beginTransmission((uint8_t)ms5611_i2caddr);
  uint8_t ret=Wire.endTransmission(true);
  return ret==0;
}

//**************************************************************************/
// Reads the PROM of MS5611
// There are in total 8 addresses resulting in a total memory of 128 bit.
// Address 0 contains factory data and the setup, addresses 1-6 calibration
// coefficients and address 7 contains the serial code and CRC.
// The command sequence is 8 bits long with a 16 bit result which is
// clocked with the MSB first.
//**************************************************************************/
void Plugin_032_read_prom() {
  I2C_write8(ms5611_i2caddr, MS5xxx_CMD_RESET);
  delay(3);

  for(uint8_t i=0;i<8;i++)
  {
      ms5611_prom[i] = I2C_read16_reg(ms5611_i2caddr, MS5xxx_CMD_PROM_RD+2*i);
  }
}

//**************************************************************************/
// Read analog/digital converter
//**************************************************************************/
unsigned long Plugin_032_read_adc(unsigned char aCMD)
{
  I2C_write8(ms5611_i2caddr, MS5xxx_CMD_ADC_CONV+aCMD); // start DAQ and conversion of ADC data
  switch (aCMD & 0x0f)
  {
    case MS5xxx_CMD_ADC_256 : delayMicroseconds(900);
    break;
    case MS5xxx_CMD_ADC_512 : delay(3);
    break;
    case MS5xxx_CMD_ADC_1024: delay(4);
    break;
    case MS5xxx_CMD_ADC_2048: delay(6);
    break;
    case MS5xxx_CMD_ADC_4096: delay(10);
    break;
  }
  // read out values
  return I2C_read24_reg(ms5611_i2caddr, MS5xxx_CMD_ADC_READ);
}


//**************************************************************************/
// Readout
//**************************************************************************/
void Plugin_032_readout() {

  unsigned long D1=0, D2=0;

  double dT;
  double OFF;
  double SENS;

  D2=Plugin_032_read_adc(MS5xxx_CMD_ADC_D2+MS5xxx_CMD_ADC_4096);
  D1=Plugin_032_read_adc(MS5xxx_CMD_ADC_D1+MS5xxx_CMD_ADC_4096);

  // calculate 1st order pressure and temperature (MS5611 1st order algorithm)
  dT=D2-ms5611_prom[5]*pow(2,8);
  OFF=ms5611_prom[2]*pow(2,16)+dT*ms5611_prom[4]/pow(2,7);
  SENS=ms5611_prom[1]*pow(2,15)+dT*ms5611_prom[3]/pow(2,8);
  ms5611_temperature=(2000+(dT*ms5611_prom[6])/pow(2,23));
  ms5611_pressure=(((D1*SENS)/pow(2,21)-OFF)/pow(2,15));

  // perform higher order corrections
  double T2=0., OFF2=0., SENS2=0.;
  if(ms5611_temperature<2000) {
    T2=dT*dT/pow(2,31);
    OFF2=5*(ms5611_temperature-2000)*(ms5611_temperature-2000)/pow(2,1);
    SENS2=5*(ms5611_temperature-2000)*(ms5611_temperature-2000)/pow(2,2);
    if(ms5611_temperature<-1500) {
      OFF2+=7*(ms5611_temperature+1500)*(ms5611_temperature+1500);
      SENS2+=11*(ms5611_temperature+1500)*(ms5611_temperature+1500)/pow(2,1);
    }
  }

  ms5611_temperature-=T2;
  OFF-=OFF2;
  SENS-=SENS2;
  ms5611_pressure=(((D1*SENS)/pow(2,21)-OFF)/pow(2,15));
}

//**************************************************************************/
// MSL pressure formula
//**************************************************************************/
double Plugin_032_pressureElevation(double atmospheric, int altitude) {
  return atmospheric / pow(1.0 - (altitude/44330.0), 5.255);
}
#endif // USES_P032
