// a bunch of common expressions

#ifndef RESTAN_EXPRESSIONTYPES_H
#define RESTAN_EXPRESSIONTYPES_H

#include <adept.h>
#include <adept_arrays.h>

#include "expression.h"

namespace restan
{
  // constant value
  class ExpressionConstant : public Expression
  {
  public:
    ExpressionConstant(ExpressionValue c);
    ExpressionConstant(double c); 
    virtual ExpressionValue getValue() override;
  private:
    ExpressionValue value;
  };
  
  // parameter lookup (directly from q)
  class ExpressionParameter : public Expression
  {
  public:
    ExpressionParameter(unsigned int parameterIndexStart);
    // parameters from a half-open range of indices:
    ExpressionParameter(unsigned int parameterIndexStart, unsigned int parameterIndexEnd);
    virtual ExpressionValue getValue() override;
  private:
    unsigned int parameterIndexStart;
    unsigned int parameterIndexEnd;
  };

  // a mathematical operation type
  enum Operation
  {
    PLUS,
    MINUS,
    TIMES,
    DOTPRODUCT,
    DIV
  };
  
  // arithmetic operation
  class ExpressionArithmetic : public Expression
  {
  public:
    ExpressionArithmetic(Operation, Expression* lhs, Expression* rhs);
    Operation operation;
    Expression* lhs;
    Expression* rhs;
    virtual ExpressionValue getValue() override;
  };
}

#endif 