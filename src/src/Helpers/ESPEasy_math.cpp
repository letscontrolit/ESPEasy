#include "../Helpers/ESPEasy_math.h"

#include <Arduino.h> 
// Need to include Arduino.h first, then cmath
// See: https://github.com/esp8266/Arduino/issues/8922#issuecomment-1542301697
#include <cmath>

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
double ESPEASY_DOUBLE_EPSILON     = ESPEASY_DOUBLE_EPSILON_FACTOR *  std::numeric_limits<double>::epsilon();
double ESPEASY_DOUBLE_EPSILON_NEG = -1.0 * ESPEASY_DOUBLE_EPSILON_FACTOR *  std::numeric_limits<double>::epsilon();
#endif
float  ESPEASY_FLOAT_EPSILON      = std::numeric_limits<float>::epsilon();
float  ESPEASY_FLOAT_EPSILON_NEG  = -1.0f * std::numeric_limits<float>::epsilon();

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
int maxNrDecimals_fpType(const double& value)
{
  int res       = ESPEASY_DOUBLE_NR_DECIMALS;
  double factor = 1;

  while ((value / factor) > 10 && res > 2) {
    factor *= 10.0;
    --res;
  }
  return res;
}
#endif
int maxNrDecimals_fpType(const float& value)
{
  int res       = ESPEASY_FLOAT_NR_DECIMALS;
  float factor = 1;

  while ((value / factor) > 10 && res > 2) {
    factor *= 10.0f;
    --res;
  }
  return res;
}

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool approximatelyEqual(const double& a, const double& b) {
  return approximatelyEqual(a, b, ESPEASY_DOUBLE_EPSILON);
}
#endif

bool approximatelyEqual(const float& a, const float& b) {
  return approximatelyEqual(a, b, ESPEASY_FLOAT_EPSILON);
}

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool approximatelyEqual(const double& a, const double& b, double epsilon)
{
  return std::abs(a - b) <= ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}
#endif

bool approximatelyEqual(const float& a, const float& b, float epsilon)
{
  return std::abs(a - b) <= ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool definitelyGreaterThan(const double& a, const double& b) {
  return definitelyGreaterThan(a, b, ESPEASY_DOUBLE_EPSILON);
}
#endif

bool definitelyGreaterThan(const float& a, const float& b) {
  return definitelyGreaterThan(a, b, ESPEASY_FLOAT_EPSILON);
}

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool definitelyGreaterThan(const double& a, const double& b, double epsilon)
{
  return (a - b) > ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}
#endif

bool definitelyGreaterThan(const float& a, const float& b, float epsilon)
{
  return (a - b) > ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool definitelyLessThan(const double& a, const double& b) {
  return definitelyLessThan(a, b, ESPEASY_DOUBLE_EPSILON);
}
#endif

bool definitelyLessThan(const float& a, const float& b) {
  return definitelyLessThan(a, b, ESPEASY_FLOAT_EPSILON);
}

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool definitelyLessThan(const double& a, const double& b, double epsilon)
{
  return (b - a) > ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}
#endif

bool definitelyLessThan(const float& a, const float& b, float epsilon)
{
  return (b - a) > ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool essentiallyEqual(const double& a, const double& b) {
  return essentiallyEqual(a, b, ESPEASY_DOUBLE_EPSILON);
}
#endif

bool essentiallyEqual(const float& a, const float& b) {
  return essentiallyEqual(a, b, ESPEASY_FLOAT_EPSILON);
}

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool essentiallyEqual(const double& a, const double& b, double epsilon)
{
  return std::abs(a - b) <= ((std::abs(a) > std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}
#endif

bool essentiallyEqual(const float& a, const float& b, float epsilon)
{
  return std::abs(a - b) <= ((std::abs(a) > std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
bool essentiallyZero(const double& a)
{
  return ESPEASY_DOUBLE_EPSILON_NEG <= a &&
         a <= ESPEASY_DOUBLE_EPSILON;
}
#endif

bool essentiallyZero(const float& a)
{
  return ESPEASY_FLOAT_EPSILON_NEG <= a &&
         a <= ESPEASY_FLOAT_EPSILON;
}

/*========================================================================*/
/*  Functions that would otherwise duplicate code                         */
/*  For example due to being implemented as macros                        */
/*  Or duplications as it is being implemented both for double and float  */
/*  Another factor is that converting from double to float on each call   */
/*  also adds up                                                          */
/*========================================================================*/

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE

// Since we already are using the double implementation of those functions, 
// no need to also add implementation of the float versions of those.
// The functions are declared in math.h
// We're simply overriding their toolchain implementation to reduce build size.


float powf(const float x, const float y)
{
  return (float)(pow((double)x, (double)y));
}

float ceilf(const float x)
{
  return (float)(ceil((double)x));
}

float floorf(const float x)
{
  return (float)(floor((double)x));
}

float fabsf(const float x)
{
  return (float)(fabs((double)x));
}

float acosf(const float x)
{
  return (float)(acos((double)x));
}

float cosf(const float x)
{
  return (float)(cos((double)x));
}

float asinf(const float x)
{
  return (float)(asin((double)x));
}

float sinf(const float x)
{
  return (float)(sin((double)x));
}

float atanf(const float x)
{
  return (float)(atan((double)x));
}

float tanf(const float x)
{
  return (float)(tan((double)x));
}

float sqrtf(const float x)
{
  return (float)(sqrt((double)x));
}



#else


// We only need the float precision when calling these functions.
// Declaration is in math.h
// But we're using a trick here to implement them like this, thus not using 
// the double implementation of those functions.
//
// N.B. this is only working because we concatenate the .cpp files in this Helpers directory.
// So if those functions are called from a .cpp file  outside this folder, it will use the double implementation.
// All rules related code is in this Helpers folder.

double pow(const double x, const double y)
{
  return (double)(powf((float)x, (float)y));
}

double ceil(const double x)
{
  return (double)(ceilf((float)x));
}

double floor(const double x)
{
  return (double)(floorf((float)x));
}

double fabs(const double x)
{
  return (double)(fabsf((float)x));
}

double acos(const double x)
{
  return (double)(acosf((float)x));
}

double cos(const double x)
{
  return (double)(cosf((float)x));
}

double asin(const double x)
{
  return (double)(asinf((float)x));
}

double sin(const double x)
{
  return (double)(sinf((float)x));
}

double atan(const double x)
{
  return (double)(atanf((float)x));
}

double tan(const double x)
{
  return (double)(tanf((float)x));
}

double sqrt(const double x)
{
  return (double)(sqrtf((float)x));
}



#endif