parameters
{
  real<lower = 1> lambda;
}
model
{
  lambda ~ normal(0, 1);
}
