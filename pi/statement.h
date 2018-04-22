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
    virtual void print(int depth = 0) const = 0;
  };
}

#endif
