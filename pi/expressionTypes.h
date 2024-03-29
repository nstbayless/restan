// a bunch of common expressions

#ifndef RESTAN_EXPRESSIONTYPES_H
#define RESTAN_EXPRESSIONTYPES_H

#include <adept.h>
#include <adept_arrays.h>

#include "expression.h"

namespace restan
{
  typedef adept::aMatrix (*InterpFunc)(adept::aMatrix*, unsigned int);
  // constant value
  class ExpressionConstant : public Expression
  {
  public:
    ExpressionConstant(ExpressionValue c);
    ExpressionConstant(double c);
    virtual ExpressionValue getValue() override;
    virtual void print(int depth) const override;
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
    virtual void print(int depth) const override;
  private:
    unsigned int parameterIndexStart;
    unsigned int parameterIndexEnd;
  };

  // variable expression
  class ExpressionVariable : public Expression
  {
  public:
    ExpressionVariable(unsigned int variableIndex);
    ExpressionVariable(unsigned int variableIndexStart, unsigned int variableIndexEnd);
    virtual ExpressionValue getValue() override;
    virtual void print(int depth) const override;
  private:
    unsigned int variableIndexStart;
    unsigned int variableIndexEnd;
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
    virtual void print(int depth) const override;
  };

  // function expression
  class ExpressionFunction : public Expression
  {
  public:
    ExpressionFunction(InterpFunc, Expression** expressions, unsigned int numExpressions);
    virtual ExpressionValue getValue() override;
    virtual void print(int depth) const override;
  private:
    InterpFunc sf;
    Expression** expressions;
    unsigned int numExpressions;
  };

  class ExpressionDereference : public Expression
  {
    public:
      ExpressionDereference(Expression* vecEXPR, Expression* indEXPR);
      virtual ExpressionValue getValue() override;
      virtual void print(int depth) const override;
    private:
      Expression* vecEXPR;
      Expression* indEXPR;
  };

  class ExpressionData : public Expression
  {
  public: 
    ExpressionData(unsigned int dataIndex);
    virtual ExpressionValue getValue() override;
    virtual void print(int depth) const override;
  private:
    unsigned int dataIndex;
  };
}

#endif
