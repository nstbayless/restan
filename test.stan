data
{
  real x;
}
parameters
{
  int<lower = 0, upper = 100> mu;
}
model
{
  x ~ normal(mu, 0.1);
}
