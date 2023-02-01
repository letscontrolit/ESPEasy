#ifndef PLUGINSTRUCTS_P146_DATA_STRUCT_H
#define PLUGINSTRUCTS_P146_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P146


# define P146_Nlines                            2
# define P146_Nchars                            128
# define P146_TaskInfoTopicIndex                0
# define P146_PublishTopicIndex                 1


struct P146_data_struct : public PluginTaskData_base {
public:

  P146_data_struct(taskIndex_t P146_TaskIndex);

  virtual ~P146_data_struct();

  uint32_t sendTaskInfoInBulk(taskIndex_t P146_TaskIndex,
                              uint32_t    maxMessageSize) const;

  uint32_t sendBinaryInBulk(taskIndex_t P146_TaskIndex,
                            uint32_t    messageSize) const;

  uint32_t sendCSVInBulk(taskIndex_t P146_TaskIndex,
                         bool        joinTimestamp,
                         bool        onlySetTasks,
                         char        separator) const;

  static bool prepareBinaryInBulk(taskIndex_t P146_TaskIndex,
                                  uint32_t    messageSize);

  static bool sendViaOriginalTask(taskIndex_t P146_TaskIndex,
                                  bool        sendTimestamp);

  static bool setPeekFilePos(int peekFileNr,
                             int peekReadPos);

  static void flush();

private:

  String getTopic(int         index,
                  taskIndex_t P146_TaskIndex) const;

  String _topics[P146_Nlines];

  /*
     int    readFileNr = 0;
     int    readPos    = 0;
   */
};


#endif // ifdef USES_P146
#endif // ifndef PLUGINSTRUCTS_P146_DATA_STRUCT_H
