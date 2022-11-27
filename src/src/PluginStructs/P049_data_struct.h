#ifndef PLUGINSTRUCTS_P049_DATA_STRUCT_H
#define PLUGINSTRUCTS_P049_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P049

# define PLUGIN_READ_TIMEOUT   300

# define PLUGIN_049_FILTER_OFF        1
# define PLUGIN_049_FILTER_OFF_ALLSAMPLES 2
# define PLUGIN_049_FILTER_FAST       3
# define PLUGIN_049_FILTER_MEDIUM     4
# define PLUGIN_049_FILTER_SLOW       5

# include <ESPeasySerial.h>


# define P049_ABC_enabled  0x01
# define P049_ABC_disabled  0x02

// Uncomment the following define to enable the detection range commands:
// #define ENABLE_DETECTION_RANGE_COMMANDS

enum MHZ19Types {
  MHZ19_notDetected,
  MHZ19_A,
  MHZ19_B
};

enum mhzCommands : uint8_t { mhzCmdReadPPM,
                             mhzCmdCalibrateZero,
                             mhzCmdABCEnable,
                             mhzCmdABCDisable,
                             mhzCmdReset,
# ifdef ENABLE_DETECTION_RANGE_COMMANDS
                             mhzCmdMeasurementRange1000,
                             mhzCmdMeasurementRange2000,
                             mhzCmdMeasurementRange3000,
                             mhzCmdMeasurementRange5000
# endif // ifdef ENABLE_DETECTION_RANGE_COMMANDS
};


struct P049_data_struct : public PluginTaskData_base {
  P049_data_struct() = default;

  virtual ~P049_data_struct();

  void reset();

  bool init(ESPEasySerialPort port,
            const int16_t     serial_rx,
            const int16_t     serial_tx,
            bool              setABCdisabled);

  bool plugin_write(struct EventStruct *event,
                    const String      & string);

  bool isInitialized() const {
    return easySerial != nullptr;
  }

  void    setABCmode(int abcDisableSetting);

  uint8_t calculateChecksum() const;

  size_t  send_mhzCmd(uint8_t CommandId);

  bool    read_ppm(unsigned int& ppm,
                   signed int  & temp,
                   unsigned int& s,
                   float       & u);

  bool       receivedCommandAcknowledgement(bool& expectReset);

  String     getBufferHexDump() const;

  MHZ19Types getDetectedDevice() const;

  uint32_t      linesHandled       = 0;
  uint32_t      checksumFailed     = 0;
  uint32_t      sensorResets       = 0;
  uint32_t      nrUnknownResponses = 0;
  unsigned long lastInitTimestamp  = 0;

  ESPeasySerial *easySerial = nullptr;
  uint8_t        mhzResp[9] = { 0 }; // 9 uint8_t response buffer

  // Default of the sensor is to run ABC
  bool ABC_Disable     = false;
  bool ABC_MustApply   = false;
  bool modelA_detected = false;
  bool initTimePassed  = false;
};


bool Plugin_049_Check_and_ApplyFilter(unsigned int  prevVal,
                                      unsigned int& newVal,
                                      uint32_t      s,
                                      const int     filterValue,
                                      String      & log);


void P049_html_show_stats(struct EventStruct *event);

bool P049_perform_init(struct EventStruct *event);


#endif // ifdef USES_P049
#endif // ifndef PLUGINSTRUCTS_P049_DATA_STRUCT_H
