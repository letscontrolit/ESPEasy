#include "../DataStructs/MessageRouteInfo.h"


MessageRouteInfo_t::MessageRouteInfo_t(const uint8_t* serializedData, size_t size) {
  deserialize(serializedData, size);
}

MessageRouteInfo_t::MessageRouteInfo_t(const uint8_t_vector& serializedData) {
  if (serializedData.size() > 0) {
    deserialize(&(serializedData[0]), serializedData.size());
  }
}

bool MessageRouteInfo_t::deserialize(const uint8_t_vector& serializedData) {
  if (serializedData.size() > 0) {
    return deserialize(&(serializedData[0]), serializedData.size());
  }
  return false;
}

bool MessageRouteInfo_t::deserialize(const uint8_t* serializedData, size_t size) {
  if (size >= 4 && serializedData != nullptr) {
    size_t index = 0;
    unit = serializedData[index++];
    count = serializedData[index++];
    dest_unit = serializedData[index++];
    const size_t traceSize = serializedData[index++];
    if (size >= (traceSize + 4)) {
      trace.resize(traceSize);
      for (size_t i = 0; i < traceSize; ++i) {
        trace[i] = serializedData[index++];
      }
      return true;
    }
  }
  return false;
}

MessageRouteInfo_t::uint8_t_vector MessageRouteInfo_t::serialize() const {
  uint8_t_vector res;
  res.resize(getSerializedSize());

  size_t index = 0;
  res[index++] = unit;
  res[index++] = count;
  res[index++] = dest_unit;
  res[index++] = trace.size();
  for (size_t i = 0; i < trace.size(); ++i) {
    res[index++] = trace[i];
  }
  return res;  
}

size_t MessageRouteInfo_t::getSerializedSize() const {
  return 4 + trace.size();
}

bool MessageRouteInfo_t::appendUnit(uint8_t unitnr) {
  if (unitnr == 0 || unitnr == 255 || traceHasUnit(unitnr)) {
    return false;
  }
  trace.push_back(unitnr);
}

bool MessageRouteInfo_t::traceHasUnit(uint8_t unitnr) const {
  if (unitnr == 0 || unitnr == 255) {
    return false;
  }
  for (auto it = trace.begin(); it != trace.end(); ++it) {
    if (*it == unitnr) return true;
  }
  return false;
}


bool UnitMessageRouteInfo_map::isNew(const MessageRouteInfo_t *info) const {
  if (info == nullptr) { return true; }
  auto it = _map.find(info->unit);

  if (it != _map.end()) {
    return it->second.count != info->count;
  }
  return true;
}

void UnitMessageRouteInfo_map::add(const MessageRouteInfo_t *info) {
  if (info == nullptr) { return; }

  if ((info->unit != 0) && (info->unit != 255)) {
    _map[info->unit] = *info;
  }
}
