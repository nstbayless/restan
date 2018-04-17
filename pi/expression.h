#ifndef RESTAN_EXPRESSION_H
#define RESTAN_EXPRESSION_H

#include <adept.h>
#include <adept_arrays.h>

namespace restan
{
  typedef adept::aMatrix ExpressionValue;

  // bread and butter of the AST
  class Expression
  {
  public:
    virtual ExpressionValue getValue() = 0;
  };
}

#endif