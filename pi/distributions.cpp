#include "distributions.h"
#include "expression.h"
#include <math.h>

using namespace restan;

const double PI  =3.141592653589793238463;
/*
  Returns the nll of the p[d|m]f
*/
ExpressionValue restan::distributions::normal(ExpressionValue* exps, unsigned int) // x, mu, sigma 
{
	ExpressionValue xMinusMulhs = (exps[0] - exps[1]);
  ExpressionValue xMinusMurhs = transpose(exps[0] - exps[1]);
  ExpressionValue xMinusMuSquared = adept::matmul(xMinusMulhs, xMinusMurhs);

  ExpressionValue sigmaSquared = adept::matmul(exps[2], transpose(exps[2]));


  //	std::cout << lhs.dimensions() << std::endl;
  //	std::cout << rhs.dimensions() << std::endl;
  return -log(1/(2*PI*sigmaSquared)) + xMinusMuSquared/(2*sigmaSquared);
}
ExpressionValue restan::distributions::uniform(ExpressionValue* exps, unsigned int) // x
{
  return 0 * exps[0];
}
ExpressionValue restan::distributions::pareto(ExpressionValue* exps, unsigned int) // x, xm, alpha
{
    // TODOwun

}
ExpressionValue restan::distributions::exponential(ExpressionValue* exps, unsigned int) // x, lambda
{
    // TODO :: This is wrong
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

double fact(int k) {
  if (k < 0) throw restan::distributions::DistributionException();

  double factorial = 1;
  for (int i = 1; i <= k; i++)
  {
    factorial *= i;
  }
  return factorial;
}

ExpressionValue restan::distributions::poisson(ExpressionValue* exps, unsigned int) // x, lambda
{   
    unsigned int n = exps[0].size(); //num variables
    ExpressionValue result(1,1);
    result << 0;


    //std::cout << round(exps[0](0,0).value()) << std::endl;
    for (int i = 0; i < n; i++) 
    {
      double x = exps[0](0,i).value();
      result +=  -log(pow(exps[1](0,i), exps[0](0,i)) * exp(-exps[1](0,i)) / fact(round(x)));
      //std::cout << "Summing nll: " << result << std::endl;
    }
    return result;
}


//log(pow(exps[1](0,i), exps[0](0,i)) * exp(-exps[1](0,i)) / 