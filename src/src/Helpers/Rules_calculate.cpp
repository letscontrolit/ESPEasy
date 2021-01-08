#include "Rules_calculate.h"

#include <Arduino.h>

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/RamTracker.h"
#include "../Helpers/ESPEasy_math.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter.h"

/********************************************************************************************\
   Calculate function for simple expressions
 \*********************************************************************************************/

double  globalstack[STACK_SIZE];
double *sp     = globalstack - 1;
double *sp_max = &globalstack[STACK_SIZE - 1];

bool isError(CalculateReturnCode returnCode) {
  return returnCode != CalculateReturnCode::OK; 
}

bool is_operator(char c)
{
  return (c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '%');
}

bool is_unary_operator(char c) 
{
  return (c == '!');
}

CalculateReturnCode push(double value)
{
  if (sp != sp_max) // Full
  {
    *(++sp) = value;
    return CalculateReturnCode::OK;
  }
  return CalculateReturnCode::ERROR_STACK_OVERFLOW;
}

double pop()
{
  if (sp != (globalstack - 1)) { // empty
    return *(sp--);
  }
  else {
    return 0.0f;
  }
}

double apply_operator(char op, double first, double second)
{
  switch (op)
  {
    case '+':
      return first + second;
    case '-':
      return first - second;
    case '*':
      return first * second;
    case '/':
      return first / second;
    case '%':
      return static_cast<int>(round(first)) % static_cast<int>(round(second));
    case '^':
      return pow(first, second);
    default:
      return 0;
  }
}

double apply_unary_operator(char op, double first)
{
  switch (op)
  {
    case '!':
      return (approximatelyEqual(round(first), 0)) ? 1 : 0;
    default:
      return 0;
  }
}

char* next_token(char *linep)
{
  while (isspace(*(linep++))) {}

  while (*linep && !isspace(*(linep++))) {}
  return linep;
}

CalculateReturnCode RPNCalculate(char *token)
{
  CalculateReturnCode ret = CalculateReturnCode::OK;
  if (token[0] == 0) {
    return ret; // Don't bother for an empty string
  }

  if (is_operator(token[0]) && (token[1] == 0))
  {
    double second = pop();
    double first  = pop();

    ret = push(apply_operator(token[0], first, second));
    if (isError(ret)) return ret;
  } else if (is_unary_operator(token[0]) && (token[1] == 0))
  {
    double first = pop();

    ret = push(apply_unary_operator(token[0], first));
    if (isError(ret)) return ret;
  } else {
    // Fetch next if there is any
    double value = 0.0;
    validDoubleFromString(token, value);

    ret = push(value); // If it is a value, push to the stack
    if (isError(ret)) return ret;
  }

  return ret;
}

// operators
// precedence   operators         associativity
// 3            !                 right to left
// 2            * / %             left to right
// 1            + - ^             left to right
int op_preced(const char c)
{
  switch (c)
  {
    case '!':
      return 4;
    case '^':
      return 3;
    case '*':
    case '/':
    case '%':
      return 2;
    case '+':
    case '-':
      return 1;
  }
  return 0;
}

bool op_left_assoc(const char c)
{
  switch (c)
  {
    case '^':
    case '*':
    case '/':
    case '+':
    case '-':
    case '%':
      return true;  // left to right
    case '!':
      return false; // right to left
  }
  return false;
}

unsigned int op_arg_count(const char c)
{
  switch (c)
  {
    case '^':
    case '*':
    case '/':
    case '+':
    case '-':
    case '%':
      return 2;
    case '!':
      return 1;
  }
  return 0;
}

CalculateReturnCode Calculate(const String& input,
                              double     &result)
{
  CalculateReturnCode returnCode = doCalculate(input.c_str(), &result);
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
        case CalculateReturnCode::OK:
          // Already handled, but need to have all cases here so the compiler can warn if we're missing one.
          break;
      }

      #ifndef BUILD_NO_DEBUG
      log += F(" input: ");
      log += input;
      log += F(" = ");

      const bool trimTralingZeros = true;
      log += doubleToString(&result, 6, trimTralingZeros);
      #endif

      addLog(LOG_LEVEL_ERROR, log);
    }
  }
  return returnCode;
}

CalculateReturnCode doCalculate(const char *input, double *result)
{
  #define TOKEN_LENGTH 25
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("Calculate"));
  #endif
  const char *strpos = input, *strend = input + strlen(input);
  char token[TOKEN_LENGTH];
  char c, oc, *TokenPos = token;
  char stack[32];      // operator stack
  unsigned int sl = 0; // stack length
  char sc;             // used for record stack element
  CalculateReturnCode error = CalculateReturnCode::OK;

  // *sp=0; // bug, it stops calculating after 50 times
  sp = globalstack - 1;
  oc = c = 0;

  if (input[0] == '=') {
    ++strpos;
    c = *strpos;
  }

  while (strpos < strend)
  {
    if ((TokenPos - &token[0]) >= (TOKEN_LENGTH - 1)) { return CalculateReturnCode::ERROR_STACK_OVERFLOW; }

    // read one token from the input stream
    oc = c;
    c  = *strpos;

    if (c != ' ')
    {
      // If the token is a number (identifier), then add it to the token queue.
      if (isxdigit(c) || ((c == 'x') && (oc == '0')) || (c == '.') || ((c == '-') && is_operator(oc)))
      {
        *TokenPos = c;
        ++TokenPos;
      }

      // If the token is an operator, op1, then:
      else if (is_operator(c) || is_unary_operator(c))
      {
        *(TokenPos) = 0;
        error       = RPNCalculate(token);
        TokenPos    = token;

        if (isError(error)) { return error; }

        while (sl > 0 && sl < 31)
        {
          sc = stack[sl - 1];

          // While there is an operator token, op2, at the top of the stack
          // op1 is left-associative and its precedence is less than or equal to that of op2,
          // or op1 has precedence less than that of op2,
          // The differing operator priority decides pop / push
          // If 2 operators have equal priority then associativity decides.
          if (is_operator(sc) && ((op_left_assoc(c) && (op_preced(c) <= op_preced(sc))) || (op_preced(c) < op_preced(sc))))
          {
            // Pop op2 off the stack, onto the token queue;
            *TokenPos = sc;
            ++TokenPos;
            *(TokenPos) = 0;
            error       = RPNCalculate(token);
            TokenPos    = token;

            if (isError(error)) { return error; }
            sl--;
          }
          else {
            break;
          }
        }

        // push op1 onto the stack.
        stack[sl] = c;
        ++sl;
      }

      // If the token is a left parenthesis, then push it onto the stack.
      else if (c == '(')
      {
        if (sl >= 32) { return CalculateReturnCode::ERROR_STACK_OVERFLOW; }
        stack[sl] = c;
        ++sl;
      }

      // If the token is a right parenthesis:
      else if (c == ')')
      {
        bool pe = false;

        // Until the token at the top of the stack is a left parenthesis,
        // pop operators off the stack onto the token queue
        while (sl > 0)
        {
          *(TokenPos) = 0;
          error       = RPNCalculate(token);
          TokenPos    = token;

          if (isError(error)) { return error; }

          if (sl > 32) { return CalculateReturnCode::ERROR_STACK_OVERFLOW; }
          sc = stack[sl - 1];

          if (sc == '(')
          {
            pe = true;
            break;
          }
          else
          {
            *TokenPos = sc;
            ++TokenPos;
            sl--;
          }
        }

        // If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
        if (!pe) {
          return CalculateReturnCode::ERROR_PARENTHESES_MISMATCHED;
        }

        // Pop the left parenthesis from the stack, but not onto the token queue.
        sl--;

        // If the token at the top of the stack is a function token, pop it onto the token queue.
        // FIXME TD-er: This sc value is never used, it is re-assigned a new value before it is being checked.
        if ((sl > 0) && (sl < 32)) {
          sc = stack[sl - 1];
        }
      }
      else {
        return CalculateReturnCode::ERROR_UNKNOWN_TOKEN;
      }
    }
    ++strpos;
  }

  // When there are no more tokens to read:
  // While there are still operator tokens in the stack:
  while (sl > 0)
  {
    sc = stack[sl - 1];

    if ((sc == '(') || (sc == ')')) {
      return CalculateReturnCode::ERROR_PARENTHESES_MISMATCHED;
    }

    *(TokenPos) = 0;
    error       = RPNCalculate(token);
    TokenPos    = token;

    if (isError(error)) { return error; }
    *TokenPos = sc;
    ++TokenPos;
    --sl;
  }

  *(TokenPos) = 0;
  error       = RPNCalculate(token);
  TokenPos    = token;

  if (isError(error))
  {
    *result = 0;
    return error;
  }
  *result = *sp;
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("Calculate2"));
  #endif
  return CalculateReturnCode::OK;
}

int CalculateParam(const String& TmpStr) {
  int returnValue;

  // Minimize calls to the Calulate function.
  // Only if TmpStr starts with '=' then call Calculate(). Otherwise do not call it
  if (TmpStr[0] != '=') {
    validIntFromString(TmpStr, returnValue);
  } else {
    double param = 0;

    // Starts with an '=', so Calculate starting at next position
    CalculateReturnCode returnCode = Calculate(TmpStr.substring(1), param);
#ifndef BUILD_NO_DEBUG
    if (!isError(returnCode)) {
      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("CALCULATE PARAM: ");
        log += TmpStr;
        log += F(" = ");
        log += round(param);
        addLog(LOG_LEVEL_DEBUG, log);
      }
    }
#endif // ifndef BUILD_NO_DEBUG
    returnValue = round(param); // return integer only as it's valid only for device and task id
  }
  return returnValue;
}