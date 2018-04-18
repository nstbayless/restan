// STAN model loss calculator

#ifndef RESTAN_PI_H
#define RESTAN_PI_H

#include <adept.h>
#include <adept_arrays.h>

#include "hmcmc/gradVal.h"
#include "expression.h"


 

namespace restan
{
  class Pi
  {
  public:  
    // loss value and gradient of given state
    GradValue getLoss();
    ExpressionValue getParams(int startIndex, int endIndex);
    //TODO: Used for testing purposes
    void setParams(const adept::Vector& parameters);
    void setLossExpression(Expression*);
  private:
    //// these variables are fixed from evaluation to evaluation (getLoss) ////
    Expression* lossExpression;
  
    //// these variables change from evaluation to evaluation (getLoss) ////

    // adept execution trace
    adept::Stack stack;
    // parameter vector
    adept::aVector params;
  };
  
  extern Pi pi;

  GradValue getLoss(const adept::Vector& q);
}

#endif