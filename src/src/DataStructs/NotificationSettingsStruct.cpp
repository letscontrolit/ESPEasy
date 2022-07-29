#include "../DataStructs/NotificationSettingsStruct.h"

#if FEATURE_NOTIFIER

NotificationSettingsStruct::NotificationSettingsStruct() : Port(0), Pin1(-1), Pin2(-1) {
    ZERO_FILL(Server);
    ZERO_FILL(Domain);
    ZERO_FILL(Sender);
    ZERO_FILL(Receiver);
    ZERO_FILL(Subject);
    ZERO_FILL(Body);
    ZERO_FILL(User);
    ZERO_FILL(Pass);
  }

  void NotificationSettingsStruct::validate() {
    ZERO_TERMINATE(Server);
    ZERO_TERMINATE(Domain);
    ZERO_TERMINATE(Sender);
    ZERO_TERMINATE(Receiver);
    ZERO_TERMINATE(Subject);
    ZERO_TERMINATE(Body);
    ZERO_TERMINATE(User);
    ZERO_TERMINATE(Pass);
  }

#endif