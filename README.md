# Re-Stan

*A re-implementation of Stan*

## Examples

### Sample from a Gaussian distribution:

```
from hmcmc import *
import distributions
from matplotlib import pyplot as plt
samples = HMCMC(distributions.stdnormal, [4, 5, 6, 7, 8, 9], 0.01, 30, 500)
plt.subplot()
plt.plot(samples, '.b')
plt.show()
```