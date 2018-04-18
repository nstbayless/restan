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

}
//// StatementAssign ////
restan::StatementAsign::StatementAsign(restan::AssignOperator op, int varIndex, Expression* expression)
:	op(op),
	varIndex(varIndex),
	expression(expression)
{}	
void restan::StatementAsign::execute()
{

}
//// StatementFunction ////
restan::StatementFunction::StatementFunction(ExpressionValue (*sf)(ExpressionValue[], int))
:	sf(sf)
{}
void restan::StatementFunction::execute()
{

}