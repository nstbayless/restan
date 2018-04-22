
      parameters
      {
        real lambda;
        real beta;
      }
      transformed parameters
      {
        real zeta <- beta;
        real alpha <- beta;
      }
      model
      {
        lambda ~ normal(0, 1);
        alpha ~ normal(5 * lambda, 1)
      }
