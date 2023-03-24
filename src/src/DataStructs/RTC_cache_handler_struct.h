#ifndef DATASTRUCTS_RTC_CACHE_HANDLER_STRUCT_H
#define DATASTRUCTS_RTC_CACHE_HANDLER_STRUCT_H



#include "../../ESPEasy_common.h"

#if FEATURE_RTC_CACHE_STORAGE

#include "../DataStructs/RTCCacheStruct.h"

#include <FS.h>
#include <vector>


// Locations where to store the cached data
// As a file on the filesystem
#define CACHE_STORAGE_SPIFFS        0

// Between the sketch and FS, including OTA area (will overwrite this area when performing OTA)
#define CACHE_STORAGE_OTA_FREE      1

// Only use the free space between sketch and FS, thus avoid OTA area
#define CACHE_STORAGE_NO_OTA_FREE   2

// Use space after FS. (e.g. on 16M flash partitioned as 4M, or 4M flash partitioned as 2M)
#define CACHE_STORAGE_BEHIND_SPIFFS 3


// #define RTC_STRUCT_DEBUG

/********************************************************************************************\
   RTC located cache
 \*********************************************************************************************/
struct RTC_cache_handler_struct
{
  RTC_cache_handler_struct();

  bool         init();

  unsigned int getFreeSpace();

  void         resetpeek();

  bool         peekDataAvailable() const;

  int          getPeekFilePos(int& peekFileNr);

  int          getPeekFileSize(int peekFileNr) const;

  void         setPeekFilePos(int peekFileNr, int peekReadPos);

  bool         peek(uint8_t     *data,
                    unsigned int size);

  // Write a single sample set to the buffer
  bool write(const uint8_t *data,
             unsigned int   size);

  // Mark all content as being processed and empty buffer.
  bool flush();

  // Return usable filename for reading.
  // Will be empty if there is no file to process.
  String getReadCacheFileName(int& readPos);

  String getNextCacheFileName(int& fileNr, bool& islast);

  bool   deleteOldestCacheBlock();

  bool   deleteAllCacheBlocks();

  bool   deleteCacheBlock(int fileNr);

  // When trying to access cache files, like deleting them, these files must be closed first.
  void   closeOpenFiles();

private:

  bool     loadMetaData();

  bool     loadData();

  bool     saveRTCcache();

  bool     saveRTCcache(unsigned int startOffset,
                        size_t       nrBytes);

  uint32_t getDataChecksum();

  void     initRTCcache_data();

  void     clearRTCcacheData();

  // Return true if any cache file found
  bool     updateRTC_filenameCounters();

  void     validateFilePos(int& fileNr, int& readPos);

  bool     prepareFileForWrite();

#ifdef RTC_STRUCT_DEBUG
  void     rtc_debug_log(const String& description,
                         size_t        nrBytes);
#endif // ifdef RTC_STRUCT_DEBUG

#ifdef ESP8266
  RTC_cache_struct    RTC_cache;
  std::vector<uint8_t>RTC_cache_data;
#endif // ifdef ESP8266
  fs::File fw;  // File handler Write
  fs::File fr;  // File handler Read
  fs::File fp;  // File handler Peek
  size_t   _peekfilenr  = 0;
  size_t   _peekreadpos = 0;

  uint8_t storageLocation = CACHE_STORAGE_SPIFFS;
  bool    writeError      = false;
};

#endif
#endif // ifndef DATASTRUCTS_RTC_CACHE_HANDLER_STRUCT_H
