#ifndef PLUGINSTRUCTS_P111_DATA_STRUCT_H
#define PLUGINSTRUCTS_P111_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P111

#include <MFRC522.h>

struct P111_data_struct : public PluginTaskData_base {

  P111_data_struct(uint8_t csPin, uint8_t rstPin);
  ~P111_data_struct();
  void init();
  uint8_t readCardStatus(unsigned long *key, bool *removedTag);
  String getCardName();

  MFRC522 *mfrc522 = nullptr;

  uint8_t counter = 0;

private:
  bool reset(int8_t csPin, int8_t resetPin);
  uint8_t readPassiveTargetID(uint8_t *uid, uint8_t *uidLength);

  uint8_t _csPin;
  uint8_t _rstPin;

  uint8_t errorCount   = 0;
  bool removedState = true; // On startup, there will usually not be a tag nearby
};

#endif // ifdef USES_P111
#endif // ifndef PLUGINSTRUCTS_P111_DATA_STRUCT_H
