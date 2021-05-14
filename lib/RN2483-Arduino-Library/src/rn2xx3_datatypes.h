#ifndef RN2XX3_DATATYPES_H
#define RN2XX3_DATATYPES_H

#include "Arduino.h"

class RN2xx3_datatypes {
public:

  enum Model {
    RN_NA  = 0, // Not set
    RN2903 = 2903,
    RN2483 = 2483
  };

  enum Firmware {
    unknown   = 0,
    pre_1_0_1 = 1,
    rev1_0_1  = 101,
    rev1_0_2  = 102,
    rev1_0_3  = 103,
    rev1_0_4  = 104,
    rev1_0_5  = 105
  };

  enum Freq_plan {
    SINGLE_CHANNEL_EU = 0,
    TTN_EU,
    TTN_US,
    DEFAULT_EU
  };

  enum TTN_stack_version {
    TTN_v2 = 0,
    TTN_v3 = 1,

    TTN_NOT_SET
  };

  enum TX_return_type {
    TX_FAIL = 0,    // The transmission failed.
                    // If you sent a confirmed message and it is not acked,
                    // this will be the returned value.

    TX_SUCCESS = 1, // The transmission was successful.
                    // Also the case when a confirmed message was acked.

    TX_WITH_RX = 2  // A downlink message was received after the transmission.
                    // This also implies that a confirmed message is acked.
  };

  static Model intToModel(int modelId);

  // Parse system version in this format:
  // RN2483 1.0.1 Dec 15 2015 09:38:09
  static Model parseVersion(const String& version,
                            Firmware    & firmware);
};

#endif // RN2XX3_DATATYPES_H
