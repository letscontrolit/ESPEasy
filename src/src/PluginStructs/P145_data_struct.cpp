// ----------------------------------------------------------------------------
// P145 "Gases - MQxxx (MQ135 CO2, MQ3 Alcohol)"
// Implementation of sensor abstraction
// See _P145_MQxxx.ino
// 2023 By flashmark
// ----------------------------------------------------------------------------
#include "../PluginStructs/P145_data_struct.h"

#ifdef USES_P145

#include "../Globals/ESPEasyWiFiEvent.h"   // Need to know when WiFi is ruining the ADC measurements
#include "../Helpers/Hardware.h"           // Need to know the ADC properties

/******************************************************************************/
// Table with sensor type specific data
// This table is stored in PROGMEM to save space in RAM
// The data for the selected sensor type is copied to RAM, see P145_data_struct
/******************************************************************************/
const struct P145_SENSORDEF sensorDefs[] PROGMEM =
{
  // User defined, open for experiments/own sensor definition
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
      "USER",       // Name
      "unknown",    // gas
  },
  // MQ-135
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
      "MQ-135",     // Name
      "CO2",        // gas

  },
    // MQ-2
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
      "MQ-2",       // Name
      "H2",         // gas
  },
  // MQ-3
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
      "MQ-3",       // Name
      "alcohol",    // gas

  },
  // MQ-4
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
      "MQ-4",       // Name
      "CH4",        // gas
  },
  // MQ-7
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
      "MQ-7",       // Name
      "CO",         // gas
  }
};
/// @brief The number of types stored in the sensorDefs[] table
constexpr const int nbrOfTypes = (int)(sizeof(sensorDefs) / sizeof(struct P145_SENSORDEF));

/*****************************************************************************/
/*!
@brief  Perform the calibration algorithm
@param[in] currentRcal  Value for Rzero determined with current measurement
@return Void
@note   Collect the highest Rzero during the calibration period
        At the end of the period take mean value of current and calculated Rzero
*/
/*****************************************************************************/
void P145_data_struct::calibrate (float currentRcal)
{
  unsigned long now = millis();
  long time = timePassedSince(last_cal);
  float lastRcal = rcal;  // Last calculated Rcal this calibration sequence
  
  if (currentRcal > rcal)
  {
    rcal = currentRcal;
  }
  const bool doit = (time > P145_CALIBRATION_INTERVAL) && (rcal > 0.0f);
  if (doit)
  {
    rzero = rcal;     // Update Rzero as determined by calibration
    rcal = 0.0f;      // Restart calibration cycle with no value
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
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: ***Calibrating*** Rzero =  "), rcal));
    }
  }
}

/*****************************************************************************/
/*!
@brief  Get the resistance of the sensor, ie. the property to be measured
@param[in] val     Analog input value read from ADC [counts]
@return The sensor resistance in Ohm
@note   Uses hard coded constants:
        MAX_ADC_VALUE Max range for ADC
        P145_VMAX     Analog  input voltage corresponding to MAX_ADC_VALUE
        P145_VCC      Voltage applied to the sensor-Rload combination
*/
/*****************************************************************************/
float P145_data_struct::getResistance(float val) const
{
  if (val > 0.0f)
  {
    return ((MAX_ADC_VALUE * P145_VCC) / (P145_VMAX * val) - 1.0f) * rload;
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
  float c;    // Correction factor

  if (sensordef.cora <= 0.0f)
  {
    return (getRZero(rSensor));
  }
  else if (temperature < 20.0f) //TODO this is an ugly constant associated with the MQ-135 sensor only
  {
    c = sensordef.cora * temperature * temperature - sensordef.corb * temperature + sensordef.corc - (humidity - 33.0f) * sensordef.cord;
  }
  else
  {
    c = sensordef.core * temperature + sensordef.corf * humidity + sensordef.corg;
  }
  return (rSensor / c) * powf((refLevel / sensordef.para), (1.0f / sensordef.parb));
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
  float c = 1.0f;   // Correction factor
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
    case p145AlgB:
      // TODO Still missing a formula to correct for temp/hum when applying this algorithm
      break;
    case p145AlgC:
      // TODO Still missing a formula to correct for temp/hum when applying this algorithm
      break;
    default:
      // Do nothing, return a default value at the end
      break;
  }
  return getPPM(rSensor/c);  // Default value should never be returned
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
/*****************************************************************************/
void P145_data_struct::setSensorData(int stype, bool comp, bool cal, float load, float zero, float ref)
{
  /* Each MQ-xxx sensor comes with its own set cof constants */
  /* Copy the correct set from program meory space           */
  if ((stype != sensorType) && (stype < nbrOfTypes) && (stype >= 0))
  {
    memcpy_P(&sensordef, &sensorDefs[stype], sizeof(struct P145_SENSORDEF));
    algorithm = sensordef.alg;
    sensorType = stype;   // Selected sensor type, index in sensor data table
  }
  compensation = comp;    // Compensation selection flag
  calibration = cal;      // Calibration selection flag
  rload = load;           // Rload, load resistor [Ohm]
  rzero = zero;           // R0, reference resistance [Ohm]
  refLevel = ref;         // Reference level for calibration [ppm]
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
    float ain = getAnalogValue();           // Analog measured and filtered data 
    float rSensor = getResistance(ain);     // Convert to sensor resistance Rs
    float rCal    = 0.0f;                   // Potential Rzero for calibration
    float value = 0.0f;                     // Return value

    // Check if temperature/Humidity compensation is selected
    if (compensation)
    {
        value = getCorrectedPPM(rSensor, temperature, humidity);
        rCal = getCorrectedRZero(rSensor, temperature, humidity);
    }
    else
    {
        value = getPPM(rSensor);
        rCal = getRZero(rSensor);
    }

    // Perform calibration if functionality is enabled
    if (calibration && (rCal < 1.0e6f)) 
    { 
        calibrate(rCal);
    }

    if (loglevelActiveFor(LOG_LEVEL_INFO))
    {
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: level= "), value));         // Calculated sensor value
#ifdef P145_DEBUG
      addLog(LOG_LEVEL_INFO, concat(concat(F("MQ-xx: Sensor type= "), sensorType), concat(F(": "), String(sensordef.name)))); 
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: Rload= "), rload));         // Load resistor Rload
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: Rzero= "), rzero));         // Rerefernce resistance Rzero
#endif 
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: Rcal= "), rCal));           // Fresh calibrated Rzero when at ref level
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: RS= "), rSensor));          // Calculated sensor resistance Rsensor
#ifdef P145_DEBUG
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: Ref= "), refLevel));        // Reference level for calibration
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: Temp= "), temperature));    // Temperature for compensation algorithm
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: Hum= "), humidity));        // Humidity for compensation algorithm
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: ain= "), ain));             // Measured analog input value
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: ovs= "), ovs_cnt));         // Oversampling count
      addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: algorithm= "), algorithm)); // Conversion algorithm
      if (calibration)
      {
        addLog(LOG_LEVEL_INFO, F("MQ-xx: Calibration enabled"));
      }
#endif
    }

    resetOversampling();  // Now reset the oversampling variables.

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
    return(true);
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
  return (rcal);
}

/**************************************************************************/
/*!
@brief  Return the Rzero value determined by autocalibration. 
@return Rzero as calculated by the autocalibration algorithm
@note   This is the internally corrected Rzero and will be unaltered when
        Autocalibration is switched off.
*/
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
  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
#ifdef P145_DEBUG
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: NAME "), String(sensordef.name)));
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
    addLog(LOG_LEVEL_INFO, concat(F("MQ-xx: PIN: "), analogPin));
#endif
  }
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
  if ((stype < nbrOfTypes) && (stype >= 0))
    return String(sensorDefs[stype].name);
  else
    return F("invalid");
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
  if ((stype < nbrOfTypes) && (stype >= 0))
    return String(sensorDefs[stype].gas);
  else
    return F("invalid");
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
#endif // USES_P145