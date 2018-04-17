// example

#include <iostream>

#include <adept.h>
#include <adept_arrays.h>

#include "hmcmc/HMCMC.h"

using namespace adept;

adept::Stack stack;

restan::GradValue normal(const adept::Vector& _q)
{
  aVector q(_q);
  Vector e3 = {0, 0, 0, 1, 0, 0};
  aReal u = 0;
  stack.new_recording();
  
  u = dot_product(q,q)/2.0;
  
  // set independent variable:
  u.set_gradient(1.0);
  stack.compute_adjoint();
  Vector grad = q.get_gradient();
  
  return restan::GradValue(u.value(), grad);
}

int main(int argc, char** args)
{
  Vector q0 = {10, 13, 7, 4, 2, 9};
  Vector samples[8000];
  restan::HMCMC(normal, q0, 0.1, 25, 8000, samples);
  for (int i = 0; i < 40; i++)
    std::cout<<samples[i]<<std::endl;
  return 0;
}