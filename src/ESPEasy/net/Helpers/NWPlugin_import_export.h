#pragma once

#include "../../../ESPEasy_common.h"
#if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS

# include "../DataTypes/NetworkIndex.h"
# include "../../../src/Helpers/KeyValueWriter.h"

namespace ESPEasy {
namespace net {

class NWPlugin_import_export
{
public:

  static String exportConfig(
    networkIndex_t  networkIndex,
    KeyValueWriter *writer,
    bool            includeCredentials = false);

  static String importConfig(
    networkIndex_t networkIndex,
    const String & json);


}; // class NWPlugin_import_export

} // namespace net
} // namespace ESPEasy

#endif // if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS
