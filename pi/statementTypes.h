#ifndef RESTAN_STATEMENTTYPES_H
#define RESTAN_STATEMENTTYPES_H

#include <adept.h>
#include <adept_arrays.h>

#include "statement.h"
#include "expressionTypes.h"

namespace restan
{
  // a = N(0,1); V ~ B x3
  class StatementBody : public Statement
  {
  public:
    StatementBody(Statement** statements, int length);
    virtual void execute() override;
  private:
    int length;
    Statement** statements;
  };


  // V ~ B x 3
  class StatementAssign : public Statement
  {
  public:
    StatementAssign( unsigned int index, Expression* expression);
    StatementAssign( unsigned int startIndex, unsigned int endIndex, Expression* expression);
    virtual void execute() override;
  private:
    unsigned int startIndex;
    unsigned int endIndex;
    Expression* expression;
  };

  // a ~ N(0,1)
  class StatementFunction : public Statement
  {
  public:
    StatementFunction(restan::ExpressionFunction* expressionFunction);
    virtual void execute() override;
  private:
    ExpressionFunction* funcEXPR;
  };


}

#endif
