#ifndef HELPERS_ESPEASY_MATH_H
#define HELPERS_ESPEASY_MATH_H


#include <limits>

// Internal ESPEasy representation of double values
#define ESPEASY_DOUBLE_NR_DECIMALS  14
#define ESPEASY_DOUBLE_EPSILON_FACTOR 1000
#define ESPEASY_FLOAT_NR_DECIMALS  6
#define ESPEASY_FLOAT_EPSILON_FACTOR 1000

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
int maxNrDecimals_fpType(const double& value);
#endif
int maxNrDecimals_fpType(const float& value);


// The following definitions are from The art of computer programming by Knuth
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool approximatelyEqual(const double& a, const double& b);
#endif
bool approximatelyEqual(const float& a, const float& b);
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool approximatelyEqual(const double& a, const double& b, double epsilon);
#endif
bool approximatelyEqual(const float& a, const float& b, float epsilon);

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool definitelyGreaterThan(const double& a, const double& b);
#endif
bool definitelyGreaterThan(const float& a, const float& b);
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool definitelyGreaterThan(const double& a, const double& b, double epsilon);
#endif
bool definitelyGreaterThan(const float& a, const float& b, float epsilon);

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool definitelyLessThan(const double& a, const double& b);
#endif
bool definitelyLessThan(const float& a, const float& b);
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool definitelyLessThan(const double& a, const double& b, double epsilon);
#endif
bool definitelyLessThan(const float& a, const float& b, float epsilon);

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool essentiallyEqual(const double& a, const double& b);
#endif
bool essentiallyEqual(const float& a, const float& b);
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool essentiallyEqual(const double& a, const double& b, double epsilon);
#endif
bool essentiallyEqual(const float& a, const float& b, float epsilon);

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool essentiallyZero(const double& a);
#endif
bool essentiallyZero(const float& a);


#endif // HELPERS_ESPEASY_MATH_H