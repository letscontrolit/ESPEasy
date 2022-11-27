#ifndef PLUGINSTRUCTS_P111_DATA_STRUCT_H
#define PLUGINSTRUCTS_P111_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P111

# include <MFRC522.h>

# define P111_CS_PIN            PIN(0)
# define P111_RST_PIN           PIN(1)
# define P111_TAG_AUTOREMOVAL   PCONFIG(0)
# define P111_SENDRESET         PCONFIG(1)
# define P111_REMOVALVALUE      PCONFIG_LONG(0)
# define P111_REMOVALTIMEOUT    PCONFIG_LONG(1)

# define P111_NO_ERROR          0
# define P111_ERROR_READ        1
# define P111_ERROR_NO_TAG      2
# define P111_ERROR_RESET_BUSY  3

# define P111_NO_KEY           0xFFFFFFFF

// #define P111_USE_REMOVAL      // Enable (real) Tag Removal detection options (but that won't work with MFRC522 reader)

enum class P111_initPhases : uint8_t {
  Ready       = 0x00,
  ResetDelay1 = 0x01,
  ResetDelay2 = 0x02,
  Undefined   = 0xFF
};

struct P111_data_struct : public PluginTaskData_base {
  P111_data_struct(int8_t csPin,
                   int8_t rstPin);
  P111_data_struct() = delete;
  virtual ~P111_data_struct();

  void init();
  bool plugin_ten_per_second(struct EventStruct *event);
  bool plugin_fifty_per_second();

private:

  MFRC522 *mfrc522 = nullptr;

  uint8_t counter = 0;

  String  getCardName();
  uint8_t readCardStatus(uint32_t *key,
                         bool     *removedTag);
  bool    reset(int8_t csPin,
                int8_t resetPin);
  uint8_t readPassiveTargetID(uint8_t *uid,
                              uint8_t *uidLength);

  int32_t timeToWait = 0;

  int8_t _csPin;
  int8_t _rstPin;

  uint8_t         errorCount   = 0;
  bool            removedState = true; // On startup, there will usually not be a tag nearby
  P111_initPhases initPhase    = P111_initPhases::Undefined;
};

#endif // ifdef USES_P111
#endif // ifndef PLUGINSTRUCTS_P111_DATA_STRUCT_H
