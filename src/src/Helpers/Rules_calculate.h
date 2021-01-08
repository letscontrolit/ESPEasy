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
 ERROR_UNKNOWN_TOKEN          = 4
};

bool isError(CalculateReturnCode returnCode);

extern double  globalstack[STACK_SIZE];
extern double *sp;
extern double *sp_max;

CalculateReturnCode push(double value);

double pop();

double apply_operator(char   op,
                      double first,
                      double second);

double apply_unary_operator(char   op,
                            double first);

char * next_token(char *linep);

CalculateReturnCode  RPNCalculate(char *token);

// operators
// precedence   operators         associativity
// 3            !                 right to left
// 2            * / %             left to right
// 1            + - ^             left to right
int          op_preced(const char c);

bool         op_left_assoc(const char c);

unsigned int op_arg_count(const char c);

CalculateReturnCode Calculate(const String& input,
                              double     &result);


CalculateReturnCode doCalculate(const char *input,
                              double     *result);

int          CalculateParam(const String& TmpStr);


#endif // ifndef HELPERS_RULES_CALCULATE_H
