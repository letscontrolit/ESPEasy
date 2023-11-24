#ifndef DATASTRUCTS_FACTORYDEFAULT_UNITNAME_NVS_H
#define DATASTRUCTS_FACTORYDEFAULT_UNITNAME_NVS_H

#include "../../ESPEasy_common.h"

#ifdef ESP32

# include "../Helpers/ESPEasy_NVS_Helper.h"


class FactoryDefault_UnitName_NVS {
private:

  void fromSettings();

  void applyToSettings() const;

public:

  bool applyToSettings_from_NVS(ESPEasy_NVS_Helper& preferences);

  void fromSettings_to_NVS(ESPEasy_NVS_Helper& preferences);

  void clear_from_NVS(ESPEasy_NVS_Helper& preferences);

private:

  // Used data:
  // byte 0: Unitnr
  // byte 1: flags
  // byte 2 ... 28 hostname
  uint8_t data[32]{};
};


#endif // ifdef ESP32


#endif // ifndef DATASTRUCTS_FACTORYDEFAULT_UNITNAME_NVS_H
