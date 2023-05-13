#include "../Globals/RulesCalculate.h"

#include "../DataStructs/TimingStats.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter_Numerical.h"

RulesCalculate_t RulesCalculate{};

/*******************************************************************************************
* Helper functions to actually interact with the rules calculation functions.
* *****************************************************************************************/
int CalculateParam(const String& TmpStr) {
  int returnValue = 0;

  // Minimize calls to the Calulate function.
  // Only if TmpStr starts with '=' then call Calculate(). Otherwise do not call it
  if (TmpStr[0] != '=') {
    validIntFromString(TmpStr, returnValue);
  } else {
    ESPEASY_RULES_FLOAT_TYPE param{};

    // Starts with an '=', so Calculate starting at next position
    CalculateReturnCode returnCode = Calculate(TmpStr.substring(1), param);

    if (!isError(returnCode)) {
#ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("CALCULATE PARAM: ");
        log += TmpStr;
        log += F(" = ");
        log += roundf(param);
        addLogMove(LOG_LEVEL_DEBUG, log);
      }
#endif // ifndef BUILD_NO_DEBUG
    }
    returnValue = roundf(param); // return integer only as it's valid only for device and task id
  }
  return returnValue;
}

CalculateReturnCode Calculate(const String& input,
                              ESPEASY_RULES_FLOAT_TYPE      & result)
{
  START_TIMER;
  CalculateReturnCode returnCode = RulesCalculate.doCalculate(
    RulesCalculate_t::preProces(input).c_str(),
    &result);

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
          log += String(F("Exceeded token length (")) + TOKEN_LENGTH + ')';
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
  STOP_TIMER(COMPUTE_STATS);
  return returnCode;
}

