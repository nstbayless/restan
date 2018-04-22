// example

#include <iostream>

#include <adept.h>
#include <adept_arrays.h>

#include "parser/parser.h"
#include "hmcmc/HMCMC.h"

const int N_SAMPLES = 70;

int main(int argc, char** args)
{
  try
  {
    restan::parseStan(R"(
      parameters
      {
        real lambda;
        real beta;
      }
      transformed parameters
      {
        real zeta;
        real alpha;
      }
      model
      {
        alpha <- beta;
        lambda ~ normal(5, 1);
        beta ~ normal(lambda , 1);
      }
    )");
  }
  catch (restan::ParseError& e)
  {
    std::cerr << "PARSE ERROR" << std::endl;
    std::cerr << e.what() << std::endl;
    return 0;
  }
  adept::Vector q0 = {10, 15};
  adept::Vector samples[N_SAMPLES];
  restan::pi.setVariables(q0);
  std::cout<<"Beginning HMCMC" << std::endl;
  restan::HMCMC(restan::getLoss, q0, 0.1, 80, N_SAMPLES, samples);
  for (int i = 0; i < 50; i++)
    std::cout<<samples[i]<<std::endl;
  restan::parseStanCleanup();
  return 0;
}
