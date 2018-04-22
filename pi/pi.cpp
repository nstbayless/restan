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

const restan::Statement* restan::Pi::getLossStatement()
{
  return statement;
}

GradValue restan::Pi::getLoss(const adept::Vector& parameters)
{
  //Reset target to 0
  vars(0) = 0;

  // TODO: throw error if parameters.size() is not equal to numParams

  // update parameters
  params = parameters;

  // begin autodiff
  stack.new_recording();

  // calculate loss
  executeStatement();

  // set independent variable
  vars(0).set_gradient(1.0);
  stack.compute_adjoint();

  // return value
  return GradValue(vars(0).value(), params.get_gradient());
}

ExpressionValue restan::Pi::getParams(unsigned int startIndex, unsigned int endIndex)
{
  int size = endIndex - startIndex;
  adept::aMatrix mParams(1,size);
  mParams << params(range(startIndex, endIndex -1));
  return mParams;
}
adept::aVector restan::Pi::getParams()
{
  return params;
}

GradValue restan::getLoss(const adept::Vector& q)
{
  return pi.getLoss(q);
}

void restan::Pi::setLossStatement(Statement *s)
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
  numVariables = variables.size();
}

void restan::Pi::updateVariables(const ExpressionValue& vals, unsigned int startIndex, unsigned int endIndex)
{
  // TODO: throw error if index exceeds numVariables
  unsigned int size = endIndex - startIndex;
  vars(range(startIndex, endIndex-1)) = vals(0, range(0, size-1));
}

//Setter only used for testing purposes
void restan::Pi::setParams(const adept::Vector& parameters, unsigned int discreteIndStart, unsigned int* discreteDomLengths)
{
  params = parameters;
  numParams = params.size();
  discreteIndexStart = discreteIndStart;
  discreteDomainLengths = discreteDomLengths;
}
void restan::Pi::setParam(unsigned int index, double value)
{
  // TODO: throw error if index exceeds numParams
  params(index) = value;
}

void restan::setParams(const adept::Vector& parameters, unsigned int discreteIndexStart, unsigned int* discreteDomainLengths)
{
  // TODO: throw error if index exceeds numParams
  pi.setParams(parameters, discreteIndexStart, discreteDomainLengths);
}

void restan::setVariables(const adept::Vector& variables)
{
  // TODO: throw error if index exceeds numVariables
  pi.setVariables(variables);
}

void restan::updateVariables(const ExpressionValue& vals, unsigned int startIndex, unsigned int endIndex)
{
  // TODO: throw error if index exceeds numVariables
  pi.updateVariables(vals, startIndex, endIndex);
}
