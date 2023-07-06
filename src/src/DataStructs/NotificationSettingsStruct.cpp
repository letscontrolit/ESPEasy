#include "../DataStructs/NotificationSettingsStruct.h"

#if FEATURE_NOTIFIER

NotificationSettingsStruct::NotificationSettingsStruct() {
  memset(this, 0, sizeof(NotificationSettingsStruct));
  Pin1 = -1;
  Pin2 = -1;
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