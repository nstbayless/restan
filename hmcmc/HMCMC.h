// hamiltonian markov monte carlo

#ifndef RESTAN_HMCMC_H
#define RESTAN_HMCMC_H

#include <armadillo>
#include "loss/loss.h"

namespace restan
{
  //! Hamiltonian Markov Monte Carlo
  // u: loss function
  // q0: initial value
  // epsilon: hyperparameter (leapfrog step-size)
  // L: hyperparameter (leapfrog step count)
  // samples: number of samples
  // samplesOut: returns an array of samples
  void HMCMC(restan::Loss (*u)(arma::vec), const arma::vec q0, double epsilon, unsigned int L, unsigned int samples, arma::vec* samplesOut);
}

#endif