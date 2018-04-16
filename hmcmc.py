# Hamiltonian Markov Monte Carlo algorithm

from autograd import grad
import autograd.numpy as np
import numpy.random

# performs hamiltonian markov monte carlo.
# u: fn ([reals]) -> real, the unnormalized negative
#    log probability distribution. (potential energy)
# init: a vector of reals to initialize
# sampleCount: number of samples to draw
# epsilon: a hyperparameter
# L: a hyperparameter
# returns: a list of sample points.
def HMCMC(u, init, epsilon, L, sampleCount):
  # state
  q = np.array(init, dtype = np.float)
  
  # dimension of state-space
  d = len(q);
  
  # list of samples
  samples = []
  
  # generate samples (TODO: warmup phase)
  while len(samples) < sampleCount:
    sample = HMCMCUpdate(u, q.copy(), d, epsilon, L)
    if (sample != None).any():
      samples.append(sample)
      
      # update position
      q = sample
  
  return samples
  
  
# Attempts to generate a single sample for HMCMC
# may reject, in which case None is returned.
def HMCMCUpdate(u, qStart, d, epsilon, L):
  # momentum (TODO: confirm this sampling method)
  p = numpy.random.randn(d)
  q = qStart
  gradu = grad(u)
  
  # leapfrog steps: (TODO: confirm this? seems to only differ on first and last step...)
  p -= gradu(q) * epsilon / 2
  for i in range(1, L - 2):
    q += p * epsilon
    p -= gradu(q) * epsilon
  q += p/2
  
  return q