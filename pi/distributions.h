// some common distributions

#ifndef RESTAN_DISTRIBUTIONS_H
#define RESTAN_DISTRIBUTIONS_H

#include "expressionTypes.h"

namespace restan
{
  namespace distributions
  {
    ExpressionValue normal(ExpressionValue* exps, unsigned int); // x, mu, sigma
    ExpressionValue uniform(ExpressionValue* exps, unsigned int); // x
    ExpressionValue pareto(ExpressionValue* exps, unsigned int); // x, xm, alpha
    ExpressionValue Exponential(ExpressionValue* exps, unsigned int); // x, lambda
  }
}

#endif
