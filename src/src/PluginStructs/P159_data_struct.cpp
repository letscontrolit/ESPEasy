#include "../PluginStructs/P159_data_struct.h"

#ifdef USES_P159

const __FlashStringHelper* Plugin_159_valuename(uint8_t value_nr,
                                                bool    displayString) {
  const __FlashStringHelper *strings[] { /*** ATTENTION: Don't change order of values as these are stored as user-selected!!! ***/
    F("Presence"), F("Presence"),
    F("Stationary Presence"), F("StationaryPresence"),
    F("Moving Presence"), F("MovingPresence"),
    F("Object distance"), F("Distance"),
    F("Stationary Object distance"), F("StationaryDistance"),
    F("Moving Object distance"), F("MovingDistance"),
    F("Stationary Object energy"), F("StationaryEnergy"),
    F("Moving Object energy"), F("MovingEnergy"),
    F("Ambient light sensor"), F("AmbientLight"),
    F("Stationary Object energy gate "), F("StatEnergyGate"),
    F("Moving Object energy gate "), F("MovingEnergyGate"),

    // TODO Add more values
  };
  const size_t index         = (2 * value_nr) + (displayString ? 0 : 1);
  constexpr size_t nrStrings = NR_ELEMENTS(strings);

  if (index < nrStrings) {
    return strings[index];
  }
  return F("");
}

P159_data_struct::P159_data_struct(ESPEasySerialPort portType,
                                   int8_t            rxPin,
                                   int8_t            txPin,
                                   bool              engineeringMode) {
  _engineeringMode = engineeringMode;

  // Try to open the associated serial port
  easySerial = new (std::nothrow) ESPeasySerial(portType, rxPin, txPin);

  if (nullptr != easySerial) {
    easySerial->begin(256000);
    radar = new (std::nothrow) ld2410();

    if (nullptr != radar) {
      if (radar->begin(*easySerial, false)) {
        bool rst = radar->requestRestart();

        // start initiated, now wait, next step: request configuration
        milestone = millis();
        state     = P159_state_e::Restarting;
        addLog(LOG_LEVEL_INFO, concat(F("LD2410: Restart sensor: "), rst ? 1 : 0));
      } else {
        delete radar;
        radar = nullptr;
      }
    }
  }
  addLog(LOG_LEVEL_INFO, concat(F("LD2410: Initialized: "), isValid() ? 1 : 0));
} // constructor

void P159_data_struct::disconnectSerial() {
  if (nullptr != easySerial) {
    delete easySerial;
    easySerial = nullptr;
  }

  if (nullptr != radar) {
    delete radar;
    radar = nullptr;
  }
} // disconnectSerial()

bool P159_data_struct::processSensor() {
  bool new_data = false;

  if (isValid()) {
    switch (state) {
      case P159_state_e::Initializing:
        addLog(LOG_LEVEL_INFO, F("LD2410: Initializing"));
        break;
      case P159_state_e::Restarting:

        if ((milestone > 0) && (timePassedSince(milestone) > P159_DELAY_RESTART)) {
          state     = P159_state_e::Configuring;
          milestone = 0;

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            radar->requestFirmwareVersion();
            addLog(LOG_LEVEL_INFO, concat(F("LD2410: Firmware version: "), radar->cmdFirmwareVersion()));

            const bool cfg = radar->requestCurrentConfiguration();
            addLog(LOG_LEVEL_INFO, concat(F("LD2410: Fetch configuration: "), cfg ? 1 : 0));

            if (cfg) {
              const uint8_t mGate   = radar->cfgMaxGate();
              const uint8_t mMvGate = radar->cfgMaxMovingGate();
              const uint8_t mStGate = radar->cfgMaxStationaryGate();
              const uint8_t mMax    = std::max(mGate, std::max(mMvGate, mStGate));
              addLog(LOG_LEVEL_INFO, strformat(F("LD2410: Sensor idle time: %d sec."), radar->cfgSensorIdleTimeInSeconds()));
              addLog(LOG_LEVEL_INFO, strformat(F("LD2410: Max. gate: %d, max. moving gate: %d, max. stationary gate: %d"),
                                               mGate, mMvGate, mStGate));

              for (uint8_t i = 0; i < mMax; ++i) {
                addLog(LOG_LEVEL_INFO, strformat(F("LD2410: Sensitivity, gate %d (%1.2f - %1.2f mtr): moving:%3d, stationary:%3d"),
                                                 i + 1, i * 0.75f, (i + 1) * 0.75f,
                                                 i < mMvGate ? radar->cfgMovingGateSensitivity(i) : 0,
                                                 i < mStGate ? radar->cfgStationaryGateSensitivity(i) : 0));
              }
            }
          }
        }
        break;
      case P159_state_e::Configuring:
        state = P159_state_e::Running;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          if (_engineeringMode) {
            addLog(LOG_LEVEL_INFO, concat(F("LD2410: Engineering mode: "), radar->requestStartEngineeringMode() ? 1 : 0));
          }
        }
        break;
      case P159_state_e::Running:
        new_data = radar->ld2410_loop();
        break;
    }
  } // isValid()

  return new_data;
}   // processSensor()

bool P159_data_struct::plugin_read(struct EventStruct *event) {
  bool result = false;

  if (isValid() && radar->isConnected()) {
    const int8_t valueCount = P159_NR_OUTPUT_VALUES;

    for (int8_t i = 0; i < valueCount; ++i) {
      const uint8_t pconfigIndex = i + P159_QUERY1_CONFIG_POS;
      bool isChanged             = false;
      UserVar[event->BaseVarIndex + i] = getRadarValue(event, i, PCONFIG(pconfigIndex), UserVar[event->BaseVarIndex + i], isChanged);

      result |= isChanged;
    }
  }
  return result;
} // plugin_read()

int P159_data_struct::getRadarValue(struct EventStruct *event,
                                    uint8_t             varIndex,
                                    int16_t             valueIndex,
                                    int                 previousValue,
                                    bool              & isChanged) {
  int result = previousValue;

  if (isValid()) {
    switch (valueIndex) {
      case P159_OUTPUT_PRESENCE:            result = radar->presenceDetected() ? 1 : 0; break;
      case P159_OUTPUT_STATIONARY_PRESENCE: result = radar->isStationary() ? 1 : 0; break;
      case P159_OUTPUT_MOVING_PRESENCE:     result = radar->isMoving() ? 1 : 0; break;
      case P159_OUTPUT_DISTANCE:            result = radar->detectionDistance(); break;
      case P159_OUTPUT_STATIONARY_DISTANCE: result = radar->stationaryTargetDistance(); break;
      case P159_OUTPUT_MOVING_DISTANCE:     result = radar->movingTargetDistance(); break;
      case P159_OUTPUT_STATIONARY_ENERGY:   result = radar->stationaryTargetEnergy(); break;
      case P159_OUTPUT_MOVING_ENERGY:       result = radar->movingTargetEnergy(); break;
    }

    if (_engineeringMode) {
      if ((valueIndex >= P159_OUTPUT_STATIC_DISTANCE_ENERGY_GATE1) && (valueIndex <= P159_OUTPUT_STATIC_DISTANCE_ENERGY_GATE8)) {
        result = radar->engStaticDistanceGateEnergy(valueIndex - P159_OUTPUT_STATIC_DISTANCE_ENERGY_GATE1);
      }
      else
      if ((valueIndex >= P159_OUTPUT_MOVING_DISTANCE_ENERGY_GATE1) && (valueIndex <= P159_OUTPUT_MOVING_DISTANCE_ENERGY_GATE8)) {
        result = radar->engMovingDistanceGateEnergy(valueIndex - P159_OUTPUT_MOVING_DISTANCE_ENERGY_GATE1);
      }
      else
      if (valueIndex == P159_OUTPUT_LIGHT_SENSOR) {
        result = radar->engLightSensorValue();
      }
    }
    isChanged = result != previousValue;
  } // isValid()
  return result;
}   // getRadarValue()

#endif // USES_P159
