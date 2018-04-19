#ifndef RESTAN_STATEMENT_H
#define RESTAN_STATEMENT_H

#include <adept.h>
#include <adept_arrays.h>

namespace restan
{
  class Statement
  {
  public:
    virtual void execute() = 0;
  };
}

#endif