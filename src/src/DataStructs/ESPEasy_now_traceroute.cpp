#include "../DataStructs/ESPEasy_now_traceroute.h"

#include <list>

ESPEasy_now_traceroute_struct::ESPEasy_now_traceroute_struct() {
//  unit_vector.reserve(32); // prevent re-allocations
}

ESPEasy_now_traceroute_struct::ESPEasy_now_traceroute_struct(uint8_t size) {
  unit_vector.resize(size);
}

void ESPEasy_now_traceroute_struct::clear()
{
  unit_vector.clear();
}

uint8_t ESPEasy_now_traceroute_struct::getUnit(uint8_t distance, uint8_t& successRate) const
{
  if (unit_vector.size() < static_cast<size_t>((distance + 1) * 2)) {
    successRate = 0;
    return 0;
  }
  successRate = unit_vector[(distance * 2) + 1];
  return unit_vector[distance * 2];
}

void ESPEasy_now_traceroute_struct::addUnit(byte unit)
{
  // Only add the unit if it isn't already part of the traceroute.
  const uint8_t index = unit_vector.size();
  for (size_t i = 0; i < index; i+=2) {
    if (unit_vector[i] == unit) {
      return;
    }
  }

  unit_vector.resize(index + 2);
  unit_vector[index]     = unit;
  // Set default success rate to average
  unit_vector[index + 1] = 127;
}

uint8_t ESPEasy_now_traceroute_struct::getDistance() const
{
  if (unit_vector.size() == 0) { return 255; }
  return (unit_vector.size() / 2) - 1;
}

const uint8_t * ESPEasy_now_traceroute_struct::getData(uint8_t& size) const
{
  size = unit_vector.size();
  return &(unit_vector[0]);
}

uint8_t * ESPEasy_now_traceroute_struct::get()
{
  if (unit_vector.size() == 0) {
    return nullptr;
  }
  return &(unit_vector[0]);
}

void ESPEasy_now_traceroute_struct::setSuccessRate_last_node(byte unit, uint8_t successRate)
{
  int index = unit_vector.size() - 2;
  int attempt = 0;

  while (index >= 0 && attempt < 2) {
    if (unit_vector[index] == unit) {
      unit_vector[index + 1] = successRate;
      return;
    }
    index -= 2;
    ++attempt;
  }
}

bool ESPEasy_now_traceroute_struct::operator<(const ESPEasy_now_traceroute_struct& other) const
{
  return computeSuccessRate() > other.computeSuccessRate();
}

int ESPEasy_now_traceroute_struct::computeSuccessRate() const
{
  const uint8_t max_distance = getDistance();
  if (max_distance == 255) {
    // No values stored, so huge penalty
    return 0;
  }
  if (max_distance == 0) {
    // End point, so assume best success rate
    return 255;
  }

  int res = 0;
  for (uint8_t distance = 0; distance <= max_distance; ++distance) {
    uint8_t successRate = 0;
    getUnit(distance, successRate);

    if (successRate < 10) {
      return 0;
    }

    res += successRate;
  }
  if (max_distance > 0) {
    res /= max_distance;
  }
  return res;
}

bool ESPEasy_now_traceroute_struct::unitInTraceRoute(byte unit) const
{
  const uint8_t max_distance = getDistance();
  for (uint8_t distance = 0; distance <= max_distance; ++distance) {
    uint8_t success_rate = 0;
    if (getUnit(distance, success_rate) == unit) {
      return true;
    }
  }
  return false;
}

String ESPEasy_now_traceroute_struct::toString() const
{
  const uint8_t max_distance = getDistance();

  if (max_distance == 255) {
      return String();
  }
  String res;
  res.reserve(4*unit_vector.size());
  for (uint8_t distance = 0; distance <= max_distance; ++distance) {
    uint8_t success_rate = 0;
    res += getUnit(distance, success_rate);
    res += '/';
    res += success_rate;
    res += ' ';
  }
  return res;
}