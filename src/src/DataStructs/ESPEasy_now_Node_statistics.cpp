#include "../DataStructs/ESPEasy_now_Node_statistics.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Helpers/ESPEasy_time_calc.h"

unsigned long ESPEasy_now_Node_statistics_t::getAge() const
{
  return timePassedSince(last_update);
}

void ESPEasy_now_Node_statistics_t::addRoute(byte unit, const ESPEasy_now_traceroute_struct& route)
{
  if (route.getDistance() == 255) {
    return;
  }
  if (timePassedSince(last_update_route[last_route_index]) < 10000) {
    // Handling a burst of updates, only add those which have a higher success rate.
    if (routes[last_route_index].computeSuccessRate() > route.computeSuccessRate()) {
      return;
    }
  }

  ++last_route_index;

  if (last_route_index >= ESPEASY_NOW_NODE_STATISTICS_NR_ROUTES) {
    last_route_index = 0;
  }
  routes[last_route_index] = route;
  routes[last_route_index].addUnit(unit);
  routes[last_route_index].setSuccessRate_last_node(unit, success_rate);
  last_update_route[last_route_index] = millis();
  last_update       = millis();

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F(ESPEASY_NOW_NAME);
    log += F(": addRoute: ");
    log += route.toString();
    addLog(LOG_LEVEL_INFO, log);
  }
}

void ESPEasy_now_Node_statistics_t::setDiscoveryRoute(byte unit, const ESPEasy_now_traceroute_struct& route)
{
  discovery_route = route;
  discovery_route.addUnit(unit);
  discovery_route.setSuccessRate_last_node(unit, success_rate);
}

void ESPEasy_now_Node_statistics_t::updateSuccessRate(byte unit, bool success)
{
  if (success) {
    if (success_rate < 255) { ++success_rate; }
    last_update = millis();
  } else {
    if (success_rate > 0) { --success_rate; }
  }

  for (unsigned int i = 0; i < ESPEASY_NOW_NODE_STATISTICS_NR_ROUTES; ++i) {
      if (timePassedSince(last_update_route[i]) > 125000) {
          last_update_route[i] = 0;
          routes[i].clear();
      } else {
        routes[i].setSuccessRate_last_node(unit, success_rate);
      }
  }
}

ESPEasy_now_traceroute_struct& ESPEasy_now_Node_statistics_t::latestRoute()
{
  return routes[last_route_index];
}

const ESPEasy_now_traceroute_struct& ESPEasy_now_Node_statistics_t::latestRoute() const
{
  return routes[last_route_index];
}

const ESPEasy_now_traceroute_struct * ESPEasy_now_Node_statistics_t::bestRoute() const
{
//  if (timePassedSince(last_update_route[last_route_index]) < 125000) {
    int bestIndex       = -1;
    int bestSuccessRate = 0;

    for (int i = 0; i < ESPEASY_NOW_NODE_STATISTICS_NR_ROUTES; ++i) {
      const int successRate = routes[i].computeSuccessRate();

      if (successRate > bestSuccessRate) {
        bestSuccessRate = successRate;
        bestIndex       = i;
      }
    }

    if (bestIndex >= 0) {
      return &routes[bestIndex];
    }
//  }

  return nullptr;
}

const ESPEasy_now_traceroute_struct& ESPEasy_now_Node_statistics_t::discoveryRoute() const
{
    return discovery_route;
}