#include "../DataTypes/LogLevels.h"

#include "../DataTypes/FormSelectorOptions.h"

constexpr int logLevels[] =
{
  LOG_LEVEL_NONE,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_INFO

#ifndef BUILD_NO_DEBUG
  , LOG_LEVEL_DEBUG
  , LOG_LEVEL_DEBUG_MORE
  , LOG_LEVEL_DEBUG_DEV
#endif // ifndef BUILD_NO_DEBUG

};

// We can't have a globally declared flash string array on ESP8266, so need to wrap it in a function.
const __FlashStringHelper* getLogLevelDisplayStringFromIndex(uint8_t index)
{
  const __FlashStringHelper *logLevelDisplayStrings[]
  {
    F("None"),
    F("Error"),
    F("Info")
#ifndef BUILD_NO_DEBUG
    , F("Debug")
    , F("Debug More")
    , F("Debug dev")
#endif // ifndef BUILD_NO_DEBUG
  };

  if (index < NR_ELEMENTS(logLevelDisplayStrings)) {
    return logLevelDisplayStrings[index];
  }
  return F("");
}

uint8_t                    LOG_LEVEL_NRELEMENTS() { return NR_ELEMENTS(logLevels); }

const __FlashStringHelper* getLogLevelDisplayString(int logLevel) {
  for (uint8_t i = 0; i < NR_ELEMENTS(logLevels); ++i) {
    if (logLevels[i] == logLevel) {
      return getLogLevelDisplayStringFromIndex(i);
    }
  }
  return F("");
}

const __FlashStringHelper* getLogLevelDisplayStringFromIndex(uint8_t index, int& logLevel) {
  if (index < NR_ELEMENTS(logLevels)) {
    logLevel = logLevels[index];
    return getLogLevelDisplayStringFromIndex(index);
  }
  logLevel = -1;
  return F("");
}

void addLogLevelFormSelectorOptions(const String& id, int choice)
{
  const __FlashStringHelper *logLevelDisplayStrings[]
  {
    F("None"),
    F("Error"),
    F("Info")
#ifndef BUILD_NO_DEBUG
    , F("Debug")
    , F("Debug More")
    , F("Debug dev")
#endif // ifndef BUILD_NO_DEBUG
  };

  const FormSelectorOptions selector(NR_ELEMENTS(logLevels), logLevelDisplayStrings, logLevels);

  selector.addSelector(id, choice);
}
