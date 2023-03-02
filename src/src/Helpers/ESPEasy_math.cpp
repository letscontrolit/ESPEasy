#include "../Helpers/ESPEasy_math.h"

#include <cmath>


double ESPEASY_DOUBLE_EPSILON     = ESPEASY_DOUBLE_EPSILON_FACTOR *  std::numeric_limits<double>::epsilon();
double ESPEASY_DOUBLE_EPSILON_NEG = -1.0 * ESPEASY_DOUBLE_EPSILON_FACTOR *  std::numeric_limits<double>::epsilon();
float  ESPEASY_FLOAT_EPSILON      = std::numeric_limits<float>::epsilon();
float  ESPEASY_FLOAT_EPSILON_NEG  = -1.0f * std::numeric_limits<float>::epsilon();


int maxNrDecimals_double(const double& value)
{
  int res       = ESPEASY_DOUBLE_NR_DECIMALS;
  double factor = 1;

  while ((value / factor) > 10 && res > 2) {
    factor *= 10.0;
    --res;
  }
  return res;
}

bool approximatelyEqual(const double& a, const double& b) {
  return approximatelyEqual(a, b, ESPEASY_DOUBLE_EPSILON);
}

bool approximatelyEqual(const float& a, const float& b) {
  return approximatelyEqual(a, b, ESPEASY_FLOAT_EPSILON);
}

bool approximatelyEqual(const double& a, const double& b, double epsilon)
{
  return std::abs(a - b) <= ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

bool approximatelyEqual(const float& a, const float& b, float epsilon)
{
  return std::abs(a - b) <= ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}


bool definitelyGreaterThan(const double& a, const double& b) {
  return definitelyGreaterThan(a, b, ESPEASY_DOUBLE_EPSILON);
}

bool definitelyGreaterThan(const float& a, const float& b) {
  return definitelyGreaterThan(a, b, ESPEASY_FLOAT_EPSILON);
}

bool definitelyGreaterThan(const double& a, const double& b, double epsilon)
{
  return (a - b) > ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

bool definitelyGreaterThan(const float& a, const float& b, float epsilon)
{
  return (a - b) > ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}


bool definitelyLessThan(const double& a, const double& b) {
  return definitelyLessThan(a, b, ESPEASY_DOUBLE_EPSILON);
}

bool definitelyLessThan(const float& a, const float& b) {
  return definitelyLessThan(a, b, ESPEASY_FLOAT_EPSILON);
}

bool definitelyLessThan(const double& a, const double& b, double epsilon)
{
  return (b - a) > ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

bool definitelyLessThan(const float& a, const float& b, float epsilon)
{
  return (b - a) > ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}


bool essentiallyEqual(const double& a, const double& b) {
  return essentiallyEqual(a, b, ESPEASY_DOUBLE_EPSILON);
}

bool essentiallyEqual(const float& a, const float& b) {
  return essentiallyEqual(a, b, ESPEASY_FLOAT_EPSILON);
}

bool essentiallyEqual(const double& a, const double& b, double epsilon)
{
  return std::abs(a - b) <= ((std::abs(a) > std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

bool essentiallyEqual(const float& a, const float& b, float epsilon)
{
  return std::abs(a - b) <= ((std::abs(a) > std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}


bool essentiallyZero(const double& a)
{
  return ESPEASY_DOUBLE_EPSILON_NEG <= a &&
         a <= ESPEASY_DOUBLE_EPSILON;
}

bool essentiallyZero(const float& a)
{
  return ESPEASY_FLOAT_EPSILON_NEG <= a &&
         a <= ESPEASY_FLOAT_EPSILON;
}
