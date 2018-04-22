// a bunch of common expressions

#include <adept.h>
#include <adept_arrays.h>

#include "statementTypes.h"
#include "expressionTypes.h"
#include "pi/pi.h"

using namespace restan;
using namespace adept;

//// StatementBody ////
restan::StatementBody::StatementBody(Statement** statements, int length)
:length(length),
statements(statements)
{}

void restan::StatementBody::execute()
{
	for (int i = 0; i < length; i++) {
		statements[i]->execute();
	}
}

void restan::StatementBody::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "+ StatementBody {\n";
	for (int i = 0; i < length; i++)
		statements[i]->print(depth + 1);
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "  }\n";
}

//// StatementAssign ////
restan::StatementAssign::StatementAssign(unsigned int startIndex, unsigned int endIndex, Expression* expression)
:	startIndex(startIndex),
	endIndex(endIndex),
	expression(expression)
{}

restan::StatementAssign::StatementAssign(unsigned int index, Expression* expression)
:	startIndex(index),
	endIndex(index + 1),
	expression(expression)
{}

void restan::StatementAssign::execute()
{
	//ASSUME::variables is a vector so we cast expression->getValue to a vector
	//TODO:: 1xn or nx1?
	ExpressionValue rhsVal = expression->getValue();
	if (startIndex == -1) {
		throw StartIndexInvalid();
	}
	//std::cout << rhsVal << std::endl;
	updateVariables(rhsVal, startIndex, endIndex);
}

void restan::StatementAssign::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "+ StatementAssign [" << startIndex;
	if (endIndex != startIndex + 1)
		std::cout << ", "<< endIndex;
	std::cout << "] <-\n";
	expression->print(depth);
}

//// StatementFunction ////
//ExpressionValue (*sf)(ExpressionValue[], int)
restan::StatementFunction::StatementFunction(restan::ExpressionFunction* expressionFunction)
:	funcEXPR(expressionFunction)
{}

void restan::StatementFunction::execute()
{
	funcEXPR->getValue();
}

void restan::StatementFunction::print(int depth) const
{
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << "+ StatementFunction:\n";
  print(funcExpr);
}
