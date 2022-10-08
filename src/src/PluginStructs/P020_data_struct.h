#ifndef PLUGINSTRUCTS_P020_DATA_STRUCT_H
#define PLUGINSTRUCTS_P020_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P020

# include <ESPeasySerial.h>

# ifndef PLUGIN_020_DEBUG
  #  define PLUGIN_020_DEBUG            false // when true: extra logging in serial out !?!?!
# endif // ifndef PLUGIN_020_DEBUG

# define P020_SERVER_PORT               ExtraTaskSettings.TaskDevicePluginConfigLong[0]
# define P020_BAUDRATE                  ExtraTaskSettings.TaskDevicePluginConfigLong[1]

# define P020_LED_PIN                   PCONFIG(0)
# define P020_SERIAL_CONFIG             PCONFIG(1)
# define P020_RX_WAIT                   PCONFIG(4)
# define P020_SERIAL_PROCESSING         PCONFIG(5)
# define P020_RESET_TARGET_PIN          PCONFIG(6)
# define P020_RX_BUFFER                 PCONFIG(7)

# define P020_FLAGS                     PCONFIG_ULONG(0)
# define P020_FLAG_IGNORE_CLIENT        0
# define P020_FLAG_MULTI_LINE           1
# define P020_FLAG_LED_ENABLED          2
# define P020_FLAG_LED_INVERTED         3
# define P020_FLAG_P044_MODE_SAVED      8
# define P020_IGNORE_CLIENT_CONNECTED   bitRead(P020_FLAGS, P020_FLAG_IGNORE_CLIENT)
# define P020_HANDLE_MULTI_LINE         bitRead(P020_FLAGS, P020_FLAG_MULTI_LINE)
# define P020_GET_LED_ENABLED           bitRead(P020_FLAGS, P020_FLAG_LED_ENABLED)
# define P020_GET_LED_INVERTED          bitRead(P020_FLAGS, P020_FLAG_LED_INVERTED)
# define P020_GET_P044_MODE_SAVED       bitRead(P020_FLAGS, P020_FLAG_P044_MODE_SAVED)

# define P020_DEFAULT_SERVER_PORT           1234
# define P020_DEFAULT_BAUDRATE              115200
# define P020_DEFAULT_RESET_TARGET_PIN      -1
# define P020_DEFAULT_RX_BUFFER             256

# define P020_STATUS_LED                    12
# define P020_DATAGRAM_MAX_SIZE             256

# define P020_DEFAULT_P044_SERVER_PORT      0
# define P020_DEFAULT_P044_BAUDRATE         9600

# define P020_CHECKSUM_LENGTH               4
# define P020_DATAGRAM_START_CHAR           '/'
# define P020_DATAGRAM_END_CHAR             '!'
# define P020_P1_DATAGRAM_MAX_SIZE          2048u

enum class P020_Events : uint8_t {
  None          = 0u,
  Generic       = 1u,
  RFLink        = 2u,
  P1WiFiGateway = 3u,
};

struct P020_Task : public PluginTaskData_base {
  enum class ParserState : uint8_t {
    WAITING,
    READING,
    CHECKSUM
  };

  P020_Task(struct EventStruct *event);
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
                                 uint8_t                 config);
  void                serialEnd();

  void                handleSerialIn(struct EventStruct *event);
  void                handleClientIn(struct EventStruct *event);
  void                discardSerialIn();
  void                rulesEngine(const String& message);

  bool                isInit() const;

  void                sendConnectedEvent(bool connected);

  void                blinkLED();
  void                checkBlinkLED();

  void                addChar(char ch);

  /*  checkDatagram
      checks whether the P020_CHECKSUM of the data received from P1 matches the P020_CHECKSUM
      attached to the telegram
   */
  bool                checkDatagram() const;

  /*
     CRC16
        based on code written by Jan ten Hove
       https://github.com/jantenhove/P1-Meter-ESP8266
   */
  static unsigned int CRC16(const String& buf,
                            int           len);

  /*
     validP1char
         Checks if the character is valid as part of the P1 datagram contents and/or checksum.
         Returns false on a datagram start ('/'), end ('!') or invalid character
   */
  static bool validP1char(char ch);
  bool        handleP1Char(char ch);

  WiFiServer    *ser2netServer = nullptr;
  uint16_t       gatewayPort   = 0;
  WiFiClient     ser2netClient;
  bool           clientConnected = false;
  String         serial_buffer;
  String         net_buffer;
  int            checkI            = 0;
  ESPeasySerial *ser2netSerial     = nullptr;
  P020_Events    serial_processing = P020_Events::None;
  taskIndex_t    _taskIndex        = INVALID_TASK_INDEX;
  bool           handleMultiLine   = false;

  unsigned long _blinkLEDStartTime = 0;
  int8_t        _ledPin            = -1;
  bool          _ledInverted       = false;
  bool          _ledEnabled        = false;
  bool          _CRCcheck          = false;
  size_t        _maxDataGramSize   = P020_DATAGRAM_MAX_SIZE;
  ParserState   state              = ParserState::WAITING;
};

#endif // ifdef USES_P020
#endif // ifndef PLUGINSTRUCTS_P020_DATA_STRUCT_H
