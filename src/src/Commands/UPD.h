#ifndef COMMAND_UDP_H
#define COMMAND_UDP_H

#include "../../ESPEasy_common.h"

String Command_UDP_Port(struct EventStruct *event,
                        const char         *Line);

#if FEATURE_ESPEASY_P2P
const __FlashStringHelper* Command_UDP_Test(struct EventStruct *event,
                                            const char         *Line);
const __FlashStringHelper* Command_UPD_SendTo(struct EventStruct *event,
                                              const char         *Line);
#endif // if FEATURE_ESPEASY_P2P
const __FlashStringHelper* Command_UDP_SendToUPD(struct EventStruct *event,
                                                 const char         *Line);
const __FlashStringHelper* Command_UDP_SendToUPD(struct EventStruct *event,
                                                 const char         *Line,
                                                 const bool          handleMix);
const __FlashStringHelper* Command_UDP_SendToUPDMix(struct EventStruct *event,
                                                    const char         *Line);

#endif // COMMAND_UDP_H
