#include "SendData_DuplicateChecker_data.h"


#include "../Helpers/ESPEasy_time_calc.h"
#include "../../ESPEasy_fdwdecl.h"

#define TIMEOUT_ASK_FOR_DUPLICATE 100

SendData_DuplicateChecker_data::SendData_DuplicateChecker_data(EventStruct *event) {
  if (event != nullptr) {
    _event = *event;
  }
}

bool SendData_DuplicateChecker_data::doSend()
{
  if (timePassedSince(_timestamp) > TIMEOUT_ASK_FOR_DUPLICATE) {
    sendData(&_event);
    return true;
  }
  return false;
}
