#include "../DataStructs/RTC_cache_handler_struct.h"

#include "../../ESPEasy_common.h"
#include "../DataStructs/RTCStruct.h"
#include "../Helpers/CRC_functions.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/StringConverter.h"

#include "../ESPEasyCore/ESPEasy_backgroundtasks.h"
#include "../ESPEasyCore/ESPEasy_Log.h"

#ifdef ESP8266
# include <user_interface.h>
#endif // ifdef ESP8266

#ifdef ESP32
  # include <soc/rtc.h>

// For ESP32 the RTC mapped structure may not be a member of an object,
// but must be declared 'static'
// This also means we can only have a single instance of this
// RTC_cache_handler_struct.
RTC_NOINIT_ATTR RTC_cache_struct RTC_cache;
RTC_NOINIT_ATTR uint8_t RTC_cache_data[RTC_CACHE_DATA_SIZE];
#endif // ifdef ESP32


/********************************************************************************************\
   RTC located cache
 \*********************************************************************************************/
RTC_cache_handler_struct::RTC_cache_handler_struct() {
  bool success = loadMetaData() && loadData();

  if (!success) {
      #ifdef RTC_STRUCT_DEBUG
    addLog(LOG_LEVEL_INFO, F("RTC  : Error reading cache data"));
      #endif // ifdef RTC_STRUCT_DEBUG
    RTC_cache.init();
    flush();
  } else {
      #ifdef RTC_STRUCT_DEBUG
    rtc_debug_log(F("Read from RTC cache"), RTC_cache.writePos);
      #endif // ifdef RTC_STRUCT_DEBUG
  }
}

unsigned int RTC_cache_handler_struct::getFreeSpace() {
  if (RTC_cache.writePos >= RTC_CACHE_DATA_SIZE) {
    return 0;
  }
  return RTC_CACHE_DATA_SIZE - RTC_cache.writePos;
}

void RTC_cache_handler_struct::resetpeek() {
  if (fp) {
    fp.close();
  }
  peekfilenr  = 0;
  peekreadpos = 0;
}

bool RTC_cache_handler_struct::peek(uint8_t *data, unsigned int size) {
  int retries = 2;

  while (retries > 0) {
    --retries;

    if (!fp) {
      int tmppos;
      String fname;

      if (peekfilenr == 0) {
        fname      = getReadCacheFileName(tmppos);
        peekfilenr = getCacheFileCountFromFilename(fname);
      } else {
        ++peekfilenr;
        fname = createCacheFilename(peekfilenr);
      }

      if (fname.isEmpty()) { return false; }
      fp = tryOpenFile(fname, "r");
    }

    if (!fp) { return false; }

    if (fp.read(data, size)) {
      return true;
    }
    fp.close();
  }
  return true;
}

// Write a single sample set to the buffer
bool RTC_cache_handler_struct::write(const uint8_t *data, unsigned int size) {
    #ifdef RTC_STRUCT_DEBUG
  rtc_debug_log(F("write RTC cache data"), size);
    #endif // ifdef RTC_STRUCT_DEBUG

  if (getFreeSpace() < size) {
    if (!flush()) {
      return false;
    }
  }

  // First store it in the buffer
  for (unsigned int i = 0; i < size; ++i) {
    RTC_cache_data[RTC_cache.writePos] = data[i];
    ++RTC_cache.writePos;
  }

  // Now store the updated part of the buffer to the RTC memory.
  // Pad some extra bytes around it to allow sample sizes not multiple of 4 bytes.
  int startOffset = RTC_cache.writePos - size;
  startOffset -= startOffset % 4;

  if (startOffset < 0) {
    startOffset = 0;
  }
  int nrBytes = RTC_cache.writePos - startOffset;

  if (nrBytes % 4 != 0) {
    nrBytes -= nrBytes % 4;
    nrBytes += 4;
  }

  if ((nrBytes + startOffset) >  RTC_CACHE_DATA_SIZE) {
    // Can this happen?
    nrBytes = RTC_CACHE_DATA_SIZE - startOffset;
  }
  return saveRTCcache(startOffset, nrBytes);
}

// Mark all content as being processed and empty buffer.
bool RTC_cache_handler_struct::flush() {
  if (prepareFileForWrite()) {
    if (RTC_cache.writePos > 0) {
      #ifdef RTC_STRUCT_DEBUG
      size_t filesize = fw.size();
      #endif // ifdef RTC_STRUCT_DEBUG
      int bytesWritten = fw.write(&RTC_cache_data[0], RTC_cache.writePos);

      delay(0);
      fw.flush();
        #ifdef RTC_STRUCT_DEBUG
      addLog(LOG_LEVEL_INFO, F("RTC  : flush RTC cache"));
        #endif // ifdef RTC_STRUCT_DEBUG


      if ((bytesWritten < RTC_cache.writePos) /*|| (fw.size() == filesize)*/) {
          #ifdef RTC_STRUCT_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log = F("RTC  : error writing file. Size before: ");
          log += filesize;
          log += F(" after: ");
          log += fw.size();
          log += F(" written: ");
          log += bytesWritten;
          addLogMove(LOG_LEVEL_ERROR, log);
        }
          #endif // ifdef RTC_STRUCT_DEBUG
        fw.close();

        if (!GarbageCollection()) {
          // Garbage collection was not able to remove anything
          writeError = true;
        }
        return false;
      }
      initRTCcache_data();
      clearRTCcacheData();
      saveRTCcache();
      return true;
    }
  }
  return false;
}

// Return usable filename for reading.
// Will be empty if there is no file to process.
String RTC_cache_handler_struct::getReadCacheFileName(int& readPos) {
  initRTCcache_data();

  for (int i = 0; i < 2; ++i) {
    String fname = createCacheFilename(RTC_cache.readFileNr);

    if (fileExists(fname)) {
      if (i != 0) {
        // First attempt failed, so stored read position is not valid
        RTC_cache.readPos = 0;
      }
      readPos = RTC_cache.readPos;
      return fname;
    }

    if (i == 0) {
      updateRTC_filenameCounters();
    }
  }

  // No file found
  RTC_cache.readPos = 0;
  readPos           = RTC_cache.readPos;
  return EMPTY_STRING;
}

String RTC_cache_handler_struct::getPeekCacheFileName(bool& islast) {
  int tmppos;
  String fname;

  if (peekfilenr == 0) {
    fname      = getReadCacheFileName(tmppos);
    peekfilenr = getCacheFileCountFromFilename(fname);
  } else {
    ++peekfilenr;
    fname = createCacheFilename(peekfilenr);
  }
  islast = peekfilenr > RTC_cache.writeFileNr;

  if (fileExists(fname)) {
    return fname;
  }
  return EMPTY_STRING;
}

bool RTC_cache_handler_struct::deleteOldestCacheBlock() {
  if (updateRTC_filenameCounters()) {
    const int nrCacheFiles = RTC_cache.writeFileNr - RTC_cache.readFileNr;

    if (nrCacheFiles > 1) {
      // read and write file nr are not the same file, remove the read file nr.
      String fname = createCacheFilename(RTC_cache.readFileNr);

      writeError = false;

      if (tryDeleteFile(fname)) {
          #ifdef RTC_STRUCT_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("RTC  : Removed file from FS: ");
          log += fname;
          addLogMove(LOG_LEVEL_INFO, log);
        }
          #endif // ifdef RTC_STRUCT_DEBUG
        updateRTC_filenameCounters();
        return true;
      }
    }
  }
#ifdef RTC_STRUCT_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, F("RTC  : No Cache files found"));
  }
#endif // ifdef RTC_STRUCT_DEBUG
  return false;
}

bool RTC_cache_handler_struct::deleteAllCacheBlocks()
{
  if (updateRTC_filenameCounters()) {
    const int nrCacheFiles = RTC_cache.writeFileNr - RTC_cache.readFileNr;

    if (nrCacheFiles > 1) {
      bool fileDeleted = false;
      int  count       = 0;

      for (int fileNr = RTC_cache.readFileNr; count < 25 && fileNr < RTC_cache.writeFileNr; ++fileNr)
      {
        String fname = createCacheFilename(fileNr);

        if (tryDeleteFile(fname)) {
          ++count;
          fileDeleted = true;
          #ifdef RTC_STRUCT_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLogMove(LOG_LEVEL_INFO, concat(F("RTC  : Removed file from FS: "), fname));
          }
          #endif // ifdef RTC_STRUCT_DEBUG
          backgroundtasks();
        }
      }

      if (fileDeleted) {
        writeError = false;
        updateRTC_filenameCounters();
        return true;
      }
    }
  }
  return false;
}

bool RTC_cache_handler_struct::loadMetaData()
{
  // No need to load on ESP32, as the data is already allocated to the RTC memory by the compiler

  #ifdef ESP8266

  if (!system_rtc_mem_read(RTC_BASE_CACHE, reinterpret_cast<uint8_t *>(&RTC_cache), sizeof(RTC_cache))) {
    return false;
  }
  #endif // ifdef ESP8266

  return RTC_cache.checksumMetadata == calc_CRC32(reinterpret_cast<const uint8_t *>(&RTC_cache), sizeof(RTC_cache) - sizeof(uint32_t));
}

bool RTC_cache_handler_struct::loadData()
{
  initRTCcache_data();

  // No need to load on ESP32, as the data is already allocated to the RTC memory by the compiler
  #ifdef ESP8266

  if (!system_rtc_mem_read(RTC_BASE_CACHE + (sizeof(RTC_cache) / 4), reinterpret_cast<uint8_t *>(&RTC_cache_data[0]), RTC_CACHE_DATA_SIZE)) {
    return false;
  }
  #endif // ifdef ESP8266

  if (RTC_cache.checksumData != getDataChecksum()) {
        #ifdef RTC_STRUCT_DEBUG
    addLog(LOG_LEVEL_ERROR, F("RTC  : Checksum error reading RTC cache data"));
        #endif // ifdef RTC_STRUCT_DEBUG
    return false;
  }
  return RTC_cache.checksumData == getDataChecksum();
}

bool RTC_cache_handler_struct::saveRTCcache() {
  return saveRTCcache(0, RTC_CACHE_DATA_SIZE);
}

bool RTC_cache_handler_struct::saveRTCcache(unsigned int startOffset, size_t nrBytes)
{
  RTC_cache.checksumData     = getDataChecksum();
  RTC_cache.checksumMetadata = calc_CRC32(reinterpret_cast<const uint8_t *>(&RTC_cache), sizeof(RTC_cache) - sizeof(uint32_t));
  #ifdef ESP32
  return true;
  #endif // ifdef ESP32

  #ifdef ESP8266

  if (!system_rtc_mem_write(RTC_BASE_CACHE, reinterpret_cast<const uint8_t *>(&RTC_cache), sizeof(RTC_cache)) || !loadMetaData())
  {
        # ifdef RTC_STRUCT_DEBUG
    addLog(LOG_LEVEL_ERROR, F("RTC  : Error while writing cache metadata to RTC"));
        # endif // ifdef RTC_STRUCT_DEBUG
    return false;
  }
  delay(0);

  if (nrBytes > 0) { // Check needed?
    const size_t address = RTC_BASE_CACHE + ((sizeof(RTC_cache) + startOffset) / 4);

    if (!system_rtc_mem_write(address, reinterpret_cast<const uint8_t *>(&RTC_cache_data[startOffset]), nrBytes))
    {
          # ifdef RTC_STRUCT_DEBUG
      addLog(LOG_LEVEL_ERROR, F("RTC  : Error while writing cache data to RTC"));
          # endif // ifdef RTC_STRUCT_DEBUG
      return false;
    }
        # ifdef RTC_STRUCT_DEBUG
    rtc_debug_log(F("Write cache data to RTC"), nrBytes);
        # endif // ifdef RTC_STRUCT_DEBUG
  }
  return true;
  #endif        // ifdef ESP8266
}

uint32_t RTC_cache_handler_struct::getDataChecksum() {
  initRTCcache_data();

  /*
     size_t dataLength = RTC_cache.writePos;

     if (dataLength > RTC_CACHE_DATA_SIZE) {
     // Is this allowed to happen?
     dataLength = RTC_CACHE_DATA_SIZE;
     }
   */

  // Only compute the checksum over the number of samples stored.
  return calc_CRC32(reinterpret_cast<const uint8_t *>(&RTC_cache_data[0]), /*dataLength*/ RTC_CACHE_DATA_SIZE);
}

void RTC_cache_handler_struct::initRTCcache_data() {
  #ifdef ESP8266

  if (RTC_cache_data.size() != RTC_CACHE_DATA_SIZE) {
    RTC_cache_data.resize(RTC_CACHE_DATA_SIZE);
  }
  #endif // ifdef ESP8266

  if (RTC_cache.writeFileNr == 0) {
    // RTC value not reliable
    updateRTC_filenameCounters();
  }
}

void RTC_cache_handler_struct::clearRTCcacheData() {
  for (size_t i = 0; i < RTC_CACHE_DATA_SIZE; ++i) {
    RTC_cache_data[i] = 0;
  }
  RTC_cache.writePos = 0;
}

// Return true if any cache file found
bool RTC_cache_handler_struct::updateRTC_filenameCounters() {
  size_t filesizeHighest;

  if (getCacheFileCounters(RTC_cache.readFileNr, RTC_cache.writeFileNr, filesizeHighest)) {
    if (filesizeHighest >= CACHE_FILE_MAX_SIZE) {
      // Start new file
      ++RTC_cache.writeFileNr;
    }
    return true;
  } else {
    // Do not use 0, since that will be the cleared content of the struct, indicating invalid RTC data.
    RTC_cache.writeFileNr = 1;
  }
  return false;
}

bool RTC_cache_handler_struct::prepareFileForWrite() {
  //    if (storageLocation != CACHE_STORAGE_SPIFFS) {
  //      return false;
  //    }
  if (SpiffsFull()) {
      #ifdef RTC_STRUCT_DEBUG
    addLog(LOG_LEVEL_ERROR, F("RTC  : FS full"));
      #endif // ifdef RTC_STRUCT_DEBUG
    return false;
  }
  unsigned int retries = 3;

  while (retries > 0) {
    --retries;

    if (fw && (fw.size() >= CACHE_FILE_MAX_SIZE)) {
      fw.close();
      GarbageCollection();
    }

    if (!fw) {
      // Open file to write
      initRTCcache_data();

      if (updateRTC_filenameCounters()) {
        if (writeError || (SpiffsFreeSpace() < ((2 * CACHE_FILE_MAX_SIZE) + SpiffsBlocksize()))) {
          // Not enough room for another file, remove the oldest one.
          deleteOldestCacheBlock();
        }
      }

      String fname = createCacheFilename(RTC_cache.writeFileNr);
      fw = tryOpenFile(fname, "a+");

      if (!fw) {
          #ifdef RTC_STRUCT_DEBUG
        addLog(LOG_LEVEL_ERROR, F("RTC  : error opening file"));
          #endif // ifdef RTC_STRUCT_DEBUG
      } else {
          #ifdef RTC_STRUCT_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("Write to ");
          log += fname;
          log += F(" size");
          rtc_debug_log(log, fw.size());
        }
          #endif // ifdef RTC_STRUCT_DEBUG
      }
    }
    delay(0);

    if (fw && (fw.size() < CACHE_FILE_MAX_SIZE)) {
      return true;
    }
  }
    #ifdef RTC_STRUCT_DEBUG
  addLog(LOG_LEVEL_ERROR, F("RTC  : prepareFileForWrite failed"));
    #endif // ifdef RTC_STRUCT_DEBUG
  return false;
}

#ifdef RTC_STRUCT_DEBUG
void RTC_cache_handler_struct::rtc_debug_log(const String& description, size_t nrBytes) {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;

    if (log.reserve(18 + description.length())) {
      log  = F("RTC  : ");
      log += description;
      log += ' ';
      log += nrBytes;
      log += F(" bytes");
      addLogMove(LOG_LEVEL_INFO, log);
    }
  }
}

#endif // ifdef RTC_STRUCT_DEBUG
