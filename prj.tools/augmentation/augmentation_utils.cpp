#define _USE_MATH_DEFINES
#include <cmath>

#include "augmentation_utils.h"

int normalValue(int mean, int sigma)
{
  double x1 = (rand() + 0.0) / RAND_MAX;
  double x2 = (rand() + 0.0) / RAND_MAX;
  double x3 = (rand() + 0.0) / RAND_MAX;
  if (x1 == 0 || x2 == 0 || x3 == 0)
    return normalValue(mean, sigma);

  double n1 = cos(2 * M_PI * x1) * sqrt(-2 * log(x2));
  double n2 = sin(2 * M_PI * x1) * sqrt(-2 * log(x2));

  int n1_norm = static_cast<int>(mean + sigma * n1 + 0.5);
  int n2_norm = static_cast<int>(mean + sigma * n2 + 0.5);

  return x3 < 0.5 ? n1_norm : n2_norm;
}

