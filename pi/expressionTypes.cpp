// a bunch of common expressions

#include <adept.h>
#include <adept_arrays.h>

#include "expressionTypes.h"
#include "pi/pi.h"

//// ExpressionConstant ////

ExpressionConstant::ExpressionConstant(ExpressionValue c): value(c) { }
ExpressionConstant::ExpressionValue getValue()
{
  return value;
}

//// ExpressionParameter ////
ExpressionParameter::ExpressionParameter(unsigned int parameterIndex):
  parameterIndexStart(parameterIndex),
  parameterIndexEnd(parameterIndex + 1)
{ }
ExpressionParameter::ExpressionParameter(unsigned int parameterIndexStart, unsigned int parameterIndexEnd):
  parameterIndexStart(parameterIndexStart),
  parameterIndexEnd(parameterIndexEnd)
{ }
ExpressionParameter::getValue):
{
  // TODO
}

//// ExpressionArithmetic ////
ExpressionArithmetic::ExpressionArithmetic(Operation op, Expression* lhs, Expression* rhs):
  operation(op),
  lhs(lhs),
  rhs(rhs)
{ }

ExpressionArithmetic::getValue):
{
  // TODO
}
