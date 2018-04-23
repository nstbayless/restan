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

  class StartIndexInvalid : public std::exception
  {
  public:
    StartIndexInvalid() {}
  };

  class PiError : public std::exception
  {
    public:
      PiError(std::string s): whatText(s) {}
      std::string what()
      {
        return whatText.c_str();
      }
    private:
    std::string whatText;
  };

  class Pi
  {
  public:
    // loss value and gradient of given state
    GradValue getLoss(const adept::Vector& parameters);
    ExpressionValue getParams(unsigned int startIndex, unsigned int endIndex);
    void setLossStatement(Statement* s);
    const restan::Statement* getLossStatement();
    void executeStatement();
    void setVariables(unsigned int numVariables);
    void setVariables(const adept::Vector& variableso);
    void updateVariables(const ExpressionValue& vals, unsigned int startIndex, unsigned int endIndex);
    ExpressionValue getVariables(unsigned int startIndex, unsigned int endIndex);

    void setParams(unsigned int numParameters);
    void setParams(const adept::Vector& parameters, unsigned int discreteIndStart, unsigned int* discreteDomainLengths);
    void setParam(unsigned int index, double value);
    ExpressionValue getData(unsigned int variableDataIndex);
    adept::aVector getParams();

    std::vector<double> output();
    unsigned int discreteIndexStart;
    unsigned int* discreteDomainLengths;
    unsigned int numParams = 0;
    unsigned int numVariables = 0;
    unsigned int numObservedData = 0;

    // pointer to pointer table of ExpressionValues
    ExpressionValue** data;

    // Converts our constrainted parameters
    std::vector<Expression*> outputExpressions;
  private:
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
  //TEST HELPERS
  void setParams(const adept::Vector& parameters, unsigned int discreteIndexStart, unsigned int* discreteDomainLengths);
  void setVariables(const adept::Vector& variables);
  void updateVariables(const ExpressionValue& vals, unsigned int startIndex, unsigned int endIndex);
}

#endif
