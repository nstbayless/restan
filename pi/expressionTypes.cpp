// a bunch of common expressions

#include <adept.h>
#include <adept_arrays.h>

#include "expressionTypes.h"
#include "pi/pi.h"

using namespace restan;

//// ExpressionConstant ////

restan::ExpressionConstant::ExpressionConstant(ExpressionValue c): value(c) { }
ExpressionValue restan::ExpressionConstant::getValue()
{
  return value;
}

//// ExpressionParameter ////
restan::ExpressionParameter::ExpressionParameter(unsigned int parameterIndex):
  parameterIndexStart(parameterIndex),
  parameterIndexEnd(parameterIndex + 1)
{ }
restan::ExpressionParameter::ExpressionParameter(unsigned int parameterIndexStart, unsigned int parameterIndexEnd):
  parameterIndexStart(parameterIndexStart),
  parameterIndexEnd(parameterIndexEnd)
{ }
ExpressionValue restan::ExpressionParameter::getValue()
{
  // TODO
}

//// ExpressionArithmetic ////
restan::ExpressionArithmetic::ExpressionArithmetic(Operation op, Expression* lhs, Expression* rhs):
  operation(op),
  lhs(lhs),
  rhs(rhs)
{ }

ExpressionValue restan::ExpressionArithmetic::getValue()
{
  // TODO
}
