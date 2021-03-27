#ifndef PLUGINSTRUCTS_P111_DATA_STRUCT_H
#define PLUGINSTRUCTS_P111_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P111

#include <MFRC522.h>

struct P111_data_struct : public PluginTaskData_base {

  P111_data_struct(byte csPin, byte rstPin);
  void init();
  byte readCardStatus(unsigned long *key, bool *removedTag);
  String getCardName();

  MFRC522 *mfrc522;

  byte counter = 0;

private:
  bool reset(int8_t csPin, int8_t resetPin);
  byte readPassiveTargetID(uint8_t *uid, uint8_t *uidLength);

  byte _csPin;
  byte _rstPin;

  byte errorCount   = 0;
  bool removedState = true; // On startup, there will usually not be a tag nearby
};

#endif // ifdef USES_P111
#endif // ifndef PLUGINSTRUCTS_P111_DATA_STRUCT_H
