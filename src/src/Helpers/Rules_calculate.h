#ifndef HELPERS_RULES_CALCULATE_H
#define HELPERS_RULES_CALCULATE_H


/********************************************************************************************\
   Calculate function for simple expressions
 \*********************************************************************************************/
#define CALCULATE_OK                            0
#define CALCULATE_ERROR_STACK_OVERFLOW          1
#define CALCULATE_ERROR_BAD_OPERATOR            2
#define CALCULATE_ERROR_PARENTHESES_MISMATCHED  3
#define CALCULATE_ERROR_UNKNOWN_TOKEN           4
#define STACK_SIZE 10 // was 50
#define TOKEN_MAX 20

extern double  globalstack[STACK_SIZE];
extern double *sp;
extern double *sp_max;

int    push(double value);

double pop();

double apply_operator(char   op,
                      double first,
                      double second);

double apply_unary_operator(char   op,
                            double first);

char * next_token(char *linep);

int    RPNCalculate(char *token);

// operators
// precedence   operators         associativity
// 3            !                 right to left
// 2            * / %             left to right
// 1            + - ^             left to right
int          op_preced(const char c);

bool         op_left_assoc(const char c);

unsigned int op_arg_count(const char c);

int          Calculate(const char *input,
                       double     *result);

int          CalculateParam(const char *TmpStr);


#endif // ifndef HELPERS_RULES_CALCULATE_H
