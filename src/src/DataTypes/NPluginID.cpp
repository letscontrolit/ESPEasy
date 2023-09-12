#include "../DataTypes/NPluginID.h"

#include "../Helpers/StringConverter.h"

npluginID_t npluginID_t::toPluginID(unsigned other)
{
  npluginID_t res;

  if (other <= 255) { res.value = other; }

  return res;
}

npluginID_t& npluginID_t::operator=(const npluginID_t& other)
{
  value = other.value;
  return *this;
}

bool npluginID_t::operator==(const npluginID_t& other) const
{
  return this->value == other.value;
}

bool npluginID_t::operator!=(const npluginID_t& other) const
{
  return this->value != other.value;
}

void npluginID_t::setInvalid()
{
  value = 0;
}

String npluginID_t::toDisplayString() const {
  if (value == 0) { return F("N---"); }
  return strformat(F("N%03d"), value);
}

const npluginID_t INVALID_N_PLUGIN_ID;
