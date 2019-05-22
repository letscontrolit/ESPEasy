#ifdef USES_P135
//#######################################################################################################
//#################################### Plugin 135: MQ135## #############################################
//#################################### by dony71 ########################################################
//#######################################################################################################
// based on https://hackaday.io/project/3475-sniffing-trinket/log/12363-mq135-arduino-library
// MQ135 library from https://github.com/GeorgK/MQ135

#define PLUGIN_135
#define PLUGIN_ID_135           135
#define PLUGIN_NAME_135         "Gases - MQ135 [TESTING]"
#define PLUGIN_VALUENAME1_135   "CO2"
#define PLUGIN_VALUENAME2_135   "AirQuality"

boolean Plugin_135_init = false;
byte Plugin_MQ135_Air_Quality_Pin = 0;

//#include <MQ135.h>

//#define FW_NAME         "wemos-mq135"
//#define FW_VERSION      "0.0.1"

/* Magic sequence for Autodetectable Binary Upload */
//const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
//const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";
/* End of magic sequence for Autodetectable Binary Upload */

//==============================================
// MQ135 LIBRARY - MQ135.h
// =============================================
//#ifndef MQ135_H
//#define MQ135_H

/// The load resistance on the board
#define RLOAD           10.0
/// Calibration resistance at atmospheric CO2 level
#define RZERO           76.63
/// Parameters for calculating ppm of CO2 from sensor resistance
#define PARA            116.6020682  // scaling factor value
#define PARB            2.769034857  // exponent value

/// Parameters to model temperature and humidity dependence
#define CORA            0.00035
#define CORB            0.02718
#define CORC            1.39538
#define CORD            0.0018
#define CORE            -0.003333333
#define CORF            -0.001923077
#define CORG            1.130128205

/// Atmospheric CO2 level for calibration purposes
#define ATMOCO2         397.13

/// Sensor pin and  data polling interval
#define SENSOR_PIN      A0
#define PUB_INTERVAL    5   // seconds

class MQ135 {
  private:
    uint8_t _pin;

  public:
    MQ135(uint8_t pin);
    float getCorrectionFactor(float temperature, float humidity);
//    float getResistance();
    float getResistance(float tempRLOAD);
//    float getCorrectedResistance(float t, float h);
    float getCorrectedResistance(float tempRLOAD, float temperature, float humidity);
//    float getPPM();
    float getPPM(float tempRLOAD, float tempRZERO);
//    float getCorrectedPPM(float t, float h);
    float getCorrectedPPM(float tempRLOAD, float tempRZERO, float temperature, float humidity);
//    float getRZero();
    float getRZero(float tempRLOAD, float tempATMOCO2);
//    float getCorrectedRZero(float t, float h);
    float getCorrectedRZero(float tempRLOAD, float tempATMOCO2, float temperature, float humidity);
};
//#endif

//==============================================
// MQ135 LIBRARY - MQ135.cpp
// =============================================
/**************************************************************************/
/*!
@brief  Default constructor
@param[in] pin  The analog input pin for the readout of the sensor
*/
/**************************************************************************/
MQ135::MQ135(uint8_t pin) {
  _pin = pin;
}

/**************************************************************************/
/*!
@brief  Get the correction factor to correct for temperature and humidity
@param[in] t  The ambient air temperature
@param[in] h  The relative humidity
@return The calculated correction factor
*/
/**************************************************************************/
float MQ135::getCorrectionFactor(float temperature, float humidity) {
// Linearization of the temperature dependency curve under and above 20 degree C
// below 20degC: fact = a * t * t - b * t - (h - 33) * d
// above 20degC: fact = a * t + b * h + c
// this assumes a linear dependency on humidity
  if (temperature < 20) {
      return CORA * temperature * temperature - CORB * temperature + CORC - (humidity-33.)*CORD;
  } else {
      return CORE * temperature + CORF * humidity + CORG;
  }
}

/**************************************************************************/
/*!
@brief  Get the resistance of the sensor, ie. the measurement value
@return The sensor resistance in kOhm
*/
/**************************************************************************/
/*
float MQ135::getResistance() {
  int val = analogRead(_pin);
  return ((1023./(float)val) - 1.)*RLOAD;
}
*/
float MQ135::getResistance(float tempRLOAD) {
  int val = analogRead(_pin);
  return ((1023./(float)val) - 1.)*tempRLOAD;
}

/**************************************************************************/
/*!
@brief  Get the resistance of the sensor, ie. the measurement value corrected
        for temp/hum
@param[in] t  The ambient air temperature
@param[in] h  The relative humidity
@return The corrected sensor resistance kOhm
*/
/**************************************************************************/
/*
float MQ135::getCorrectedResistance(float t, float h) {
  return getResistance()/getCorrectionFactor(t, h);
}
*/
float MQ135::getCorrectedResistance(float tempRLOAD, float temperature, float humidity) {
  return getResistance(tempRLOAD)/getCorrectionFactor(temperature, humidity);
}

/**************************************************************************/
/*!
@brief  Get the ppm of CO2 sensed (assuming only CO2 in the air)
@return The ppm of CO2 in the air
*/
/**************************************************************************/
/*
float MQ135::getPPM() {
  return PARA * pow((getResistance()/RZERO), -PARB);
}
*/
float MQ135::getPPM(float tempRLOAD, float tempRZERO) {
  return PARA * pow((getResistance(tempRLOAD)/tempRZERO), -PARB);
}

/**************************************************************************/
/*!
@brief  Get the ppm of CO2 sensed (assuming only CO2 in the air), corrected
        for temp/hum
@param[in] t  The ambient air temperature
@param[in] h  The relative humidity
@return The ppm of CO2 in the air
*/
/**************************************************************************/
/*
float MQ135::getCorrectedPPM(float t, float h) {
  return PARA * pow((getCorrectedResistance(t, h)/RZERO), -PARB);
}
*/
float MQ135::getCorrectedPPM(float tempRLOAD, float tempRZERO, float temperature, float humidity) {
  return PARA * pow((getCorrectedResistance(tempRLOAD, temperature, humidity)/tempRZERO), -PARB);
}

/**************************************************************************/
/*!
@brief  Get the resistance RZero of the sensor for calibration purposes
@return The sensor resistance RZero in kOhm
*/
/**************************************************************************/
/*
float MQ135::getRZero() {
  return getResistance() * pow((ATMOCO2/PARA), (1./PARB));
}
*/
float MQ135::getRZero(float tempRLOAD, float tempATMOCO2) {
  return getResistance(tempRLOAD) * pow((tempATMOCO2/PARA), (1./PARB));
}

/**************************************************************************/
/*!
@brief  Get the corrected resistance RZero of the sensor for calibration
        purposes
@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The corrected sensor resistance RZero in kOhm
*/
/**************************************************************************/
/*
float MQ135::getCorrectedRZero(float t, float h) {
  return getCorrectedResistance(t, h) * pow((ATMOCO2/PARA), (1./PARB));
}
*/
float MQ135::getCorrectedRZero(float tempRLOAD, float tempATMOCO2, float temperature, float humidity) {
  return getCorrectedResistance(tempRLOAD, temperature, humidity) * pow((tempATMOCO2/PARA), (1./PARB));
}

MQ135*  Plugin_135_MQ135[TASKS_MAX] = { NULL, };

//==============================================
// PLUGIN
// =============================================

boolean Plugin_135(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_135;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_DUAL;
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
        string = F(PLUGIN_NAME_135);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_135));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_135));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormSeparator(2);
        addFormTextBox(F("Load Resistance"), F("plugin_135_RLOAD"), String(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0]), 33);
        addUnit(F("10.00"));
        addFormTextBox(F("R Zero"), F("plugin_135_RZERO"), String(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1]), 33);
        addUnit(F("76.63"));
        addFormTextBox(F("CO2 Level ref."), F("plugin_135_ATMOCO2"), String(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2]), 33);
        addUnit(F("397.13"));

        addFormSeparator(2);
        // mode
        addFormCheckBox(F("Enable temp/humid compensation"), F("plugin_135_enable_compensation"), Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
        addFormNote(F("If this is enabled, the Temperature and Humidity values below need to be configured."));
        // temperature
        addHtml(F("<TR><TD>Temperature:<TD>"));
        addTaskSelect(F("plugin_135_temperature_task"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
        LoadTaskSettings(Settings.TaskDevicePluginConfig[event->TaskIndex][1]); // we need to load the values from another task for selection!
        addHtml(F("<TR><TD>Temperature Value:<TD>"));
        addTaskValueSelect(F("plugin_135_temperature_value"), Settings.TaskDevicePluginConfig[event->TaskIndex][2], Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
        // humidity
        addHtml(F("<TR><TD>Humidity:<TD>"));
        addTaskSelect(F("plugin_135_humidity_task"), Settings.TaskDevicePluginConfig[event->TaskIndex][3]);
        LoadTaskSettings(Settings.TaskDevicePluginConfig[event->TaskIndex][3]); // we need to load the values from another task for selection!
        addHtml(F("<TR><TD>Humidity Value:<TD>"));
        addTaskValueSelect(F("plugin_135_humidity_value"), Settings.TaskDevicePluginConfig[event->TaskIndex][4], Settings.TaskDevicePluginConfig[event->TaskIndex][3]);
        LoadTaskSettings(event->TaskIndex); // we need to restore our original taskvalues!

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0] = getFormItemFloat(F("plugin_135_RLOAD"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = getFormItemFloat(F("plugin_135_RZERO"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] = getFormItemFloat(F("plugin_135_ATMOCO2"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = isFormItemChecked(F("plugin_135_enable_compensation") );
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_135_temperature_task"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("plugin_135_temperature_value"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = getFormItemInt(F("plugin_135_humidity_task"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][4] = getFormItemInt(F("plugin_135_humidity_value"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (Plugin_135_MQ135[event->TaskIndex])
            delete Plugin_135_MQ135[event->TaskIndex];
        Plugin_135_MQ135[event->TaskIndex] = new MQ135(SENSOR_PIN);

        Plugin_135_init = true;
        pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT);
        Plugin_MQ135_Air_Quality_Pin = Settings.TaskDevicePin1[event->TaskIndex];

        success = true;
        break;
      }

    case PLUGIN_READ:
        {
        if (!Plugin_135_MQ135[event->TaskIndex])
            return success;

        const float tempRLOAD = Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0];
        const float tempRZERO = Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1];
        const float tempATMOCO2 = Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2];

        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0]) {
            // we're checking a var from another task, so calculate that basevar
            byte TaskIndex1    = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
            byte BaseVarIndex1 = TaskIndex1 * VARS_PER_TASK + Settings.TaskDevicePluginConfig[event->TaskIndex][2];
            float temperature = UserVar[BaseVarIndex1]; // in degrees C

            byte TaskIndex2    = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
            byte BaseVarIndex2 = TaskIndex2 * VARS_PER_TASK + Settings.TaskDevicePluginConfig[event->TaskIndex][4];
            float humidity     = UserVar[BaseVarIndex2]; // in % relative

            UserVar[event->BaseVarIndex] = Plugin_135_MQ135[event->TaskIndex]->getCorrectedPPM(tempRLOAD, tempRZERO, temperature, humidity);
            UserVar[event->BaseVarIndex + 2] = Plugin_135_MQ135[event->TaskIndex]->getCorrectedRZero(tempRLOAD, tempATMOCO2, temperature, humidity);
        } else {
            UserVar[event->BaseVarIndex] = Plugin_135_MQ135[event->TaskIndex]->getPPM(tempRLOAD, tempRZERO);
            UserVar[event->BaseVarIndex + 2] = Plugin_135_MQ135[event->TaskIndex]->getRZero(tempRLOAD, tempATMOCO2);
        }

        UserVar[event->BaseVarIndex + 1] = digitalRead(Plugin_MQ135_Air_Quality_Pin);

        String log = F("MQ135: co2: ");
        log += UserVar[event->BaseVarIndex];
        addLog(LOG_LEVEL_INFO, log);
        log = F("MQ135: air quality: ");
        log += UserVar[event->BaseVarIndex + 1];
        addLog(LOG_LEVEL_INFO, log);
        log = F("MQ135: rzero: ");
        log += UserVar[event->BaseVarIndex + 2];
        addLog(LOG_LEVEL_INFO, log);

        success = true;
        break;
      }
  }
  return success;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // USES_P135
