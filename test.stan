parameters
{
  real lambda;
  real beta;
}
transformed parameters
{
  real zeta <- beta;
  real alpha <- beta;
  real gamma <- beta;
  real rl <- beta;
}
model
{
  lambda ~ normal(0, 1);
  beta ~ normal(5*lambda, 1);
}
