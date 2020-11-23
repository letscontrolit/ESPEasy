#include "Rules_calculate.h"

#include <Arduino.h>

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/RamTracker.h"
#include "../Helpers/StringConverter.h"

/********************************************************************************************\
   Calculate function for simple expressions
 \*********************************************************************************************/

float  globalstack[STACK_SIZE];
float *sp     = globalstack - 1;
float *sp_max = &globalstack[STACK_SIZE - 1];

bool is_operator(char c)
{
  return (c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '%');
}

bool is_unary_operator(char c) 
{
  return (c == '!');
}

int push(float value)
{
  if (sp != sp_max) // Full
  {
    *(++sp) = value;
    return 0;
  }
  else {
    return CALCULATE_ERROR_STACK_OVERFLOW;
  }
}

float pop()
{
  if (sp != (globalstack - 1)) { // empty
    return *(sp--);
  }
  else {
    return 0.0f;
  }
}

float apply_operator(char op, float first, float second)
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

float apply_unary_operator(char op, float first)
{
  switch (op)
  {
    case '!':
      return (round(first) == 0) ? 1 : 0;
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

int RPNCalculate(char *token)
{
  if (token[0] == 0) {
    return 0; // geen moeite doen voor een lege string
  }

  if (is_operator(token[0]) && (token[1] == 0))
  {
    float second = pop();
    float first  = pop();

    if (push(apply_operator(token[0], first, second))) {
      return CALCULATE_ERROR_STACK_OVERFLOW;
    }
  } else if (is_unary_operator(token[0]) && (token[1] == 0))
  {
    float first = pop();

    if (push(apply_unary_operator(token[0], first))) {
      return CALCULATE_ERROR_STACK_OVERFLOW;
    }
  } else // Als er nog een is, dan deze ophalen
  if (push(atof(token))) { // is het een waarde, dan op de stack plaatsen
    return CALCULATE_ERROR_STACK_OVERFLOW;
  }

  return 0;
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

int Calculate(const char *input, float *result)
{
  #define TOKEN_LENGTH 25
  checkRAM(F("Calculate"));
  const char *strpos = input, *strend = input + strlen(input);
  char token[TOKEN_LENGTH];
  char c, oc, *TokenPos = token;
  char stack[32];      // operator stack
  unsigned int sl = 0; // stack length
  char sc;             // used for record stack element
  int  error = 0;

  // *sp=0; // bug, it stops calculating after 50 times
  sp = globalstack - 1;
  oc = c = 0;

  if (input[0] == '=') {
    ++strpos;
    c = *strpos;
  }

  while (strpos < strend)
  {
    if ((TokenPos - &token[0]) >= (TOKEN_LENGTH - 1)) { return CALCULATE_ERROR_STACK_OVERFLOW; }

    // read one token from the input stream
    oc = c;
    c  = *strpos;

    if (c != ' ')
    {
      // If the token is a number (identifier), then add it to the token queue.
      if (((c >= '0') && (c <= '9')) || (c == '.') || ((c == '-') && is_operator(oc)))
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

        if (error) { return error; }

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

            if (error) { return error; }
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
        if (sl >= 32) { return CALCULATE_ERROR_STACK_OVERFLOW; }
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

          if (error) { return error; }

          if (sl > 32) { return CALCULATE_ERROR_STACK_OVERFLOW; }
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
          return CALCULATE_ERROR_PARENTHESES_MISMATCHED;
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
        return CALCULATE_ERROR_UNKNOWN_TOKEN;
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
      return CALCULATE_ERROR_PARENTHESES_MISMATCHED;
    }

    *(TokenPos) = 0;
    error       = RPNCalculate(token);
    TokenPos    = token;

    if (error) { return error; }
    *TokenPos = sc;
    ++TokenPos;
    --sl;
  }

  *(TokenPos) = 0;
  error       = RPNCalculate(token);
  TokenPos    = token;

  if (error)
  {
    *result = 0;
    return error;
  }
  *result = *sp;
  checkRAM(F("Calculate2"));
  return CALCULATE_OK;
}

int CalculateParam(const char *TmpStr) {
  int returnValue;

  // Minimize calls to the Calulate function.
  // Only if TmpStr starts with '=' then call Calculate(). Otherwise do not call it
  if (TmpStr[0] != '=') {
    returnValue = str2int(TmpStr);
  } else {
    float param = 0;

    // Starts with an '=', so Calculate starting at next position
    int returnCode = Calculate(&TmpStr[1], &param);

    if (returnCode != CALCULATE_OK) {
      String errorDesc;

      switch (returnCode) {
        case CALCULATE_ERROR_STACK_OVERFLOW:
          errorDesc = F("Stack Overflow");
          break;
        case CALCULATE_ERROR_BAD_OPERATOR:
          errorDesc = F("Bad Operator");
          break;
        case CALCULATE_ERROR_PARENTHESES_MISMATCHED:
          errorDesc = F("Parenthesis mismatch");
          break;
        case CALCULATE_ERROR_UNKNOWN_TOKEN:
          errorDesc = F("Unknown token");
          break;
        default:
          errorDesc = F("Unknown error");
          break;
      }

      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String log = String(F("CALCULATE PARAM ERROR: ")) + errorDesc;
        addLog(LOG_LEVEL_ERROR, log);
        log  = F("CALCULATE PARAM ERROR details: ");
        log += TmpStr;
        log += F(" = ");
        log += round(param);
        addLog(LOG_LEVEL_ERROR, log);
      }
    }
#ifndef BUILD_NO_DEBUG
    else {
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