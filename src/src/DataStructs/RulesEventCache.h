#ifndef DATASTRUCTS_RULESEVENTCACHE_H
#define DATASTRUCTS_RULESEVENTCACHE_H

#include "../../ESPEasy_common.h"

#include <vector>

struct RulesEventCache_element {
  RulesEventCache_element(const String& filename, size_t pos, const String& event, const String& action)
    : _filename(filename), _posInFile(pos), _event(event), _action(action)
  {}


  String _filename;
  size_t _posInFile;
  String _event;
  String _action;
  size_t _nrTimesMatched = 0;
};

typedef std::vector<RulesEventCache_element> RulesEventCache_vector;

class RulesEventCache {
public:

  RulesEventCache() = default;

  void clear();

  bool isInitialized() const {
    return _initialized;
  }

  void initialize();

  // Split a rules line into 2 parts:
  // - event: The part between on ... do
  // - action: The optional part after the " do"
  static bool getEventFromRulesLine(const String& line,
                                    String      & event,
                                    String      & action);

  bool addLine(const String& line,
               const String& filename,
               size_t        pos);

  RulesEventCache_vector::const_iterator findMatchingRule(const String& event);

  RulesEventCache_vector::const_iterator end() const {
    return _eventCache.end();
  }

private:

  RulesEventCache_vector _eventCache;
  bool _initialized = false;
};

#endif // ifndef DATASTRUCTS_RULESEVENTCACHE_H
