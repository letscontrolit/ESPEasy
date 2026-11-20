#pragma once

#include "../../ESPEasy_common.h"

#if FEATURE_BUILD_DESCRIPTION
# include <vector>

struct xPluginEnumerator {
public:

  xPluginEnumerator();

  bool   isEmpty();

  void   fill(const uint16_t arr[]);

  String getString(char separator) const;

private:

  std::vector<uint16_t>_bitmap;

};

String CreateBuildDescription(char separator);
#endif // if FEATURE_BUILD_DESCRIPTION
