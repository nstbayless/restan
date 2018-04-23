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
#include "pi/pi.h"

using namespace restan;
using namespace std;
using namespace adept;



void HMCMCUpdate(GradValue (*u)(const adept::Vector&), Vector q, const double epsilon, const unsigned int L, Vector* sampleOut);


bool randomized = false;
void restan::HMCMC(GradValue (*u)(const adept::Vector&), const Vector q0, double epsilon, unsigned int L, unsigned int samples, Vector* samplesOut)
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

void HMCMCUpdate(GradValue (*u)(const adept::Vector&), Vector q, const double epsilon, const unsigned int L, Vector* sampleOut)
{
  unsigned int d = q.size();
  unsigned int contIndexEnd = pi.discreteIndexStart -1;
  // momentum resample
  Vector p(d);
  for (int i = 0; i < d; i++)
  {
    p[i] = stdnormal(generator);
  }
  
  // leapfrog (confirm this? seems to differ only on first and last step) - CONFIRMED, telescopes at t+eps!
  GradValue uqStart(u(q));
  p -= (epsilon / 2) * uqStart.second;
  
  for (int i = 0; i < L; i++)
  {
    q(range(0, contIndexEnd)) += p(range(0, contIndexEnd)) * epsilon;
    p -= epsilon * u(q).second;
  }
  
  q(range(0, contIndexEnd)) += p(range(0, contIndexEnd)) * epsilon / 2;
  
  GradValue uqEnd;
  uqEnd = u(q); 
  
  // accept or reject:
  if (stdunif(generator) > exp(uqEnd.first - uqStart.first))
    throw SampleRejected();
  
  *sampleOut = q;
}
