#ifndef HELPERS_CONTROLLER_C019_ESPEASYNOW_HELPER_H
#define HELPERS_CONTROLLER_C019_ESPEASYNOW_HELPER_H

#include "../../ESPEasy_common.h"

#ifdef USES_ESPEASY_NOW

#include "../DataStructs/ESPEasy_Now_p2p_data.h"

struct C019_ESPEasyNow_helper {
  static bool process_receive(struct EventStruct *event);

  static void process_received_PluginData(const ESPEasy_Now_p2p_data& data);
};

#endif

#endif // HELPERS_CONTROLLER_C019_ESPEASYNOW_HELPER_H
