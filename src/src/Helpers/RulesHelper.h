#ifndef HELPERS_RULESHELPER_H
#define HELPERS_RULESHELPER_H

#include "../../ESPEasy_common.h"

#include <FS.h>
#include <map>

// Helper class to handle reading from the rules file(s).
// Opening a file on ESP32 with a relatively large LittleFS file system
// takes a considerable amount of time.
// Also there is no need to parse the rules file for events which will
// not be handled at all as they are not described in the rules files.
// Thus we must also provide some kind of caching of handled in the rules files.


class RulesHelperClass {
public:

  RulesHelperClass();

  ~RulesHelperClass();

  void closeAllFiles();

private:

#ifdef ESP8266
  size_t read(const String& filename,
              size_t      & pos,
              uint8_t      *buffer,
              size_t        length);

#endif // ifdef ESP8266

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

#ifdef ESP32

  // Cache the entire rules file contents in memory
  typedef std::vector<String>          RulesLines;
  typedef std::map<String, RulesLines> FileHandleMap;
#else // ifdef ESP32

  // Keep a handle to a file for low-memory systems
  typedef std::map<String, fs::File> FileHandleMap;
#endif // ifdef ESP32

  FileHandleMap _fileHandleMap;
};

#endif // ifndef HELPERS_RULESHELPER_H
