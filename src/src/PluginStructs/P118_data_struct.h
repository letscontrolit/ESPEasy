#ifndef PLUGINSTRUCTS_P118_DATA_STRUCT_H
#define PLUGINSTRUCTS_P118_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P118

# include "../../ESPEasy-Globals.h"

# include <SPI.h>
# include "IthoCC1101.h"
# include "IthoPacket.h"

# define P118_DEBUG_LOG         // Enable for some (extra) logging
# define P118_FEATURE_ORCON   1 // Enable use of Orcon commands

# if defined(LIMIT_BUILD_SIZE) && defined(P118_DEBUG_LOG)
#  undef P118_DEBUG_LOG
# endif // if defined(LIMIT_BUILD_SIZE) && defined(P118_DEBUG_LOG)
# ifdef LIMIT_BUILD_SIZE
// #  if P118_FEATURE_ORCON
// #   undef P118_FEATURE_ORCON
// #   define P118_FEATURE_ORCON   0
// #  endif // if P118_FEATURE_ORCON
# endif // ifdef LIMIT_BUILD_SIZE

# define P118_TIMEOUT_LIMIT   5000 // If initialization takes > 5 seconds, most likely the hardware is not correctly connected

# define P118_CSPIN           PIN(1)
# define P118_IRQPIN          PIN(0)
# define P118_CONFIG_LOG      PCONFIG(0)
# define P118_CONFIG_DEVID1   PCONFIG(1)
# define P118_CONFIG_DEVID2   PCONFIG(2)
# define P118_CONFIG_DEVID3   PCONFIG(3)
# define P118_CONFIG_RF_LOG   PCONFIG(4)
# define P118_CONFIG_ORCON    PCONFIG(5)

// Timer values for hardware timer in Fan in seconds
# define PLUGIN_118_Time1     (10 * 60)
# define PLUGIN_118_Time2     (20 * 60)
# define PLUGIN_118_Time3     (30 * 60)

// For reference only as the Orcon ventilation unit runs the timer inside
# if P118_FEATURE_ORCON
#  define PLUGIN_118_OrconTime0       (12 * 60 * 60)
#  define PLUGIN_118_OrconTime1       (60 * 60)
#  define PLUGIN_118_OrconTime2       (13 * 60 * 60)
#  define PLUGIN_118_OrconTime3       (60 * 60)
# endif // if P118_FEATURE_ORCON


// This extra settings struct is needed because the default settingsstruct doesn't support strings
struct PLUGIN_118_ExtraSettingsStruct {
  char ID1[9] = { 0 };
  char ID2[9] = { 0 };
  char ID3[9] = { 0 };
};

struct P118_data_struct : public PluginTaskData_base {
public:

  P118_data_struct(int8_t csPin,
                   int8_t irqPin,
                   bool   logData,
                   bool   rfLog);

  P118_data_struct() = delete;
  virtual ~P118_data_struct();

  bool plugin_init(struct EventStruct *event);
  bool plugin_exit(struct EventStruct *event);
  bool plugin_once_a_second(struct EventStruct *event);
  bool plugin_fifty_per_second(struct EventStruct *event);
  bool plugin_read(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);

private:

  void ITHOcheck();
  void PublishData(struct EventStruct *event);
  void PluginWriteLog(const String& command);

  bool isInitialized() {
    return _rf != nullptr;
  }

  # if P118_FEATURE_ORCON
  void SetDestIDSrcID(struct EventStruct *event,
                      uint8_t (&srcID)[3],
                      uint8_t (&destID)[3],
                      char(&tmpTmpID)[9]);
  # endif // if P118_FEATURE_ORCON

  IthoCC1101 *_rf = nullptr;

  // extra for interrupt handling
  bool _ITHOhasPacket  = false;
  int  _State          = 1; // after startup it is assumed that the fan is running low
  int  _OldState       = 1;
  int  _Timer          = 0;
  int  _LastIDindex    = 0;
  int  _OldLastIDindex = 0;
  bool _InitRunned     = false;

  int8_t _csPin  = -1;
  int8_t _irqPin = -1;
  bool   _log    = false;
  bool   _rfLog  = false;

  PLUGIN_118_ExtraSettingsStruct _ExtraSettings;

  volatile bool _Int = false;

  static void ISR_ithoCheck(P118_data_struct *self) ICACHE_RAM_ATTR;
};
#endif // ifdef USES_P118
#endif // ifndef PLUGINSTRUCTS_P118_DATA_STRUCT_H
