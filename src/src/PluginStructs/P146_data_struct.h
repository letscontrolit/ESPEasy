#ifndef PLUGINSTRUCTS_P146_DATA_STRUCT_H
#define PLUGINSTRUCTS_P146_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P146


# include "../DataStructs/ESPEasyControllerCache_CSV_dumper.h"
# include <list>

# define P146_Nlines                            2
# define P146_Nchars                            128
# define P146_TaskInfoTopicIndex                0
# define P146_PublishTopicIndex                 1


# define P146_TASKVALUE_FILENR  UserVar[event->BaseVarIndex + 0]
# define P146_TASKVALUE_FILEPOS UserVar[event->BaseVarIndex + 1]

# define P146_GET_SEND_BINARY       bitRead(PCONFIG(0), 0)
# define P146_SET_SEND_BINARY(X)    bitWrite(PCONFIG(0), 0, X)

# define P146_GET_SEND_BULK         bitRead(PCONFIG(0), 1)
# define P146_SET_SEND_BULK(X)      bitWrite(PCONFIG(0), 1, X)

# define P146_GET_SEND_TIMESTAMP    bitRead(PCONFIG(0), 2)
# define P146_SET_SEND_TIMESTAMP(X) bitWrite(PCONFIG(0), 2, X)

# define P146_GET_SEND_READ_POS     bitRead(PCONFIG(0), 3)
# define P146_SET_SEND_READ_POS(X)  bitWrite(PCONFIG(0), 3, X)

# define P146_GET_JOIN_TIMESTAMP    bitRead(PCONFIG(0), 4)
# define P146_SET_JOIN_TIMESTAMP(X) bitWrite(PCONFIG(0), 4, X)

# define P146_GET_ONLY_SET_TASKS    bitRead(PCONFIG(0), 5)
# define P146_SET_ONLY_SET_TASKS(X) bitWrite(PCONFIG(0), 5, X)

# define P146_GET_ERASE_BINFILES    bitRead(PCONFIG(0), 6)
# define P146_SET_ERASE_BINFILES(X) bitWrite(PCONFIG(0), 6, X)


# define P146_SEPARATOR_CHARACTER   PCONFIG(1)


# define P146_MINIMAL_SEND_INTERVAL             PCONFIG_LONG(0)
# define P146_MQTT_MESSAGE_LENGTH               PCONFIG_LONG(1)
# define P146_MQTT_SEND_TASKVALUENAMES_INTERVAL PCONFIG_LONG(2)


struct P146_data_struct : public PluginTaskData_base {
public:

  P146_data_struct(struct EventStruct *event);

  virtual ~P146_data_struct();

  uint32_t sendTaskInfoInBulk(struct EventStruct *event) const;

  uint32_t sendBinaryInBulk(taskIndex_t P146_TaskIndex,
                            uint32_t    messageSize) const;

  uint32_t sendCSVInBulk(taskIndex_t P146_TaskIndex,
                         uint32_t    maxMessageSize);

  bool     prepareCSVInBulk(taskIndex_t P146_TaskIndex,
                            bool        joinTimestamp,
                            bool        onlySetTasks,
                            char        separator);

  static bool prepare_BulkMQTT_message(taskIndex_t P146_TaskIndex);

  static bool sendViaOriginalTask(taskIndex_t P146_TaskIndex,
                                  bool        sendTimestamp);

  static bool setPeekFilePos(int peekFileNr,
                             int peekReadPos);

  static void flush();

private:

  bool getPeekFilePos(int& peekFileNr,
                      int& peekReadPos,
                      int& peekFileSize) const;

  String getTopic(int         index,
                  taskIndex_t P146_TaskIndex) const;

  String _topics[P146_Nlines];

  ESPEasyControllerCache_CSV_dumper *dumper = nullptr;

  std::list<ESPEasyControllerCache_CSV_element>lines;
};


#endif // ifdef USES_P146
#endif // ifndef PLUGINSTRUCTS_P146_DATA_STRUCT_H
