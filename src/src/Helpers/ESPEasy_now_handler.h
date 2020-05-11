#ifndef HELPERS_ESPEASY_NOW_HANDLER_H
#define HELPERS_ESPEASY_NOW_HANDLER_H

#include "../Globals/ESPEasy_now_state.h"

#ifdef USES_ESPEASY_NOW

# include "../DataStructs/ESPEasy_now_message_header_struct.h"


class ESPEasy_now_handler_t {
public:

  ESPEasy_now_handler_t() {}

  bool begin();
  void end();
};

#endif // ifdef USES_ESPEASY_NOW

#endif // HELPERS_ESPEASY_NOW_HANDLER_H
