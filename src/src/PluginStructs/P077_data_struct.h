#ifndef PLUGINSTRUCTS_P077_DATA_STRUCT_H
#define PLUGINSTRUCTS_P077_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P077


# define CSE_NOT_CALIBRATED          0xAA
# define CSE_PULSES_NOT_INITIALIZED  -1
# define CSE_PREF                    1000
# define CSE_UREF                    100
# define CSE_PREF_PULSE              12530 // was 4975us = 201Hz = 1000W
# define CSE_UREF_PULSE              1950  // was 1666us = 600Hz = 220V
# define CSE_IREF_PULSE              3500  // was 1666us = 600Hz = 4.545A

struct P077_data_struct : public PluginTaskData_base {
public:

  P077_data_struct() {}

  ~P077_data_struct();

  bool processCseReceived(struct EventStruct *event);

  bool processSerialData();

  bool init(ESPEasySerialPort port,
            const int16_t     serial_rx,
            const int16_t     serial_tx,
            unsigned long     baudrate,
            uint8_t           config);

  bool plugin_write(struct EventStruct *event,
                    String              string);
  void reset();
  bool isInitialized() const;

  # ifndef BUILD_NO_DEBUG
  int  serial_Available();
  # endif // ifndef BUILD_NO_DEBUG

  //  uint8_t cse_receive_flag = 0;

  uint8_t serial_in_buffer[32] = { 0 };
  long    voltage_cycle        = 0;
  long    current_cycle        = 0;
  long    power_cycle          = 0;
  long    power_cycle_first    = 0;
  long    cf_pulses            = 0;
  long    cf_pulses_last_time  = CSE_PULSES_NOT_INITIALIZED;
  long    cf_frequency         = 0;
  float   energy_voltage       = 0; // 123.1 V
  float   energy_current       = 0; // 123.123 A
  float   energy_power         = 0; // 123.1 W

  // stats
  long     t_max       = 0;
  long     t_all       = 0;
  long     t_pkt       = 0;
  long     t_pkt_tmp   = 0;
  uint16_t count_bytes = 0;
  uint16_t count_max   = 0;
  uint16_t count_pkt   = 0;
  uint8_t  checksum    = 0;
  uint8_t  adjustment  = 0;

private:

  ESPeasySerial *easySerial = nullptr;
};

#endif // ifdef USES_P077
#endif // ifndef PLUGINSTRUCTS_P077_DATA_STRUCT_H
