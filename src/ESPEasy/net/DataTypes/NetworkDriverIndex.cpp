#include "../DataTypes/NetworkDriverIndex.h"

#include "../../../src/CustomBuild/ESPEasyLimits.h"

namespace ESPEasy {
namespace net {

networkDriverIndex_t INVALID_NETWORKDRIVER_INDEX = networkDriverIndex_t::toNetworkDriverIndex(NETWORKDRIVER_INDEX_MAX);

} // namespace net
} // namespace ESPEasy
