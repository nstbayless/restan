// example

#include <iostream>

#include <adept.h>
#include <adept_arrays.h>

#include "parser/parser.h"
#include "hmcmc/HMCMC.h"

const int N_SAMPLES = 8000;

int main(int argc, char** args)
{
  /* //Different test
    __BEGIN_STAN_CODE__
    model
    {
      target += 3;
	  cat ~ normal(0,0);
	  param ~ beta(1);
	  real dog;
	  dog ~ poisson(3);
    }
    __END_STAN_CODE__ */

  try
  {
    restan::parseStan(R"(
    __BEGIN_STAN_CODE__
    model
    {
      target += 3;
    }
    __END_STAN_CODE__
  )");
  }
  catch (restan::ParseError& e)
  {
    std::cerr << "PARSE ERROR" << std::endl;
    std::cerr << e.what() << std::endl;
    return 0;
  }
  adept::Vector q0 = {10};
  adept::Vector samples[N_SAMPLES];
  restan::HMCMC(restan::getLoss, q0, 0.1, 25, N_SAMPLES, samples);
  ////for (int i = 0; i < 40; i++)
///std::cout<<samples[i]<<std::endl;
  restan::parseStanCleanup();
  return 0;
}
