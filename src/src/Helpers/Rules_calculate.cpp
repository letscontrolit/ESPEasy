#include "../Helpers/Rules_calculate.h"

#include "../DataStructs/TimingStats.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/RamTracker.h"
#include "../Helpers/ESPEasy_math.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter.h"


RulesCalculate_t::RulesCalculate_t() {
  for (int i = 0; i < STACK_SIZE; ++i) {
    globalstack[i] = 0.0;
  }
}

/********************************************************************************************\
   Calculate function for simple expressions
 \*********************************************************************************************/
bool isError(CalculateReturnCode returnCode) {
  return returnCode != CalculateReturnCode::OK;
}

bool RulesCalculate_t::is_number(char oc, char c, char pc)
{
  // Check if it matches part of a number (identifier)
  return
    (c == '.')   ||                                // A decimal point of a floating point number.
    ((oc == '0') && ((c == 'x') || (c == 'b'))) || // HEX (0x) or BIN (0b) prefixes.
    isxdigit(c)  ||                                // HEX digit also includes normal decimal numbers
    ((is_operator(oc) || ('\0' == oc)) && (c == '-') 
        && (isdigit(pc) || ('\0' == pc))) // Beginning of a negative number after an operator or 'separator' and before a digit or end-of-digit.
  ;
}

bool RulesCalculate_t::is_operator(char c)
{
  return c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '%';
}

bool RulesCalculate_t::is_unary_operator(char c)
{
  const UnaryOperator op = static_cast<UnaryOperator>(c);
  return (op == UnaryOperator::Not || (
          c >= static_cast<char>(UnaryOperator::Log) &&
          c <= static_cast<char>(UnaryOperator::ArcTan_d)));
/*
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
  */
}

// quinary = 5-argument functions
bool RulesCalculate_t::is_quinary_operator(char c)
{
  const UnaryOperator op = static_cast<UnaryOperator>(c);

  return op == UnaryOperator::Map || op == UnaryOperator::MapC;
}

CalculateReturnCode RulesCalculate_t::push(ESPEASY_RULES_FLOAT_TYPE value)
{
  if (sp != sp_max) // Full
  {
    *(++sp) = value;
    return CalculateReturnCode::OK;
  }
  return CalculateReturnCode::ERROR_STACK_OVERFLOW;
}

ESPEASY_RULES_FLOAT_TYPE RulesCalculate_t::pop()
{
  if (sp != (globalstack - 1)) { // empty
    return *(sp--);
  }
  else {
    return 0.0;
  }
}

ESPEASY_RULES_FLOAT_TYPE RulesCalculate_t::apply_operator(char op, ESPEASY_RULES_FLOAT_TYPE first, ESPEASY_RULES_FLOAT_TYPE second)
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
    #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      return static_cast<int>(round(first)) % static_cast<int>(round(second));
    #else
      return static_cast<int>(roundf(first)) % static_cast<int>(roundf(second));
    #endif
    case '^':
    #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      return pow(first, second);
    #else
      return powf(first, second);
    #endif
    default:
      return 0;
  }
}

ESPEASY_RULES_FLOAT_TYPE RulesCalculate_t::apply_unary_operator(char op, ESPEASY_RULES_FLOAT_TYPE first)
{
  ESPEASY_RULES_FLOAT_TYPE ret{};
  const UnaryOperator un_op = static_cast<UnaryOperator>(op);

  switch (un_op) {
    case UnaryOperator::Not:
    #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      return essentiallyZero(round(first)) ? 1 : 0;
    #else
      return essentiallyZero(roundf(first)) ? 1 : 0;
    #endif
    case UnaryOperator::Log:
    #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      return log10(first);
    #else
      return log10f(first);
    #endif
    case UnaryOperator::Ln:
    #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      return log(first);
    #else
      return logf(first);
    #endif
    case UnaryOperator::Abs:
    #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      return fabs(first);
    #else
      return fabsf(first);
    #endif
    case UnaryOperator::Exp:
    #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      return exp(first);
    #else
      return expf(first);
    #endif
    case UnaryOperator::Sqrt:
    #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      return sqrt(first);
    #else
      return sqrtf(first);
    #endif
    case UnaryOperator::Sq:
      return first * first;
    case UnaryOperator::Round:
    #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      return round(first);
    #else
      return roundf(first);
    #endif
    default:
      break;
  }

#if FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES
  const bool useDegree = angleDegree(un_op);

  // First the trigonometric functions with angle as output
  switch (un_op) {
    case UnaryOperator::ArcSin:
    case UnaryOperator::ArcSin_d:
    #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      ret = asin(first);
    #else
      ret = asinf(first);
    #endif
      return useDegree ? degrees(ret) : ret;
    case UnaryOperator::ArcCos:
    case UnaryOperator::ArcCos_d:
    #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      ret = acos(first);
    #else
      ret = acosf(first);
    #endif
      return useDegree ? degrees(ret) : ret;
    case UnaryOperator::ArcTan:
    case UnaryOperator::ArcTan_d:
    #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      ret = atan(first);
    #else
      ret = atanf(first);
    #endif
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
    #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      return sin(first);
    #else
      return sinf(first);
    #endif
    case UnaryOperator::Cos:
    case UnaryOperator::Cos_d:
    #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      return cos(first);
    #else
      return cosf(first);
    #endif
    case UnaryOperator::Tan:
    case UnaryOperator::Tan_d:
    #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      return tan(first);
    #else
      return tanf(first);
    #endif
    default:
      break;
  }
#else // if FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES

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
      addLog(LOG_LEVEL_ERROR, F("FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES not defined in build"));
      break;
    default:
      break;
  }
#endif // if FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES
  return ret;
}

ESPEASY_RULES_FLOAT_TYPE RulesCalculate_t::apply_quinary_operator(char op, 
                                                                  ESPEASY_RULES_FLOAT_TYPE first,
                                                                  ESPEASY_RULES_FLOAT_TYPE second,
                                                                  ESPEASY_RULES_FLOAT_TYPE third,
                                                                  ESPEASY_RULES_FLOAT_TYPE fourth,
                                                                  ESPEASY_RULES_FLOAT_TYPE fifth)
{
  ESPEASY_RULES_FLOAT_TYPE ret{};
  const UnaryOperator qu_op = static_cast<UnaryOperator>(op);

  if (UnaryOperator::Map == qu_op || UnaryOperator::MapC == qu_op) {

    // Clamp the result if the operator is MapC
    if (qu_op == UnaryOperator::MapC) {
      first = constrain(first, min(second, third), max(second, third));;
    }
    ret = mapADCtoFloat(first, second, third, fourth, fifth);
  }
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
    ESPEASY_RULES_FLOAT_TYPE second = pop();
    ESPEASY_RULES_FLOAT_TYPE first  = pop();

    ret = push(apply_operator(token[0], first, second));
    // addLog(LOG_LEVEL_INFO, strformat(F("RPNCalculate operator %c: 1: %.2f 2: %.2f"), token[0], first, second));

// FIXME TD-er: Regardless whether it is an error, all code paths return ret;
//    if (isError(ret)) { return ret; }
  } else if (is_unary_operator(token[0]) && (token[1] == 0))
  {
    ESPEASY_RULES_FLOAT_TYPE first = pop();

    ret = push(apply_unary_operator(token[0], first));
    // addLog(LOG_LEVEL_INFO, strformat(F("RPNCalculate unary %d: 1: %.2f"), token[0], first));

// FIXME TD-er: Regardless whether it is an error, all code paths return ret;
//    if (isError(ret)) { return ret; }
  } else if (is_quinary_operator(token[0]) && (token[1] == 0))
  {
    ESPEASY_RULES_FLOAT_TYPE fifth  = pop();
    ESPEASY_RULES_FLOAT_TYPE fourth = pop();
    ESPEASY_RULES_FLOAT_TYPE third  = pop();
    ESPEASY_RULES_FLOAT_TYPE second = pop();
    ESPEASY_RULES_FLOAT_TYPE first  = pop();

    ret = push(apply_quinary_operator(token[0], first, second, third, fourth, fifth));
    // addLog(LOG_LEVEL_INFO, strformat(F("RPNCalculate quinary %d: 1: %.2f 2: %.2f 3: %.2f 4: %.2f 5: %.2f"), token[0], first, second, third, fourth, fifth));

  } else {
    // Fetch next if there is any
    ESPEASY_RULES_FLOAT_TYPE value{};
    if (validDoubleFromString(token, value)) {

    //   addLog(LOG_LEVEL_INFO, strformat(F("RPNCalculate push value: %.2f token: %s"), value, token));
    // } else {
    //   addLog(LOG_LEVEL_INFO, strformat(F("RPNCalculate unknown token: %s"), token));
    }
    ret = push(value); // If it is a value, push to the stack

// FIXME TD-er: Regardless whether it is an error, all code paths return ret;
//    if (isError(ret)) { return ret; }
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
/*
  // FIXME TD-er: Disabled the check as the return value is false anyway.
  if (is_unary_operator(c)) { return false; // right to left
  }
  */
  return false;
}

/* unused:
unsigned int RulesCalculate_t::op_arg_count(const char c)
{
  if (is_unary_operator(c)) { return 1; }

  if (is_operator(c)) { return 2; }

  if (is_quinary_operator(c)) { return 5; }
  return 0;
}
*/

CalculateReturnCode RulesCalculate_t::doCalculate(const char *input, ESPEASY_RULES_FLOAT_TYPE *result)
{

  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("Calculate"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  const char *strpos = input, *strend = input + strlen(input);
  char token[TOKEN_LENGTH]{};
  char c, oc, pc, *TokenPos = token;
  const char *pcpos;
  char stack[OPERATOR_STACK_SIZE]{}; // operator stack
  unsigned int sl = 0;               // stack length
  char sc;                           // used for record stack element
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
      // Get peek-ahead char
      pcpos = strpos + 1;
      pc = *pcpos;
      while ((pcpos < strend) && (' ' == pc)) {
        ++pcpos;
        pc = *pcpos;
      }
      // addLog(LOG_LEVEL_INFO, strformat(F("doCalculate is_number: oc: %d c: %d/%c pc: %d token: %s input: %s sl: %u"), oc, c, c, pc, token, input, sl));
      if ((pcpos >= strend)) {
        pc = '\0';
      }
      // If the token is a number (identifier), then add it to the token queue.
      if (is_number(oc, c, pc))
      {
        *TokenPos = c;
        ++TokenPos;
        *(TokenPos) = 0; // Mark current end of token string
      }

      // If the token is any operator, op1, then:
      else if (is_operator(c) || is_unary_operator(c) || is_quinary_operator(c))
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

      // Process the token at a colon (separator)
      else if (c == ':')
      {
        *(TokenPos) = 0; // Mark end of token string
        error       = RPNCalculate(token);
        TokenPos    = token;
        c           = 0; // reset
      }

      // If the token is a left parenthesis, then push it onto the stack.
      else if (c == '(')
      {
        if (sl >= OPERATOR_STACK_SIZE) { return CalculateReturnCode::ERROR_STACK_OVERFLOW; }
        stack[sl] = c;
        ++sl;
        c = 0; // reset
      }

      // If the token is a right parenthesis:
      else if (c == ')')
      {
        bool pe = false;
        sc = '\0';
        // addLog(LOG_LEVEL_INFO, strformat(F("doCalculate stack content 0x%s"), formatToHex_array(reinterpret_cast<const uint8_t*>(stack), sl + 1).c_str()));

        // Until the token at the top of the stack is a left parenthesis,
        // pop operators off the stack onto the token queue
        while (sl < OPERATOR_STACK_SIZE)
        {
          *(TokenPos) = 0; // Mark end of token string
          // addLog(LOG_LEVEL_INFO, strformat(F("doCalculate popping stack sl: %u token: %s sc: %d"), sl, token, sc));
          if (sc == '(') {
            ESPEASY_RULES_FLOAT_TYPE first = pop(); // Get last value from stack
            error = push(first); // push back
            error = push(first); // Push as a result of ()
            // addLog(LOG_LEVEL_INFO, strformat(F("doCalculate pop&push 2x last value: %.2f sl: %u"), first, sl));
          } else {
            error = RPNCalculate(token);
            // ESPEASY_RULES_FLOAT_TYPE first = pop(); // Get last value from stack
            // error = push(first); // push back
            // addLog(LOG_LEVEL_INFO, strformat(F("doCalculate last value on stack: %.2f sl: %u"), first, sl));
          }
          TokenPos = token;

          if (isError(error)) { return error; }

          // we're not growing the stack: if (sl > OPERATOR_STACK_SIZE) { return CalculateReturnCode::ERROR_STACK_OVERFLOW; }
          if (sl > 0) {
            sc = stack[sl - 1];
          }
          if (pe || (sl == 0)) {
            break;
          }

          if (sc == '(')
          {
            pe = true;
            if (sl > 1) {
              sc = stack[sl - 2];
              if (!(is_operator(sc) || is_unary_operator(sc) || is_quinary_operator(sc))) {
                sc = '(';
              }
            }
          }
          // else
          {
            *TokenPos = sc;
            ++TokenPos;
            if (sl > 0) {
              sl--;
            }
            // addLog(LOG_LEVEL_INFO, strformat(F("doCalculate next token: %s sl: %u sc: %d"), token, sl, sc));
          }
        }

        // If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
        if (!pe) {
          return CalculateReturnCode::ERROR_PARENTHESES_MISMATCHED;
        }

        // Pop the left parenthesis from the stack, but not onto the token queue.
        if (sl > 0) {
          sl--;
        }

        // If the token at the top of the stack is a function token, pop it onto the token queue.
        // FIXME TD-er: This sc value is never used, it is re-assigned a new value before it is being checked.
        // if ((sl > 0) && (sl < OPERATOR_STACK_SIZE)) {
        //   sc = stack[sl - 1];
        // }
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

  if (find.isEmpty()) { return; }
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

const __FlashStringHelper* toString(UnaryOperator op)
{
  switch (op) {
    case UnaryOperator::Not:
      break; // No need to replace
    case UnaryOperator::Log:
      return F("log");
    case UnaryOperator::Ln:
      return F("ln");
    case UnaryOperator::Abs:
      return F("abs");
    case UnaryOperator::Exp:
      return F("exp");
    case UnaryOperator::Sqrt:
      return F("sqrt");
    case UnaryOperator::Sq:
      return F("sq");
    case UnaryOperator::Round:
      return F("round");
    case UnaryOperator::Sin:
      return F("sin");
    case UnaryOperator::Sin_d:
      return F("sin_d");
    case UnaryOperator::Cos:
      return F("cos");
    case UnaryOperator::Cos_d:
      return F("cos_d");
    case UnaryOperator::Tan:
      return F("tan");
    case UnaryOperator::Tan_d:
      return F("tan_d");
    case UnaryOperator::ArcSin:
      return F("asin");
    case UnaryOperator::ArcSin_d:
      return F("asin_d");
    case UnaryOperator::ArcCos:
      return F("acos");
    case UnaryOperator::ArcCos_d:
      return F("acos_d");
    case UnaryOperator::ArcTan:
      return F("atan");
    case UnaryOperator::ArcTan_d:
      return F("atan_d");
    case UnaryOperator::Map:
      return F("map");
    case UnaryOperator::MapC:
      return F("mapc");
  }
  return F("");
}

String RulesCalculate_t::preProces(const String& input)
{
  String preprocessed = input;

  const UnaryOperator operators[] = {
    UnaryOperator::Not
    ,UnaryOperator::Log
    ,UnaryOperator::Ln
    ,UnaryOperator::Abs
    ,UnaryOperator::Exp
    ,UnaryOperator::Sqrt
    ,UnaryOperator::Sq
    ,UnaryOperator::Round
    #if FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES

    // Try the "arc" functions first or else "sin" is already replaced when "asin" is tried.
    ,UnaryOperator::ArcSin
    ,UnaryOperator::ArcSin_d
    ,UnaryOperator::Sin
    ,UnaryOperator::Sin_d

    ,UnaryOperator::ArcCos
    ,UnaryOperator::ArcCos_d
    ,UnaryOperator::Cos
    ,UnaryOperator::Cos_d

    ,UnaryOperator::ArcTan
    ,UnaryOperator::ArcTan_d
    ,UnaryOperator::Tan
    ,UnaryOperator::Tan_d
    #endif // if FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES
    ,UnaryOperator::Map
    ,UnaryOperator::MapC

  };

  constexpr size_t nrOperators = NR_ELEMENTS(operators);

  for (size_t i = 0; i < nrOperators; ++i) {
    const UnaryOperator op = operators[i];
#if FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES
    if (op == UnaryOperator::ArcSin && preprocessed.indexOf(F("sin")) == -1) i += 3;
    else if (op == UnaryOperator::ArcCos && preprocessed.indexOf(F("cos")) == -1) i += 3;
    else if (op == UnaryOperator::ArcTan && preprocessed.indexOf(F("tan")) == -1) i += 3;
    else 
#endif
    {
      preProcessReplace(preprocessed, op);
    }
  }
  return preprocessed;
}

