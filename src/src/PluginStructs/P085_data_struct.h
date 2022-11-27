#ifndef PLUGINSTRUCTS_P085_DATA_STRUCT_H
#define PLUGINSTRUCTS_P085_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P085


# define P085_DEV_ID         PCONFIG(0)
# define P085_DEV_ID_LABEL   PCONFIG_LABEL(0)
# define P085_MODEL          PCONFIG(1)
# define P085_MODEL_LABEL    PCONFIG_LABEL(1)
# define P085_BAUDRATE       PCONFIG(2)
# define P085_BAUDRATE_LABEL PCONFIG_LABEL(2)
# define P085_QUERY1         PCONFIG(3)
# define P085_QUERY2         PCONFIG(4)
# define P085_QUERY3         PCONFIG(5)
# define P085_QUERY4         PCONFIG(6)
# define P085_DEPIN          CONFIG_PIN3

# define P085_NR_OUTPUT_VALUES   VARS_PER_TASK
# define P085_QUERY1_CONFIG_POS  3

# define P085_QUERY_V       0
# define P085_QUERY_A       1
# define P085_QUERY_W       2
# define P085_QUERY_Wh_imp  3
# define P085_QUERY_Wh_exp  4
# define P085_QUERY_Wh_tot  5
# define P085_QUERY_Wh_net  6
# define P085_QUERY_h_tot   7
# define P085_QUERY_h_load  8
# define P085_NR_OUTPUT_OPTIONS   9 // Must be the last one

# define P085_DEV_ID_DFLT   1       // Modbus communication address
# define P085_MODEL_DFLT    0       // AcuDC24x
# define P085_BAUDRATE_DFLT 4       // 19200 baud
# define P085_QUERY1_DFLT   P085_QUERY_V
# define P085_QUERY2_DFLT   P085_QUERY_A
# define P085_QUERY3_DFLT   P085_QUERY_W
# define P085_QUERY4_DFLT   P085_QUERY_Wh_tot

# define P085_MEASUREMENT_INTERVAL 60000L

# include <ESPeasySerial.h>
# include "src/Helpers/Modbus_RTU.h"
# include "src/DataStructs/ESPEasy_packed_raw_data.h"

struct P085_data_struct : public PluginTaskData_base {
  P085_data_struct() = default;

  virtual ~P085_data_struct();

  void reset();

  bool init(ESPEasySerialPort port,
            const int16_t     serial_rx,
            const int16_t     serial_tx,
            int8_t            dere_pin,
            unsigned int      baudrate,
            uint8_t           modbusAddress);

  bool isInitialized() const;

  ModbusRTU_struct modbus;
};


const __FlashStringHelper* Plugin_085_valuename(uint8_t value_nr,
                                                bool    displayString);

int                        p085_storageValueToBaudrate(uint8_t baudrate_setting);

float                      p085_readValue(uint8_t             query,
                                          struct EventStruct *event);

void                       p085_showValueLoadPage(uint8_t             query,
                                                  struct EventStruct *event);


#endif // ifdef USES_P085
#endif // ifndef PLUGINSTRUCTS_P085_DATA_STRUCT_H
