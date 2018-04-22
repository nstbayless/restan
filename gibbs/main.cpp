#include <adept.h>
#include <adept_arrays.h>
#include "pi/pi.h"
#include "pi/expressionTypes.h"
#include "pi/statementTypes.h"
#include "pi/distributions.h"
#include "pi/functions.h"

#include "hmcmc/HMCMC.h"

using namespace restan;
using namespace adept;


void lambdaFromNormalTest() 
{
	//Build my own Pi
	//Pass in Pi.getLoss()
	//Generates Samples
	//Do gibbs on discrete variables
	double lambda = 0.5;
	Vector parameters = {lambda}; //lambda, Z
	setParams(parameters, 1);

	double target = 0;
	Vector variables = {target};
	setVariables(variables); //target is the sum of nll 

	ExpressionParameter lambdaEXPR(0);
	ExpressionConstant muEXPR(0.0);
	ExpressionConstant sigmaEXPR(1.0);

	restan::Expression* EXPRArray[3];
	EXPRArray[0] = &lambdaEXPR;
	EXPRArray[1] = &muEXPR;
	EXPRArray[2] = &sigmaEXPR;
	ExpressionFunction normalFuncEXPR(restan::distributions::normal, EXPRArray, 3);

	ExpressionVariable targetEXPR(0);

	ExpressionArithmetic targetSumEXPR(PLUS, &targetEXPR, &normalFuncEXPR);
	//std::cout << targetSumEXPR.getValue() << std::endl;
	StatementAssign sa(0, &targetSumEXPR);
	//std::cout << targetEXPR.getValue() << std::endl;
	pi.setLossStatement(&sa);


	int numSamples = 8000;
	Vector samples[numSamples];
	restan::HMCMC(getLoss, parameters, 0.1, 25, numSamples, samples);
	double average = 0;
  	for (int i = 0; i < numSamples; i++) {
  		average += samples[i][0];
		//std::cout<<samples[i]<<std::endl;
  	}
  	std::cout << "Average: " << average/numSamples << std::endl;
}

void discreteTest()
{
	double logLambda = 0.5;
	double Z = 3;
	double k = 2*Z + 3;
	double target = 0;

	Vector parameters = {logLambda, Z};
	setParams(parameters, 1);

	Vector variables = {target, k};
	setVariables(variables);


	ExpressionParameter logLambdaEXPR(0);
	ExpressionParameter ZEXPR(1);
	ExpressionVariable targetEXPR(0);
	ExpressionVariable kEXPR(1);
	ExpressionConstant muEXPR(0);
	ExpressionConstant sigmaEXPR(1);
	ExpressionConstant twoEXPR(2);
	ExpressionConstant threeEXPR(3);

	//log lambda ~ N(0,1)
	restan::Expression* NormalEXPRArray[3];
	NormalEXPRArray[0] = &logLambdaEXPR;
	NormalEXPRArray[1] = &muEXPR;
	NormalEXPRArray[2] = &sigmaEXPR;
	
	ExpressionFunction normalFuncEXPR(restan::distributions::normal, NormalEXPRArray, 3);
	ExpressionArithmetic targetNormalSumEXPR(PLUS, &targetEXPR, &normalFuncEXPR);
	StatementAssign normalStatement(0, &targetNormalSumEXPR);

	//k ~ 2Z + 3
	ExpressionArithmetic twoZEXPR(TIMES, &twoEXPR, &ZEXPR);
	ExpressionArithmetic twoZPlusThreeEXPR(PLUS, &threeEXPR, &twoZEXPR);
	StatementAssign kStatement(1, &twoZPlusThreeEXPR);

	//k ~ Poisson(exp(log lambda))

	restan::Expression* ExpLogLambdaArray[1];
	ExpLogLambdaArray[0] = &logLambdaEXPR;
	ExpressionFunction expLogLambdaEXPR(restan::functions::exp, ExpLogLambdaArray, 1);

	restan::Expression* PoissonEXPRArray[2];
	PoissonEXPRArray[0] = &ZEXPR;
	PoissonEXPRArray[1] = &expLogLambdaEXPR;

	ExpressionFunction poissonFuncEXPR(restan::distributions::poisson, PoissonEXPRArray, 2);
	ExpressionArithmetic targetPoissonSumEXPR(PLUS, &targetEXPR, &poissonFuncEXPR);
	StatementAssign poissonStatement(0, &targetPoissonSumEXPR);

	//Put all statements into statement body
	restan::Statement* StatementArray[3];
	StatementArray[0] = &normalStatement;
	StatementArray[1] = &kStatement;
	StatementArray[2] = &poissonStatement;
	StatementBody piStatement(StatementArray, 3);

	pi.setLossStatement(&piStatement);


	int numSamples = 5000;
	Vector samples[numSamples];
	restan::HMCMC(getLoss, parameters(range(0, 0)), 0.1, 25, numSamples, samples);

	double averageLambda = 0;
  	for (int i = 0; i < numSamples; i++) {
  		averageLambda += samples[i](0);
		//std::cout<<samples[i]<<std::endl;
  	}
  	std::cout << averageLambda/numSamples << std::endl;
}


int main(int argc, char** args)
{
	discreteTest();
  	return 0;
}