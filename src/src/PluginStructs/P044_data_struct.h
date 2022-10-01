#ifndef PLUGINSTRUCTS_P044_DATA_STRUCT_H
#define PLUGINSTRUCTS_P044_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P044

# include <ESPeasySerial.h>

// #define PLUGIN_044_DEBUG  // extra logging in serial out

# define P044_WIFI_SERVER_PORT      ExtraTaskSettings.TaskDevicePluginConfigLong[0]
# define P044_BAUDRATE              ExtraTaskSettings.TaskDevicePluginConfigLong[1]
# define P044_RX_WAIT               PCONFIG(0)
# define P044_SERIAL_CONFIG         PCONFIG(1)
# define P044_RESET_TARGET_PIN      CONFIG_PIN1
# define P044_LED_PIN               CONFIG_PIN2
# define P044_LED_ENABLED           PCONFIG(2)
# define P044_LED_INVERTED          PCONFIG(3)


# define P044_STATUS_LED                    12
# define P044_CHECKSUM_LENGTH               4
# define P044_DATAGRAM_START_CHAR           '/'
# define P044_DATAGRAM_END_CHAR             '!'
# define P044_DATAGRAM_MAX_SIZE             2048u


struct P044_Task : public PluginTaskData_base {
  enum class ParserState : uint8_t {
    WAITING,
    READING,
    CHECKSUM
  };

  P044_Task(struct EventStruct *event);

  ~P044_Task();

  inline static bool  serverActive(WiFiServer *server);


  void                startServer(uint16_t portnumber);

  void                checkServer();

  void                stopServer();

  bool                hasClientConnected();

  void                discardClientIn();

  void                blinkLED();

  void                checkBlinkLED();

  void                clearBuffer();

  void                addChar(char ch);

  /*  checkDatagram
      checks whether the P044_CHECKSUM of the data received from P1 matches the P044_CHECKSUM
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

  void        serialBegin(const ESPEasySerialPort port,
                          int16_t                 rxPin,
                          int16_t                 txPin,
                          unsigned long           baud,
                          uint8_t                 config);

  void serialEnd();

  void handleSerialIn(struct EventStruct *event);

  bool handleChar(char ch);

  void discardSerialIn();

  bool isInit() const;

  WiFiServer    *P1GatewayServer = nullptr;
  uint16_t       gatewayPort     = 0;
  WiFiClient     P1GatewayClient;
  bool           clientConnected = false;
  String         serial_buffer;
  ParserState    state             = ParserState::WAITING;
  int            checkI            = 0;
  boolean        CRCcheck          = false;
  ESPeasySerial *P1EasySerial      = nullptr;
  unsigned long  blinkLEDStartTime = 0;
  size_t         maxMessageSize    = P044_DATAGRAM_MAX_SIZE / 4;

  int8_t _ledPin      = P044_STATUS_LED; // Former default
  bool   _ledEnabled  = true;            // Former default
  bool   _ledInverted = false;
};

#endif // ifdef USES_P044
#endif // ifndef PLUGINSTRUCTS_P044_DATA_STRUCT_H
