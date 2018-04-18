// STAN model loss calculator

#ifndef RESTAN_PI_H
#define RESTAN_PI_H

#include <adept.h>
#include <adept_arrays.h>

#include "hmcmc/gradVal.h"
#include "expression.h"
#include "statement.h"

 

namespace restan
{
  class Pi
  {
  public:  
    // loss value and gradient of given state
    GradValue getLoss(const adept::Vector& parameters);
    ExpressionValue getParams(int startIndex, int endIndex);
    void  setStatement(Statement* s);
    void executeStatement();
    void setVariables(const adept::Vector& parameters);
    ExpressionValue getVariables(int startIndex, int endIndex);

    //TODO: Used for testing purposes
    void setParams(const adept::Vector& parameters);

  private:
    //// these variables are fixed from evaluation to evaluation (getLoss) ////
    Expression* lossExpression;
  
    //// these variables change from evaluation to evaluation (getLoss) ////

    // adept execution trace
    adept::Stack stack;
    // parameter vector
    adept::aVector params;
    // variable vector
    adept::aVector vars;

    // statement list
    Statement* statement;
  };
  
  extern Pi pi;

  GradValue getLoss(const adept::Vector& q);
  void setParams(const adept::Vector& parameters);

}

#endif