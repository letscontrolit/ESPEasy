// ----------------------------------------------------------------------------
// P145 "Gases - MQxxx (MQ135 CO2, MQ3 Alcohol)"
// Implementation of sensor abstraction
// See _P145_MQxxx.ino
// 2023 By flashmark
// ----------------------------------------------------------------------------
#include "../PluginStructs/P145_data_struct.h"

#ifdef USES_P145

// Enable testing 
//#define P145_TEST
//#define P145_CALIBRATION_INTERVAL (5*60*1000)

#include "../Globals/ESPEasyWiFiEvent.h"   // Need to know when WiFi is ruining the ADC measurements
#include "../Helpers/Hardware.h"           // Need to know the ADC properties

// The table sensorDefs[] contains string items for representation. 
// Storage is in PROGMEM where a (fixed format) C-style string does not fit well
// An enum like int is used instead, together with a enum to string conversion function
// Below the defines used to represent these enums

// Gas type identifiers used to condense the sensorDefs table to fixed format
#define P145_GASUSER    0
#define P145_GASCO2     1
#define P145_GASH2      2
#define P145_GASALCOHOL 3
#define P145_GASCH4     4
#define P145_GASLPG     5
#define P145_GASCO      6

// Sensor type identifier used to condense the SensorDefs table to fixed format
#define P145_SENSUSER   0
#define P145_SENSMQ135  1
#define P145_SENSMQ2    2
#define P145_SENSMQ3    3
#define P145_SENSMQ4    4
#define P145_SENSMQ5    5
#define P145_SENSMQ6    6
#define P145_SENSMQ7    7
#define P145_SENSMQ8    8



/******************************************************************************/
// Table with sensor type specific data
// This table is stored in PROGMEM to save space in RAM
// The data for the selected sensor type is copied to RAM, see P145_data_struct
/******************************************************************************/
const struct P145_SENSORDEF sensorDefs[] PROGMEM =
{
  // User defined, output plain ratio (Rsensor/Rzero)
  {     
      0.0f,         // cleanRatio
      0.0f,         // PARA scaling factor value
      0.0f,         // PARB exponent value
      0.0f,         // CORA
      0.0f,         // CORB
      0.0f,         // CORC
      0.0f,         // CORD
      0.0f,         // CORE
      0.0f,         // CORF
      0.0f,         // CORG
      p145AlgNone,  // preferred/tuned algorithm
      P145_SENSUSER, // Name
      P145_GASUSER, // gas
  },
  // *** MQ-135 - CO2 ***
  {
      0.0f,         // cleanRatio
      116.6020682f, // PARA scaling factor value
      2.769034857f, // PARB exponent value
      0.00035f,     // CORA
      0.02718f,     // CORB
      1.39538f,     // CORC
      0.0018f,      // CORD
      -0.003333333f,// CORE
      -0.001923077f,// CORF
      1.130128205f, // CORG
      p145AlgA,     // preferred/tuned algorithm
      P145_SENSMQ135,  // Name
      P145_GASCO2,  // gas
  },
    // *** MQ-2 - H2 ***
  {
      9.83f,        // cleanRatio
      987.99f,      // PARA scaling factor
      -2.162f,      // PARB exponent value
      0.0f,         // CORA
      0.0f,         // CORB
      0.0f,         // CORC
      0.0f,         // CORD
      0.0f,         // CORE
      0.0f,         // CORF
      0.0f,         // CORG
      p145AlgB,     // preferred/tuned algorithm
      P145_SENSMQ2, // Name
      P145_GASH2,   // gas
  },
  // *** MQ-3 - alcohol ***
  {
      60.0f,        // cleanRatio
      0.3934f,      // PARA scaling factor
      -1.504f,      // PARB exponent value
      0.0f,         // CORA
      0.0f,         // CORB
      0.0f,         // CORC
      0.0f,         // CORD
      0.0f,         // CORE
      0.0f,         // CORF
      0.0f,         // CORG
      p145AlgB,     // preferred/tuned algorithm
      P145_SENSMQ3, // Name
      P145_GASALCOHOL,  // gas
  },
  // *** MQ-4 - CH4 ***
  {
      4.4f,         // cleanRatio
      1012.7f,      // PARA scaling
      -2.786f,      // PARB exponent
      0.0f,         // CORA
      0.0f,         // CORB
      0.0f,         // CORC
      0.0f,         // CORD
      0.0f,         // CORE
      0.0f,         // CORF
      0.0f,         // CORG
      p145AlgB,     // preferred/tuned algorithm
      P145_SENSMQ4, // Name
      P145_GASCH4,  // gas
  },
    // *** MQ-5 - H2 ***
  {
      5.5f,         // cleanRatio
      1163.8f,      // PARA scaling
      -3.874f,      // PARB exponent
      0.0f,         // CORA
      0.0f,         // CORB
      0.0f,         // CORC
      0.0f,         // CORD
      0.0f,         // CORE
      0.0f,         // CORF
      0.0f,         // CORG
      p145AlgB,     // preferred/tuned algorithm
      P145_SENSMQ5, // Name
      P145_GASCH4,  // gas
  },
     // *** MQ-5 - LPG ***
  {
      5.5f,         // cleanRatio
      80.897f,      // PARA scaling
      -2.431f,      // PARB exponent
      0.0f,         // CORA
      0.0f,         // CORB
      0.0f,         // CORC
      0.0f,         // CORD
      0.0f,         // CORE
      0.0f,         // CORF
      0.0f,         // CORG
      p145AlgB,     // preferred/tuned algorithm
      P145_SENSMQ5, // Name
      P145_GASLPG,  // gas
  },
  // *** MQ-6 - LPG ***
  {
      10.0f,        // cleanRatio
      1009.2f,      // PARA scaling
      -2.35f,       // PARB exponent
      0.0f,         // CORA
      0.0f,         // CORB
      0.0f,         // CORC
      0.0f,         // CORD
      0.0f,         // CORE
      0.0f,         // CORF
      0.0f,         // CORG
      p145AlgB,     // preferred/tuned algorithm
      P145_SENSMQ6, // Name
      P145_GASLPG,  // gas
  },
  // *** MQ-7 - CO ***
  {
      27.5f,        // cleanRatio
      491204.0f,    // PARA scaling
      -5.826f,      // PARB exponent
      0.0f,         // CORA
      0.0f,         // CORB
      0.0f,         // CORC
      0.0f,         // CORD
      0.0f,         // CORE
      0.0f,         // CORF
      0.0f,         // CORG   
      p145AlgB,     // preferred/tuned algorithm
      P145_SENSMQ7, // Name
      P145_GASCO,   // gas
  },
  // *** MQ-8 - H2 ***
  {
      70.0f,        // cleanRatio
      976.97f,      // PARA scaling
      -0.688f,      // PARB exponent
      0.0f,         // CORA
      0.0f,         // CORB
      0.0f,         // CORC
      0.0f,         // CORD
      0.0f,         // CORE
      0.0f,         // CORF
      0.0f,         // CORG   
      p145AlgB,     // preferred/tuned algorithm
      P145_SENSMQ8, // Name
      P145_GASH2,   // gas
  }
};
/// @brief The number of types stored in the sensorDefs[] table
constexpr const int nbrOfTypes = (int)(sizeof(sensorDefs) / sizeof(struct P145_SENSORDEF));

// Digital ouput value to swicth heater ON/OFF 
#define P145_HEATER_OFF  LOW
#define P145_HEATER_ON   HIGH

//Timeout values for Heater control
#define HEATER_WARMUP_TIME (90*1000)
#define HEATER_ON_TIME (60*1000)
#define HEATER_OFF_TIME (60*1000)
#define HEATER_MEAS_TIME (30*1000)

/*****************************************************************************/
/*!
@brief  Get the resistance of the sensor, ie. the property to be measured
@param[in] val     Analog input value read from ADC [counts]
@return The sensor resistance in Ohm
@note   Uses hard coded constants:
        MAX_ADC_VALUE Max range for ADC
        P145_VMAX     Analog  input voltage corresponding to MAX_ADC_VALUE
        P145_VCC      Standard voltage applied to the sensor-Rload combination
        P145_VCCLOW   Low voltage used to align with ESP input range
*/
/*****************************************************************************/
float P145_data_struct::getResistance(float val) const
{
  if (val > 0.0f)
  {
    float vcc = P145_VCC;   // Voltage applied to the sensor
    if (lowvcc)
    {
      vcc = P145_VCCLOW;
    }
    return ((MAX_ADC_VALUE * vcc) / (P145_VMAX * val) - 1.0f) * rload;
  }
  else
  {
    return (1e9f); // Very high resistance value of 1G Ohm
  }
}

/*****************************************************************************/
/*!
@brief  Get the resistance RZero of the sensor for calibration purposes
@param[in] rSensor    Sensor resistance value Rs
@return The sensor resistance RZero in kOhm
*/
/*****************************************************************************/
float P145_data_struct::getRZero(float rSensor) const
{
  float newValue = rSensor;  // Default to current measured Rsensor
  
  if (newValue < 0.0f) newValue = 0.0f;  // Clip negative values
  switch (algorithm)
  {
    case p145AlgA:  // MQ-135
      newValue = (rSensor * powf((refLevel / sensordef.para), (1.0f / sensordef.parb)));
      break;
    case p145AlgB:  // Miquel5612 Exponential
      if (sensordef.cleanRatio > 0.0f)
      {
        newValue = (rSensor / sensordef.cleanRatio);
      }
      break;
    case p145AlgC:  // Miquel5612 Linear
      if (sensordef.cleanRatio > 0.0f)
      {
        newValue = (rSensor / sensordef.cleanRatio);
      }
      break;
    case p145AlgNone:
      newValue = rzero;
    default:
      if (sensordef.cleanRatio > 0.0f)
      {
        newValue = (rSensor / sensordef.cleanRatio);
      }
      break;    
  };
  return newValue;
}

/*****************************************************************************/
/*!
@brief  Get the corrected resistance RZero of the sensor for calibration
        purposes
@param[in] rSensor      Sensor resistance value Rs
@param[in] temperature  The ambient air temperature
@param[in] humidity     The relative humidity

@return The corrected sensor resistance RZero in kOhm
*/
/*****************************************************************************/
float P145_data_struct::getCorrectedRZero(float rSensor, float temperature, float humidity) const
{
  float c = getTempHumCorrection(temperature, humidity);
  return getRZero(rSensor/c);
}

/*****************************************************************************/
/*!
@brief  Get the ppm of CO2 sensed (assuming only CO2 in the air)
@param[in] sensorType   Sensor type identifier
@param[in] rSensor      Sensor resistance value Rs
@param[in] rZero        R0 Resistance zero value
@return The ppm of CO2 in the air
*/
/*****************************************************************************/
float P145_data_struct::getPPM(float rSensor)
{
  switch (algorithm)
  {
    case p145AlgA:  // MQ-135
      return (sensordef.para * powf((rSensor/rzero), -sensordef.parb));
      break;
    case p145AlgB:  // Miquel5612 Exponential
      return (sensordef.para * powf((rSensor/rzero), sensordef.parb));
      break;
    case p145AlgC:  // Miquel5612 Linear
      return (powf(10.0f, (log10f(rSensor/rzero)-sensordef.parb)/sensordef.para));
      break;
    case p145AlgNone:
      return rSensor/rzero;  // No conversion, return unconverted ratio
    default:
      return (0.0f);
      break;
  }
  return 0.0f;   // Default value should never be returned
}

/*****************************************************************************/
/*!
@brief  Get the ppm of CO2 sensed (assuming only CO2 in the air), corrected
        for temp/hum
@param[in] rSensor      Sensor resistance value Rs
@param[in] temperature  The ambient air temperature
@param[in] humidity     The relative humidity
@return The ppm of CO2 in the air
*/
/*****************************************************************************/
float P145_data_struct::getCorrectedPPM(float rSensor, float temperature, float humidity)
{
  float c = getTempHumCorrection(temperature, humidity);
  return getPPM(rSensor/c);
}

/*****************************************************************************/
/*!
@brief  Get the correction factor for temperature & humidity correction
@param[in] temperature  The ambient air temperature
@param[in] humidity     The relative humidity
@return The correction factor for Rzero to compensate temperature & humidity
@note Datasheet suggests temp & hum relation to Rzero is a multiplication factor c
      here we try to calculate this factor.
*/
/*****************************************************************************/
float P145_data_struct::getTempHumCorrection(float temperature, float humidity) const
{
  float c = 1.0f;   // Correction factor, default no correction, factor is 1.0
  if (compensation)
  {
    switch (algorithm)
    {
      case p145AlgA:
        if (temperature < 20.0f)
        {
          c = sensordef.cora * temperature * temperature - sensordef.corb * temperature + sensordef.corc - (humidity - 33.0f) * sensordef.cord;
        }
        else
        {
          c = sensordef.core * temperature + sensordef.corf * humidity + sensordef.corg;
        }
        break;
      case p145AlgB:
      case p145AlgC:
        // Assume a 2nd order polynomial for temperature and 1st order poynomial for humidit
        // A + B*t + C*t*t + D*h
        c = (((sensordef.corc * temperature ) + sensordef.corb) * temperature) + sensordef.cora + (sensordef.cord * humidity);
        break;
      case p145AlgNone:
      default:
        // No correction
        c = 1.0f;
    }   
  }
  if (c <=0.0f)  // In case no or wrong correction factors are specified disable correction (multiplier = 1.0)
  {
    c = 1.0f;
  }
#ifdef P145_DEBUG
  addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: correction= "), c)); 
#endif
  return c;
}

/*****************************************************************************/
/*!
@brief  Get the analog input value
@param[in] event   Plugin context
@return The the filtered value of the analog input over the sampling interval
@note   The sensor is expected to be sampled ten times per second.
        Lowest and highest value is dropped to remove outliers.
        All other values are averaged to a single float value.
        This algorithm is copied from P002_ADC.
*/
/*****************************************************************************/
float P145_data_struct::getAnalogValue()
{
  float ain = last_ain;                        // Build result, start with previous
  float sum = (float)(ovs_value);              // OversamplingValue
  int count = ovs_cnt;                         // OversamplingCount
  if (count > 0)                               // Any samples gathered?
  {
    if (count >= 3)
    {
      sum -= (float)ovs_max;                   // remove OversamplingMaxVal
      sum -= (float)ovs_min;                   // remove OversamplingMinVal
      count -= 2;
    }
    ain = sum / (float)count;                  // Scale to single sample value
    last_ain = (unsigned long)(sum / 100);     // Remember result
  }
  return ain;
}

/*****************************************************************************/
/*!
@brief  Set the configuration data for the sensor  
@param[in] stype   Sensor type [enum]
@param[in] comp    Temperature/humidity compensation enable/disable
@param[in] cal     Autocalibration enable/disable
@param[in] load    Load resistence Rload
@param[in] zero    Rzero, Sensor resistor value for reference level
@param[in] ref     Reference level for calibration
@note   These parameters must be set before the plugin can calculate the level
        They are determined by the plugin configuration
 *****************************************************************************/
void P145_data_struct::setSensorData(int stype, bool comp, bool cal, bool vcclow, float load, float zero, float ref)
{
  /* Each MQ-xxx sensor comes with its own set of constants */
  /* Copy the correct set from program meory space          */
  if ((stype != sensorType) && (stype < nbrOfTypes) && (stype >= 0))
  {
    memcpy_P(&sensordef, &sensorDefs[stype], sizeof(struct P145_SENSORDEF));
    algorithm = sensordef.alg;
    sensorType = stype;   // Selected sensor type, index in sensor data table
  }
  lowvcc = vcclow;        // Low power supply value
  compensation = comp;    // Compensation selection flag
  calibration = cal;      // Calibration selection flag
  rload = load;           // Rload, load resistor [Ohm]
  rzero = zero;           // R0, reference resistance [Ohm]
  refLevel = ref;         // Reference level for calibration [ppm]
  rcal_act = 0.0f;        // Reset Rzero estimation
  cal_data = 0.0f;        // Reset autocalibration
}

/*****************************************************************************/
/*!
@brief  Set the connection pins for the sensor  
@param[in] aPin   Analog pin for the analog sensor input value
@param[in] hPin   Ootput pin for heater control
@note   These values must be set before the plugin can measure the level
        They are determined by the plugin configuration
 *****************************************************************************/
void P145_data_struct::setSensorPins(int aPin, int hPin)
{
  analogPin = aPin;
  heaterPin = hPin;
}

/*****************************************************************************/
/*!
@brief  Return the current/latest value acquired from the sensor 
@param[in] temperature  Temperature for compensation
@param[in] humidity     Humidity for compensation
@return The calculated gas concentration [ppm or mg/l]
@note   This is the entry point to start collecting the oversampling data,
        convert the analog value to Rsensor and convert Rsensor to the gas
        concentration level. It also performs autocalibration if selected.
        Oversampling data will be reset as a side effec.
        This function shall be called at regular interval for proper performance.
*/
/*****************************************************************************/
float P145_data_struct::readValue(float temperature, float humidity)
{
    float ain = 0.0f;             // Analog measured and filtered data 
    float rSensor = 0.0f;         // Sensor resistance Rs
    float value = 0.0f;           // Return value
#ifdef P145_DEBUG
    uint  ovs = 0;                // Oversampling count (for debugging)
#endif
#ifdef P145_TEST
    static float injector = 50.0f;
    static float injstep = 50.0f;
    ain = injector;
    injector += injstep;
    if (injector >= 400.0f)
    {
      injstep = -50.0f;
    }
    if (injector <= 100)
    {
      injstep = 50.0f;
    }
#else
    if (validGpio(heaterPin))     // Check if heater control is enabled
    {
      ain = latchedAnalogInput;   // Use value measured by heater control cycle
    }
    else
    {
      ain = getAnalogValue();     // Use acually being measured value
#ifdef P145_DEBUG
      ovs = ovs_cnt;
#endif
      resetOversampling();        // Reset the oversampling variables.
    }
#endif
    if (ain > 0.0f)   // Skip unsuccesful measurements
    {
      rSensor = getResistance(ain); // Convert to Rsensor value
      value = getCorrectedPPM(rSensor, temperature, humidity);
      rcal_act = getCorrectedRZero(rSensor, temperature, humidity);

      // Perform calibration if functionality is enabled
      if (calibration && (rcal_act < 1.0e6f)) 
      { 
        calibrate(rcal_act);
      }
    }
    if (loglevelActiveFor(LOG_LEVEL_INFO))
    {
#ifdef P145_DEBUG
      addLog(LOG_LEVEL_INFO, concat(concat(F("MQ-xx: Sensor type= "), sensorType), concat(F(": "), getTypeName(sensorType)))); 
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: algorithm= "), algorithm)); // Conversion algorithm
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: Rload= "), rload));         // Load resistor Rload
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: Rzero= "), rzero));         // Rerefernce resistance Rzero
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: Ratio= "), (rSensor/rzero)));  // Ratio, input for non-linear conversion
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: RS= "), rSensor));          // Calculated sensor resistance Rsensor
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: Ref= "), refLevel));        // Reference level for calibration
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: Temp= "), temperature));    // Temperature for compensation algorithm
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: Hum= "), humidity));        // Humidity for compensation algorithm
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: ain= "), ain));             // Measured analog input value
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: ovs= "), ovs));             // Oversampling count
      if (calibration)
      {
        addLog(LOG_LEVEL_INFO, F("MQ-xx: Calibration enabled"));
      }
#endif
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: level= "), value));         // Calculated sensor value
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: Rcal= "), rcal_act));       // Fresh calibrated Rzero when at ref level
    }

    return(value);
  }

/*****************************************************************************/
/*!
@brief  Reset the input oversampling algorithm
*/
/*****************************************************************************/
void P145_data_struct::resetOversampling()
{
    ovs_value = 0;                    // OversamplingValue
    ovs_cnt = 0;                      // OversamplingCount
    ovs_min = MAX_ADC_VALUE;          // OversamplingMinVal
    ovs_max = 0;                      // OversamplingMaxVal
}
/*****************************************************************************/
/*!
@brief  Initialize the pluging stuct/class 
@return always true
@note   This function is expected to be called as part of the plugin 
        PLUGIN_INIT functionality
*/
/*****************************************************************************/
bool P145_data_struct::plugin_init()
{
  resetOversampling();
  last_ain = 0;                   // No measured analog input value yet
  last_cal = millis();            // Initialise the calibration interval
  return true;
}

/*****************************************************************************/
/*!
@brief  Handle the 10Hz processing for the plugin struct/class 
@return Always true
@note   This function is expected to be called as part of the plugin 
        PLUGIN_TEN_PER_SECOND functionality
*/
/*****************************************************************************/
bool P145_data_struct::plugin_ten_per_second()
{
    uint16_t currentValue;

    // Measure the analog input value for oversampling
    // Algorithm uses mean value with exeption of max and min values
    // See declaraion of the global variables for their usage
    // Skip measurement in analog input is used to calibrate WiFi
    if (!WiFiEventData.wifiConnectInProgress)
    {
      currentValue = espeasy_analogRead(analogPin);
      ovs_value += currentValue;
      ovs_cnt++;

      if (currentValue > ovs_max)
      {
        ovs_max = currentValue;
      }
      if (currentValue < ovs_min)
      {
        ovs_min = currentValue;
      }
    }
    return true;
}

/**************************************************************************/
/*!
@brief  Return the actual Calibration value. 
@return Rzero associated with the current measurement
@note   The value depends on the conversion algorithm associated with the 
        sensor type.
*/
/**************************************************************************/
float P145_data_struct::getCalibrationValue() const
{
  return rcal_act;
}

/**************************************************************************/
/*!
@brief  Return the Rzero value determined by autocalibration. 
@return Rzero as calculated by the autocalibration algorithm
@note   This is the internally corrected Rzero and will be unaltered when
        Autocalibration is switched off.
*/
/**************************************************************************/
float P145_data_struct::getAutoCalibrationValue() const
{
  return rzero;
}

/**************************************************************************/
/*!
@brief  Dump data from the internal plugin struct/class structure to the log
@note   This function is only used for debugging
*/
/**************************************************************************/
void P145_data_struct::dump() const
{
#ifdef P145_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: NAME "), getTypeName(sensordef.name)));
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: GAS  "), getGasName(sensordef.name)));
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: CleanRatio "), sensordef.cleanRatio));
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: Algorithm "), sensordef.alg));
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: PARA "), sensordef.para));
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: PARB "), sensordef.parb));
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: CORA "), sensordef.cora));
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: CORB "), sensordef.corb));
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: CORC "), sensordef.corc));
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: CORD "), sensordef.cord));
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: CORE "), sensordef.core));
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: CORF "), sensordef.corf));
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: CORG "), sensordef.corg));
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: rzero "), rzero));
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: rcal  "), rcal_act));
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: analog PIN: "), analogPin));
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: heater PIN: "), heaterPin));
  }
#endif
}

/**************************************************************************/
/* @brief Provide the sensor name associated with a sensor type
   @param[in] stype Sensor type (index in sensor table)
   @return Pointer to sensor name as stored in flash memory
   @note Returns "invalid" for invalid sensor types
         This is a static member function
*/
/**************************************************************************/
const String P145_data_struct::getTypeName(int stype)
{
  switch (sensorDefs[stype].name)
  {
    case P145_SENSMQ135: return F("MQ135");
    case P145_SENSMQ2:   return F("MQ-2");
    case P145_SENSMQ3:   return F("MQ-3");
    case P145_SENSMQ4:   return F("MQ-4");
    case P145_SENSMQ5:   return F("MQ-5");
    case P145_SENSMQ6:   return F("MQ-6");
    case P145_SENSMQ7:   return F("MQ-7");
    case P145_SENSMQ8:   return F("MQ-8");
    case P145_SENSUSER:  return F("user");
    default:             return F("invalid");
  }
}

/**************************************************************************/
/* @brief Provide the name of the gas measured by a sensor type
   @param[in] stype Sensor type (index in sensor table)
   @return Pointer to gas name as stored in flash memory
   @note Returns "invalid" for invalid sensor types
         This is a static member function
*/
/**************************************************************************/
const String P145_data_struct::getGasName(int stype)
{
  switch (sensorDefs[stype].gas)
  {
    case P145_GASUSER:    return F("user");
    case P145_GASCH4:     return F("CH4");
    case P145_GASCO2:     return F("CO2");
    case P145_GASCO:      return F("CO");
    case P145_GASH2:      return F("H2");
    case P145_GASLPG:     return F("LPG");
    case P145_GASALCOHOL: return F("alcohol");
    default:              return F("invalid");
  }
}

/**************************************************************************/
/* @brief Returns the number of predefined sensor/gas entries in the table
   @return Number of predefined entries
   @note This is a static member function
*/
/**************************************************************************/
int   P145_data_struct::getNbrOfTypes()
{
  return nbrOfTypes;
}

/*****************************************************************************/
/*!
@brief  Perform the calibration algorithm
@param[in] currentRcal  Value for Rzero determined with current measurement
@return Void
@note   Collect the highest Rzero during the calibration period
        At the end of the period use it to set the Rzero for next calculations
*/
/*****************************************************************************/
void P145_data_struct::calibrate (float currentRcal)
{
  unsigned long now = millis();
  long time = timePassedSince(last_cal);
  float lastRcal = cal_data;  // Last calculated Rcal this calibration sequence
  
  if (currentRcal > lastRcal)
  {
    cal_data = currentRcal;  // Remember highest estimated Rzero during calibration period
  }
  const bool doit = (time > P145_CALIBRATION_INTERVAL) && (cal_data > 0.0f);
  if (doit)
  {
    rzero = cal_data; // Update Rzero as determined by calibration
    cal_data = 0.0f;  // Restart calibration cycle with no value
    last_cal = now;   // Remember calibration moment
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
#ifdef P145_DEBUG
    // Calculated Rzero if calibration concentration is applied
    // Rcal as calculated previous sample
    addLog(LOG_LEVEL_INFO, concat(concat(F("MQ-xx: Calibration with Rcal =  "), currentRcal), concat(F(" Rlast = "), lastRcal))); 
#endif
    if (doit)
    {
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: ***Calibrating*** Rzero =  "), cal_data));
    }
  }
}

/**************************************************************************/
/* @brief Evaluate the heater control algorithm
   @return void
   @note Heater control switches the heater pin depending on a statemachine 
        The algorithm is tuned for MQ-7, see datasheet
        Heater is assumed to be sued when the associated pin is in use
*/
/**************************************************************************/
void P145_data_struct::heaterControl(void)
{
  unsigned long now = millis();
  long time = timePassedSince(heaterChangeTime);  // Time a state is active
  P145_heaterState lastState = heaterState;       // To detect a state change

  // Check if the heaterPin has been changed => change in controller
  if (heaterPin != lastHeaterPin)
  {
    if (validGpio(lastHeaterPin))       // Disable last used pin
    {
      pinMode(heaterPin, INPUT);        // Safe value when pin is not ours?
    }
    if (validGpio(heaterPin))           // Enable new pin
    {
      pinMode(heaterPin, OUTPUT);       // Safe value when pin is not ours?
      digitalWrite(heaterPin, P145_HEATER_ON);
      heaterState = P145HeaterWarmup;   // Start warming up the sensor
    }
    else
    {
      heaterState = P145HeaterDisabled; // Disable teh controller
    }
    heaterChangeTime = now;             // Reset state timer
    lastHeaterPin = heaterPin;          // Remember selection
  }

  // State machine evaluation
  switch (heaterState)
  {
    // Heater control is disabled (default)
    case P145HeaterDisabled:   
      // State transition only based upon pin selection above
      break;
    // Heater switched on, warming up
    case P145HeaterWarmup:    
      if (time >= HEATER_WARMUP_TIME)
      {
          heaterState = P145HeaterHighVolt;
          heaterChangeTime = now;
      }
      break;
    // Apply high voltage to heater
    case P145HeaterHighVolt:   
      if (time >= HEATER_ON_TIME)
      {
          digitalWrite(heaterPin, P145_HEATER_OFF); // Switch heater
          heaterState = P145HeaterLowVolt;
          heaterChangeTime = now;
      }
      break;
    // Apply low voltage to heater
    case P145HeaterLowVolt:    
     if (time >= HEATER_OFF_TIME)
      {
          resetOversampling();                      // Start oversampling sequence
          heaterState = P145HeaterMeasure;
          heaterChangeTime = now;
      }
      break;
      // Measurement phase
    case P145HeaterMeasure:    
      if (time >= HEATER_MEAS_TIME)
      {
          latchedAnalogInput = getAnalogValue();    // Read the sensor value
          digitalWrite(heaterPin, P145_HEATER_ON);  // Switch heater
          heaterState = P145HeaterHighVolt;
          heaterChangeTime = now;
      }
      break;
    default:
      heaterState = P145HeaterDisabled; // Back to a known state
      break;
  }

#ifdef P145_DEBUG
  if (heaterState != lastState)
  {
    int x = time / 1000;
    addLog(LOG_LEVEL_INFO, concat(concat(F("MQ-xx: $$$ Heater state: "), heaterState), concat(F(", time: "), x)));
  }
#endif  // P145_DEBUG
}
#endif // USES_P145