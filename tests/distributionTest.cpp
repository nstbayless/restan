#include "tests/test.h"


TEST_CASE ("Scalar Normal Distribution test")
{
	Vector parameters = {1,2,3,4,5,6}; 
	setParams(parameters,6, NULL);

	ExpressionConstant xEXPR(10.0);
	ExpressionConstant muEXPR(5.0);
	ExpressionConstant sigmaEXPR(2.0);

	restan::Expression* EXPRArray[3];

	EXPRArray[0] = &xEXPR; 
	EXPRArray[1] = &muEXPR; 
	EXPRArray[2] = &sigmaEXPR; 

	ExpressionFunction fEXPR(restan::distributions::normal, EXPRArray, 3);

	std::cout << fEXPR.getValue() << std::endl;
}

TEST_CASE ("Vector Normal Distribution test")
{
	Vector parameters = {12.0, 24.0, 3.0, 4.0, 0.5, 0.5}; //x x mu mu sigma
	setParams(parameters, 7, NULL);

	ExpressionParameter xEXPR(0,2);
	ExpressionParameter muEXPR(2,4);
	ExpressionParameter sigmaEXPR(4);

	restan::Expression* EXPRArray[3];

	EXPRArray[0] = &xEXPR; 
	EXPRArray[1] = &muEXPR; 
	EXPRArray[2] = &sigmaEXPR; 

	ExpressionFunction fEXPR(restan::distributions::normal, EXPRArray, 3);

	std::cout << fEXPR.getValue() << std::endl;
}