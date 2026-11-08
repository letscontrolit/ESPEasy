#ifndef HELPERS_ESPEASY_MATH_H
#define HELPERS_ESPEASY_MATH_H


#include <limits>

// Internal ESPEasy representation of double values
#define ESPEASY_DOUBLE_NR_DECIMALS  14
#define ESPEASY_DOUBLE_EPSILON_FACTOR 2.0
#define ESPEASY_FLOAT_NR_DECIMALS  6
#define ESPEASY_FLOAT_EPSILON_FACTOR 2.0

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
int maxNrDecimals_fpType(const double& value);
#endif
int maxNrDecimals_fpType(const float& value);

#ifdef ESP32

constexpr int maxNrDecimals_fpType_constexpr(const ESPEASY_RULES_FLOAT_TYPE& value)
{
# if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  int res = ESPEASY_DOUBLE_NR_DECIMALS;
# else
  int res = ESPEASY_FLOAT_NR_DECIMALS;
# endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  ESPEASY_RULES_FLOAT_TYPE factor = 1;

  while ((value / factor) > 10 && res > 2) {
    factor *= 10.0f;
    --res;
  }
  return res;
}

#endif // ifdef ESP32
#ifdef ESP8266

constexpr unsigned maxNrDecimals_fpType_constexpr_(ESPEASY_RULES_FLOAT_TYPE value, ESPEASY_RULES_FLOAT_TYPE factor, unsigned res)
{
  return ((value / factor) > 10 && res > 2)
    ? maxNrDecimals_fpType_constexpr_(value, factor * 10.0f, res - 1)
    : res;
}

constexpr int maxNrDecimals_fpType_constexpr(const ESPEASY_RULES_FLOAT_TYPE& value)
{
  return maxNrDecimals_fpType_constexpr_(
    value, 
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
    1.0, ESPEASY_DOUBLE_NR_DECIMALS
#else
    1.0f, ESPEASY_FLOAT_NR_DECIMALS
#endif
  );
}

#endif // ifdef ESP8266

int      quick_pow10(uint8_t n);

uint64_t computeDecimalFactorForDecimals(int nrDecimals);

// The following definitions are from The art of computer programming by Knuth
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool approximatelyEqual(const double& a,
                        const double& b);
#endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool approximatelyEqual(const float& a,
                        const float& b);
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool approximatelyEqual(const double& a,
                        const double& b,
                        double        estimatedEpsilon);
#endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool approximatelyEqual(const float& a,
                        const float& b,
                        float        estimatedEpsilon);

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool definitelyGreaterThan(const double& a,
                           const double& b);
#endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool definitelyGreaterThan(const float& a,
                           const float& b);
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool definitelyGreaterThan(const double& a,
                           const double& b,
                           double        estimatedEpsilon);
#endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool definitelyGreaterThan(const float& a,
                           const float& b,
                           float        estimatedEpsilon);

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool definitelyLessThan(const double& a,
                        const double& b);
#endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool definitelyLessThan(const float& a,
                        const float& b);
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool definitelyLessThan(const double& a,
                        const double& b,
                        double        estimatedEpsilon);
#endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool definitelyLessThan(const float& a,
                        const float& b,
                        float        estimatedEpsilon);

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool essentiallyEqual(const double& a,
                      const double& b);
#endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool essentiallyEqual(const float& a,
                      const float& b);
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool essentiallyEqual(const double& a,
                      const double& b,
                      double        estimatedEpsilon);
#endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool essentiallyEqual(const float& a,
                      const float& b,
                      float        estimatedEpsilon);

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool                     essentiallyZero(const double& a);
#endif
bool                     essentiallyZero(const float& a);

ESPEASY_RULES_FLOAT_TYPE mapADCtoFloat(ESPEASY_RULES_FLOAT_TYPE float_value,
                                       ESPEASY_RULES_FLOAT_TYPE adc1,
                                       ESPEASY_RULES_FLOAT_TYPE adc2,
                                       ESPEASY_RULES_FLOAT_TYPE out1,
                                       ESPEASY_RULES_FLOAT_TYPE out2);


#endif // HELPERS_ESPEASY_MATH_H
