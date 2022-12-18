#ifndef PLUGINSTRUCTS_P146_DATA_STRUCT_H
#define PLUGINSTRUCTS_P146_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P146

struct P146_data_struct : public PluginTaskData_base {
public:

  P146_data_struct();

  virtual ~P146_data_struct();


static bool sendViaOriginalTask(taskIndex_t P146_TaskIndex, bool sendTimestamp);

static bool setPeekFilePos(int peekFileNr, int peekReadPos);

private:

  
  int readFileNr = 0;
  int readPos    = 0;



};


#endif // ifdef USES_P146
#endif // ifndef PLUGINSTRUCTS_P146_DATA_STRUCT_H
