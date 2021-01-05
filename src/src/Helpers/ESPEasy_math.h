#ifndef HELPERS_ESPEASY_MATH_H
#define HELPERS_ESPEASY_MATH_H


#include <limits>


// The following definitions are from The art of computer programming by Knuth


bool approximatelyEqual(float a, float b, float epsilon = std::numeric_limits<float>::epsilon());

bool essentiallyEqual(float a, float b, float epsilon = std::numeric_limits<float>::epsilon());

bool definitelyGreaterThan(float a, float b, float epsilon = std::numeric_limits<float>::epsilon());

bool definitelyLessThan(float a, float b, float epsilon = std::numeric_limits<float>::epsilon());

#endif // HELPERS_ESPEASY_MATH_H