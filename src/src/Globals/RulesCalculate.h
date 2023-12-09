#ifndef GLOBALS_RULESCALCULATE_H
#define GLOBALS_RULESCALCULATE_H

#include "../Helpers/Rules_calculate.h"

/********************************************************************************************\
   Instance of the RulesCalculate to perform calculations
   These functions are wrapped in a class to
    - make it more clear what external functions to use
    - Make sure generic function names will not cause conflicts
    - Prevent external access to calculate only variables.
 \*********************************************************************************************/

extern RulesCalculate_t RulesCalculate;

/*******************************************************************************************
* Helper functions to actually interact with the rules calculation functions.
* *****************************************************************************************/

int                 CalculateParam(const String& TmpStr);

CalculateReturnCode Calculate_preProcessed(const String& preprocessd_input,
                              ESPEASY_RULES_FLOAT_TYPE      & result);

CalculateReturnCode Calculate(const String& input,
                              ESPEASY_RULES_FLOAT_TYPE      & result);



#endif