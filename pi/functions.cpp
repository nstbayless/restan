#include "functions.h"
#include "expression.h"
#include <math.h>

using namespace restan;

ExpressionValue restan::functions::exp(ExpressionValue* exps, unsigned int) // x
{
  return exp(exps[0]);
}