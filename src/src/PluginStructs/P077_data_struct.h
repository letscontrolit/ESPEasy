#ifndef PLUGINSTRUCTS_P077_DATA_STRUCT_H
#define PLUGINSTRUCTS_P077_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P077
#include "../Helpers/OversamplingHelper.h"

# define CSE_NOT_CALIBRATED          0xAA
# define CSE_PULSES_NOT_INITIALIZED  -1
# define CSE_PREF                    1000
# define CSE_UREF                    100
# define CSE_PREF_PULSE              12530 // was 4975us = 201Hz = 1000W
# define CSE_UREF_PULSE              1950  // was 1666us = 600Hz = 220V
# define CSE_IREF_PULSE              3500  // was 1666us = 600Hz = 4.545A

# define P077_NR_OUTPUT_VALUES   VARS_PER_TASK


# define P077_UREF       PCONFIG(0)
# define P077_UREF_LABEL PCONFIG_LABEL(0)
# define P077_IREF       PCONFIG(1)
# define P077_IREF_LABEL PCONFIG_LABEL(1)
# define P077_PREF       PCONFIG(2)
# define P077_PREF_LABEL PCONFIG_LABEL(2)


# define P077_QUERY1_CONFIG_POS  3
# define P077_QUERY1         PCONFIG(3) // P077_QUERY1_CONFIG_POS
# define P077_QUERY2         PCONFIG(4) // P077_QUERY1_CONFIG_POS + 1
# define P077_QUERY3         PCONFIG(5) // P077_QUERY1_CONFIG_POS + 2
# define P077_QUERY4         PCONFIG(6) // P077_QUERY1_CONFIG_POS + 3


enum class P077_query : uint8_t {
  P077_QUERY_VOLTAGE        = 0,
  P077_QUERY_ACTIVE_POWER   = 1,
  P077_QUERY_CURRENT        = 2,
  P077_QUERY_PULSES         = 3,
  P077_QUERY_KWH            = 4,
  P077_QUERY_VA             = 5,
  P077_QUERY_PF             = 6,
  P077_QUERY_REACTIVE_POWER = 7,


  P077_QUERY_NR_OUTPUT_OPTIONS
};

const __FlashStringHelper* Plugin_077_valuename(P077_query value_nr,
                                                bool       displayString);

P077_query                 Plugin_077_from_valuename(const String& valuename);

struct P077_data_struct : public PluginTaskData_base {
public:

  P077_data_struct() {}

  ~P077_data_struct();

  uint32_t get_24bit_value(uint8_t offset) const;

  bool     processCseReceived(struct EventStruct *event);

  bool     processSerialData();

  bool     checksumMatch() const;

  bool     init(ESPEasySerialPort port,
                const int16_t     serial_rx,
                const int16_t     serial_tx,
                unsigned long     baudrate,
                uint8_t           config);

  bool plugin_read(struct EventStruct *event);

  bool plugin_write(struct EventStruct *event,
                    String              string);
  void reset();
  bool isInitialized() const;

  # ifndef BUILD_NO_DEBUG
  int  serial_Available();
  # endif // ifndef BUILD_NO_DEBUG

  void setOutputValue(struct EventStruct *event,
                      P077_query          outputType,
                      float               value);

  float getValue(P077_query outputType) const;

  //  uint8_t cse_receive_flag = 0;


  uint8_t  serial_in_buffer[24] = { 0 };
  long     power_cycle_first    = 0;
  uint32_t last_cf_pulses       = 0;
  uint32_t cf_pulses            = 0;
  float    cf_frequency         = (1e9f / 5364000);
  uint32_t last_cf_pulses_moment{};

private:
  OversamplingHelper<float> _cache[static_cast<uint8_t>(P077_query::P077_QUERY_NR_OUTPUT_OPTIONS)]{};

public:
  // stats
  long     t_max       = 0;
  long     t_all       = 0;
  long     t_pkt       = 0;
  long     t_pkt_tmp   = 0;
  uint16_t count_bytes = 0;
  uint16_t count_max   = 0;
  uint16_t count_pkt   = 0;
  uint8_t  adjustment  = 0;

  bool newValue = false;

private:

  ESPeasySerial *easySerial = nullptr;
};

#endif // ifdef USES_P077
#endif // ifndef PLUGINSTRUCTS_P077_DATA_STRUCT_H
