// hamiltonian markov monte carlo

#ifndef RESTAN_HMCMC_H
#define RESTAN_HMCMC_H

#include <adept.h>
#include <adept_arrays.h>

#include "gradVal.h"

namespace restan
{

  class SampleRejected : public std::exception
  {
  public:
    SampleRejected() {}
  };


  //! Hamiltonian Markov Monte Carlo
  // u: loss function returning value and gradient
  // q0: initial value
  // epsilon: hyperparameter (leapfrog step-size)
  // L: hyperparameter (leapfrog step count)
  // samples: number of samples
  // samplesOut: returns an array of samples
  
  void HMCMC(restan::GradValue (*u)(const adept::Vector&), adept::Vector q0, double epsilon, unsigned int L, unsigned int numSamples, std::vector<double>* samples);
}

#endif