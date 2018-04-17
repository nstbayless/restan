// loss functions

#ifndef RESTAN_LOSS_H
#define RESTAN_LOSS_H

#include <armadillo>

namespace restan
{
  struct Loss
  {
    double value;
    arma::vec gradient;
    
    Loss(double v, arma::vec g): value(v), gradient(g)
    { }
  };
}

#endif