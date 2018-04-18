#include "pi.h"
#include <adept.h>
#include <adept_arrays.h>

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

ExpressionValue restan::Pi::getParams(int startIndex, int endIndex) 
{
  //TODO: Check if range access returns new Vector
  int size = endIndex - startIndex;
  adept::aMatrix mParams(1,size); 
  mParams << params(range(startIndex, endIndex -1));
  return mParams;
}

void restan::Pi::setParams(const adept::Vector& parameters)
{
  params = parameters;
}


GradValue restan::getLoss(const adept::Vector& q)
{
  return pi.getLoss(q);
}

void restan::setParams(const adept::Vector& parameters)
{
  pi.setParams(parameters);
}