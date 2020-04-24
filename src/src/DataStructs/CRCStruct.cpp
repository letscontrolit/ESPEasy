#include "../DataStructs/CRCStruct.h"

#include <string.h>

bool CRCStruct::checkPassed() const
{
  return memcmp(compileTimeMD5, runTimeMD5, 16) == 0;
}
