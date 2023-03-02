#ifndef COMMAND_HTTP_H
#define COMMAND_HTTP_H

#include <Arduino.h>

#if FEATURE_SEND_TO_HTTP || FEATURE_POST_TO_HTTP
const __FlashStringHelper* httpEmitToHTTP(struct EventStruct        *event,
                                          const __FlashStringHelper *logIdentifier,
                                          const __FlashStringHelper *HttpMethod,
                                          const char                *Line,
                                          const int                  timeout,
                                          const bool                 waitForAck,
                                          const bool                 useHeader,
                                          const bool                 useBody);
#endif // if FEATURE_SEND_TO_HTTP || FEATURE_POST_TO_HTTP
#if FEATURE_SEND_TO_HTTP
const __FlashStringHelper* Command_HTTP_SendToHTTP(struct EventStruct *event,
                                                   const char         *Line);
#endif // FEATURE_SEND_TO_HTTP
#if FEATURE_POST_TO_HTTP
const __FlashStringHelper* Command_HTTP_PostToHTTP(struct EventStruct *event,
                                                   const char         *Line);
#endif // if FEATURE_POST_TO_HTTP

#endif // COMMAND_HTTP_H
