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
const void restan::ExpressionConstant::printTree(int depth) {
	int space = 2*depth;
	while(space--)
		std::cout << " " <<;
	std::cout << "-Expression Constant" << std::endl;
	value.printTree(depth+1);
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
  // Returns a vector of all parameters
  if (parameterIndexStart == -1)
    throw StartIndexInvalid();
  return pi.getParams(parameterIndexStart, parameterIndexEnd);
}
const void restan::ExpressionParameter::printTree(int depth) {
	int space = 2*depth;
	while(space--)
		std::cout << " " <<;
	std::cout << "-Expression Parameter: indStart: " << parameterIndexStart <<
	"indEnd: " << parameterIndexEnd << std::endl;
}


//// ExpressionVariable ////
restan::ExpressionVariable::ExpressionVariable(unsigned int variableIndex):
  variableIndexStart(variableIndex),
  variableIndexEnd(variableIndex + 1)
{ }
restan::ExpressionVariable::ExpressionVariable(unsigned int variableIndexStart, unsigned int variableIndexEnd):
  variableIndexStart(variableIndexStart),
  variableIndexEnd(variableIndexEnd)
{ }
ExpressionValue restan::ExpressionVariable::getValue()
{
  // Returns a vector of all parameters
  if (variableIndexStart == -1)
    throw StartIndexInvalid();
  return pi.getVariables(variableIndexStart, variableIndexEnd);
}
const void restan::ExpressionVariable::printTree(int depth) {
	int space = 2*depth;
	while(space--)
		std::cout << " " <<;
	std::cout << "-Expression Variable: indStart: " << variableIndexStart <<
	"indEnd: " << variableIndexEnd << std::endl;
}


//// ExpressionArithmetic ////
restan::ExpressionArithmetic::ExpressionArithmetic(Operation op, Expression* lhs, Expression* rhs):
  operation(op),
  lhs(lhs),
  rhs(rhs)
{ }

// Recursively evaluates lhs and rhs
//TODO:: Add exception handling for linear algebra exceptions
ExpressionValue restan::ExpressionArithmetic::getValue()
{
  ExpressionValue arithResult;
  ExpressionValue lhsVal = lhs->getValue();
  ExpressionValue rhsVal = rhs->getValue();
  int lhs_row = lhsVal.dimension(0);
  int lhs_col = lhsVal.size() / lhs_row;
  int rhs_row = rhsVal.dimension(0);
  int rhs_col = rhsVal.size() / rhs_row;


  switch (operation) {
    case MINUS:
      rhsVal *= -1;
    case PLUS:
      //Scalar + Matrix || Matrix + Scalar
      if (lhs_row == lhs_col) {
        arithResult = rhsVal + lhsVal(0,0);
      }
      if (rhs_row == rhs_col) {
        arithResult = lhsVal + rhsVal(0,0);
      }
      arithResult = lhsVal + rhsVal;
      break;
    case TIMES:
      arithResult = adept::matmul(lhsVal, rhsVal);
      break;
    case DOTPRODUCT:
      //Dimensions must be 1x4 * 4*1
      if (lhs_col == rhs_row)
        arithResult = adept::matmul(lhsVal, rhsVal);
      else
        std::cout<< "Dot product dimensions must be 1xn * n*1" << std::endl;
      break;
    case DIV:
      if (rhs_row == rhs_col)
      {
        arithResult = lhsVal / rhsVal(0,0);
        break;
      }
    default:
      std::cout << "Invalid Operation\n" << std::endl;
  }

  return arithResult;
}
const void restan::ExpressionArithmetic::printTree(int depth) {
	int space = 2*depth;
	std::string operationName[5] = {"PLUS", "MINUS", "TIMES", "DOTPRODUCT", "DIV"};

	while(space--)
		std::cout << " " <<;
	std::cout << "-Expression Arithmetic: Operation: " << operationName[operation] << std::endl;
	lhs.printTree(depth+1);
	rhs.printTree(depth+1);
}

//// ExpressionFunction ////
restan::ExpressionFunction::ExpressionFunction(InterpFunc sf, Expression** expressions, unsigned int numExpressions)
: sf(sf),
  expressions(expressions),
  numExpressions(numExpressions)
{}

ExpressionValue restan::ExpressionFunction::getValue()
{
  ExpressionValue expressionVals[numExpressions];
  for (int i = 0; i < numExpressions; i++) {
    expressionVals[i] = expressions[i]->getValue();
  }
  return sf(expressionVals, numExpressions);
}
