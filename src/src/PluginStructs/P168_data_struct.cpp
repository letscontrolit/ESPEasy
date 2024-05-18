#include "../PluginStructs/P168_data_struct.h"

#ifdef USES_P168

# include "../Helpers/CRC_functions.h"

/**************************************************************************
* Constructor
**************************************************************************/
P168_data_struct::P168_data_struct(uint8_t alsGain,
                                   uint8_t alsIntegration,
                                   uint8_t psmMode,
                                   uint8_t readMethod) :
  _als_gain(alsGain), _als_integration(alsIntegration), _psm_mode(psmMode), _readMethod(readMethod), initialized(false)
{}

P168_data_struct::~P168_data_struct() {
  delete veml;
}

bool P168_data_struct::init(struct EventStruct *event) {
  veml = new (std::nothrow) Adafruit_VEML7700();

  // - Read sensor serial number
  if ((nullptr != veml) &&
      veml->begin()) {
    // Set config & start sensor
    veml->setGain(_als_gain);
    veml->setIntegrationTime(_als_integration);
    veml->setPowerSaveMode(_psm_mode);
    veml->enable(true);

    addLog(LOG_LEVEL_INFO, F("VEML : 6030/7700 Initialized."));

    initialized = true;
  } else {
    addLog(LOG_LEVEL_ERROR, F("VEML : 6030/7700 Init ERROR."));
  }

  return isInitialized();
}

/*****************************************************
* plugin_read
*****************************************************/
bool P168_data_struct::plugin_read(struct EventStruct *event)           {
  bool success = false;

  if (isInitialized() && veml->readReady()) {
    uint16_t amb = veml->readALS();
    float    lux = veml->readLux(static_cast<luxMethod>(_readMethod));
    uint16_t whi = veml->readWhite();

    if (luxMethod::VEML_LUX_AUTO == static_cast<luxMethod>(_readMethod)) {
      addLog(LOG_LEVEL_INFO, strformat(F("VEML : 6030/7700 AutoLux, Lux: %.2f, Gain: %.3f, Integration: %d"),
                                       lux, veml->getGainValue(), veml->getIntegrationTimeValue()));
    }
    UserVar.setFloat(event->TaskIndex, 0, lux);
    UserVar.setFloat(event->TaskIndex, 1, whi);
    UserVar.setFloat(event->TaskIndex, 2, amb);

    success = true;
  }

  return success;
}

/*****************************************************
* plugin_get_config_value
*****************************************************/
bool P168_data_struct::plugin_get_config_value(struct EventStruct *event,
                                               String            & string) {
  bool success = false;

  if (isInitialized()) {
    const String val = parseString(string, 1, '.');

    if (equals(val, F("gain"))) {
      string  = toString(veml->getGainValue(), 3);
      success = true;
    } else
    if (equals(val, F("integration"))) {
      string  = veml->getIntegrationTimeValue();
      success = true;
    }
  }

  return success;
}

#endif // ifdef USES_P168
