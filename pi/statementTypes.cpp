// a bunch of common expressions

#include <adept.h>
#include <adept_arrays.h>

#include "statementTypes.h"
#include "pi/pi.h"

using namespace restan;
using namespace adept;

//// StatementBody ////
restan::StatementBody::StatementBody(Statement* statements, int length)
:length(length),
statements(statements)
{}

void restan::StatementBody::execute()
{
	for (int i = 0; i < length; i++) {
		statements[i].execute();
	}
}
//// StatementAssign ////
restan::StatementAssign::StatementAssign(restan::AssignOperator op, unsigned int startIndex, unsigned int endIndex, Expression* expression)
:	op(op),
	startIndex(startIndex),
	endIndex(endIndex),
	expression(expression)
{}	

void restan::StatementAssign::execute()
{
	//ASSUME::variables is a vector so we cast expression->getValue to a vector
	//TODO:: 1xn or nx1?
	ExpressionValue rhsVal = expression->getValue();



	switch(op) {
		case EQUALS:
			updateVariables(rhsVal, startIndex, endIndex);
			break;
		default:
			std::cout << "Assignment Operator invalid!!" << std::endl;
	}
}
//// StatementFunction ////
restan::StatementFunction::StatementFunction(ExpressionValue (*sf)(ExpressionValue[], int))
:	sf(sf)
{}
void restan::StatementFunction::execute()
{

}