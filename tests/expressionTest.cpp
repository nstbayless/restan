#include "tests/test.h"



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

	setParams(testV, 7);


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
	setParams(testParams, 7);

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


TEST_CASE ("StartIndexInvalid error") 
{
	Vector variables = {10, 11, 12, 13};
	setVariables(variables);

	ExpressionVariable singleVariable(-1);

	CHECK_THROWS(singleVariable.getValue());

	ExpressionParameter singleParam(-1);

	CHECK_THROWS(singleParam.getValue());
}