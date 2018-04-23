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

	setParams(testV, 7, NULL);


	ExpressionParameter myParam(0);
	REQUIRE( myParam.getValue()(0,0) == testV(0));

	ExpressionParameter myLongParam(1,3);
	ExpressionValue mv = myLongParam.getValue();
	ExpressionValue testEV(1, 2); testEV << testV(range(1,2));

	std::cout << testEV << std::endl;
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
	setParams(testParams, 7, NULL);

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

TEST_CASE ("ExpressionDereference testing")
{
	//Create testVector
	//Vector testV = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
	//setParams(testV, 7, NULL);

	ExpressionParameter vectorEXPR(1,4); //2, 3, 4
	ExpressionConstant indexEXPR(1);

	ExpressionDereference derefEXPR(&vectorEXPR, &indexEXPR);
	std::cout << derefEXPR.getValue() << std::endl;

	REQUIRE( derefEXPR.getValue()(0,0).value() == 3);
}

TEST_CASE ("Testing simple setter for Params and Variables")
{
	pi.setParams(6);
	pi.setVariables(4);

	std::cout << pi.getParams() << std::endl;
}

TEST_CASE ("Testing ExpressionData")
{
	//Set Paramsr
	adept::Vector testParams = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
	setParams(testParams, 7, NULL);
	Vector variables = {10, 11, 12, 13};
	setVariables(variables);


	//3 variales, 4 observed each
	ExpressionValue data1(1,4); data1 << 1, 2, 3, 4;
	ExpressionValue data2(1,4); data2 << 5, 6, 7, 8;
	ExpressionValue data3(1,4); data3 << 10, 11, 12, 14;
	ExpressionValue* data[3];
	data[0] = &data1;
	data[1] = &data2;
	data[2] = &data3;
	pi.data = data;

	ExpressionData data1EXPR(0);
	ExpressionData data2EXPR(1);
	ExpressionData data3EXPR(2);

/*	std::cout << data1EXPR.getValue() << std::endl;
	std::cout << data2EXPR.getValue() << std::endl;
	std::cout << data3EXPR.getValue() << std::endl;*/
}				

TEST_CASE ("Testing transformed parameters")
{
	//Set Params
	adept::Vector testParams = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
	setParams(testParams, 7, NULL);

	std::cout << "Transforming parameters" << std::endl;
	std::cout << pi.getParams() << std::endl;


	ExpressionParameter paramsToTransform1(1, 3); //2, 3
	restan::Expression* EXPRArray[1];
	EXPRArray[0] = &paramsToTransform1;
	ExpressionFunction func1(restan::functions::exp, EXPRArray, 1);

	ExpressionParameter paramsToTransform2(5); // 6
	restan::Expression* EXPRArray2[1];
	EXPRArray2[0] = &paramsToTransform2;

	ExpressionFunction func2(restan::functions::exp, EXPRArray2, 1);

	restan::Expression* f1Ptr = &func1;
	restan::Expression* f2Ptr = &func2;
	


	std::vector<restan::Expression*> outputExpressions = {f1Ptr};
	outputExpressions.push_back(f2Ptr);

	std::cout << "Reassigning output expression" << std::endl;
	pi.outputExpressions = outputExpressions;

	std::cout << "Calling pi.output()" <<std::endl;
	std::vector<double> output = pi.output();
	std::cout << "Output params: Should be ~7 20 403" << std::endl;
	for (double param : output)
	{
		std::cout << param << " ";
	}
	std::cout << std::endl;

}