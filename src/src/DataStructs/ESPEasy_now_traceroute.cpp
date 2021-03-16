#include "ESPEasy_now_traceroute.h"

#include <list>

ESPEasy_now_traceroute_struct::ESPEasy_now_traceroute_struct() {}

ESPEasy_now_traceroute_struct::ESPEasy_now_traceroute_struct(uint8_t size) {
  unit_vector.resize(size);
}

uint8_t ESPEasy_now_traceroute_struct::getUnit(uint8_t distance, int8_t& rssi) const
{
  if (unit_vector.size() < static_cast<size_t>((distance + 1) * 2)) {
    return 0;
  }
  const int temp_rssi = unit_vector[(distance * 2) + 1];

  rssi = temp_rssi - 127;
  return unit_vector[distance * 2];
}

void ESPEasy_now_traceroute_struct::addUnit(uint8_t unit, int8_t rssi)
{
  const uint8_t index = unit_vector.size();

  unit_vector.resize(index + 2);
  unit_vector[index]     = unit;
  unit_vector[index + 1] = static_cast<uint8_t>(static_cast<int>(rssi + 127));
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

void ESPEasy_now_traceroute_struct::setRSSI_last_node(int8_t rssi) const
{
  const uint8_t index = unit_vector.size();

  if (index == 0) {
    return;
  }
  unit_vector[index - 1] = static_cast<uint8_t>(static_cast<int>(rssi + 127));
}

bool ESPEasy_now_traceroute_struct::operator<(const ESPEasy_now_traceroute_struct& other) const
{
  return compute_penalty() < other.compute_penalty();
}

int ESPEasy_now_traceroute_struct::compute_penalty() const
{
  const uint8_t max_distance = getDistance();

  // FIXME TD-er: for now just base it on the distance, not on the combination of RSSI & distance
  return max_distance;
/*
  if (max_distance == 255) {
    // No values stored, so huge penalty
    return max_distance * 100;
  }


  int penalty = 0;

  for (uint8_t distance = 0; distance <= max_distance; ++distance) {
    int8_t rssi = 0;
    getUnit(distance, rssi);

    if (rssi >= 0) {
      // Some "average" RSSI values for unknown RSSI
      rssi = -75;
    } else if (rssi < -80) {
      // Some extra penalty for low quality signals
      rssi -= 10;
    }

    // RSSI values are negative, with lower value being a worse signal
    penalty -= rssi;
  }
  return penalty;
  */
}

void ESPEasy_now_traceroute_struct::sanetize()
{
  std::list<uint8_t> units;

  for (size_t i = 0; i < unit_vector.size(); i += 2) {
    units.push_back(unit_vector[i]);
  }

  // Remove duplicates by sort and unique
  units.sort();
  units.unique();

  if (units.size() == (unit_vector.size() / 2)) {
    // No loops
    return;
  }

  std::vector<uint8_t> new_unit_vector;

  new_unit_vector.reserve(units.size() * 2);

  const uint8_t max_distance = getDistance();

  for (uint8_t distance = 0; distance <= max_distance; ++distance) {
    const uint8_t unit = unit_vector[distance * 2];
    bool found         = false;

    for (auto it = units.begin(); !found && it != units.end(); ++it) {
      if (*it == unit) {
        // Present in the unique list so remove it
        units.erase(it);
        found = true;
      }
    }

    if (found) {
      // It was present in the unique list, so we can add it to the new vector
      new_unit_vector.push_back(unit);
      new_unit_vector.push_back(unit_vector[(distance * 2) + 1]);
    }
  }
  unit_vector = new_unit_vector;
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
    int8_t rssi = 0;
    res += getUnit(distance, rssi);
    res += '/';
    res += rssi;
    res += ' ';
  }
  return res;
}