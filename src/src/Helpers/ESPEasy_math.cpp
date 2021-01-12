#include "ESPEasy_math.h"

#include <cmath>

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

bool approximatelyEqual(const double& a, const double& b, double epsilon)
{
  return std::abs(a - b) <= ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

bool approximatelyEqual(const float& a, const float& b, float epsilon)
{
  return std::abs(a - b) <= ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

bool definitelyGreaterThan(const double& a, const double& b, double epsilon)
{
  return (a - b) > ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

bool definitelyGreaterThan(const float& a, const float& b, float epsilon)
{
  return (a - b) > ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

bool definitelyLessThan(const double& a, const double& b, double epsilon)
{
  return (b - a) > ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

bool definitelyLessThan(const float& a, const float& b, float epsilon)
{
  return (b - a) > ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

bool essentiallyEqual(const double& a, const double& b, double epsilon)
{
  return std::abs(a - b) <= ((std::abs(a) > std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

bool essentiallyEqual(const float& a, const float& b, float epsilon)
{
  return std::abs(a - b) <= ((std::abs(a) > std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}
