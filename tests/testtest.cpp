#include <iostream>
#include "catch.hpp"
#include <adept.h>
#include <adept_arrays.h>
#include "pi/pi.h"
#include "pi/expressionTypes.h"
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
	Vector testV = {1.0, 2.0, 3.0};

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
/*
Tests to write:
Expression constant: Return constant
Expression parameter: Return the index in the vector
Expression arithmetic: Evaluation the lhs, Evaluate the rhs, switch on op

TODO: Add a constructor for const that takes a double
*/