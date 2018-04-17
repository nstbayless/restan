# Re-Stan

*A stan interpreter*

## Setup

### Dependencies

Download and install (in this order):
 - openBLAS (>= 0.2.20 stable). [Website](https://www.openblas.net/), [GitHub](https://github.com/xianyi/OpenBLAS).
   If compiling from GitHub, be sure to switch to the `master` branch (which is not the default), and make with
   ```
   make -j 8
   make install
   ```
   `make -j 8` parallelizes over 8 processors (use whatever value you find appropriate).
   You'll also need to configure the install to `/usr/lib` on Ubuntu.
 - adept 2 (>=2.0.5) from [here](http://www.met.reading.ac.uk/clouds/adept/download.html)
   - configure with `./configure --with-blas=openblas`

### Compilation

Once the dependencies above are installed, compile with:

```
cmake .
make
```

## Examples

Once compiled, run `restan` to generate gaussian noise