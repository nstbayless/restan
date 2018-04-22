#ifndef TEST_H
#define TEST_H

#include <iostream>
#include "catch.hpp"
#include <adept.h>
#include <adept_arrays.h>
#include "pi/pi.h"
#include "pi/expressionTypes.h"
#include "pi/statementTypes.h"
#include "pi/distributions.h"


using namespace restan;
using namespace adept;

/*
	Helper function to compare two, two dimensional matrixes
*/
bool compareEV(ExpressionValue A, ExpressionValue B);
#endif