# sample from a normal distribution

from hmcmc import *
import distributions
from matplotlib import pyplot as plt
import numpy as np

# obtain samples
samples = HMCMC(distributions.stdnormal, [4, 5, 6, 7, 8, 9], 0.01, 25, 200)

# plot samples
lengths = []
projec = []
mu0 = []
mu = []
stv = []
i = 0
for sample in samples:
  lengths.append(np.linalg.norm(sample))
  projec.append(sample[3])
  if i >= 15:
    mu.append(np.mean(projec[15:i+1]))
    stv.append(np.std(projec[15:i+1]))
  else:
    mu.append(numpy.nan)
    stv.append(numpy.nan)
  mu0.append(0)
  i += 1
fir = plt.subplot(211)
fir.plot(lengths, '.b')
fir.set_title("norm of 6-dimensional Gaussian samples")
sec = plt.subplot(212)
sec.set_title("single-axis projection")
for f in range(20):
  sec.plot(f*f/20.0/20.0 * np.array(stv), '#4040ff13')
  sec.plot(-f*f/20.0/20.0 * np.array(stv), '#4040ff13')
sec.plot(mu0, 'y')
sec.plot(mu, 'g')
sec.plot(projec, '.r')
#plt.show()