#include "../PluginStructs/P159_data_struct.h"

#ifdef USES_P159

const __FlashStringHelper* Plugin_159_valuename(uint8_t value_nr,
                                                bool    displayString) {
  const __FlashStringHelper *strings[] { /*** ATTENTION: Don't change order of values as these are stored as user-selected!!! ***/
    F("Presence"), F("Presence"),
    F("Stationary Presence"), F("StatPresence"),
    F("Moving Presence"), F("MovPresence"),
    F("Object distance"), F("Distance"),
    F("Stationary Object distance"), F("StatDistance"),
    F("Moving Object distance"), F("MovDistance"),
    F("Stationary Object energy"), F("StatEnergy"),
    F("Moving Object energy"), F("MovEnergy"),
    F("Ambient light sensor"), F("AmbLight"),
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
    uint32_t iStart     = millis(); // FIXME Remove log
    P159_state_e sState = state;

    switch (state) {
      case P159_state_e::Initializing:
        addLog(LOG_LEVEL_INFO, F("LD2410: Initializing"));
        break;
      case P159_state_e::Restarting:

        if ((milestone > 0) && (timePassedSince(milestone) > P159_DELAY_RESTART)) {
          state     = P159_state_e::GetVersion;
          milestone = 0;
          radar->requestFirmwareVersion();
        }
        break;
      case P159_state_e::GetVersion:

        state = P159_state_e::GetConfiguration;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, concat(F("LD2410: Firmware version: "), radar->cmdFirmwareVersion()));
        }
        _configurationRead = radar->requestCurrentConfiguration();
        break;
      case P159_state_e::GetConfiguration:

        state = P159_state_e::Engineering;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, concat(F("LD2410: Fetch configuration: "), _configurationRead ? 1 : 0));

          if (_configurationRead) {
            const uint8_t mGate   = radar->cfgMaxGate();
            const uint8_t mMvGate = radar->cfgMaxMovingGate();
            const uint8_t mStGate = radar->cfgMaxStationaryGate();
            const uint8_t mMax    = std::max(mGate, std::max(mMvGate, mStGate));
            addLog(LOG_LEVEL_INFO, strformat(F("LD2410: Sensor idle time: %d sec."), radar->cfgSensorIdleTimeInSeconds()));
            addLog(LOG_LEVEL_INFO, strformat(F("LD2410: Max. gate: %d, max. moving gate: %d, max. stationary gate: %d"),
                                             mGate, mMvGate, mStGate));

            for (uint8_t i = 0; i <= mMax; ++i) {
              addLog(LOG_LEVEL_INFO, strformat(F("LD2410: Sensitivity, gate %d (%.2f - %.2f mtr): moving:%3d, stationary:%3d"),
                                               i, i * 0.75f, (i + 1) * 0.75f,
                                               i <= mMvGate ? radar->cfgMovingGateSensitivity(i) : 0,
                                               i <= mStGate ? radar->cfgStationaryGateSensitivity(i) : 0));
            }
          }
        }
        break;
      case P159_state_e::Engineering:
        state = P159_state_e::Running;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          if (_engineeringMode) {
            addLog(LOG_LEVEL_INFO, concat(F("LD2410: Engineering mode: "), radar->requestStartEngineeringMode() ? 1 : 0));
          }
          addLog(LOG_LEVEL_INFO, F("LD2410: Running mode."));
        }
        break;
      case P159_state_e::Running:
        new_data = radar->ld2410_loop();
        break;
    }

    if (P159_state_e::Running != sState) { // FIXME Remove log
      addLog(LOG_LEVEL_INFO, strformat(F("LD2410: Starting state: %d duration: %u msec."), static_cast<uint8_t>(sState), millis() - iStart));
    }
  } // isValid()

  return new_data;
}   // processSensor()

bool P159_data_struct::plugin_read(struct EventStruct *event) {
  bool result = false;

  if (isValid() && radar->isConnected()) {
    const int8_t valueCount = P159_NR_OUTPUT_VALUES;
    result = !P159_GET_UPDATE_DIFF_ONLY;

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

void P159_data_struct::addJavascript() {
  addHtml(F("<script type='text/javascript'>"
            "function chDis(s,m) {"
            "for(i=1;i<=m;i++){" // "console.log('pc_arg'+i);" // For debugging the js code
            "document.getElementById('pc_arg'+i).disabled=!s"
            "}"
            "};</script>"
            ));
}

bool P159_data_struct::plugin_webform_load(struct EventStruct *event) {
  bool result = false;

  if (isValid()) {
    if (!_configurationRead) {
      _configurationRead = radar->requestCurrentConfiguration();
    }

    if (_configurationRead) {
      const uint8_t  mGate    = radar->cfgMaxGate();
      const uint8_t  mMvGate  = radar->cfgMaxMovingGate();
      const uint8_t  mStGate  = radar->cfgMaxStationaryGate();
      const uint8_t  mMax     = std::max(mGate, std::max(mMvGate, mStGate));
      const uint16_t idleTime = radar->cfgSensorIdleTimeInSeconds();

      addJavascript();
      addFormCheckBox(F("Modify sensor settings"), F("saveSens"), false);

      int idx = 0;
      addFormNumericBox(F("Idle seconds"), getPluginCustomArgName(idx++), idleTime, 0, 65535, EMPTY_STRING, true);
      addUnit(F("0..65535 sec."));
      addFormNumericBox(F("Max. Moving gates"), getPluginCustomArgName(idx++), mMvGate, 2, 8, EMPTY_STRING, true);
      addUnit(F("2..8"));
      addFormNumericBox(F("Max. Stationary gates"), getPluginCustomArgName(idx++), mStGate, 2, 8, EMPTY_STRING, true);
      addUnit(F("2..8"));

      addRowLabel(F("Sensitivity"));
      html_table(EMPTY_STRING, false); // Sub-table
      html_table_header(F("Gate"),       200);
      html_table_header(F("Moving"),     100);
      html_table_header(F("Stationary"), 100);

      for (uint8_t i = 0; i <= mMax; ++i) {
        html_TR_TD();
        addHtml(strformat(F("Gate %d (%.2f - %.2f mtr)"), i, i * 0.75f, (i + 1) * 0.75f));
        html_TD();
        addNumericBox(getPluginCustomArgName(idx++), radar->cfgMovingGateSensitivity(i), 0, 100, true);
        html_TD();
        addNumericBox(getPluginCustomArgName(idx++), radar->cfgStationaryGateSensitivity(i), 0, 100, true);
      }
      html_end_table();
      addHtml(strformat(F("\n<script type='text/javascript'>document.getElementById('saveSens')."
                          "onclick=function(){chDis(this.checked,%d)};</script>"),
                        idx));
      result = true;
    }
  }
  return result;
}

bool P159_data_struct::plugin_webform_save(struct EventStruct *event) {
  bool result       = false;
  const bool doSave = isFormItemChecked(F("saveSens"));

  if (isValid() && doSave) {
    int idx              = 0;
    const uint16_t idle  = getFormItemInt(getPluginCustomArgName(idx++));
    const uint8_t  gMove = getFormItemInt(getPluginCustomArgName(idx++));
    const uint8_t  gStat = getFormItemInt(getPluginCustomArgName(idx++));
    addLog(LOG_LEVEL_INFO, F("LD2410: Save sensitivity settings to sensor, start..."));
    radar->requestConfigurationModeBegin();
    radar->setMaxValues(gMove, gStat, idle);
    const uint16_t maxGate = radar->cfgMaxGate();

    for (uint16_t gate = 0; gate <= maxGate; ++gate) {
      const uint16_t sMove = getFormItemInt(getPluginCustomArgName(idx++));
      const uint16_t sStat = getFormItemInt(getPluginCustomArgName(idx++));

      // Set sensitivity (level) to 100 to effectively disable sensitivity
      radar->setGateSensitivityThreshold(gate, gate <= gMove ? sMove : 100, gate <= gStat ? sStat : 100);
    }
    radar->requestConfigurationModeEnd();
    addLog(LOG_LEVEL_INFO, F("LD2410: Save sensitivity settings to sensor, done."));
  }
  return result;
}

#endif // USES_P159
