#ifndef PLUGINSTRUCTS_P093_DATA_STRUCT_H
#define PLUGINSTRUCTS_P093_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P093


#ifndef BUILD_NO_DEBUG
# define PLUGIN_093_DEBUG
#endif


/*
 *
 * Bi-directional communication with the heat pump.
 *
 * Should support all Mitsubishi HP units with CN105 connector.
 *
 * Plugin is based on "Arduino library to control Mitsubishi Heat Pumps" from
 * https://github.com/SwiCago/HeatPump.
 *
 */

static const uint8_t PACKET_LEN      = 22;
static const uint8_t READ_BUFFER_LEN = 32;

static const uint8_t INFOMODE[] = {
  0x02, // request a settings packet
  0x03, // request the current room temp
  0x06  // request status
};

struct P093_data_struct : public PluginTaskData_base {
  P093_data_struct(const ESPEasySerialPort port,
                   const int16_t           serialRx,
                   const int16_t           serialTx,
                   bool                    includeStatus);

  P093_data_struct() = delete;
  virtual ~P093_data_struct() = default;

  void init();

  bool sync();

  bool read(String& result) const;

  void write(const String& command,
             const String& value);

private:

  struct Tuple {
    uint8_t                    value;
    const __FlashStringHelper *mapping;
  };

  struct Mappings {
    Tuple power[2];
    Tuple mode[5];
    Tuple fan[6];
    Tuple vane[7];
    Tuple wideVane[7];

    Mappings() :
      power{
            { 0x00, F("OFF") },
            { 0x01, F("ON") }
            },
      mode{
           { 0x01, F("HEAT") },
           { 0x02, F("DRY") },
           { 0x03, F("COOL") },
           { 0x07, F("FAN") },
           { 0x08, F("AUTO") }
           },
      fan{
          { 0x00, F("AUTO") },
          { 0x01, F("QUIET") },
          { 0x02, F("1") },
          { 0x03, F("2") },
          { 0x05, F("3") },
          { 0x06, F("4") }
          },
      vane{
           { 0x00, F("AUTO") },
           { 0x01, F("1") },
           { 0x02, F("2") },
           { 0x03, F("3") },
           { 0x04, F("4") },
           { 0x05, F("5") },
           { 0x07, F("SWING") }
           },
      wideVane{
               { 0x01, F("<<") },
               { 0x02, F("<") },
               { 0x03, F("|") },
               { 0x04, F(">") },
               { 0x05, F(">>") },
               { 0x08, F("<>") },
               { 0x0C, F("SWING") }
               } {}
  };

  enum State {
    Invalid      = -1,
    NotConnected = 0,
    Connecting,
    Connected,
    UpdatingStatus,
    StatusUpdated,
    ScheduleNextStatusUpdate,
    WaitingForScheduledStatusUpdate,
    ApplyingSettings,
    SettingsApplied,
    ReadTimeout
  };

  static const uint8_t Temperature = 0x01;
  static const uint8_t Power       = 0x02;
  static const uint8_t Mode        = 0x04;
  static const uint8_t Fan         = 0x08;
  static const uint8_t Vane        = 0x10;
  static const uint8_t WideVane    = 0x20;

  struct WriteStatus {
    WriteStatus() : _flags(0) {}

    void set(uint8_t flag) {
      _flags |= flag;
    }

    void clear() {
      _flags = 0;
    }

    bool isDirty(uint8_t flag) const {
      return (_flags & flag) != 0;
    }

    bool isDirty() const {
      return _flags != 0;
    }

private:

    uint8_t _flags;
  };

  struct Values {
    uint8_t power;
    bool    iSee;
    uint8_t mode;
    float   temperature;
    uint8_t fan;
    uint8_t vane;
    uint8_t wideVane;
    float   roomTemperature;
    bool    operating;
    uint8_t compressorFrequency;

    Values() :
      power(0),
      iSee(false),
      mode(0),
      temperature(0),
      fan(0),
      vane(0),
      wideVane(0),
      roomTemperature(0),
      operating(false),
      compressorFrequency(0) {}

    bool operator!=(const Values& rhs) const {
      return power != rhs.power ||
             mode != rhs.mode ||
             temperature != rhs.temperature ||
             fan != rhs.fan ||
             vane != rhs.vane ||
             wideVane != rhs.wideVane ||
             iSee != rhs.iSee ||
             roomTemperature != rhs.roomTemperature ||
             operating != rhs.operating ||
             compressorFrequency != rhs.compressorFrequency;
    }
  };

private:

  void          setState(State newState);

  static bool   shouldTransition(State from,
                                 State to);

  void          didTransition(State from,
                              State to);

  void          applySettingsLocally();

  void          cancelWaitingAndTransitTo(State state);

  void          responseReceived();

  void          updateStatus();

  void          applySettings();

  void          connect();

  unsigned long getBaudRate() const;

  void          sendPacket(const uint8_t *packet,
                           size_t         size);

  void          addByteToReadBuffer(uint8_t value);

  bool          readIncommingBytes();

  bool          processIncomingPacket(const uint8_t *packet,
                                      uint8_t        length,
                                      uint8_t        checksum);

  bool         parseValues(const uint8_t *data,
                           size_t         length);

  static State checkIncomingPacket(const uint8_t *packet,
                                   uint8_t        length,
                                   uint8_t        checksum);

  static uint8_t                    checkSum(const uint8_t *bytes,
                                             size_t         length);

  static const __FlashStringHelper* findByValue(uint8_t     value,
                                                const Tuple list[],
                                                size_t      count);

  static bool findByMapping(const String& mapping,
                            const Tuple   list[],
                            size_t        count,
                            uint8_t     & value);

  # ifdef PLUGIN_093_DEBUG
  static const __FlashStringHelper* stateToString_f(State state);

  static String                     stateToString(State state);

  static void                       dumpPacket(const uint8_t *packet,
                                               size_t         length,
                                               String       & result);

  static String dumpOutgoingPacket(const uint8_t *packet,
                                   size_t         length);

  static String dumpIncomingPacket(const uint8_t *packet,
                                   int            length);
  # endif // ifdef PLUGIN_093_DEBUG

private:

  ESPeasySerial  _serial;
  State          _state;
  bool           _fastBaudRate;
  uint8_t        _readBuffer[READ_BUFFER_LEN] = { 0 };
  uint8_t        _readPos;
  unsigned long  _writeTimeout;
  Values         _currentValues;
  Values         _wantedSettings;
  uint8_t        _infoModeIndex;
  unsigned long  _statusUpdateTimeout;
  bool           _tempMode;
  bool           _wideVaneAdj;
  bool           _valuesInitialized;
  bool           _includeStatus;
  WriteStatus    _writeStatus;
  const Mappings _mappings;
};

#endif // ifdef USES_P093
#endif // ifndef PLUGINSTRUCTS_P093_DATA_STRUCT_H
