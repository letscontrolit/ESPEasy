#ifndef PLUGINSTRUCTS_P020_DATA_STRUCT_H
#define PLUGINSTRUCTS_P020_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P020

# include <ESPeasySerial.h>

# ifndef PLUGIN_020_DEBUG
  #  define PLUGIN_020_DEBUG                 false // extra logging in serial out
# endif // ifndef PLUGIN_020_DEBUG

# define P020_STATUS_LED                    12
# define P020_DATAGRAM_MAX_SIZE             256
struct P020_Task : public PluginTaskData_base {
  P020_Task(taskIndex_t taskIndex);
  ~P020_Task();

  inline static bool serverActive(WiFiServer *server);


  void               startServer(uint16_t portnumber);

  void               checkServer();
  
  void               stopServer();
  
  bool               hasClientConnected();
  
  void               discardClientIn();
  
  void               clearBuffer();

  void               serialBegin(const ESPEasySerialPort port,
                                 int16_t                 rxPin,
                                 int16_t                 txPin,
                                 unsigned long           baud,
                                 byte                    config);

  void serialEnd();

  void handleSerialIn(struct EventStruct *event);
  void handleClientIn(struct EventStruct *event);
  void rulesEngine(String message);

  void discardSerialIn();

  bool isInit() const;

  void sendConnectedEvent(bool connected);

  WiFiServer    *ser2netServer = nullptr;
  uint16_t       gatewayPort   = 0;
  WiFiClient     ser2netClient;
  bool           clientConnected = false;
  String         serial_buffer;
  String         net_buffer;
  int            checkI            = 0;
  ESPeasySerial *ser2netSerial     = nullptr;
  byte           serial_processing = 0;
  taskIndex_t    _taskIndex = INVALID_TASK_INDEX;
};

#endif // ifdef USES_P020
#endif // ifndef PLUGINSTRUCTS_P020_DATA_STRUCT_H
