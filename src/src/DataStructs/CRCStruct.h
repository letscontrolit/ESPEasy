#ifndef DATASTRUCTS_CRCSTRUCT_H
#define DATASTRUCTS_CRCSTRUCT_H

#include <stdint.h>

/*********************************************************************************************\
* CRCStruct
\*********************************************************************************************/
struct CRCStruct {
  char     crcStructTagID[16] =
#if defined(ESP8266)
// 1234567890123456
  "ID_EASY_ESP8266";
#elif defined(ESP32)
  "ID_EASY_ESP32";
#endif
  int offsetof_compileTimeMD5 = __builtin_offsetof(CRCStruct,compileTimeMD5);
  int offsetof_binaryFilename = __builtin_offsetof(CRCStruct,binaryFilename);
  int offsetof_compileTime    = __builtin_offsetof(CRCStruct,compileTime);
  int offsetof_compileDate    = __builtin_offsetof(CRCStruct,compileDate);
  int offset_last_one         = 0;
  char     compileTimeMD5[16 + 32 + 1] = "MD5_MD5_MD5_MD5_BoundariesOfTheSegmentsGoHere...";
  char     binaryFilename[32 + 32 + 1] = "ThisIsTheDummyPlaceHolderForTheBinaryFilename64ByteLongFilenames";
  char     compileTime[16]             = __TIME__;
  char     compileDate[16]             = __DATE__;
  uint8_t  runTimeMD5[16]              = { 0 };
  uint32_t numberOfCRCBytes            = 0;

  bool checkPassed() const;
};


#endif // DATASTRUCTS_CRCSTRUCT_H
