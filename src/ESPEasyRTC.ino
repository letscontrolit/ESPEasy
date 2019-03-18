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
// 128  Cache (C014) metadata  4 blocks
// 132  Cache (C014) data  6 blocks per sample => max 10 samples




/********************************************************************************************\
  Save RTC struct to RTC memory
  \*********************************************************************************************/
boolean saveToRTC()
{
  #if defined(ESP32)
    return false;
  #else
    if (!system_rtc_mem_write(RTC_BASE_STRUCT, (byte*)&RTC, sizeof(RTC)) || !readFromRTC())
    {
      addLog(LOG_LEVEL_ERROR, F("RTC  : Error while writing to RTC"));
      return(false);
    }
    else
    {
      return(true);
    }
  #endif
}


/********************************************************************************************\
  Initialize RTC memory
  \*********************************************************************************************/
void initRTC()
{
  memset(&RTC, 0, sizeof(RTC));
  RTC.ID1 = 0xAA;
  RTC.ID2 = 0x55;
  saveToRTC();

  memset(&UserVar, 0, sizeof(UserVar));
  saveUserVarToRTC();
}

/********************************************************************************************\
  Read RTC struct from RTC memory
  \*********************************************************************************************/
boolean readFromRTC()
{
  #if defined(ESP32)
    return false;
  #else
    if (!system_rtc_mem_read(RTC_BASE_STRUCT, (byte*)&RTC, sizeof(RTC)))
      return(false);
    return (RTC.ID1 == 0xAA && RTC.ID2 == 0x55);
  #endif
}


/********************************************************************************************\
  Save values to RTC memory
\*********************************************************************************************/
boolean saveUserVarToRTC()
{
  #if defined(ESP32)
    return false;
  #else
    //addLog(LOG_LEVEL_DEBUG, F("RTCMEM: saveUserVarToRTC"));
    byte* buffer = (byte*)&UserVar;
    size_t size = sizeof(UserVar);
    uint32_t sum = calc_CRC32(buffer, size);
    boolean ret = system_rtc_mem_write(RTC_BASE_USERVAR, buffer, size);
    ret &= system_rtc_mem_write(RTC_BASE_USERVAR+(size>>2), (byte*)&sum, 4);
    return ret;
  #endif
}


/********************************************************************************************\
  Read RTC struct from RTC memory
\*********************************************************************************************/
boolean readUserVarFromRTC()
{
  #if defined(ESP32)
    return false;
  #else
    //addLog(LOG_LEVEL_DEBUG, F("RTCMEM: readUserVarFromRTC"));
    byte* buffer = (byte*)&UserVar;
    size_t size = sizeof(UserVar);
    boolean ret = system_rtc_mem_read(RTC_BASE_USERVAR, buffer, size);
    uint32_t sumRAM = calc_CRC32(buffer, size);
    uint32_t sumRTC = 0;
    ret &= system_rtc_mem_read(RTC_BASE_USERVAR+(size>>2), (byte*)&sumRTC, 4);
    if (!ret || sumRTC != sumRAM)
    {
      addLog(LOG_LEVEL_ERROR, F("RTC  : Checksum error on reading RTC user var"));
      memset(buffer, 0, size);
    }
    return ret;
  #endif
}


/********************************************************************************************\
  RTC located cache
\*********************************************************************************************/
struct RTC_cache_handler_struct
{
  RTC_cache_handler_struct() {
    bool success = loadMetaData() && loadData();
    if (!success) {
      addLog(LOG_LEVEL_INFO, F("RTC  : Error reading cache data"));
      memset(&RTC_cache, 0, sizeof(RTC_cache));
      flush();
    } else {
      String log = F("RTC  : Read from cache: ");
      log += RTC_cache.writePos;
      addLog(LOG_LEVEL_INFO, log);
    }
  }

  unsigned int getFreeSpace() {
    if (RTC_cache.writePos >= RTC_CACHE_DATA_SIZE) {
      return 0;
    }
    return RTC_CACHE_DATA_SIZE - RTC_cache.writePos;
  }

  // Write a single sample set to the buffer
  bool write(uint8_t* data, unsigned int size) {
    addLog(LOG_LEVEL_INFO, F("RTC  : write RTC cache data"));
    if (getFreeSpace() < size) {
      return false;
    }
    // First store it in the buffer
    for (unsigned int i = 0; i < size; ++i) {
      RTC_cache_data[RTC_cache.writePos] = data[i];
      ++RTC_cache.writePos;
    }
/*
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
    if (nrBytes > 0) { // Check needed?
      const size_t address = RTC_BASE_CACHE + ((sizeof(RTC_cache) + startOffset) / 4);
      system_rtc_mem_write(address, (byte*)&RTC_cache_data[startOffset], nrBytes);
    }
  */
    return saveRTCcache();
  }

  uint8_t* getBuffer(unsigned int& size) {
    size = RTC_cache.writePos;
    return &RTC_cache_data[0];
  }

  // Mark all content as being processed and empty buffer.
  void flush() {
    addLog(LOG_LEVEL_INFO, F("RTC  : flush RTC cache"));
    initRTCcache_data(true);
    saveRTCcache();
  }

private:

  bool loadMetaData()
  {
    #if defined(ESP32)
      return false;
    #else
      if (!system_rtc_mem_read(RTC_BASE_CACHE, (byte*)&RTC_cache, sizeof(RTC_cache)))
        return(false);

      return (RTC_cache.checksumMetadata == calc_CRC32((byte*)&RTC_cache, sizeof(RTC_cache) - sizeof(uint32_t)));
    #endif
  }

  bool loadData()
  {
    #if defined(ESP32)
      return false;
    #else
      initRTCcache_data(true);
      if (!system_rtc_mem_read(RTC_BASE_CACHE + (sizeof(RTC_cache) / 4), (byte*)&RTC_cache_data[0], RTC_CACHE_DATA_SIZE))
        return(false);

      if (RTC_cache.checksumData != getDataChecksum()) {
        addLog(LOG_LEVEL_ERROR, F("RTC  : Checksum error reading RTC cache data"));
        return(false);
      }
      return (RTC_cache.checksumData == getDataChecksum());
    #endif
  }

  bool saveRTCcache()
  {
    #if defined(ESP32)
      return false;
    #else
      RTC_cache.checksumData = getDataChecksum();
      RTC_cache.checksumMetadata = calc_CRC32((byte*)&RTC_cache, sizeof(RTC_cache) - sizeof(uint32_t));
      if (!system_rtc_mem_write(RTC_BASE_CACHE, (byte*)&RTC_cache, sizeof(RTC_cache)) || !loadMetaData())
      {
        addLog(LOG_LEVEL_ERROR, F("RTC  : Error while writing cache metadata to RTC"));
        return(false);
      }
      delay(0);
      if (!system_rtc_mem_write(RTC_BASE_CACHE + (sizeof(RTC_cache) / 4), (byte*)&RTC_cache_data[0], RTC_CACHE_DATA_SIZE))
      {
        addLog(LOG_LEVEL_ERROR, F("RTC  : Error while writing cache data to RTC"));
        return(false);
      }
      addLog(LOG_LEVEL_INFO, F("RTC  : Write RTC cache"));
      return(true);
    #endif
  }

  uint32_t getDataChecksum() {
    initRTCcache_data(false);
    size_t dataLength = RTC_cache.writePos;
    if (dataLength > RTC_CACHE_DATA_SIZE) {
      // Is this allowed to happen?
      dataLength = RTC_CACHE_DATA_SIZE;
    }
    // Only compute the checksum over the number of samples stored.
    return calc_CRC32((byte*)&RTC_cache_data[0], RTC_CACHE_DATA_SIZE);
  }

  void initRTCcache_data(bool clear) {
    if (RTC_cache_data.size() != RTC_CACHE_DATA_SIZE) {
      RTC_cache_data.resize(RTC_CACHE_DATA_SIZE);
    }
    if (clear) {
      for (size_t i = 0; i < RTC_CACHE_DATA_SIZE; ++i) {
        RTC_cache_data[i] = 0;
      }
      RTC_cache.writePos = 0;
    }
  }



private:

  RTC_cache_struct RTC_cache;
  std::vector<uint8_t> RTC_cache_data;
};
