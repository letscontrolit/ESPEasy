#include "../Globals/RulesCalculate.h"

#include "../DataStructs/TimingStats.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringConverter_Numerical.h"

RulesCalculate_t RulesCalculate{};

/*******************************************************************************************
* Helper functions to actually interact with the rules calculation functions.
* *****************************************************************************************/
int CalculateParam(const String& TmpStr, int errorValue) {
  int32_t returnValue = errorValue;

  if (TmpStr.length() == 0) {
    return returnValue;
  }

  // Minimize calls to the Calulate function.
  // Only if TmpStr starts with '=' then call Calculate(). Otherwise do not call it
  if (TmpStr[0] != '=') {
    if (!validIntFromString(TmpStr, returnValue)) {
      return errorValue;
    }
  } else {
    ESPEASY_RULES_FLOAT_TYPE param{};

    // Starts with an '=', so Calculate starting at next position
    CalculateReturnCode returnCode = Calculate(TmpStr.substring(1), param);

    if (!isError(returnCode)) {
#ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        addLogMove(LOG_LEVEL_DEBUG,
                   strformat(F("CALCULATE PARAM: %s = %.6g"), TmpStr.c_str(), roundf(param)));
      }
#endif // ifndef BUILD_NO_DEBUG
    } else {
      return errorValue;
    }
    returnValue = roundf(param); // return integer only as it's valid only for device and task id
  }
  return returnValue;
}

CalculateReturnCode Calculate_preProcessed(const String& preprocessd_input,
                              ESPEASY_RULES_FLOAT_TYPE      & result)
{
  START_TIMER;
  CalculateReturnCode returnCode = RulesCalculate.doCalculate(
    preprocessd_input.c_str(),
    &result);

  STOP_TIMER(COMPUTE_STATS);
  return returnCode;
}


CalculateReturnCode Calculate(const String& input,
                              ESPEASY_RULES_FLOAT_TYPE      & result)
{
  CalculateReturnCode returnCode = Calculate_preProcessed(
    RulesCalculate_t::preProces(input),
    result);
#ifndef LIMIT_BUILD_SIZE
  if (isError(returnCode)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("Calculate: ");

      switch (returnCode) {
        case CalculateReturnCode::ERROR_STACK_OVERFLOW:
          log += F("Stack Overflow");
          break;
        case CalculateReturnCode::ERROR_BAD_OPERATOR:
          log += F("Bad Operator");
          break;
        case CalculateReturnCode::ERROR_PARENTHESES_MISMATCHED:
          log += F("Parenthesis mismatch");
          break;
        case CalculateReturnCode::ERROR_UNKNOWN_TOKEN:
          log += F("Unknown token");
          break;
        case CalculateReturnCode::ERROR_TOKEN_LENGTH_EXCEEDED:
          log += strformat(F("Exceeded token length (%d)"), TOKEN_LENGTH);
          break;
        case CalculateReturnCode::OK:
          // Already handled, but need to have all cases here so the compiler can warn if we're missing one.
          break;
      }

      #ifndef BUILD_NO_DEBUG
      log += F(" input: ");
      log += input;
      log += F(" = ");

      const bool trimTrailingZeros = true;
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      log += doubleToString(result, 6, trimTrailingZeros);
#else
      log += floatToString(result, 6, trimTrailingZeros);
#endif
      #endif // ifndef BUILD_NO_DEBUG

      addLogMove(LOG_LEVEL_ERROR, log);
    }
  }
#endif
  return returnCode;
}

