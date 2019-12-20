#include "src/Globals/RTC.h"
#include "src/DataStructs/RTCStruct.h"


/*********************************************************************************************\
* RTC memory stored values
\*********************************************************************************************/

/*
   During deep sleep, only RTC still working, so maybe we need to save some user data in RTC memory.
   Only user data area can be used by user.
 |<--------system data--------->|<-----------------user data--------------->|
 | 256 bytes                    | 512 bytes                                 |
   Note:
   RTC memory is 4 bytes aligned for read and write operations.
   Address parameter refers to block number(4 bytes per block).
   So, if we want to access some data at the beginning of user data area,
   address: 256/4 = 64
   data   : data pointer
   size   : data length, byte

   Prototype:
    bool system_rtc_mem_read (
      uint32 src_addr,
      void * data,
      uint32 save_size
    )

    bool system_rtc_mem_write (
      uint32 des_addr,
      void * data,
      uint32 save_size
    )
 */

// RTC layout ESPeasy:
// these offsets are in blocks, bytes = blocks * 4
// 64   RTCStruct  max 40 bytes: ( 74 - 64 ) * 4
// 74   UserVar
// 122  UserVar checksum:  RTC_BASE_USERVAR + (sizeof(UserVar) / 4)
// 128  Cache (C016) metadata  4 blocks
// 132  Cache (C016) data  6 blocks per sample => max 10 samples

// #define RTC_STRUCT_DEBUG


// Locations where to store the cached data
// As a file on the SPIFFS filesystem
#define CACHE_STORAGE_SPIFFS        0

// Between the sketch and SPIFFS, including OTA area (will overwrite this area when performing OTA)
#define CACHE_STORAGE_OTA_FREE      1

// Only use the free space between sketch and SPIFFS, thus avoid OTA area
#define CACHE_STORAGE_NO_OTA_FREE   2

// Use space after SPIFFS. (e.g. on 16M flash partitioned as 4M, or 4M flash partitioned as 2M)
#define CACHE_STORAGE_BEHIND_SPIFFS 3


/********************************************************************************************\
   Save RTC struct to RTC memory
 \*********************************************************************************************/
boolean saveToRTC(void)
{
  #if defined(ESP32)
  return false;
  #else // if defined(ESP32)

  if (!system_rtc_mem_write(RTC_BASE_STRUCT, (byte *)&RTC, sizeof(RTC)) || !readFromRTC(void))
  {
      # ifdef RTC_STRUCT_DEBUG
    addLog(LOG_LEVEL_ERROR, F("RTC  : Error while writing to RTC"));
      # endif // ifdef RTC_STRUCT_DEBUG
    return false;
  }
  else
  {
    return true;
  }
  #endif // if defined(ESP32)
}

/********************************************************************************************\
   Initialize RTC memory
 \*********************************************************************************************/
void initRTC(void)
{
  memset(&RTC, 0, sizeof(RTC));
  RTC.ID1 = 0xAA;
  RTC.ID2 = 0x55;
  saveToRTC(void);

  memset(&UserVar, 0, sizeof(UserVar));
  saveUserVarToRTC(void);
}

/********************************************************************************************\
   Read RTC struct from RTC memory
 \*********************************************************************************************/
boolean readFromRTC(void)
{
  #if defined(ESP32)
  return false;
  #else // if defined(ESP32)

  if (!system_rtc_mem_read(RTC_BASE_STRUCT, (byte *)&RTC, sizeof(RTC))) {
    return false;
  }
  return RTC.ID1 == 0xAA && RTC.ID2 == 0x55;
  #endif // if defined(ESP32)
}

/********************************************************************************************\
   Save values to RTC memory
 \*********************************************************************************************/
boolean saveUserVarToRTC(void)
{
  #if defined(ESP32)
  return false;
  #else // if defined(ESP32)

  // addLog(LOG_LEVEL_DEBUG, F("RTCMEM: saveUserVarToRTC"));
  byte    *buffer = (byte *)&UserVar;
  size_t   size   = sizeof(UserVar);
  uint32_t sum    = calc_CRC32(buffer, size);
  boolean  ret    = system_rtc_mem_write(RTC_BASE_USERVAR, buffer, size);
  ret &= system_rtc_mem_write(RTC_BASE_USERVAR + (size >> 2), (byte *)&sum, 4);
  return ret;
  #endif // if defined(ESP32)
}

/********************************************************************************************\
   Read RTC struct from RTC memory
 \*********************************************************************************************/
boolean readUserVarFromRTC(void)
{
  #if defined(ESP32)
  return false;
  #else // if defined(ESP32)

  // addLog(LOG_LEVEL_DEBUG, F("RTCMEM: readUserVarFromRTC"));
  byte    *buffer = (byte *)&UserVar;
  size_t   size   = sizeof(UserVar);
  boolean  ret    = system_rtc_mem_read(RTC_BASE_USERVAR, buffer, size);
  uint32_t sumRAM = calc_CRC32(buffer, size);
  uint32_t sumRTC = 0;
  ret &= system_rtc_mem_read(RTC_BASE_USERVAR + (size >> 2), (byte *)&sumRTC, 4);

  if (!ret || (sumRTC != sumRAM))
  {
      # ifdef RTC_STRUCT_DEBUG
    addLog(LOG_LEVEL_ERROR, F("RTC  : Checksum error on reading RTC user var"));
      # endif // ifdef RTC_STRUCT_DEBUG
    memset(buffer, 0, size);
  }
  return ret;
  #endif // if defined(ESP32)
}

/********************************************************************************************\
   RTC located cache
 \*********************************************************************************************/
struct RTC_cache_handler_struct
{
  RTC_cache_handler_struct(void) {
    bool success = loadMetaData(void) && loadData(void);

    if (!success) {
      #ifdef RTC_STRUCT_DEBUG
      addLog(LOG_LEVEL_INFO, F("RTC  : Error reading cache data"));
      #endif // ifdef RTC_STRUCT_DEBUG
      memset(&RTC_cache, 0, sizeof(RTC_cache));
      flush(void);
    } else {
      #ifdef RTC_STRUCT_DEBUG
      rtc_debug_log(F("Read from RTC cache"), RTC_cache.writePos);
      #endif // ifdef RTC_STRUCT_DEBUG
    }
  }

  unsigned int getFreeSpace(void) {
    if (RTC_cache.writePos >= RTC_CACHE_DATA_SIZE) {
      return 0;
    }
    return RTC_CACHE_DATA_SIZE - RTC_cache.writePos;
  }

  void resetpeek(void) {
    if (fp) {
      fp.close(void);
    }
    peekfilenr  = 0;
    peekreadpos = 0;
  }

  bool peek(uint8_t *data, unsigned int size) {
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

        if (fname.length(void) == 0) { return false; }
        fp = tryOpenFile(fname.c_str(void), "r");
      }

      if (!fp) { return false; }

      if (fp.read(data, size)) {
        return true;
      }
      fp.close(void);
    }
    return true;
  }

  // Write a single sample set to the buffer
  bool write(uint8_t *data, unsigned int size) {
    #ifdef RTC_STRUCT_DEBUG
    rtc_debug_log(F("write RTC cache data"), size);
    #endif // ifdef RTC_STRUCT_DEBUG

    if (getFreeSpace(void) < size) {
      if (!flush(void)) {
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
  bool flush(void) {
    if (prepareFileForWrite(void)) {
      if (RTC_cache.writePos > 0) {
        size_t filesize    = fw.size(void);
        int    bytesWriten = fw.write(&RTC_cache_data[0], RTC_cache.writePos);

        if ((bytesWriten < RTC_cache.writePos) || (fw.size(void) == filesize)) {
          #ifdef RTC_STRUCT_DEBUG
          String log = F("RTC  : error writing file. Size before: ");
          log += filesize;
          log += F(" after: ");
          log += fw.size(void);
          log += F(" writen: ");
          log += bytesWriten;
          addLog(LOG_LEVEL_ERROR, log);
          #endif // ifdef RTC_STRUCT_DEBUG
          fw.close(void);

          if (!GarbageCollection(void)) {
            // Garbage collection was not able to remove anything
            writeerror = true;
          }
          return false;
        }
        delay(0);
        fw.flush(void);
        #ifdef RTC_STRUCT_DEBUG
        addLog(LOG_LEVEL_INFO, F("RTC  : flush RTC cache"));
        #endif // ifdef RTC_STRUCT_DEBUG
        initRTCcache_data(void);
        clearRTCcacheData(void);
        saveRTCcache(void);
        return true;
      }
    }
    return false;
  }

  // Return usable filename for reading.
  // Will be empty if there is no file to process.
  String getReadCacheFileName(int& readPos) {
    initRTCcache_data(void);

    for (int i = 0; i < 2; ++i) {
      String fname = createCacheFilename(RTC_cache.readFileNr);

      if (SPIFFS.exists(fname)) {
        if (i != 0) {
          // First attempt failed, so stored read position is not valid
          RTC_cache.readPos = 0;
        }
        readPos = RTC_cache.readPos;
        return fname;
      }

      if (i == 0) {
        updateRTC_filenameCounters(void);
      }
    }

    // No file found
    RTC_cache.readPos = 0;
    readPos           = RTC_cache.readPos;
    return "";
  }

  String getPeekCacheFileName(bool& islast) {
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

    if (SPIFFS.exists(fname)) {
      return fname;
    }
    return "";
  }

  bool deleteOldestCacheBlock(void) {
    if (updateRTC_filenameCounters(void)) {
      if (RTC_cache.readFileNr != RTC_cache.writeFileNr) {
        // read and write file nr are not the same file, remove the read file nr.
        String fname = createCacheFilename(RTC_cache.readFileNr);

        if (tryDeleteFile(fname)) {
          #ifdef RTC_STRUCT_DEBUG
          String log = F("RTC  : Removed file from SPIFFS: ");
          log += fname;
          addLog(LOG_LEVEL_INFO, String(log));
          #endif // ifdef RTC_STRUCT_DEBUG
          updateRTC_filenameCounters(void);
          writeerror = false;
          return true;
        }
      }
    }
    return false;
  }

private:

  bool loadMetaData(void)
  {
    #if defined(ESP32)
    return false;
    #else // if defined(ESP32)

    if (!system_rtc_mem_read(RTC_BASE_CACHE, (byte *)&RTC_cache, sizeof(RTC_cache))) {
      return false;
    }

    return RTC_cache.checksumMetadata == calc_CRC32((byte *)&RTC_cache, sizeof(RTC_cache) - sizeof(uint32_t));
    #endif // if defined(ESP32)
  }

  bool loadData(void)
  {
    #if defined(ESP32)
    return false;
    #else // if defined(ESP32)
    initRTCcache_data(void);

    if (!system_rtc_mem_read(RTC_BASE_CACHE + (sizeof(RTC_cache) / 4), (byte *)&RTC_cache_data[0], RTC_CACHE_DATA_SIZE)) {
      return false;
    }

    if (RTC_cache.checksumData != getDataChecksum(void)) {
        # ifdef RTC_STRUCT_DEBUG
      addLog(LOG_LEVEL_ERROR, F("RTC  : Checksum error reading RTC cache data"));
        # endif // ifdef RTC_STRUCT_DEBUG
      return false;
    }
    return RTC_cache.checksumData == getDataChecksum(void);
    #endif // if defined(ESP32)
  }

  bool saveRTCcache(void) {
    return saveRTCcache(0, RTC_CACHE_DATA_SIZE);
  }

  bool saveRTCcache(unsigned int startOffset, size_t nrBytes)
  {
    #if defined(ESP32)
    return false;
    #else // if defined(ESP32)
    RTC_cache.checksumData     = getDataChecksum(void);
    RTC_cache.checksumMetadata = calc_CRC32((byte *)&RTC_cache, sizeof(RTC_cache) - sizeof(uint32_t));

    if (!system_rtc_mem_write(RTC_BASE_CACHE, (byte *)&RTC_cache, sizeof(RTC_cache)) || !loadMetaData(void))
    {
        # ifdef RTC_STRUCT_DEBUG
      addLog(LOG_LEVEL_ERROR, F("RTC  : Error while writing cache metadata to RTC"));
        # endif // ifdef RTC_STRUCT_DEBUG
      return false;
    }
    delay(0);

    if (nrBytes > 0) { // Check needed?
      const size_t address = RTC_BASE_CACHE + ((sizeof(RTC_cache) + startOffset) / 4);

      if (!system_rtc_mem_write(address, (byte *)&RTC_cache_data[startOffset], nrBytes))
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
    #endif // if defined(ESP32)
  }

  uint32_t getDataChecksum(void) {
    initRTCcache_data(void);
    size_t dataLength = RTC_cache.writePos;

    if (dataLength > RTC_CACHE_DATA_SIZE) {
      // Is this allowed to happen?
      dataLength = RTC_CACHE_DATA_SIZE;
    }

    // Only compute the checksum over the number of samples stored.
    return calc_CRC32((byte *)&RTC_cache_data[0], /*dataLength*/ RTC_CACHE_DATA_SIZE);
  }

  void initRTCcache_data(void) {
    if (RTC_cache_data.size(void) != RTC_CACHE_DATA_SIZE) {
      RTC_cache_data.resize(RTC_CACHE_DATA_SIZE);
    }

    if (RTC_cache.writeFileNr == 0) {
      // RTC value not reliable
      updateRTC_filenameCounters(void);
    }
  }

  void clearRTCcacheData(void) {
    for (size_t i = 0; i < RTC_CACHE_DATA_SIZE; ++i) {
      RTC_cache_data[i] = 0;
    }
    RTC_cache.writePos = 0;
  }

  // Return true if any cache file found
  bool updateRTC_filenameCounters(void) {
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

  bool prepareFileForWrite(void) {
    //    if (storageLocation != CACHE_STORAGE_SPIFFS) {
    //      return false;
    //    }
    if (SpiffsFull(void)) {
      #ifdef RTC_STRUCT_DEBUG
      addLog(LOG_LEVEL_ERROR, String(F("RTC  : SPIFFS full")));
      #endif // ifdef RTC_STRUCT_DEBUG
      return false;
    }
    unsigned int retries = 3;

    while (retries > 0) {
      --retries;

      if (fw && (fw.size(void) >= CACHE_FILE_MAX_SIZE)) {
        fw.close(void);
        GarbageCollection(void);
      }

      if (!fw) {
        // Open file to write
        initRTCcache_data(void);

        if (updateRTC_filenameCounters(void)) {
          if (writeerror || (SpiffsFreeSpace(void) < ((2 * CACHE_FILE_MAX_SIZE) + SpiffsBlocksize(void)))) {
            // Not enough room for another file, remove the oldest one.
            deleteOldestCacheBlock(void);
          }
        }

        String fname = createCacheFilename(RTC_cache.writeFileNr);
        fw = tryOpenFile(fname.c_str(void), "a+");

        if (!fw) {
          #ifdef RTC_STRUCT_DEBUG
          addLog(LOG_LEVEL_ERROR, String(F("RTC  : error opening file")));
          #endif // ifdef RTC_STRUCT_DEBUG
        } else {
          #ifdef RTC_STRUCT_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("Write to ");
            log += fname;
            log += F(" size");
            rtc_debug_log(log, fw.size(void));
          }
          #endif // ifdef RTC_STRUCT_DEBUG
        }
      }
      delay(0);

      if (fw && (fw.size(void) < CACHE_FILE_MAX_SIZE)) {
        return true;
      }
    }
    #ifdef RTC_STRUCT_DEBUG
    addLog(LOG_LEVEL_ERROR, String(F("RTC  : prepareFileForWrite failed")));
    #endif // ifdef RTC_STRUCT_DEBUG
    return false;
  }

#ifdef RTC_STRUCT_DEBUG
  void rtc_debug_log(const String& description, size_t nrBytes) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      log.reserve(18 + description.length(void));
      log  = F("RTC  : ");
      log += description;
      log += ' ';
      log += nrBytes;
      log += F(" bytes");
      addLog(LOG_LEVEL_INFO, log);
    }
  }

#endif // ifdef RTC_STRUCT_DEBUG

  RTC_cache_struct    RTC_cache;
  std::vector<uint8_t>RTC_cache_data;
  File                fw;
  File                fr;
  File                fp;
  size_t              peekfilenr  = 0;
  size_t              peekreadpos = 0;

  byte storageLocation = CACHE_STORAGE_SPIFFS;
  bool writeerror      = false;
};
