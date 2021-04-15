#ifndef HELPERS_ESPEASY_MATH_H
#define HELPERS_ESPEASY_MATH_H


#include <limits>

// Internal ESPEasy representation of double values
#define ESPEASY_DOUBLE_NR_DECIMALS  14
#define ESPEASY_DOUBLE_EPSILON_FACTOR 1000

int maxNrDecimals_double(const double& value);

// The following definitions are from The art of computer programming by Knuth
bool approximatelyEqual(const double& a, const double& b, double epsilon = ESPEASY_DOUBLE_EPSILON_FACTOR *  std::numeric_limits<double>::epsilon());
bool approximatelyEqual(const float& a, const float& b, float epsilon = std::numeric_limits<float>::epsilon());

bool definitelyGreaterThan(const double& a, const double& b, double epsilon = ESPEASY_DOUBLE_EPSILON_FACTOR *  std::numeric_limits<double>::epsilon());
bool definitelyGreaterThan(const float& a, const float& b, float epsilon = std::numeric_limits<float>::epsilon());

bool definitelyLessThan(const double& a, const double& b, double epsilon = ESPEASY_DOUBLE_EPSILON_FACTOR *  std::numeric_limits<double>::epsilon());
bool definitelyLessThan(const float& a, const float& b, float epsilon = std::numeric_limits<float>::epsilon());

bool essentiallyEqual(const double& a, const double& b, double epsilon = ESPEASY_DOUBLE_EPSILON_FACTOR *  std::numeric_limits<double>::epsilon());
bool essentiallyEqual(const float& a, const float& b, float epsilon = std::numeric_limits<float>::epsilon());


#endif // HELPERS_ESPEASY_MATH_H