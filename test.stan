
      __BEGIN_STAN_CODE__
      parameters
      {
        real lambda;
      }
      model
      {
        lambda ~ normal(0, 1);
      }
      __END_STAN_CODE__
