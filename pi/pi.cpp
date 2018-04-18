#include "pi.h"
#include <adept.h>
#include <adept_arrays.h>

using namespace restan;
using namespace adept;

Pi restan::pi;

void restan::Pi::executeStatement() 
{
  statement->execute();
}

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

ExpressionValue restan::Pi::getParams(unsigned int startIndex, unsigned int endIndex) 
{
  int size = endIndex - startIndex;
  adept::aMatrix mParams(1,size); 
  mParams << params(range(startIndex, endIndex -1));
  return mParams;
}

GradValue restan::getLoss(const adept::Vector& q)
{
  return pi.getLoss(q);
}

void restan::Pi::setStatement(Statement *s)
{
  statement = s;
}


ExpressionValue restan::Pi::getVariables(unsigned int startIndex, unsigned int endIndex)
{
  unsigned int size = endIndex - startIndex;
  adept::aMatrix mVars(1,size); 
  mVars << vars(range(startIndex, endIndex -1));
  //std::cout << "GetVariables" << std::endl;
  //std::cout << vars(range(startIndex, endIndex -1)) << std::endl;
  return mVars;
}

void restan::Pi::setVariables(const adept::Vector& variables) 
{
  vars = variables;
}

void restan::Pi::updateVariables(const ExpressionValue& vals, unsigned int startIndex, unsigned int endIndex) 
{
  unsigned int size = endIndex - startIndex;
  vars(range(startIndex, endIndex-1)) = vals(0, range(0, size-1));
}

//Setter only used for testing purposes
void restan::Pi::setParams(const adept::Vector& parameters)
{
  params = parameters;
}


void restan::setParams(const adept::Vector& parameters)
{
  pi.setParams(parameters);
}

void restan::setVariables(const adept::Vector& variables) 
{
  pi.setVariables(variables);
}

void restan::updateVariables(const ExpressionValue& vals, unsigned int startIndex, unsigned int endIndex) 
{
  pi.updateVariables(vals, startIndex, endIndex);
}

