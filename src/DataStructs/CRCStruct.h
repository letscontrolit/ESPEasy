#ifndef DATASTRUCTS_CRCSTRUCT_H
#define DATASTRUCTS_CRCSTRUCT_H

/*********************************************************************************************\
 * CRCStruct
\*********************************************************************************************/
struct CRCStruct{
  char compileTimeMD5[16+32+1]= "MD5_MD5_MD5_MD5_BoundariesOfTheSegmentsGoHere...";
  char binaryFilename[32+32+1]= "ThisIsTheDummyPlaceHolderForTheBinaryFilename64ByteLongFilenames";
  char compileTime[16]= __TIME__;
  char compileDate[16]= __DATE__;
  uint8_t runTimeMD5[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  bool checkPassed (void){ return memcmp(compileTimeMD5,runTimeMD5,16)==0 ; }
  uint32_t numberOfCRCBytes=0;
};


#endif // DATASTRUCTS_CRCSTRUCT_H