// hamiltonian markov monte carlo

#include <cmath>
#include <stdexcept>
#include <iostream>
#include <cstdio>
#include <random>
#include <ctime>

#include <adept.h>
#include <adept_arrays.h>

#include "HMCMC.h"

using namespace restan;
using namespace std;
using namespace adept;

class SampleRejected : public std::exception
{
public:
  SampleRejected() {}
};

void HMCMCUpdate(GradValue (*u)(adept::Vector), Vector q, const double epsilon, const unsigned int L, Vector* sampleOut);

bool randomized = false;

void restan::HMCMC(GradValue (*u)(adept::Vector), const Vector q0, double epsilon, unsigned int L, unsigned int samples, Vector* samplesOut)
{ 
  if (!randomized)
    srand(time(0));
  randomized = true;
  Vector q(q0);
  
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

default_random_engine generator;
normal_distribution<double> stdnormal(0, 1.0);
uniform_real_distribution<double> stdunif(0, 1.0);

void HMCMCUpdate(GradValue (*u)(adept::Vector), Vector q, const double epsilon, const unsigned int L, Vector* sampleOut)
{
  unsigned int d = q.size();
  
  // momentum resample
  Vector p(d);
  for (int i = 0; i < d; i++)
  {
    p[i] = stdnormal(generator);
  }
  
  // leapfrog (TODO: confirm this? seems to differ only on first and last step)
  GradValue uqStart(u(q));
  p -= (epsilon / 2) * uqStart.second;
  
  for (int i = 0; i < L; i++)
  {
    q += p * epsilon;
    p -= epsilon * u(q).second;
  }
  
  q += p * epsilon / 2;
  
  GradValue uqEnd;
  uqEnd = u(q); 
  
  // accept or reject:
  if (stdunif(generator) > exp(uqEnd.first - uqStart.first))
    throw SampleRejected();
  
  *sampleOut = q;
}