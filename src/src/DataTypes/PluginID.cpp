#include "../DataTypes/PluginID.h"

#include "../Helpers/StringConverter.h"

pluginID_t pluginID_t::toPluginID(unsigned other)
{
    pluginID_t res;
    if (other <= 255) res.value = other;

    return res;
}

pluginID_t& pluginID_t::operator=(const pluginID_t& other)
{
    value = other.value;
    return *this;
}

bool pluginID_t::operator==(const pluginID_t& other) const
{
    return this->value == other.value;
}

bool pluginID_t::operator!=(const pluginID_t& other) const
{
    return this->value != other.value;
}

void pluginID_t::setInvalid()
{
    value = 0;
}

String pluginID_t::toDisplayString() const {
    if (value == 0) return F("P---");
    return strformat(F("P%03d"), value);
}

pluginID_t INVALID_PLUGIN_ID;
