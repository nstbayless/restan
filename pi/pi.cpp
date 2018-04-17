#include "pi.h"

using namespace restan;
using namespace adept;

Pi restan::pi;

GradValue restan::Pi::getLoss(const adept::Vector& parameters)
{
  params = parameters;
  adouble loss = 0;
  stack.new_recording();
  loss = (lossExpression->getValue())(0,0);
  
  // set independent variable
  loss.set_gradient(1.0);
  stack.compute_adjoint();
  
  // return value
  return GradValue(loss.value(), params.get_gradient());
}

GradValue getLoss(const adept::Vector& q)
{
  return pi.getLoss(q);
}