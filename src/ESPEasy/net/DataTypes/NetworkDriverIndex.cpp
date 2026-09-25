#include "../DataTypes/NetworkDriverIndex.h"

#include "../../../src/CustomBuild/ESPEasyLimits.h"

namespace ESPEasy {
namespace net {

const networkDriverIndex_t INVALID_NETWORKDRIVER_INDEX = networkDriverIndex_t::toNetworkDriverIndex(NETWORKDRIVER_INDEX_MAX + 1);

} // namespace net
} // namespace ESPEasy
