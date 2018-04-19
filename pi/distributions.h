// some common distributions

#ifndef RESTAN_DISTRIBUTIONS_H
#define RESTAN_DISTRIBUTIONS_H

#include "expressionTypes.h"

namespace restan
{
  namespace distributions
  {
    InterpFunc normal; // x, mu, sigma
    InterpFunc uniform; // x
    InterpFunc pareto; // x, xm, alpha
    InterpFunc Exponential; // x, lambda
  }
}

#endif
