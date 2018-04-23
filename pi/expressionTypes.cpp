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
void restan::ExpressionConstant::print(int depth) const {
	int space = 2*depth;
	while(space--)
		std::cout << " ";
	std::cout << "- ExpressionConstant (" << std::endl;
	std::cout << value << ")\n";
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
void restan::ExpressionParameter::print(int depth) const {
	int space = 2*depth;
	while(space--)
		std::cout << " ";
	std::cout << "- ExpressionParameter: p";
  if (parameterIndexStart >= restan::pi.discreteIndexStart)
    std::cout << "#";
  std::cout << "[" << parameterIndexStart;
  if (parameterIndexEnd != parameterIndexStart + 1)
    std::cout <<", "<< parameterIndexEnd;
  std::cout << "]\n";
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
void restan::ExpressionVariable::print(int depth) const {
	int space = 2*depth;
	while(space--)
		std::cout << " ";
    std::cout << "- ExpressionVariable: v[" << variableIndexStart;
    if (variableIndexEnd != variableIndexStart + 1)
      std::cout <<", "<< variableIndexEnd;
    std::cout << "]\n";
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
      //Dimensions must be 1xn * nx1
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
void restan::ExpressionArithmetic::print(int depth) const {
	int space = 2*depth;
	std::string operationName[5] = {"PLUS", "MINUS", "TIMES", "DOTPRODUCT", "DIV"};

	while(space--)
		std::cout << " ";
	std::cout << "- ExpressionArithmetic: Operation: " << operationName[operation] << std::endl;
	lhs->print(depth+1);
	rhs->print(depth+1);
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

void restan::ExpressionFunction::print(int depth) const {
	int space = 2*depth;
	while(space--)
		std::cout << " ";
	std::cout << "- ExpressionFunction @" << sf << std::endl;
  for (int i = 0; i < numExpressions; i++)
  {
    expressions[i]->print(depth + 1);
  }
}

//// ExpressionDereference ////
restan::ExpressionDereference::ExpressionDereference(Expression* vecEXPR, Expression* indEXPR)
: vecEXPR(vecEXPR),
  indEXPR(indEXPR)
{}

ExpressionValue restan::ExpressionDereference::getValue()
{
  int index = indEXPR->getValue()(0,0).value();
  double value = vecEXPR->getValue()(0,index).value();
  ExpressionValue result(1,1);
  result << value;
  return result;
}
void restan::ExpressionDereference::print(int depth) const {
  int space = 2*depth;
  while(space--)
    std::cout << " ";
  std::cout << "- ExpressionDereference " << std::endl;
  vecEXPR->print(depth+1);
  indEXPR->print(depth+1);
}


//// ExpressionData ////
restan::ExpressionData::ExpressionData(unsigned int dataIndex)
:dataIndex(dataIndex)
{}

//Gets the corresponding vector in Pi
ExpressionValue restan::ExpressionData::getValue()
{
  return pi.getData(dataIndex);
}

void restan::ExpressionData::print(int depth) const {
  int space = 2*depth;
  while(space--)
    std::cout << " ";
  std::cout << "- ExpressionData: d[" << dataIndex << "]" << std::endl;
}
