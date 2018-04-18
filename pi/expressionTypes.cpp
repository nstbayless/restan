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
restan::ExpressionConstant::ExpressionConstant(double c): value(1, 1) {
  value(0,0) = c;
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
  // Returns a vector of all parameters
  return pi.getParams(parameterIndexStart, parameterIndexEnd);
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