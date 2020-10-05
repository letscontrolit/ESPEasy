#include "ESPEasy_Now_p2p_data.h"

#include <cstddef>


ESPEasy_Now_p2p_data::~ESPEasy_Now_p2p_data() {
  if (data != nullptr) {
    delete data;
    data = nullptr;
  }
}

bool ESPEasy_Now_p2p_data::addFloat(float value) {
  return addBinaryData((byte *)(&value), sizeof(float));
}

bool ESPEasy_Now_p2p_data::getFloat(float& value, size_t& offset) const {
  if ((offset + sizeof(float)) >= dataSize) {
    return false;
  }
  memcpy((byte *)(&value), &data[offset], sizeof(float));
  offset += sizeof(float);
  return true;
}

size_t ESPEasy_Now_p2p_data::getTotalSize() const {
  return dataOffset + dataSize;
}

bool ESPEasy_Now_p2p_data::addBinaryData(uint8_t *binaryData, size_t size) {
  size_t oldSize;

  if (allocate(size, oldSize)) {
    memcpy(&data[oldSize], binaryData, size);
    return true;
  }
  return false;
}

const uint8_t * ESPEasy_Now_p2p_data::getBinaryData(size_t offset, size_t& size) const {
  if (offset >= dataSize) {
    return nullptr;
  }

  const size_t available = dataSize - offset;

  if (size < available) {
    size = available;
  }
  return &data[offset];
}

uint8_t * ESPEasy_Now_p2p_data::prepareBinaryData(size_t& size) {
  size_t oldSize;

  if (data != nullptr) {
    delete data;
    data     = nullptr;
    dataSize = 0;
  }

  if (!allocate(size, oldSize)) {
    return nullptr;
  }
  size = dataSize;
  return data;
}

bool ESPEasy_Now_p2p_data::allocate(size_t size, size_t& oldSize) {
  oldSize = dataSize;
  const size_t newSize = oldSize + size;

  uint8_t *tmp_ptr = new (std::nothrow) uint8_t[newSize];

  if (tmp_ptr == nullptr) {
    return false;
  }

  if (data != nullptr) {
    memcpy(tmp_ptr, data, oldSize);
    delete data;
  }
  data     = tmp_ptr;
  dataSize = newSize;
  return true;
}
