#include "distributions.h"
#include "expression.h"
#include <math.h>

using namespace restan;

ExpressionValue restan::distributions::normal(ExpressionValue* exps, unsigned int) // x, mu, sigma 
{
	ExpressionValue lhs = (exps[0] - exps[1]);
  	ExpressionValue rhs = transpose(exps[0] - exps[1]);

  //	std::cout << lhs.dimensions() << std::endl;
  //	std::cout << rhs.dimensions() << std::endl;
  return adept::matmul(lhs, rhs) / (2.0 * exps[2](0,0));
}
ExpressionValue restan::distributions::uniform(ExpressionValue* exps, unsigned int) // x
{
  return 0 * exps[0];
}
ExpressionValue restan::distributions::pareto(ExpressionValue* exps, unsigned int) // x, xm, alpha
{
    // TODOwun

}
ExpressionValue restan::distributions::Exponential(ExpressionValue* exps, unsigned int) // x, lambda
{
    // TODO
    unsigned int n = exps[0].size();

    ExpressionValue result(1, n);
    for (int i = 0; i < n; i++) {
    	if (exps[0](0,i) < 0)
    		result(0, i) = 0;
    	else 
    		result(0,i) = exps[1](0,0) * exp(-exps[1](0,0)*exps[0](0,i));
    }
    return result;
}
