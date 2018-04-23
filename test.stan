parameters
{
  real<lower = 2, upper = 4> lambda;
}
model
{
  lambda ~ normal(3.5, 2);
}
