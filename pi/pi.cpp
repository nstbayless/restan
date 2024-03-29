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
  if (parameters.size() != numParams)
  {
     throw PiError("params.size() is not numParams in Pi::getLoss:28");
  }

  // update parameters
  params(range(0, discreteIndexStart - 1)) = parameters(range(0, discreteIndexStart - 1));
  //params = parameters;
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

void restan::Pi::setVariables(unsigned int numVars)
{
  Vector v(numVars);
  vars = v;
  numVariables = numVars;
}

void restan::Pi::updateVariables(const ExpressionValue& vals, unsigned int startIndex, unsigned int endIndex)
{
  if (endIndex > numVariables )
  {
     throw PiError("endIndex exceeds numVariables in restan::updateVariables");
  }
  // TODO: throw error if index exceeds numVariables
  unsigned int size = endIndex - startIndex;
  vars(range(startIndex, endIndex-1)) = vals(0, range(0, size-1));
}

void restan::Pi::setParams(unsigned int numParameters)
{
  Vector v(numParameters);
  for (int i = 0; i < numParameters; i++)
    v[i] = 0;
  params =  v;
  numParams = numParameters;
}

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
  if (index > numParams)
  {
     throw PiError("index exceeds numParams in Pi::setParam");
  }

  params(index) = value;
}


void restan::setParams(const adept::Vector& parameters, unsigned int discreteIndexStart, unsigned int* discreteDomainLengths)
{
  //TODO: check if abstraction layer correct, and working as desired - SUSPICIOUS atm
  /*if (pi.getNumParams() == 0)
  {
	pi.numParams = parameters.size();
	for (int i=0; i<numParams; i++)
		pi.params[i] = 0;
  }

  if (parameters.size() > pi.getNumParams() ) //TODO: check
  {
     throw PiError("index exceeds numParams in restan::setParams");
  }*/
  // TODO: throw error if index exceeds numParams
  pi.setParams(parameters, discreteIndexStart, discreteDomainLengths);
}

void restan::setVariables(const adept::Vector& variables)  //TODO: check
{
  if (variables.size() > pi.numVariables )
  {
     throw PiError("index exceeds numVariables in Pi::setVariables");
  }
  // TODO: throw error if index exceeds numVariables
  pi.setVariables(variables);
}

void restan::updateVariables(const ExpressionValue& vals, unsigned int startIndex, unsigned int endIndex)
{
  if (endIndex > pi.numVariables)
  {
     throw PiError("endIndex exceeds numVariables in Pi::updateVariables");
  }
  // TODO: throw error if index exceeds numVariables
  pi.updateVariables(vals, startIndex, endIndex);
}

// Retransforms the parameters that have been constrained to log or log-odds space
std::vector<double> restan::Pi::output()
{
  std::vector<double> outputParams = {};
  int i = 0;
  for (Expression* exprP : pi.outputExpressions)
  {
    ExpressionValue outputParamEXPR = exprP->getValue();
    //std::cout << outputParamEXPR << std::endl;
    unsigned int numParams = outputParamEXPR.size();
    for (int j = 0; j < numParams; j++)
    {
      outputParams.push_back( outputParamEXPR(0, j).value() );
      i++;
    }
  }
  return outputParams;
}

// TODO:: return smarter
ExpressionValue restan::Pi::getData(unsigned int dataIndex)
{
  return *(pi.data[dataIndex]);
}
