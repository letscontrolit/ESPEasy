#include "../DataStructs/FactoryDefault_UnitName_NVS.h"

#ifdef ESP32

# include "../Globals/Settings.h"
# include "../Helpers/StringConverter.h"

// Max. 15 char keys for ESPEasy Factory Default marked keys
# define FACTORY_DEFAULT_NVS_UNIT_NAME_KEY  "UnitName"


void FactoryDefault_UnitName_NVS::fromSettings() {
  bitWrite(data[1], 0, Settings.appendUnitToHostname());
  data[0] = Settings.Unit;
  memcpy((char *)(data + 2), Settings.Name, sizeof(Settings.Name));
  data[2 + sizeof(Settings.Name)] = Settings.UDPPort >> 8;
  data[3 + sizeof(Settings.Name)] = Settings.UDPPort & 0xFF;
}

void FactoryDefault_UnitName_NVS::applyToSettings() const {
  Settings.appendUnitToHostname(bitRead(data[1], 0));
  Settings.Unit = data[0];
  memcpy(Settings.Name, (char *)(data + 2), sizeof(Settings.Name));

  Settings.UDPPort = data[2 + sizeof(Settings.Name)] << 8 | data[3 + sizeof(Settings.Name)];
}

bool FactoryDefault_UnitName_NVS::applyToSettings_from_NVS(ESPEasy_NVS_Helper& preferences) {
  if (preferences.getPreference(F(FACTORY_DEFAULT_NVS_UNIT_NAME_KEY), data, sizeof(data))) {
    applyToSettings();
    return true;
  }
  return false;
}

void FactoryDefault_UnitName_NVS::fromSettings_to_NVS(ESPEasy_NVS_Helper& preferences) {
  fromSettings();
  preferences.setPreference(F(FACTORY_DEFAULT_NVS_UNIT_NAME_KEY), data, sizeof(data));
}

void FactoryDefault_UnitName_NVS::clear_from_NVS(ESPEasy_NVS_Helper& preferences) {
  preferences.remove(F(FACTORY_DEFAULT_NVS_UNIT_NAME_KEY));
}

#endif // ifdef ESP32
