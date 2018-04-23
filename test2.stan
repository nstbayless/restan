parameters
{
  real x; real mu;
}
transformed parameters
{
  real sigma_sq = 0.125;
  real alpha = 0; real gamma_sq = 1.175;
  real zeta <- 2*mu+1;
}
model
{
  x ~ normal(mu, sigma_sq);
  mu ~ normal(alpha, gamma_sq);
}


