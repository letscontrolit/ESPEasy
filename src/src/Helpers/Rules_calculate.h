#ifndef HELPERS_RULES_CALCULATE_H
#define HELPERS_RULES_CALCULATE_H


#include "../../ESPEasy_common.h"

/********************************************************************************************\
   Calculate function for simple expressions
 \*********************************************************************************************/
#define STACK_SIZE 10 // was 50
#define TOKEN_MAX 20

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
#define TOKEN_LENGTH 40
#else
#define TOKEN_LENGTH 25
#endif
#define OPERATOR_STACK_SIZE 32

enum class CalculateReturnCode : uint8_t{
  OK                           = 0u,
  ERROR_STACK_OVERFLOW         = 1u,
  ERROR_BAD_OPERATOR           = 2u,
  ERROR_PARENTHESES_MISMATCHED = 3u,
  ERROR_UNKNOWN_TOKEN          = 4u,
  ERROR_TOKEN_LENGTH_EXCEEDED  = 5u
};

bool isError(CalculateReturnCode returnCode);

/********************************************************************************************\
   Special char definitions to represent multi character operators
   like: log, sin, cos, tan, etc.
 \*********************************************************************************************/

enum class UnaryOperator : uint8_t {
  Not = '!',
  Log = 192u, // Start at some ASCII code we don't expect in the rules.
  Ln,        // Natural logarithm
  Abs,       // Absolute value
  Exp,       // exponential value, e^x
  Sqrt,      // Square Root
  Sq,        // Square, x^2
  Round,     // Rounds to the nearest integer, but rounds halfway cases away from zero (instead of to the nearest even integer).
  Sin,       // Sine (radian)
  Sin_d,     // Sine (degree)
  Cos,       // Cosine (radian)
  Cos_d,     // Cosine (degree)
  Tan,       // Tangent (radian)
  Tan_d,     // Tangent (degree)
  ArcSin,    // Arc Sine (radian)
  ArcSin_d,  // Arc Sine (degree)
  ArcCos,    // Arc Cosine (radian)
  ArcCos_d,  // Arc Cosine (degree)
  ArcTan,    // Arc Tangent (radian)
  ArcTan_d,  // Arc Tangent (degree)
  Map,       // Map (value, lowFrom, highFrom, lowTo, highTo) (not really unary...)
  MapC,      // Map (value, lowFrom, highFrom, lowTo, highTo) and clamp to lowTo/highTo
};

void   preProcessReplace(String      & input,
                         UnaryOperator op);
bool   angleDegree(UnaryOperator op);
const __FlashStringHelper* toString(UnaryOperator op);

class RulesCalculate_t {
private:

  ESPEASY_RULES_FLOAT_TYPE globalstack[STACK_SIZE]{};
  ESPEASY_RULES_FLOAT_TYPE *sp     = globalstack - 1;
  const ESPEASY_RULES_FLOAT_TYPE *sp_max = &globalstack[STACK_SIZE - 1];

  // Check if it matches part of a number (identifier)
  // @param oc  Previous character
  // @param c   Current character
  bool                is_number(char oc,
                                char c,
                                char pc);

  bool                is_operator(char c);

  bool                is_unary_operator(char c);

  bool                is_quinary_operator(char c);

  CalculateReturnCode push(ESPEASY_RULES_FLOAT_TYPE value);

  ESPEASY_RULES_FLOAT_TYPE              pop();

  ESPEASY_RULES_FLOAT_TYPE              apply_operator(char   op,
                                     ESPEASY_RULES_FLOAT_TYPE first,
                                     ESPEASY_RULES_FLOAT_TYPE second);

  ESPEASY_RULES_FLOAT_TYPE apply_unary_operator(char   op,
                              ESPEASY_RULES_FLOAT_TYPE first);

  ESPEASY_RULES_FLOAT_TYPE apply_quinary_operator(char op, 
                                                  ESPEASY_RULES_FLOAT_TYPE first,
                                                  ESPEASY_RULES_FLOAT_TYPE second,
                                                  ESPEASY_RULES_FLOAT_TYPE third,
                                                  ESPEASY_RULES_FLOAT_TYPE fourth,
                                                  ESPEASY_RULES_FLOAT_TYPE fifth);

  //  char              * next_token(char *linep);

  CalculateReturnCode RPNCalculate(char *token);

  // operators
  // precedence   operators         associativity
  // 4            !                 right to left
  // 3            ^                 left to right
  // 2            * / %             left to right
  // 1            + -               left to right
  int          op_preced(const char c);

  bool         op_left_assoc(const char c);

  // unused: unsigned int op_arg_count(const char c);

public:

  RulesCalculate_t();

  CalculateReturnCode doCalculate(const char *input,
                                  ESPEASY_RULES_FLOAT_TYPE     *result);

  // Try to replace multi byte operators with single character ones.
  // For example log, sin, cos, tan.
  static String preProces(const String& input);
};



#endif // ifndef HELPERS_RULES_CALCULATE_H
