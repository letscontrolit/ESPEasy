#ifndef DATASTRUCTS_USERVARSTRUCT_H
#define DATASTRUCTS_USERVARSTRUCT_H

#include "../../ESPEasy_common.h"

#include "../DataTypes/TaskIndex.h"

#include <vector>

struct UserVarStruct {
  UserVarStruct();

  // Overloading [] operator to access elements in array style
  float       & operator[](unsigned int index);
  const float & operator[](unsigned int index) const;

  unsigned long getSensorTypeLong(taskIndex_t taskIndex) const;
  void          setSensorTypeLong(taskIndex_t   taskIndex,
                                  unsigned long value);

  // 32 bit unsigned int stored at the memory location of the float
  uint32_t getUint32(taskIndex_t taskIndex,
                     uint8_t        varNr) const;
  void     setUint32(taskIndex_t taskIndex,
                     uint8_t        varNr,
                     uint32_t    value);


  size_t getNrElements() const;

  uint8_t * get();

private:

  std::vector<float>_data;
};

#endif // ifndef DATASTRUCTS_USERVARSTRUCT_H
