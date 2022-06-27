#include "../DataStructs/RulesEventCache.h"

#include "../DataStructs/TimingStats.h"
#include "../Helpers/RulesMatcher.h"


void RulesEventCache::clear()
{
  _eventCache.clear();
  _initialized = false;
}

void RulesEventCache::initialize()
{
  _initialized = true;
}

bool RulesEventCache::addLine(const String& line, const String& filename, size_t pos)
{
  String event, action;

  if (getEventFromRulesLine(line, event, action)) {
    _eventCache.emplace_back(filename, pos, std::move(event), std::move(action));
    return true;
  }
  return false;
}

RulesEventCache_vector::const_iterator RulesEventCache::findMatchingRule(const String& event, bool optimize)
{
  RulesEventCache_vector::iterator it   = _eventCache.begin();
//  RulesEventCache_vector::iterator prev = _eventCache.end();

  // FIXME TD-er: Disable optimize as it has some side effects.
  // For example, matching a specific event first and then a more generic one is perfectly normal to do.
  // But this optimization will then put the generic one in front as it will be matched more often.
  // Thus it will never match the more specific one anymore.


  for (; it != _eventCache.end(); ++it)
  {
    START_TIMER
    const bool match = ruleMatch(event, it->_event);
    STOP_TIMER(RULES_MATCH);

    if (match) {
      /*
      if (optimize) {
        it->_nrTimesMatched++;

        if (prev != _eventCache.end()) {
          // Check to see if we need to place this one more to the front of the vector
          // to speed up parsing.
          if (prev->_nrTimesMatched < it->_nrTimesMatched) {
            std::swap(*prev, *it);
            return prev;
          }
        }
      }
      */
      return it;
    }
/*
    if (optimize) {
      if (prev == _eventCache.end()) {
        prev = it;
      }
      else if (prev->_nrTimesMatched > it->_nrTimesMatched) {
        // Found one that's having a lower match rate
        prev = it;
      }
    }
    */
  }
  return it;
}
