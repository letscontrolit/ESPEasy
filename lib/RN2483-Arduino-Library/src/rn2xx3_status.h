#ifndef RN2XX3_STATUS_H
#define RN2XX3_STATUS_H

#include "Arduino.h"

#include "rn2xx3_datatypes.h"


// This class decodes the received data from the command:
// mac get status
// The implementation of these status bits has changed between firmware versions.
// See: https://www.thethingsnetwork.org/forum/t/rn2483-how-to-handle-if-connection-to-ttn-is-lost/30956/13?u=td-er
class RN2xx3_status {
public:

  RN2xx3_status();
  RN2xx3_status(uint32_t value);

  enum MacState_t {
    Idle                  = 0, // Idle (transmissions are possible)
    TransmissionOccurring = 1, // Transmission occurring
    PreOpenReceiveWindow1 = 2, // Before the opening of Receive window 1
    ReceiveWindow1Open    = 3, // Receive window 1 is open
    BetwReceiveWindow1_2  = 4, // Between Receive window 1 and Receive window 2
    ReceiveWindow2Open    = 5, // Receive window 2 is open
    RetransDelay          = 6, // Retransmission delay - used for ADR_ACK delay, FSK can occur
    APB_delay             = 7, // APB_delay
    Class_C_RX2_1_open    = 8, // Class C RX2 1 open
    Class_C_RX2_2_open    = 9  // Class C RX2 2 open
  } MacState;

  // Joined does not seem to be updated in the status bits.
  // Assume joined at first unless a transmit command returns "not_joined".
  // This will prevent a lot of unneeded join requests.
  bool Joined = true;
  bool AutoReply;
  bool ADR;
  bool SilentImmediately; // indicates the device has been silenced by the network. To enable: "mac forceENABLE"
  bool MacPause;          // Temporary disable the LoRaWAN protocol interpreter. (e.g. to change radio settings)
  bool RxDone;
  bool LinkCheck;
  bool ChannelsUpdated;
  bool OutputPowerUpdated;
  bool NbRepUpdated; // NbRep is the number of repetitions for unconfirmed packets
  bool PrescalerUpdated;
  bool SecondReceiveWindowParamUpdated;
  bool RXtimingSetupUpdated;
  bool RejoinNeeded;
  bool Multicast;

  void     setModelVersion(const String& version);
  bool     modelVersionSet() const;
  bool     decode(uint32_t value);
  bool     saveSettingsNeeded() const;
  bool     clearSaveSettingsNeeded();
  uint32_t getRawStatus() const;

private:

  RN2xx3_datatypes::Firmware _firmware = RN2xx3_datatypes::Firmware::unknown;
  RN2xx3_datatypes::Model _model       = RN2xx3_datatypes::Model::RN_NA;

  uint32_t _rawstatus      = 0;
  bool _saveSettingsNeeded = false;
};


#endif // RN2XX3_STATUS_H
