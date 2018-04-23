#include "utils.h"

void printVector (std::vector<double> vec)
{
	for (double d : vec)
	{
		std::cout << d << " ";
	}
	std::cout << std::endl;
}