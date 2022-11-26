#ifndef PLUGINSTRUCTS_P108_DATA_STRUCT_H
#define PLUGINSTRUCTS_P108_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P108


# define P108_DEV_ID         PCONFIG(0)
# define P108_DEV_ID_LABEL   PCONFIG_LABEL(0)
# define P108_MODEL          PCONFIG(1)
# define P108_MODEL_LABEL    PCONFIG_LABEL(1)
# define P108_BAUDRATE       PCONFIG(2)
# define P108_BAUDRATE_LABEL PCONFIG_LABEL(2)
# define P108_QUERY1         PCONFIG(3)
# define P108_QUERY2         PCONFIG(4)
# define P108_QUERY3         PCONFIG(5)
# define P108_QUERY4         PCONFIG(6)
# define P108_DEPIN          CONFIG_PIN3

# define P108_NR_OUTPUT_VALUES   VARS_PER_TASK
# define P108_QUERY1_CONFIG_POS  3

# define P108_QUERY_V       0
# define P108_QUERY_A       1
# define P108_QUERY_W       2
# define P108_QUERY_VA      3
# define P108_QUERY_PF      4
# define P108_QUERY_F       5
# define P108_QUERY_Wh_imp  6
# define P108_QUERY_Wh_exp  7
# define P108_QUERY_Wh_tot  8
# define P108_NR_OUTPUT_OPTIONS  9 // Must be the last one

# define P108_DEV_ID_DFLT   1      // Modbus communication address
# define P108_MODEL_DFLT    0      // DDS238
# define P108_BAUDRATE_DFLT 3      // 9600 baud
# define P108_QUERY1_DFLT   P108_QUERY_V
# define P108_QUERY2_DFLT   P108_QUERY_A
# define P108_QUERY3_DFLT   P108_QUERY_W
# define P108_QUERY4_DFLT   P108_QUERY_Wh_tot

# define P108_MEASUREMENT_INTERVAL 60000L

# include <ESPeasySerial.h>
# include "src/Helpers/Modbus_RTU.h"
# include "src/DataStructs/ESPEasy_packed_raw_data.h"


struct P108_data_struct : public PluginTaskData_base {
  P108_data_struct() = default;

  virtual ~P108_data_struct();

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


const __FlashStringHelper* Plugin_108_valuename(uint8_t value_nr,
                                                bool    displayString);

int                        p108_storageValueToBaudrate(uint8_t baudrate_setting);

float                      p108_readValue(uint8_t             query,
                                          struct EventStruct *event);

void                       p108_showValueLoadPage(uint8_t             query,
                                                  struct EventStruct *event);

#endif // ifdef USES_P108
#endif // ifndef PLUGINSTRUCTS_P108_DATA_STRUCT_H
