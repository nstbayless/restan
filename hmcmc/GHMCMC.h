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
  // epoch : how long it cycles between HMCMC and gibbs before choosing a sample
  // L2: how long it iterates gibbs sampling
  void GHMCMC(restan::GradValue (*u)(const adept::Vector&), adept::Vector q0, double epsilon, unsigned int L, unsigned int numSamples, std::vector<double>* samplesOut, unsigned int epoch, unsigned int L2);
}

#endif