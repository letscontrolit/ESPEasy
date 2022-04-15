#ifndef HELPERS_RULESHELPER_H
#define HELPERS_RULESHELPER_H

#include "../../ESPEasy_common.h"

#include "../DataStructs/RulesEventCache.h"

#include <FS.h>
#include <map>

#ifdef ESP32
# define CACHE_RULES_IN_MEMORY
#endif // ifdef ESP32


// Helper class to handle reading from the rules file(s).
// Opening a file on ESP32 with a relatively large LittleFS file system
// takes a considerable amount of time.
// Also there is no need to parse the rules file for events which will
// not be handled at all as they are not described in the rules files.
// Thus we must also provide some kind of caching of handled in the rules files.

#ifdef CACHE_RULES_IN_MEMORY
# include <vector>
#endif // ifdef CACHE_RULES_IN_MEMORY


class RulesHelperClass {
public:

  RulesHelperClass();

  ~RulesHelperClass();

  void closeAllFiles();

  void init();

  bool findMatchingRule(const String& event,
                        String      & filename,
                        size_t      & pos);

private:

#ifndef CACHE_RULES_IN_MEMORY
  size_t read(const String& filename,
              size_t      & pos,
              uint8_t      *buffer,
              size_t        length);

#endif // ifndef CACHE_RULES_IN_MEMORY

  bool addChar(char    c,
               String& line,
               bool  & firstNonSpaceRead,
               bool  & commentFound);

public:

  String readLn(const String& filename,
                size_t      & pos,
                bool        & moreAvailable,
                bool          searchNextOnBlock);

private:

#ifdef CACHE_RULES_IN_MEMORY

  // Cache the entire rules file contents in memory
  typedef std::vector<String>         RulesLines;
  typedef std::map<String, RulesLines>FileHandleMap;
#else // ifdef CACHE_RULES_IN_MEMORY

  // Keep a handle to a file for low-memory systems
  typedef std::map<String, fs::File> FileHandleMap;
#endif // ifdef CACHE_RULES_IN_MEMORY

  RulesEventCache _eventCache;

  FileHandleMap _fileHandleMap;
};

#endif // ifndef HELPERS_RULESHELPER_H
