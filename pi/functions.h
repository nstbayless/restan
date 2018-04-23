#ifndef RESTAN_FUNCTIONS_H
#define RESTAN_FUNCTIONS_H

#include "expressionTypes.h"

namespace restan
{
  namespace functions
  {
  	class FunctionException : public std::exception
	  {
	  public:
	    FunctionException() {}
	  };

    ExpressionValue exp(ExpressionValue* exps, unsigned int); // x
  }
}

#endif
