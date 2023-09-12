#include "../DataTypes/DeviceIndex.h"

#include "../CustomBuild/ESPEasyLimits.h"


//deviceIndex_t::deviceIndex_t(int other) : value(other) {}

deviceIndex_t deviceIndex_t::toDeviceIndex(int other) {
    deviceIndex_t res;
    if (other > DEVICE_INDEX_MAX)
      res.value = DEVICE_INDEX_MAX;
    else
      res.value = other;

    return res;
}

deviceIndex_t& deviceIndex_t::operator=(int other) {
  value = other;
  return *this;
}

deviceIndex_t& deviceIndex_t::operator=(const deviceIndex_t& other) {
  value = other.value;
  return *this;
}

bool deviceIndex_t::operator<(int other) const
{
    return value < other;
}

bool deviceIndex_t::operator<=(int other) const
{
    return value <= other;
}

bool deviceIndex_t::operator!=(int other) const
{
    return value != other;
}

bool deviceIndex_t::operator!=(const deviceIndex_t& other) const
{
    return value != other.value;
}

deviceIndex_t& deviceIndex_t::operator++() {
    // pre-increment, ++a
    ++value;
    return *this;
}

/*
bool deviceIndex_t::isValid() const {
    return value != DEVICE_INDEX_MAX;
}
*/

deviceIndex_t INVALID_DEVICE_INDEX = deviceIndex_t::toDeviceIndex(DEVICE_INDEX_MAX);
