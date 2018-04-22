#include <adept.h>
#include <adept_arrays.h>
#include "pi/pi.h"
#include "pi/expressionTypes.h"
#include "pi/statementTypes.h"
#include "pi/distributions.h"


#include "hmcmc/HMCMC.h"

using namespace restan;
using namespace adept;


int main(int argc, char** args)
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
		std::cout<<samples[i]<<std::endl;
  	}
  	std::cout << "Average: " << average/numSamples << std::endl;
  	return 0;
}