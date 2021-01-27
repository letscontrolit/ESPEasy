#include "Rules_calculate.h"

#include <Arduino.h>

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/RamTracker.h"
#include "../Helpers/ESPEasy_math.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter.h"


/********************************************************************************************\
   Instance of the RulesCalculate to perform calculations
   These functions are wrapped in a class to
    - make it more clear what external functions to use
    - Make sure generic function names will not cause conflicts
    - Prevent external access to calculate only variables.
 \*********************************************************************************************/
RulesCalculate_t RulesCalculate;


/********************************************************************************************\
   Calculate function for simple expressions
 \*********************************************************************************************/
bool isError(CalculateReturnCode returnCode) {
  return returnCode != CalculateReturnCode::OK;
}

bool RulesCalculate_t::is_number(char oc, char c)
{
  // Check if it matches part of a number (identifier)
  return
    isxdigit(c)  ||                                // HEX digit also includes normal decimal numbers
    ((oc == '0') && ((c == 'x') || (c == 'b'))) || // HEX (0x) or BIN (0b) prefixes.
    (c == '.')   ||                                // A decimal point of a floating point number.
    (is_operator(oc) && (c == '-'))                // Beginning of a negative number after an operator.
  ;
}

bool RulesCalculate_t::is_operator(char c)
{
  return c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '%';
}

bool RulesCalculate_t::is_unary_operator(char c)
{
  const UnaryOperator op = static_cast<UnaryOperator>(c);

  switch (op) {
    case UnaryOperator::Not:
    case UnaryOperator::Log:
    case UnaryOperator::Ln:
    case UnaryOperator::Abs:
    case UnaryOperator::Exp:
    case UnaryOperator::Sqrt:
    case UnaryOperator::Sq:
    case UnaryOperator::Round:
    case UnaryOperator::Sin:
    case UnaryOperator::Cos:
    case UnaryOperator::Tan:
    case UnaryOperator::ArcSin:
    case UnaryOperator::ArcCos:
    case UnaryOperator::ArcTan:
    case UnaryOperator::Sin_d:
    case UnaryOperator::Cos_d:
    case UnaryOperator::Tan_d:
    case UnaryOperator::ArcSin_d:
    case UnaryOperator::ArcCos_d:
    case UnaryOperator::ArcTan_d:
      return true;
  }
  return false;
}

CalculateReturnCode RulesCalculate_t::push(double value)
{
  if (sp != sp_max) // Full
  {
    *(++sp) = value;
    return CalculateReturnCode::OK;
  }
  return CalculateReturnCode::ERROR_STACK_OVERFLOW;
}

double RulesCalculate_t::pop()
{
  if (sp != (globalstack - 1)) { // empty
    return *(sp--);
  }
  else {
    return 0.0f;
  }
}

double RulesCalculate_t::apply_operator(char op, double first, double second)
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

double RulesCalculate_t::apply_unary_operator(char op, double first)
{
  double ret                = 0.0;
  const UnaryOperator un_op = static_cast<UnaryOperator>(op);

  switch (un_op) {
    case UnaryOperator::Not:
      return (approximatelyEqual(round(first), 0)) ? 1 : 0;
    case UnaryOperator::Log:
      return log10(first);
    case UnaryOperator::Ln:
      return log(first);
    case UnaryOperator::Abs:
      return fabs(first);
    case UnaryOperator::Exp:
      return exp(first);
    case UnaryOperator::Sqrt:
      return sqrt(first);
    case UnaryOperator::Sq:
      return first * first;
    case UnaryOperator::Round:
      return round(first);
    default:
      break;
  }

#ifdef USE_TRIGONOMETRIC_FUNCTIONS_RULES
  const bool useDegree = angleDegree(un_op);

  // First the trigonometric functions with angle as output
  switch (un_op) {
    case UnaryOperator::ArcSin:
    case UnaryOperator::ArcSin_d:
      ret = asin(first);
      return useDegree ? degrees(ret) : ret;
    case UnaryOperator::ArcCos:
    case UnaryOperator::ArcCos_d:
      ret = acos(first);
      return useDegree ? degrees(ret) : ret;
    case UnaryOperator::ArcTan:
    case UnaryOperator::ArcTan_d:
      ret = atan(first);
      return useDegree ? degrees(ret) : ret;
    default:
      break;
  }

  // Now the trigonometric functions with angle as input
  if (useDegree) {
    first = radians(first);
  }

  switch (un_op) {
    case UnaryOperator::Sin:
    case UnaryOperator::Sin_d:
      return sin(first);
    case UnaryOperator::Cos:
    case UnaryOperator::Cos_d:
      return cos(first);
    case UnaryOperator::Tan:
    case UnaryOperator::Tan_d:
      return tan(first);
    default:
      break;
  }
#else // ifdef USE_TRIGONOMETRIC_FUNCTIONS_RULES

  switch (un_op) {
    case UnaryOperator::Sin:
    case UnaryOperator::Sin_d:
    case UnaryOperator::Cos:
    case UnaryOperator::Cos_d:
    case UnaryOperator::Tan:
    case UnaryOperator::Tan_d:
    case UnaryOperator::ArcSin:
    case UnaryOperator::ArcSin_d:
    case UnaryOperator::ArcCos:
    case UnaryOperator::ArcCos_d:
    case UnaryOperator::ArcTan:
    case UnaryOperator::ArcTan_d:
      addLog(LOG_LEVEL_ERROR, F("USE_TRIGONOMETRIC_FUNCTIONS_RULES not defined in build"));
      break;
    default:
      break;
  }
#endif // ifdef USE_TRIGONOMETRIC_FUNCTIONS_RULES
  return ret;
}

/*
   char * RulesCalculate_t::next_token(char *linep)
   {
   while (isspace(*(linep++))) {}

   while (*linep && !isspace(*(linep++))) {}
   return linep;
   }
 */
CalculateReturnCode RulesCalculate_t::RPNCalculate(char *token)
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

    if (isError(ret)) { return ret; }
  } else if (is_unary_operator(token[0]) && (token[1] == 0))
  {
    double first = pop();

    ret = push(apply_unary_operator(token[0], first));

    if (isError(ret)) { return ret; }
  } else {
    // Fetch next if there is any
    double value = 0.0;
    validDoubleFromString(token, value);

    ret = push(value); // If it is a value, push to the stack

    if (isError(ret)) { return ret; }
  }

  return ret;
}

// operators
// precedence   operators         associativity
// 4            !                 right to left
// 3            ^                 left to right
// 2            * / %             left to right
// 1            + -               left to right
int RulesCalculate_t::op_preced(const char c)
{
  if (is_unary_operator(c)) { return 4; // right to left
  }

  switch (c)
  {
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

bool RulesCalculate_t::op_left_assoc(const char c)
{
  if (is_operator(c)) { return true;        // left to right
  }

  if (is_unary_operator(c)) { return false; // right to left
  }
  return false;
}

unsigned int RulesCalculate_t::op_arg_count(const char c)
{
  if (is_unary_operator(c)) { return 1; }

  if (is_operator(c)) { return 2; }
  return 0;
}

CalculateReturnCode RulesCalculate_t::doCalculate(const char *input, double *result)
{
  #define TOKEN_LENGTH 25
  #define OPERATOR_STACK_SIZE 32
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("Calculate"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  const char *strpos = input, *strend = input + strlen(input);
  char token[TOKEN_LENGTH];
  char c, oc, *TokenPos = token;
  char stack[OPERATOR_STACK_SIZE]; // operator stack
  unsigned int sl = 0;             // stack length
  char sc;                         // used for record stack element
  CalculateReturnCode error = CalculateReturnCode::OK;

  // *sp=0; // bug, it stops calculating after 50 times
  sp = globalstack - 1;
  oc = c = 0;

  if (input[0] == '=') {
    ++strpos;

    if (strpos < strend) {
      c = *strpos;
    }
  }

  while (strpos < strend)
  {
    if ((TokenPos - &token[0]) >= (TOKEN_LENGTH - 1)) { return CalculateReturnCode::ERROR_TOKEN_LENGTH_EXCEEDED; }

    // read one token from the input stream
    oc = c;
    c  = *strpos;

    if (c != ' ')
    {
      // If the token is a number (identifier), then add it to the token queue.
      if (is_number(oc, c))
      {
        *TokenPos = c;
        ++TokenPos;
      }

      // If the token is an operator, op1, then:
      else if (is_operator(c) || is_unary_operator(c))
      {
        *(TokenPos) = 0; // Mark end of token string
        error       = RPNCalculate(token);
        TokenPos    = token;

        if (isError(error)) { return error; }

        while (sl > 0 && sl < (OPERATOR_STACK_SIZE - 1))
        {
          sc = stack[sl - 1];

          // While there is an operator token, op2, at the top of the stack
          // op1 is left-associative and its precedence is less than or equal to that of op2,
          // or op1 has precedence less than that of op2,
          // The differing operator priority decides pop / push
          // If 2 operators have equal priority then associativity decides.
          if (is_operator(sc) &&
              (
                (op_left_assoc(c) && (op_preced(c) <= op_preced(sc))) ||
                (op_preced(c) < op_preced(sc))
              )
              )
          {
            // Pop op2 off the stack, onto the token queue;
            *TokenPos = sc;
            ++TokenPos;
            *(TokenPos) = 0; // Mark end of token string
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
        if (sl >= OPERATOR_STACK_SIZE) { return CalculateReturnCode::ERROR_STACK_OVERFLOW; }
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
          *(TokenPos) = 0; // Mark end of token string
          error       = RPNCalculate(token);
          TokenPos    = token;

          if (isError(error)) { return error; }

          if (sl > OPERATOR_STACK_SIZE) { return CalculateReturnCode::ERROR_STACK_OVERFLOW; }
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
        if ((sl > 0) && (sl < OPERATOR_STACK_SIZE)) {
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

    *(TokenPos) = 0; // Mark end of token string
    error       = RPNCalculate(token);
    TokenPos    = token;

    if (isError(error)) { return error; }
    *TokenPos = sc;
    ++TokenPos;
    --sl;
  }

  *(TokenPos) = 0; // Mark end of token string
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
  #endif // ifndef BUILD_NO_RAM_TRACKER
  return CalculateReturnCode::OK;
}

void preProcessReplace(String& input, UnaryOperator op) {
  String find = toString(op);

  if (find.length() == 0) { return; }
  find += '('; // Add opening parenthesis.

  const String replace = String(static_cast<char>(op)) + '(';

  input.replace(find, replace);
}

bool angleDegree(UnaryOperator op)
{
  switch (op) {
    case UnaryOperator::Sin_d:
    case UnaryOperator::Cos_d:
    case UnaryOperator::Tan_d:
    case UnaryOperator::ArcSin_d:
    case UnaryOperator::ArcCos_d:
    case UnaryOperator::ArcTan_d:
      return true;
    default:
      break;
  }
  return false;
}

String toString(UnaryOperator op)
{
  String find;

  switch (op) {
    case UnaryOperator::Not:
      break; // No need to replace
    case UnaryOperator::Log:
      find = F("log");
      break;
    case UnaryOperator::Ln:
      find = F("ln");
      break;
    case UnaryOperator::Abs:
      find = F("abs");
      break;
    case UnaryOperator::Exp:
      find = F("exp");
      break;
    case UnaryOperator::Sqrt:
      find = F("sqrt");
      break;
    case UnaryOperator::Sq:
      find = F("sq");
      break;
    case UnaryOperator::Round:
      find = F("round");
      break;
    case UnaryOperator::Sin:
    case UnaryOperator::Sin_d:
      find = F("sin");
      break;
    case UnaryOperator::Cos:
    case UnaryOperator::Cos_d:
      find = F("cos");
      break;
    case UnaryOperator::Tan:
    case UnaryOperator::Tan_d:
      find = F("tan");
      break;
    case UnaryOperator::ArcSin:
    case UnaryOperator::ArcSin_d:
      find = F("asin");
      break;
    case UnaryOperator::ArcCos:
    case UnaryOperator::ArcCos_d:
      find = F("acos");
      break;
    case UnaryOperator::ArcTan:
    case UnaryOperator::ArcTan_d:
      find = F("atan");
      break;
  }

  if (angleDegree(op)) {
    find += F("_d");
  }
  return find;
}

String RulesCalculate_t::preProces(const String& input)
{
  String preprocessed = input;

  preProcessReplace(preprocessed, UnaryOperator::Not);
  preProcessReplace(preprocessed, UnaryOperator::Log);
  preProcessReplace(preprocessed, UnaryOperator::Ln);
  preProcessReplace(preprocessed, UnaryOperator::Abs);
  preProcessReplace(preprocessed, UnaryOperator::Exp);
  preProcessReplace(preprocessed, UnaryOperator::Sqrt);
  preProcessReplace(preprocessed, UnaryOperator::Sq);
  preProcessReplace(preprocessed, UnaryOperator::Round);
#ifdef USE_TRIGONOMETRIC_FUNCTIONS_RULES

  // Try the "arc" functions first, or else "sin" is already replaced when "asin" is tried.
  if (preprocessed.indexOf(F("sin")) != -1) {
    preProcessReplace(preprocessed, UnaryOperator::ArcSin);
    preProcessReplace(preprocessed, UnaryOperator::ArcSin_d);
    preProcessReplace(preprocessed, UnaryOperator::Sin);
    preProcessReplace(preprocessed, UnaryOperator::Sin_d);
  }

  if (preprocessed.indexOf(F("cos")) != -1) {
    preProcessReplace(preprocessed, UnaryOperator::ArcCos);
    preProcessReplace(preprocessed, UnaryOperator::ArcCos_d);
    preProcessReplace(preprocessed, UnaryOperator::Cos);
    preProcessReplace(preprocessed, UnaryOperator::Cos_d);
  }

  if (preprocessed.indexOf(F("tan")) != -1) {
    preProcessReplace(preprocessed, UnaryOperator::ArcTan);
    preProcessReplace(preprocessed, UnaryOperator::ArcTan_d);
    preProcessReplace(preprocessed, UnaryOperator::Tan);
    preProcessReplace(preprocessed, UnaryOperator::Tan_d);
  }
#endif // ifdef USE_TRIGONOMETRIC_FUNCTIONS_RULES
  return preprocessed;
}

/*******************************************************************************************
* Helper functions to actually interact with the rules calculation functions.
* *****************************************************************************************/
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

    if (!isError(returnCode)) {
#ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("CALCULATE PARAM: ");
        log += TmpStr;
        log += F(" = ");
        log += round(param);
        addLog(LOG_LEVEL_DEBUG, log);
      }
#endif // ifndef BUILD_NO_DEBUG
    }
    returnValue = round(param); // return integer only as it's valid only for device and task id
  }
  return returnValue;
}

CalculateReturnCode Calculate(const String& input,
                              double      & result)
{
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
      log += doubleToString(result, 6, trimTrailingZeros);
      #endif // ifndef BUILD_NO_DEBUG

      addLog(LOG_LEVEL_ERROR, log);
    }
  }
  return returnCode;
}
