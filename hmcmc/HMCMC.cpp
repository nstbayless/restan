// hamiltonian markov monte carlo

#include <cmath>
#include <stdexcept>
#include <iostream>
#include <cstdio>

#include <armadillo>
#include "loss/loss.h"
#include "HMCMC.h"

using namespace arma;
using namespace restan;

class SampleRejected : public std::exception
{
public:
  SampleRejected() {}
};

void HMCMCUpdate(Loss (*u)(vec), vec q, const double epsilon, const unsigned int L, vec* sampleOut);

void restan::HMCMC(Loss (*u)(vec), const vec q0, double epsilon, unsigned int L, unsigned int samples, vec* samplesOut)
{ 
  arma_rng::set_seed_random();
  vec q = q0;
  
  // generate samples (TODO: warmup):
  while (samples > 0)
  {
    try
    {
      HMCMCUpdate(u, q, epsilon, L, samplesOut);
      q = *samplesOut;
      samplesOut++;
      samples--;
    }
    catch (const SampleRejected&)
    {
      // sample rejected; try again.
      continue; 
    }
  }
}

void HMCMCUpdate(Loss (*u)(vec), vec q, const double epsilon, const unsigned int L, vec* sampleOut)
{
  // momentum
  vec p(q.n_elem, fill::randn);
  
  // leapfrog (TODO: confirm this? seems to differ only on first and last step)
  Loss uqStart = u(q);
  p -= (epsilon / 2) * uqStart.gradient;
  
  for (int i = 0; i < L; i++)
  {
    q += p * epsilon;
    p -= epsilon * u(q).gradient;
  }
  
  q += p * epsilon / 2;
  
  Loss uqEnd = u(q); 
  
  // accept or reject:
  if (randu() > exp(uqEnd.value - uqStart.value))
    throw SampleRejected();
  
  *sampleOut = q;
}