#ifndef PLUGINSTRUCTS_P075_DATA_STRUCT_H
#define PLUGINSTRUCTS_P075_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P075


# include <ESPeasySerial.h>


// Configuration Settings. Custom Configuration Memory must be less than 1024 Bytes (per TD'er findings).
// #define P75_Nlines 12            // Custom Config, Number of user entered Command Statment Lines. DO NOT USE!
// #define P75_Nchars 64            // Custom Config, Length of user entered Command Statment Lines. DO NOT USE!
# define P75_Nlines 20              // Custom Config, Number of user entered Command Statments.
# define P75_Nchars 51              // Custom Config, Length of user entered Command Statments.

// Nextion defines
# define RXBUFFSZ  64               // Serial RxD buffer (Local staging buffer and ESPeasySerial).
# define RXBUFFWARN (RXBUFFSZ - 16) // Warning, Rx buffer close to being full.
# define TOUCH_BASE 500             // Base offset for 0X65 Touch Event Send Component ID.

// Serial defines
# define P075_B9600    0
# define P075_B38400   1
# define P075_B57600   2
# define P075_B115200  3

# define P075_BaudRate         PCONFIG(1)
# define P075_IncludeValues    PCONFIG(2) // Web GUI checkbox flag; false = don't send idx & value data at interval.


struct P075_data_struct : public PluginTaskData_base {
  P075_data_struct(ESPEasySerialPort port,
                   int               rx,
                   int               tx,
                   uint32_t          baud);
  P075_data_struct() = delete;
  virtual ~P075_data_struct();

  void   loadDisplayLines(taskIndex_t taskIndex);

  String getLogString() const;

  ESPeasySerial *easySerial = nullptr;
  int            rxPin      = -1;
  int            txPin      = -1;
  uint32_t       baudrate   = 9600UL;

  String displayLines[P75_Nlines];
};

#endif // ifdef USES_P075
#endif // ifndef PLUGINSTRUCTS_P075_DATA_STRUCT_H
