// hamiltonian markov monte carlo with Gibbs sampling for discrete parameters

#ifndef RESTAN_GHMCMC_H
#define RESTAN_GHMCMC_H

#include "HMCMC.h"

namespace restan
{
  //! Discrete Hamiltonian Markov Monte Carlo
  // u: loss function returning value and gradient
  // q0: initial value
  // epsilon: hyperparameter (leapfrog step-size)
  // L: hyperparameter (leapfrog step count)
  // samples: number of samples
  // samplesOut: returns an array of samples
  // epoch : the number of cycles of a leap frog step and gibbs sampling before taking a sample
  // L2: the number of samples each discrete coordinate is sampled
  void GHMCMC(restan::GradValue (*u)(const adept::Vector&), adept::Vector q0, double epsilon, unsigned int L, unsigned int numSamples, std::vector<double>* samplesOut, unsigned int epoch, unsigned int L2);
}

#endif