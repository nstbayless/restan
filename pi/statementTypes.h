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
  class StatementAsign : public Statement
  {
  public:
    StatementAsign(restan::AssignOperator op, int varIndex, Expression* expression);
    virtual void execute() override;
  private:
    AssignOperator op;
    int varIndex;
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