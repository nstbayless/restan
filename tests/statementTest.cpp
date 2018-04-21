#include "tests/test.h"


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

