#ifndef DATASTRUCTS_RTC_CACHE_STRUCT_H
#define DATASTRUCTS_RTC_CACHE_STRUCT_H

#include "../../ESPEasy_common.h"

/********************************************************************************************\
   RTC_cache_struct
 \*********************************************************************************************/
struct RTC_cache_struct
{
  void init() {
    checksumData     = 0;
    readFileNr       = 0;
    writeFileNr      = 0;
    readPos          = 0;
    writePos         = 0;
    checksumMetadata = 0;
  }

  uint32_t checksumData     = 0;
  uint16_t readFileNr       = 0; // File number used to read from.
  uint16_t writeFileNr      = 0; // File number to write to.
  uint16_t readPos          = 0; // Read position in file based cache
  uint16_t writePos         = 0; // Write position in the RTC memory
  uint32_t checksumMetadata = 0;
};

#endif // ifndef DATASTRUCTS_RTC_CACHE_STRUCT_H
