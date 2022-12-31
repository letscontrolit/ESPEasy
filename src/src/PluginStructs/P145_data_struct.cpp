#include "../PluginStructs/P145_data_struct.h"

#ifdef USES_P145

#include "../Globals/ESPEasyWiFiEvent.h"   // Need to know when WiFi is ruining the ADC measurements
#include "../Helpers/Hardware.h"           // Need to know the ADC properties

/******************************************************************************/
// Table with sensor type specific data
// This table is stored in PROGMEM to save space in RAM
// The data for the selected sensor type is copied to RAM, see P145_data_struct
/******************************************************************************/
const struct P145_SENSORDEF sensorData[] PROGMEM =
{
  // User defined, open for experiments/ own snsor definition
  {     
      "USER",       // Name
      "unknown",    // gas
      p145AlgNone,  // preferred/tuned algorithm
      0.0,          // cleanRatio
      0.0,          // PARA scaling factor value
      0.0,          // PARB exponent value
      0.0,          // CORA
      0.0,          // CORB
      0.0,          // CORC
      0.0,          // CORD
      0.0,          // CORE
      0.0,          // CORF
      0.0,          // CORG
  },
  // MQ-135
  {
      "MQ-135",     // Name
      "CO2",        // gas
      p145AlgA,     // preferred/tuned algorithm
      0.0,          // cleanRatio
      116.6020682,  // PARA scaling factor value
      2.769034857,  // PARB exponent value
      0.00035,      // CORA
      0.02718,      // CORB
      1.39538,      // CORC
      0.0018,       // CORD
      -0.003333333, // CORE
      -0.001923077, // CORF
      1.130128205,  // CORG
  },
    // MQ-2
  {
      "MQ-2",       // Name
      "H2",         // gas
      p145AlgB,     // preferred/tuned algorithm
      9.83,         // cleanRatio
      987.99,       // PARA scaling factor
      -2.162,       // PARB exponent value
      0.0,          // CORA
      0.0,          // CORB
      0.0,          // CORC
      0.0,          // CORD
      0.0,          // CORE
      0.0,          // CORF
      0.0,          // CORG
  },
  // MQ-3
  {
      "MQ-3",       // Name
      "alcohol",    // gas
      p145AlgB,     // preferred/tuned algorithm
      60.0,         // cleanRatio
      0.3934,       // PARA scaling factor
      -1.504,       // PARB exponent value
      0.0,          // CORA
      0.0,          // CORB
      0.0,          // CORC
      0.0,          // CORD
      0.0,          // CORE
      0.0,          // CORF
      0.0,          // CORG
  },
  // MQ-4
  {
      "MQ-4",       // Name
      "CH4",        // gas
      p145AlgB,     // preferred/tuned algorithm
      4.4,          // cleanRatio
      1012.7,       // PARA scaling
      -2.786,       // PARB exponent
      0.0,          // CORA
      0.0,          // CORB
      0.0,          // CORC
      0.0,          // CORD
      0.0,          // CORE
      0.0,          // CORF
      0.0,          // CORG
  },
  // MQ-7
  {
      "MQ-7",       // Name
      "CO",         // gas
      p145AlgB,     // preferred/tuned algorithm
      27.5,         // cleanRatio
      491204,       // PARA scaling
      -5.826,       // PARB exponent
      0.0,          // CORA
      0.0,          // CORB
      0.0,          // CORC
      0.0,          // CORD
      0.0,          // CORE
      0.0,          // CORF
      0.0,          // CORG   
  }
};
const int nbrOfTypes = (int)(sizeof(sensorData) / sizeof(struct P145_SENSORDEF));

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
  unsigned long time = now - last_cal;
  float lastRcal = rcal;  // Last calculated Rcal this calibration sequence
  bool doit = false;
  
  if ((currentRcal > rcal) || (rcal <= 0.0))
  {
    rcal = currentRcal;
  }
  doit = (time > P145_CALIBRATION_INTERVAL) && (lastRcal > 0.0);
  if (doit)
  {
    rzero = rcal;     // Update Rzero as determined by calibration
    rcal = 0.0;       // Restart calibration cycle with no value
    last_cal = now;   // Remember calibration moment
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log;
    log = F("MQ-xx: Calibration with Rcal =  ");
    log += currentRcal;      // Calculated Rzero if calibration concentration is applied
    log += F(" Rlast = ");
    log += lastRcal;  // Rcal as calculated previous sample
    addLog(LOG_LEVEL_INFO, log);
    if (doit)
    {
      log = F("MQ-xx: ***Calibrating*** Rzero =  ");
      log += rcal;
      addLog(LOG_LEVEL_INFO, log);
    }
  }
}

/*****************************************************************************/
/*!
@brief  Get the resistance of the sensor, ie. the property to be measured
@param[in] val     Analog input value read from ADC [counts]
@return The sensor resistance in Ohm
@note   Uses hard coded constants:
        MAXSCALE   Max range for ADC
        VMAX       Analog  input voltage corresponding to MAXSCALE
        VCC        Voltage applied to the sensor-Rload combination
*/
/*****************************************************************************/
float P145_data_struct::getResistance(float val)
{
  return ((P145_MAXSCALE * P145_VCC) / (P145_VMAX * val) - 1.0) * rload;
}

/*****************************************************************************/
/*!
@brief  Get the resistance RZero of the sensor for calibration purposes
@param[in] rSensor    Sensor resistance value Rs
@return The sensor resistance RZero in kOhm
*/
/*****************************************************************************/
float P145_data_struct::getRZero(float rSensor)
{
  float newValue = rSensor;  // Default to current measured Rsensor
  
  if (newValue < 0.0) newValue = 0.0;  // Clip negative values
  switch (algorithm)
  {
    case p145AlgA:  // MQ-135
      newValue = (rSensor * pow((refLevel / sensordef.para), (1.0 / sensordef.parb)));
      break;
    case p145AlgB:  // Miquel5612 Exponential
      if (sensordef.cleanRatio > 0.0)
      {
        newValue = (rSensor / sensordef.cleanRatio);
      }
      break;
    case p145AlgC:  // Miquel5612 Linear
      if (sensordef.cleanRatio > 0.0)
      {
        newValue = (rSensor / sensordef.cleanRatio);
      }
      break;
    default:
      if (sensordef.cleanRatio > 0.0)
      {
        newValue = (rSensor / sensordef.cleanRatio);
      }
      break;    
  };
  return (newValue);
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
float P145_data_struct::getCorrectedRZero(float rSensor, float temperature, float humidity)
{
  float c;    // Correction factor

  if (sensordef.cora <= 0.0)
  {
    return (getRZero(rSensor));
  }
  else if (temperature < 20)
  {
    c = sensordef.cora * temperature * temperature - sensordef.corb * temperature + sensordef.corc - (humidity - 33.) * sensordef.cord;
  }
  else
  {
    c = sensordef.core * temperature + sensordef.corf * humidity + sensordef.corg;
  }
  return (rSensor / c) * pow((refLevel / sensordef.para), (1. / sensordef.parb));
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
      return (sensordef.para * pow((rSensor/rzero), -sensordef.parb));
      break;
    case p145AlgB:  // Miquel5612 Exponential
      return (sensordef.para *pow((rSensor/rzero), sensordef.parb));
      break;
    case p145AlgC:  // Miquel5612 Linear
      return (pow(10, (log10(rSensor/rzero)-sensordef.parb)/sensordef.para));
      break;
    default:
      return (0.0);
      break;
  }
  return 0.0;   // Default value should never be returned
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
  switch (algorithm)
  {
    case p145AlgA:
      float c;                      // Temperature & humidity correction factor
      if (temperature < 20)
      {
        c = sensordef.cora * temperature * temperature - sensordef.corb * temperature + sensordef.corc - (humidity - 33.) * sensordef.cord;
      }
      else
      {
        c = sensordef.core * temperature + sensordef.corf * humidity + sensordef.corg;
      }
      return (sensordef.para * pow(((rSensor / c) / rzero), -sensordef.parb));
    case p145AlgB:
      // TODO
      break;
    case p145AlgC:
      // TODO
      break;
    default:
      return (0.0);
      break;
  }
  return 0.0;  // Default value should never be returned
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
    memcpy_P(&sensordef, &sensorData[stype], sizeof(struct P145_SENSORDEF));
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
    float rCal    = 0.0;                    // Potential Rzero for calibration
    float value = 0.0;                      // Return value

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
    if (calibration && (rCal < 1.0e6)) 
    { 
        calibrate(rCal);
    }

    if (loglevelActiveFor(LOG_LEVEL_INFO))
    {
      String log;                       // Helper string to build log text
      log = F("MQ-xx: level= ");
      log += value;                     // Calculated sensor value
      addLog(LOG_LEVEL_INFO, log);
      log = F("MQ-xx: Sensor type= ");
      log += sensorType;                // Selected sensor type
      log += F(": ");
      log += sensordef.name;
      addLog(LOG_LEVEL_INFO, log);              
      log = F("MQ-xx: Rload= ");
      log += rload;                     // Rload
      addLog(LOG_LEVEL_INFO, log);              
      log = F("MQ-xx: Rzero= ");
      log += rzero;                     // R0
      addLog(LOG_LEVEL_INFO, log);              
      log = F("MQ-xx: Rcal= ");
      log += rCal;                      // R0 when calibrating
      addLog(LOG_LEVEL_INFO, log);
      log = F("MQ-xx: RS= ");
      log += rSensor;                   // Calculated sensor resistance
      addLog(LOG_LEVEL_INFO, log);
      log = F("MQ-xx: Ref= ");
      log += refLevel;                  // Reference level for calibration
      addLog(LOG_LEVEL_INFO, log);
      log = F("MQ-xx: ain= ");
      log += ain;                       // Measured analog input value
      addLog(LOG_LEVEL_INFO, log);
      log = F("MQ-xx: ovs= ");
      log += ovs_cnt;                   // Oversampling count
      addLog(LOG_LEVEL_INFO, log);
      log = F("MQ-xx: algorithm= ");
      log += algorithm;                 // Conversion algorithm
      addLog(LOG_LEVEL_INFO, log);
      if (calibration)
      {
        log += F("MQ-xx: Calibration enabled");
        addLog(LOG_LEVEL_INFO, log);
      }
    }

    // Now reset the oversampling variables.
    resetOversampling();

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
@brief  Handle the 10Hz processing for the plugin strct/class 
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
@brief  Dump data from the internal plugin struct/class structure to the log
@note   This function is only used for debugging
*/
/**************************************************************************/
void P145_data_struct::dump()
{
  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log;                       // Helper string to build log text
    log = F("MQ-xx: NAME ");
    log += sensordef.name;
    addLog(LOG_LEVEL_INFO, log);
    log = F("MQ-xx: CleanRatio ");
    log += sensordef.cleanRatio;
    addLog(LOG_LEVEL_INFO, log);
    log = F("MQ-xx: Algorithm ");
    log += sensordef.alg;
    addLog(LOG_LEVEL_INFO, log);
    log = F("MQ-xx: PARA ");
    log += sensordef.para;
    addLog(LOG_LEVEL_INFO, log);
    log = F("MQ-xx: PARB ");
    log += sensordef.parb;
    addLog(LOG_LEVEL_INFO, log);
    log = F("MQ-xx: CORA ");
    log += sensordef.cora;
    addLog(LOG_LEVEL_INFO, log);
    log = F("MQ-xx: CORB ");
    log += sensordef.corb;
    addLog(LOG_LEVEL_INFO, log);
    log = F("MQ-xx: CORC ");
    log += sensordef.corc;
    addLog(LOG_LEVEL_INFO, log);
    log = F("MQ-xx: CORD ");
    log += sensordef.cord;
    addLog(LOG_LEVEL_INFO, log);
    log = F("MQ-xx: CORE ");
    log += sensordef.core;
    addLog(LOG_LEVEL_INFO, log);
    log = F("MQ-xx: CORF ");
    log += sensordef.corf;
    addLog(LOG_LEVEL_INFO, log);
    log = F("MQ-xx: CORG ");
    log += sensordef.corg;
    addLog(LOG_LEVEL_INFO, log);
    log = F("MQ-xx: rzero ");
    log += rzero;
    addLog(LOG_LEVEL_INFO, log);
  }
}

/**************************************************************************/
/* @brief Provide the sensor name associated with a sensor type
   @param[in] stype Sensor type (index in sensor table)
   @return Pointer to sensor name as stored in flash memory
   @note Returns "invalid" for invalid sensor types
*/
/**************************************************************************/
const __FlashStringHelper * P145_data_struct::getTypeName(int stype)
{
  if ((stype < nbrOfTypes) && (stype >= 0))
    return (const __FlashStringHelper *)&sensorData[stype].name;
  else
    return F("invalid");
}

/**************************************************************************/
/* @brief Provide the name of the gas measured by a sensor type
   @param[in] stype Sensor type (index in sensor table)
   @return Pointer to gas name as stored in flash memory
   @note Returns "invalid" for invalid sensor types
*/
/**************************************************************************/
const __FlashStringHelper * P145_data_struct::getGasName(int stype)
{
  if ((stype < nbrOfTypes) && (stype >= 0))
    return (const __FlashStringHelper *)&sensorData[stype].gas;
  else
    return F("invalid");
}

/**************************************************************************/
/* @brief Returns the number of predefined sensor/gas entries in the table
   @return Number of predefined entries
*/
/**************************************************************************/
int   P145_data_struct::getNbrOfTypes()
{
  return nbrOfTypes;
}
#endif // USES_P145