// a bunch of common expressions

#include <adept.h>
#include <adept_arrays.h>

#include "statementTypes.h"
#include "pi/pi.h"

using namespace restan;

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
restan::StatementAsign::StatementAsign(restan::AssignOperator op, int varIndex, Expression* expression)
:	op(op),
	varIndex(varIndex),
	expression(expression)
{}	
void restan::StatementAsign::execute()
{
	ExpressionValue rhsVal = expression->getValue();

	switch(op) {
		case EQUALS:
			break;
		case PLUSEQUALS:
			break;
		case MINUSEQUALS:
			break;
		case TIMESEQUALS:
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