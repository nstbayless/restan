// STAN model loss calculator

#ifndef RESTAN_PI_H
#define RESTAN_PI_H

#include <adept.h>
#include <adept_arrays.h>

#include "hmcmc/HMCMC.h"
#include "expression.h"

namespace restan
{
  class Pi
  {
  public:  
    // loss value and gradient of given state
    GradValue getLoss(const Vector q& parameters)

  private:
    //// these variables are fixed from evaluation to evaluation (getLoss) ////
    Expression* lossExpression;
  
    //// these variables change from evaluation to evaluation (getLoss) ////
    
    // adept execution trace
    adept::Stack stack;
    
    // parameter vector
    adept::aVector params;
    
    // cached expressions
    adept::aVector tparams;
  } pi;
  
  

  GradValue getLoss(const Vector q&)
  {
    return pi.getLoss(q);
  }
}

#endif