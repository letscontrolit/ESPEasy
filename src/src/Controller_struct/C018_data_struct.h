#ifndef CONTROLLER_STRUCT_C018_DATA_STRUCT_H
#define CONTROLLER_STRUCT_C018_DATA_STRUCT_H

#include "src/Helpers/_CPlugin_Helper.h"

#ifdef USES_C018

# include <rn2xx3.h>


struct C018_data_struct {
private:

  void C018_logError(const __FlashStringHelper *command) const;
  void updateCacheOnInit();

public:

  C018_data_struct();

  ~C018_data_struct();

  void reset();

  bool init(const uint8_t port,
            const int8_t  serial_rx,
            const int8_t  serial_tx,
            unsigned long baudrate,
            bool          joinIsOTAA,
            taskIndex_t   sampleSet_Initiator,
            int8_t        reset_pin);

  bool isInitialized() const;

  bool hasJoined() const;

  bool useOTAA() const;

  bool command_finished() const;

  bool txUncnfBytes(const uint8_t *data,
                    uint8_t        size,
                    uint8_t        port);

  bool txHexBytes(const String& data,
                  uint8_t       port);

  bool txUncnf(const String& data,
               uint8_t       port);

  bool setTTNstack(RN2xx3_datatypes::TTN_stack_version version);

  bool setFrequencyPlan(RN2xx3_datatypes::Freq_plan plan,
                        uint32_t                    rx2_freq);

  bool setSF(uint8_t sf);

  bool setAdaptiveDataRate(bool enabled);

  bool initOTAA(const String& AppEUI,
                const String& AppKey,
                const String& DevEUI);

  bool initABP(const String& addr,
               const String& AppSKey,
               const String& NwkSKey);

  String        sendRawCommand(const String& command);

  int           getVbat();

  String        peekLastError();

  String        getLastError();

  String        getDataRate();

  int           getRSSI();

  uint32_t      getRawStatus();

  RN2xx3_status getStatus() const;

  bool          getFrameCounters(uint32_t& dnctr,
                                 uint32_t& upctr);

  bool          setFrameCounters(uint32_t dnctr,
                                 uint32_t upctr);

  // Cached data, only changing occasionally.

  String  getDevaddr();

  String  hweui();

  String  sysver();

  uint8_t getSampleSetCount() const;

  uint8_t getSampleSetCount(taskIndex_t taskIndex);

  float   getLoRaAirTime(uint8_t pl) const;

  void    async_loop();

private:

  void triggerAutobaud();

  ESPeasySerial *C018_easySerial = nullptr;
  rn2xx3        *myLora          = nullptr;
  String         cacheDevAddr;
  String         cacheHWEUI;
  String         cacheSysVer;
  unsigned long  _baudrate          = 57600;
  uint8_t        sampleSetCounter   = 0;
  taskIndex_t    sampleSetInitiator = INVALID_TASK_INDEX;
  int8_t         _resetPin          = -1;
  bool           autobaud_success   = false;
};

#endif // ifdef USES_C018

#endif // ifndef CONTROLLER_STRUCT_C018_DATA_STRUCT_H
