#include "../DataStructs/ProtocolStruct.h"

ProtocolStruct::ProtocolStruct() :
    defaultPort(0), usesMQTT(false), usesAccount(false), usesPassword(false),
    usesTemplate(false), usesID(false), Custom(false), usesHost(true), usesPort(true),
    usesQueue(true), usesCheckReply(true), usesTimeout(true), usesSampleSets(false), 
    usesExtCreds(false), needsNetwork(true), allowsExpire(true), allowLocalSystemTime(false)
  #if FEATURE_MQTT_TLS
  , usesTLS(false)
  #endif
    {}
