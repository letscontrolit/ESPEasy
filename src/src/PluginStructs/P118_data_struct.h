#ifndef PLUGINSTRUCTS_P118_DATA_STRUCT_H
#define PLUGINSTRUCTS_P118_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P118

# include "../../ESPEasy-Globals.h"

# include <SPI.h>
# include "IthoCC1101.h"
# include "IthoPacket.h"

# define P118_DEBUG_LOG                // Enable for some (extra) logging

# define P118_INTERUPT_HANDLER_COUNT 3 // This should only be changed if the Interrupt handlers are also adjusted

// Timer values for hardware timer in Fan in seconds
# define PLUGIN_118_Time1      10 * 60
# define PLUGIN_118_Time2      20 * 60
# define PLUGIN_118_Time3      30 * 60

// This extra settings struct is needed because the default settingsstruct doesn't support strings
struct PLUGIN__ExtraSettingsStruct {
  char ID1[9];
  char ID2[9];
  char ID3[9];
};

struct P118_data_struct : public PluginTaskData_base {
public:

  P118_data_struct(uint8_t logData);

  P118_data_struct() = delete;
  ~P118_data_struct();

  bool plugin_init(struct EventStruct *event);
  void plugin_init_part2();
  bool plugin_exit(struct EventStruct *event);
  bool plugin_once_a_second(struct EventStruct *event);
  bool plugin_fifty_per_second(struct EventStruct *event);
  bool plugin_read(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);
  void ITHOcheck();
  void PublishData(struct EventStruct *event);
  void PluginWriteLog(const String& command);

  bool isInitialized() {
    return PLUGIN_118_rf != nullptr;
  }

private:

  IthoCC1101 *PLUGIN_118_rf = nullptr;

  // extra for interrupt handling
  bool   PLUGIN_118_ITHOhasPacket  = false;
  int    PLUGIN_118_State          = 1; // after startup it is assumed that the fan is running low
  int    PLUGIN_118_OldState       = 1;
  int    PLUGIN_118_Timer          = 0;
  int    PLUGIN_118_LastIDindex    = 0;
  int    PLUGIN_118_OldLastIDindex = 0;
  int8_t Plugin_118_IRQ_pin        = -1;
  bool   PLUGIN_118_InitRunned     = false;
  bool   PLUGIN_118_Log            = false;

  PLUGIN__ExtraSettingsStruct PLUGIN_118_ExtraSettings;
};
#endif // ifdef USES_P118
#endif // ifndef PLUGINSTRUCTS_P118_DATA_STRUCT_H
