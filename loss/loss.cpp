#include "loss.h"
#include "lossfunctions.h"

using namespace restan;

Loss restan::stdnormal(arma::vec q)
{
  double v = dot(q,q)/2;
  return Loss(v, q);
}