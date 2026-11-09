#include "../DataStructs/NotificationSettingsStruct.h"

#if FEATURE_NOTIFIER

# if FEATURE_EMAIL_TLS

const __FlashStringHelper * NotificationSettingsStruct::toString(EncryptionType encType)
{
  switch (encType)
  {
    case EncryptionType::Auto:         return F("Auto");
    case EncryptionType::NoEncryption: return F("No Encryption");
//    case EncryptionType::SSL:          return F("SSL");
//    case EncryptionType::TLS:          return F("TLS");

    case EncryptionType::MAX_ENCRYPTION_TYPE: break;
  }
  return F("");
}

# endif // if FEATURE_EMAIL_TLS

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

# if FEATURE_EMAIL_TLS

  if (EncryptionSelector >= static_cast<uint8_t>(EncryptionType::MAX_ENCRYPTION_TYPE)) {
    EncryptionSelector = 0;
  }
# endif // if FEATURE_EMAIL_TLS
}

#endif // if FEATURE_NOTIFIER
