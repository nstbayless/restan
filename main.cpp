// example

#include <iostream>
#include <armadillo>
#include "hmcmc/HMCMC.h"
#include "loss/lossfunctions.h"

using namespace arma;

int main(int argc, char** args)
{
  constexpr unsigned int N_SAMPLES = 8000;
  constexpr unsigned int dimension = 6;
  vec samples[N_SAMPLES];
  vec q0(dimension, fill::zeros);
  restan::HMCMC(restan::stdnormal, q0, 0.5, 25, N_SAMPLES, samples);
  
  for (unsigned int i = 0; i < N_SAMPLES; i++)
    std::cout << samples[i] << endl;
  
  return 0;
}