#ifndef HELPERS_RULES_CALCULATE_H
#define HELPERS_RULES_CALCULATE_H


#include "../../ESPEasy_common.h"

/********************************************************************************************\
   Calculate function for simple expressions
 \*********************************************************************************************/
#define STACK_SIZE 10 // was 50
#define TOKEN_MAX 20

enum class CalculateReturnCode {
  OK                           = 0,
  ERROR_STACK_OVERFLOW         = 1,
  ERROR_BAD_OPERATOR           = 2,
  ERROR_PARENTHESES_MISMATCHED = 3,
  ERROR_UNKNOWN_TOKEN          = 4,
  ERROR_TOKEN_LENGTH_EXCEEDED  = 5
};

bool isError(CalculateReturnCode returnCode);

/********************************************************************************************\
   Special char definitions to represent multi character operators
   like: log, sin, cos, tan, etc.
 \*********************************************************************************************/

enum class UnaryOperator  {
  Not = '!',
  Log = 192, // Start at some ASCII code we don't expect in the rules.
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
  ArcTan_d   // Arc Tangent (degree)
};

void   preProcessReplace(String      & input,
                         UnaryOperator op);
bool   angleDegree(UnaryOperator op);
String toString(UnaryOperator op);

class RulesCalculate_t {
private:

  double globalstack[STACK_SIZE];
  double *sp     = globalstack - 1;
  double *sp_max = &globalstack[STACK_SIZE - 1];

  // Check if it matches part of a number (identifier)
  // @param oc  Previous character
  // @param c   Current character
  bool                is_number(char oc,
                                char c);

  bool                is_operator(char c);

  bool                is_unary_operator(char c);

  CalculateReturnCode push(double value);

  double              pop();

  double              apply_operator(char   op,
                                     double first,
                                     double second);

  double apply_unary_operator(char   op,
                              double first);

  //  char              * next_token(char *linep);

  CalculateReturnCode RPNCalculate(char *token);

  // operators
  // precedence   operators         associativity
  // 3            !                 right to left
  // 2            * / %             left to right
  // 1            + - ^             left to right
  int          op_preced(const char c);

  bool         op_left_assoc(const char c);

  unsigned int op_arg_count(const char c);

public:

  CalculateReturnCode doCalculate(const char *input,
                                  double     *result);

  // Try to replace multi byte operators with single character ones.
  // For example log, sin, cos, tan.
  static String preProces(const String& input);
};

extern RulesCalculate_t RulesCalculate;

/*******************************************************************************************
* Helper functions to actually interact with the rules calculation functions.
* *****************************************************************************************/

int                 CalculateParam(const String& TmpStr);

CalculateReturnCode Calculate(const String& input,
                              double      & result);


#endif // ifndef HELPERS_RULES_CALCULATE_H
