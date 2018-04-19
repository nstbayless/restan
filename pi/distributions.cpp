#include "distributions.h"
#include "expression.h"

using namespace restan;

ExpressionValue normal(ExpressionValue* exps, unsigned int) // x, mu, sigma
{
  return (exps[0] - exps[1]) * (exps[0] - exps[1]).transpose() / (2.0 * exps[2](0,0));
}
ExpressionValue uniform(ExpressionValue* exps, unsigned int) // x
{
  return 0 * exps[0];
}
ExpressionValue pareto(ExpressionValue* exps, unsigned int) // x, xm, alpha
{
    // TODO
}
ExpressionValue Exponential(ExpressionValue* exps, unsigned int) // x, lambda
{
    // TODO
}
