#ifndef COMMAND_HTTP_H
#define COMMAND_HTTP_H

#include "../../ESPEasy_common.h"

#if FEATURE_SEND_TO_HTTP
const __FlashStringHelper* Command_HTTP_SendToHTTP(struct EventStruct *event,
                                                   const char         *Line);
#if FEATURE_HTTP_TLS
const __FlashStringHelper* Command_HTTP_SendToHTTPS(struct EventStruct *event,
                                                    const char         *Line);
#endif // if FEATURE_HTTP_TLS
#endif // FEATURE_SEND_TO_HTTP
#if FEATURE_POST_TO_HTTP
const __FlashStringHelper* Command_HTTP_PostToHTTP(struct EventStruct *event,
                                                   const char         *Line);
#if FEATURE_HTTP_TLS
const __FlashStringHelper* Command_HTTP_PostToHTTPS(struct EventStruct *event,
                                                    const char         *Line);
#endif // if FEATURE_HTTP_TLS
#endif // if FEATURE_POST_TO_HTTP
#if FEATURE_PUT_TO_HTTP
const __FlashStringHelper* Command_HTTP_PutToHTTP(struct EventStruct *event,
                                                  const char         *Line);
#if FEATURE_HTTP_TLS
const __FlashStringHelper* Command_HTTP_PutToHTTPS(struct EventStruct *event,
                                                   const char         *Line);
#endif // if FEATURE_HTTP_TLS
#endif // if FEATURE_PUT_TO_HTTP

#endif // COMMAND_HTTP_H
