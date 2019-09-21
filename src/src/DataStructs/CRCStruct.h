#ifndef DATASTRUCTS_CRCSTRUCT_H
#define DATASTRUCTS_CRCSTRUCT_H

#include <stdint.h>

/*********************************************************************************************\
* CRCStruct
\*********************************************************************************************/
struct CRCStruct {
  char     compileTimeMD5[16 + 32 + 1] = "MD5_MD5_MD5_MD5_BoundariesOfTheSegmentsGoHere...";
  char     binaryFilename[32 + 32 + 1] = "ThisIsTheDummyPlaceHolderForTheBinaryFilename64ByteLongFilenames";
  char     compileTime[16]             = __TIME__;
  char     compileDate[16]             = __DATE__;
  uint8_t  runTimeMD5[16]              = { 0 };
  uint32_t numberOfCRCBytes            = 0;

  bool checkPassed() const;
};


#endif // DATASTRUCTS_CRCSTRUCT_H
