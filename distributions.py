# various loss functions

import numpy as np

def stdnormal(q):
  s = 0
  for i in range(len(q)):
    s += q[i] * q[i]
  return s