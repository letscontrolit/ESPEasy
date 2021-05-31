#include "../DataStructs/UnitMessageCount.h"

bool UnitLastMessageCount_map::isNew(const UnitMessageCount_t *count) const {
  if (count == nullptr) { return true; }
  auto it = _map.find(count->unit);

  if (it != _map.end()) {
    return it->second != count->count;
  }
  return true;
}

void UnitLastMessageCount_map::add(const UnitMessageCount_t *count) {
  if (count == nullptr) { return; }

  if ((count->unit != 0) && (count->unit != 255)) {
    _map[count->unit] = count->count;
  }
}
