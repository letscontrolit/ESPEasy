#ifndef DATATYPES_NPLUGINID_H
#define DATATYPES_NPLUGINID_H

#include "../../ESPEasy_common.h"

struct npluginID_t {
  npluginID_t();

  static npluginID_t toPluginID(unsigned other);

  npluginID_t      & operator=(const npluginID_t& other);

  bool               operator==(const npluginID_t& other) const;
  bool               operator!=(const npluginID_t& other) const;

  void               setInvalid();

  String             toDisplayString() const;

  uint8_t value{};
};


extern const npluginID_t INVALID_N_PLUGIN_ID;


#endif // ifndef DATATYPES_NPLUGINID_H
