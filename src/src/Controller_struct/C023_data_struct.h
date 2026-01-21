#ifndef CONTROLLER_STRUCT_C023_DATA_STRUCT_H
#define CONTROLLER_STRUCT_C023_DATA_STRUCT_H

#include "../Helpers/_CPlugin_Helper.h"

#ifdef USES_C023

# include "../Controller_config/C023_AT_commands.h"
# include "../Controller_config/C023_config.h"


class ESPeasySerial;


struct C023_data_struct {
public:

  C023_data_struct();

  ~C023_data_struct();

  void reset();

  bool init(
    const C023_ConfigStruct& config,
    taskIndex_t              sampleSet_Initiator);

  bool isInitialized() const {
    return _easySerial != nullptr;
  }

  bool hasJoined();

  bool useOTAA();

  bool command_finished() const;

  bool txUncnfBytes(const uint8_t *data,
                    uint8_t        size,
                    uint8_t        port);

  bool txHexBytes(const String& data,
                  uint8_t       port);

  bool txUncnf(const String& data,
               uint8_t       port);

  bool setDR(LoRa_Helper::LoRaWAN_DR dr);


  bool initOTAA(const String& AppEUI,
                const String& AppKey,
                const String& DevEUI);

  bool initABP(const String& addr,
               const String& AppSKey,
               const String& NwkSKey);

  bool join(bool    enable            = true, // Set to false for stop joining
            bool    autoJoin          = 0,    // for Auto-join on power up
            uint8_t reattemptInterval = 8,    // 7 - 255 seconds
            uint8_t nrJoinAttempts    = 1);   // No. of join attempts: 0 - 255

  String                  sendRawCommand(const String& command);

  int                     getVbat();

  String                  peekLastError();

  String                  getLastError();

  LoRa_Helper::LoRaWAN_DR getDataRate();
  String                  getDataRate_str();

  int                     getRSSI();

  uint32_t                getRawStatus();


  bool                    getFrameCounters(uint32_t& dnctr,
                                           uint32_t& upctr);

  bool                    setFrameCounters(uint32_t dnctr,
                                           uint32_t upctr);

  // Cached data, only changing occasionally.

  String  getDevaddr();

  String  hweui();

  String  sysver();

  uint8_t getSampleSetCount() const;

  uint8_t getSampleSetCount(taskIndex_t taskIndex);

  float   getLoRaAirTime(uint8_t pl);

  bool    async_loop(bool processingSetCommand = false);

  bool    writeCachedValues(KeyValueWriter          *writer,
                            C023_AT_commands::AT_cmd start,
                            C023_AT_commands::AT_cmd end);

private:

  bool   sendSetValue(C023_AT_commands::AT_cmd at_cmd,
                      const String           & value);
  bool   sendSetValue(C023_AT_commands::AT_cmd at_cmd,
                      int                      value);
  bool   sendSetValue(const String& value);


  String get(C023_AT_commands::AT_cmd at_cmd,
             uint32_t               & lastChange);
  int    getInt(C023_AT_commands::AT_cmd at_cmd,
                int                      errorvalue,
                uint32_t               & lastChange);
  float  getFloat(C023_AT_commands::AT_cmd at_cmd,
                  float                    errorvalue,
                  uint32_t               & lastChange);

  String get(C023_AT_commands::AT_cmd at_cmd);
  int    getInt(C023_AT_commands::AT_cmd at_cmd,
                int                      errorvalue);
  float  getFloat(C023_AT_commands::AT_cmd at_cmd,
                  float                    errorvalue);

  bool   processReceived(const String& receivedData,
                         bool          processingSetCommand);
  bool   processReceived_Dragino_LA66(const String           & receivedData,
                                      C023_AT_commands::AT_cmd at_cmd,
                                      bool                     processingSetCommand);
  bool   processReceived_RAK_3172(const String           & receivedData,
                                  C023_AT_commands::AT_cmd at_cmd,
                                  bool                     processingSetCommand);

  bool processPendingQuery(const String& receivedData);

  bool processDownlinkMessage(const String& receivedData,
                              bool          hexEncoded);

  void sendQuery(C023_AT_commands::AT_cmd at_cmd,
                 bool                     prioritize = false);

  void sendQuery(C023_AT_commands::AT_cmd at_cmd[],
                 size_t                   count);

  void sendNextQueuedQuery();

  bool queryPending() const {
    return _queryPending != C023_AT_commands::AT_cmd::Unknown &&
           _querySent != 0 && timePassedSince(_querySent) < 2000;
  }

  void clearQueryPending() {
    _queryPending = C023_AT_commands::AT_cmd::Unknown;
    _querySent    = 0;
  }

  void          cacheValue(C023_AT_commands::AT_cmd at_cmd,
                           const String           & value);
  void          cacheValue(C023_AT_commands::AT_cmd at_cmd,
                           String                && value);

  static String getValueFromReceivedData(const String& receivedData);

  // Decode HEX stream
  String        getValueFromReceivedBinaryData(int         & port,
                                               const String& receivedData,
                                               bool          hexEncoded);

  std::map<size_t, C023_timestamped_value>_cachedValues;
  std::list<size_t>                       _queuedQueries;
  C023_AT_commands::AT_cmd                _queryPending = C023_AT_commands::AT_cmd::Unknown;
  uint32_t                                _querySent{};


  ESPeasySerial          *_easySerial        = nullptr;
  unsigned long           _baudrate          = 9600;
  uint8_t                 sampleSetCounter   = 0;
  taskIndex_t             sampleSetInitiator = INVALID_TASK_INDEX;
  int8_t                  _resetPin          = -1;
  bool                    _isClassA{};
  bool                    _in_async_loop{}; // Mutex to prevent recursive calls to async_loop()
  LoRa_Helper::LoRaWAN_DR _dr = LoRa_Helper::LoRaWAN_DR::ADR;

  LoRa_Helper::DownlinkEventFormat_e _eventFormatStructure = LoRa_Helper::DownlinkEventFormat_e::PortNr_in_eventPar;
  C023_AT_commands::LoRaModule_e     _loraModule           = C023_AT_commands::LoRaModule_e::Dragino_LA66;


  String _fromLA66;

};

#endif // ifdef USES_C023

#endif // ifndef CONTROLLER_STRUCT_C023_DATA_STRUCT_H
