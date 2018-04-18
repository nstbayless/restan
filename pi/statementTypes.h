#ifndef RESTAN_STATEMENTTYPES_H
#define RESTAN_STATEMENTTYPES_H

#include <adept.h>
#include <adept_arrays.h>

#include "statement.h"
#include "expression.h"

namespace restan
{
  enum AssignOperator
  {
    EQUALS,
    PLUSEQUALS,
    MINUSEQUALS,
    TIMESEQUALS
  };
  // a = N(0,1); V ~ B x3
  class StatementBody : public Statement
  {
  public:
    StatementBody(Statement* statements, int length);
    virtual void execute() override;
  private:
    int length;
    Statement* statements;
  };


  // V ~ B x 3
  class StatementAssign : public Statement
  {
  public:
    StatementAssign(restan::AssignOperator op, unsigned int startIndex, unsigned int endIndex, Expression* expression);
    virtual void execute() override;
  private:
    AssignOperator op;
    unsigned int startIndex;
    unsigned int endIndex;
    Expression* expression;
  };
  
  // a ~ N(0,1) 
  class StatementFunction : public Statement
  {
  public:
    StatementFunction(ExpressionValue (*sf)(ExpressionValue[], int));
    virtual void execute() override;
  private:
    ExpressionValue (*sf)(ExpressionValue[], int);
  };


}

#endif 