#include <iostream>
#include "catch.hpp"
#include <adept.h>
#include <adept_arrays.h>
#include "pi/pi.h"
#include "pi/expressionTypes.h"
#include "pi/statementTypes.h"
#include "pi/distributions.h"

using namespace restan;
using namespace adept;

//TODO: Pass by ref?
/*
	Helper function to compare two, two dimensional matrixes
*/
bool compareEV(ExpressionValue A, ExpressionValue B)
{
	int num_A_rows = A.dimension(0);
	int num_A_cols = A.dimension(1);
	int num_B_rows = B.dimension(0);
	int num_B_cols = B.dimension(1);

	if ((num_A_rows != num_B_rows) || (num_A_cols != num_B_cols)) {
		printf("\ntesttest.cpp/compareEV Dimensions of A and B do not match! A_row: %d, A_col: %d, B_row: %d, B_col: %d\n",
			num_A_rows, num_A_cols, num_B_rows, num_B_cols);
		return false;
	}

	for (int r = 0; r < num_A_rows; r++) 
		for (int c = 0; c < num_A_cols; c++) {
			if (A(r, c).value() != B(r, c).value()) {
				printf("\ntesttestcpp/compareEV values of A(%d, %d): %f and B(%d, %d): %f differ!\n", r, c, A(r,c).value(), r, c, B(r,c).value());
				return false;
			}
		}
	return true;
}

/*// Row order flattening of a matrix to vector
Vector matrixToVector(ExpressionValue EV)
{
	Vector flattenedVector(EV.size());
	int num_rows = EV.dimension(0);
	int num_cols = EV.dimension(1);
	for (int i = 0; i < num_rows; i++) {
		flattenedVector << EV(i, range(0, num_cols-1));
	}
	return flattenedVector;
}*/

TEST_CASE( "Testing constant expressions") 
{
	adept::aMatrix A(1,1);
	A(0, 0) = 5.0;
	REQUIRE(A(0,0) == 5.0);
	restan::ExpressionConstant myConstant(A);
	REQUIRE( myConstant.getValue()(0,0) == A(0,0) );
}

TEST_CASE( "Testing double ExpressionConstant constructor")
{
	double constantDouble = 5.0;
	restan::ExpressionConstant myConstant(constantDouble);
	REQUIRE( myConstant.getValue()(0,0) == constantDouble); 
}

TEST_CASE( "Testing parameter expressions" )
{
	//Create testVector
	Vector testV = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};

	setParams(testV);


	ExpressionParameter myParam(0);
	REQUIRE( myParam.getValue()(0,0) == testV(0));

	ExpressionParameter myLongParam(1,3);
	ExpressionValue mv = myLongParam.getValue();
	ExpressionValue testEV(1, 2); testEV << testV(range(1,2));

	//std::cout << testEV << std::endl;
	//std::cout << mv << std::endl;
	REQUIRE( compareEV(mv, testEV) );
}

TEST_CASE( "Testing arithmetic expressions" ) 
{
	ExpressionValue tMatrix(4,4);
	tMatrix << 	1,2,3,4,
				1,2,3,4,
				1,2,3,4,
				1,2,3,4;
	ExpressionValue tMatrix2(4,4);
	tMatrix2 <<	5,6,7,8,
				5,6,7,8,
				5,6,7,8,
				5,6,7,8;
	ExpressionValue tRowVector(1,4);
	tRowVector <<	2,4,6,8;
	ExpressionValue tRowVector2(1,5);
	tRowVector2 << 10, 11, 12, 13, 14;
	ExpressionValue tColVector(4,1);
	tColVector <<	1,3,5,7;
	ExpressionValue scalar(1,1);
	scalar	<<	25;

	//Set Paramsr
	adept::Vector testParams = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
	setParams(testParams);

	ExpressionParameter vecParams(0,3); //1 2 3
	ExpressionParameter vecParams2(3, 6); //4 5 6
	ExpressionValue trueAddResult(1,3); trueAddResult << 5, 7, 9;
	ExpressionValue trueSubResult(1,3); trueSubResult << -3, -3, -3;

	//std::cout << vecParams.getValue() << std::endl;
	//std::cout << vecParams2.getValue() << std::endl;

	ExpressionArithmetic addArithPass(PLUS, &vecParams, &vecParams2);
	ExpressionValue addVal = addArithPass.getValue();
	//std::cout << addVal << std::endl;
	REQUIRE( compareEV(addVal, trueAddResult));

	ExpressionArithmetic subArithPass(MINUS, &vecParams, &vecParams2);
	ExpressionValue subVal = subArithPass.getValue();
	//std::cout << subVal << std::endl;
	REQUIRE( compareEV(subVal, trueSubResult));
}

TEST_CASE( " ExpressionVariable testing" ) 
{
	Vector variables = {10, 11, 12, 13};
	setVariables(variables);

	ExpressionVariable singleVariable(1);
	REQUIRE( singleVariable.getValue()(0,0) == 11);

	ExpressionValue trueMultipleVariables(1,3);	trueMultipleVariables << 10, 11, 12;
	ExpressionVariable multipleVariables(0,3);
	REQUIRE( compareEV(trueMultipleVariables, multipleVariables.getValue()) );
}

TEST_CASE( "Statement testing" )
{
	Vector variables = {10, 11, 12, 13};
	setVariables(variables);
	Vector parameters = {1,2,3,4,5,6};
	setParams(parameters);

	ExpressionVariable varEXPR(0,4);
	//std::cout << varEXPR.getValue() << std::endl;

	//[v1, v2] = 2*[p1, p2]
	ExpressionParameter paramsExpr(1,3); //11, 12
	ExpressionConstant constExpr(2.0);

	ExpressionArithmetic arithExpr(TIMES, &constExpr, &paramsExpr);

	StatementAssign sa(restan::EQUALS, 1, 3, &arithExpr);
	sa.execute();

/*	
	//Testing updateVariables
	ExpressionValue hardCodedMatrix(1,2); hardCodedMatrix << 100, 101;
	std::cout << hardCodedMatrix << std::endl;
	std::cout << hardCodedMatrix(0, range(0, 1));

	updateVariables(hardCodedMatrix, 1,3);*/

	//variables should now be {10, 4, 6, 13}
	//std::cout << varEXPR.getValue() << std::endl;
	ExpressionValue expectedAssignedVariables(1,4); expectedAssignedVariables << 10, 4, 6, 13;
	REQUIRE( compareEV(varEXPR.getValue(), expectedAssignedVariables) );

}

TEST_CASE ( "Expression Function and Statement Function testing " )
{
	Vector parameters = {1,2,3,4,5,6};
	setParams(parameters);

	restan::Expression* EXPRArray[4];


	ExpressionConstant cEXPR1(5.0);
	ExpressionConstant cEXPR2(10.0);
	ExpressionParameter pEXPR1(1,3);
	ExpressionArithmetic aEXPR(PLUS, &cEXPR1, &cEXPR2);

	EXPRArray[0] = &cEXPR1; //5
	EXPRArray[1] = &cEXPR2; //10
	EXPRArray[2] = &pEXPR1; //[2, 3]
	EXPRArray[3] = &aEXPR; 	//15

	ExpressionFunction fEXPR(restan::distributions::normal, EXPRArray, 4);

//	std::cout << fEXPR.getValue() << std::endl;
}

TEST_CASE ("Scalar Normal Distribution test")
{
	Vector parameters = {1,2,3,4,5,6}; 
	setParams(parameters);

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
	setParams(parameters);

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
/*
TESTS:
Matrix/Vector operations

Statement constructors/getters/setters

ExpressionVariables
*/