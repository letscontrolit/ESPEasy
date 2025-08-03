#ifndef PLUGINSTRUCTS_P140_DATA_STRUCT_H
#define PLUGINSTRUCTS_P140_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P140

# define P140_DEBUG                 0 // Enable/disable development debug logging

# define P140_I2C_ADDR              0x5F

# define P140_SEND_EVENTS           PCONFIG(0)
# define P140_EXEC_COMMAND          PCONFIG(1)
# define P140_GET_INPUT             PCONFIG(2)
# define P140_INPUT_BUFFER_SIZE     130

struct P140_data_struct : public PluginTaskData_base {
public:

  P140_data_struct(struct EventStruct *event);

  P140_data_struct() = delete;
  virtual ~P140_data_struct();

  bool plugin_read(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    String            & string);
  bool getBufferValue(String& string);
  bool plugin_ten_per_second(struct EventStruct *event);
  void clear();

private:

  bool _events    = false;
  bool _exec      = false;
  bool _input     = false;
  int  _inCounter = 0;
  char _buffer[P140_INPUT_BUFFER_SIZE]{};
};

#endif // ifdef USES_P140
#endif // ifndef PLUGINSTRUCTS_P140_DATA_STRUCT_H
