#include "rn2xx3_status.h"


RN2xx3_status::RN2xx3_status() {
  decode(0);
}

RN2xx3_status::RN2xx3_status(uint32_t value) {
  decode(value);
}

bool RN2xx3_status::decode(uint32_t value) {
  _rawstatus = value;

  MacState                        = static_cast<MacState_t>(value & 0xF);
  value                           = value >> 4;
  Joined                          = Joined | (value & 1); value = value >> 1;
  AutoReply                       = (value & 1); value = value >> 1;
  ADR                             = (value & 1); value = value >> 1;
  SilentImmediately               = (value & 1); value = value >> 1;
  MacPause                        = (value & 1); value = value >> 1;
  RxDone                          = (value & 1); value = value >> 1;
  LinkCheck                       = (value & 1); value = value >> 1;
  ChannelsUpdated                 = ChannelsUpdated                 | (value & 1); value = value >> 1;
  OutputPowerUpdated              = OutputPowerUpdated              | (value & 1); value = value >> 1;
  NbRepUpdated                    = NbRepUpdated                    | (value & 1); value = value >> 1;
  PrescalerUpdated                = PrescalerUpdated                | (value & 1); value = value >> 1;
  SecondReceiveWindowParamUpdated = SecondReceiveWindowParamUpdated | (value & 1); value = value >> 1;
  RXtimingSetupUpdated            = RXtimingSetupUpdated            | (value & 1); value = value >> 1;
  RejoinNeeded                    = (value & 1); value = value >> 1;
  Multicast                       = (value & 1); value = value >> 1;


  /*
     The following bits are cleared after issuing a “mac get status” command:
     - 11 (Channels updated)
     - 12 (Output power updated)
     - 13 (NbRep updated)
     - 14 (Prescaler updated)
     - 15 (Second Receive window parameters updated)
     - 16 (RX timing setup updated)

     So we must keep track of them to see if they were updated since the last time they were saved to the
   */

  _saveSettingsNeeded =
    _saveSettingsNeeded ||
    ChannelsUpdated ||
    OutputPowerUpdated ||
    NbRepUpdated ||
    PrescalerUpdated ||
    SecondReceiveWindowParamUpdated ||
    RXtimingSetupUpdated;
  return _saveSettingsNeeded;
}

bool RN2xx3_status::saveSettingsNeeded() const {
  return _saveSettingsNeeded;
}

bool RN2xx3_status::clearSaveSettingsNeeded() {
  bool ret = _saveSettingsNeeded;

  _saveSettingsNeeded             = false;
  ChannelsUpdated                 = false;
  OutputPowerUpdated              = false;
  NbRepUpdated                    = false;
  PrescalerUpdated                = false;
  SecondReceiveWindowParamUpdated = false;
  RXtimingSetupUpdated            = false;

  return ret;
}

uint32_t RN2xx3_status::getRawStatus() const {
  return _rawstatus;
}
