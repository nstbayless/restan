#include "tests/test.h"

bool compareEV(ExpressionValue A, ExpressionValue B)
{
	int num_A_rows = A.dimension(0);
	int num_A_cols = A.dimension(1);
	int num_B_rows = B.dimension(0);
	int num_B_cols = B.dimension(1);

	if ((num_A_rows != num_B_rows) || (num_A_cols != num_B_cols)) {
		printf("\ntesttest.cpp/compareEV Dimensions of A and B do not match! A_row: %d, A_col: %d, B_row: %d, B_col: %d\n",
			num_A_rows, num_A_cols, num_B_rows, num_B_cols);
		return false;
	}

	for (int r = 0; r < num_A_rows; r++) 
		for (int c = 0; c < num_A_cols; c++) {
			if (A(r, c).value() != B(r, c).value()) {
				printf("\ntesttestcpp/compareEV values of A(%d, %d): %f and B(%d, %d): %f differ!\n", r, c, A(r,c).value(), r, c, B(r,c).value());
				return false;
			}
		}
	return true;
}