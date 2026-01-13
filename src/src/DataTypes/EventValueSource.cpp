#include "../DataTypes/EventValueSource.h"

const __FlashStringHelper * EventValueSource::toString(Enum source)
{
  switch (source)
  {
    case EventValueSource::Enum::VALUE_SOURCE_SYSTEM:           return F("System");
    case EventValueSource::Enum::VALUE_SOURCE_SERIAL:           return F("Serial");
    case EventValueSource::Enum::VALUE_SOURCE_UDP:              return F("UDP");
    case EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND:     return F("Web");
    case EventValueSource::Enum::VALUE_SOURCE_RULES:            return F("Rules");
    case EventValueSource::Enum::VALUE_SOURCE_HTTP:             return F("HTTP");
    case EventValueSource::Enum::VALUE_SOURCE_MQTT:             return F("MQTT");
    case EventValueSource::Enum::VALUE_SOURCE_RULES_RESTRICTED: return F("Restricted Rules");

    case EventValueSource::Enum::VALUE_SOURCE_NOT_SET:
    case EventValueSource::Enum::VALUE_SOURCE_NR_VALUES:
      break;
  }
  return F("");
}

bool EventValueSource::partOfGroup(EventValueSource::Enum source, EventValueSourceGroup::Enum group)
{
  switch (source)
  {
    case EventValueSource::Enum::VALUE_SOURCE_NOT_SET:
    case EventValueSource::Enum::VALUE_SOURCE_NR_VALUES:
      return false;
    case EventValueSource::Enum::VALUE_SOURCE_SYSTEM:
    case EventValueSource::Enum::VALUE_SOURCE_SERIAL:
    case EventValueSource::Enum::VALUE_SOURCE_UDP:
    case EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND:
    case EventValueSource::Enum::VALUE_SOURCE_RULES:
      return true;
    case EventValueSource::Enum::VALUE_SOURCE_HTTP:
    case EventValueSource::Enum::VALUE_SOURCE_MQTT:
    case EventValueSource::Enum::VALUE_SOURCE_RULES_RESTRICTED:
      return group == EventValueSourceGroup::Enum::ALL;
  }
  return false;
}

bool EventValueSource::SourceNeedsStatusUpdate(EventValueSource::Enum eventSource)
{
  switch (eventSource)
  {
    case EventValueSource::Enum::VALUE_SOURCE_HTTP:
    case EventValueSource::Enum::VALUE_SOURCE_SERIAL:
    case EventValueSource::Enum::VALUE_SOURCE_MQTT:
    case EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND:
      return true;

    default:
      break;
  }
  return false;
}

bool EventValueSource::isExternalSource(EventValueSource::Enum eventSource)
{
  switch (eventSource)
  {
    case EventValueSource::Enum::VALUE_SOURCE_NOT_SET:
    case EventValueSource::Enum::VALUE_SOURCE_NR_VALUES:
    case EventValueSource::Enum::VALUE_SOURCE_SYSTEM:
    case EventValueSource::Enum::VALUE_SOURCE_RULES:
    case EventValueSource::Enum::VALUE_SOURCE_RULES_RESTRICTED:
    break;

    case EventValueSource::Enum::VALUE_SOURCE_SERIAL:
    case EventValueSource::Enum::VALUE_SOURCE_UDP:
    case EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND:
    case EventValueSource::Enum::VALUE_SOURCE_HTTP:
    case EventValueSource::Enum::VALUE_SOURCE_MQTT:
      return true;
}
      return false;
}