parameters
{
  real lambda;
  real beta;
}
transformed parameters
{
  real zeta <- beta * 3;
  real alpha <- zeta + 4;
}
model
{
  lambda ~ normal(0, 1);
  alpha ~ normal(55 + lambda, 1);
}
