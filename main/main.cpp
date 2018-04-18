// example

#include <iostream>

#include <adept.h>
#include <adept_arrays.h>

#include "parser/parser.h"
#include "hmcmc/HMCMC.h"

const int N_SAMPLES = 8000;

int main(int argc, char** args)
{
  //restan::parseStan("model { \nincrement_log_prob(4 * 4 + 3 * alpha * alpha  + 4) ;\n}");
  restan::parseStan("4 * 4 + 3 * alpha * alpha  + 4");
  adept::Vector q0 = {10};
  adept::Vector samples[N_SAMPLES];
  restan::HMCMC(restan::getLoss, q0, 0.1, 25, N_SAMPLES, samples);
  ////for (int i = 0; i < 40; i++)
///std::cout<<samples[i]<<std::endl;
  restan::parseStanCleanup();
  return 0;
}
