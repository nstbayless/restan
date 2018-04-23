parameters
{
  real<lower = 0> lambda;
  int<lower = 0, upper = 4> z;
  int<lower = 0, upper = 8> zb;
}
model
{
  lambda ~ normal(1, 0.2);
  z ~ poisson(lambda);
}
