#include "GHMCMC.h"
#include "HMCMC.h"
#include "pi/pi.h"

using namespace restan;
using namespace adept;

Pi pi;
void restan::GHMCMC(restan::GradValue (*u)(const adept::Vector&), adept::Vector q0, double epsilon, unsigned int L, unsigned int samples, adept::Vector* samplesOut, unsigned int epoch, unsigned int L2)
{
	while (samples > 0)
	{
		restan::HMCMC(u, q0, epsilon, L, 1, samplesOut);
		//From all the discrete parameters, choose a parameter
		// 	For all possible discrete values
		//  	Compute loss using u (but make sure that continuous parameters don't change)
		//		Sample from -exp( loss )

		samples--;
		samplesOut++;
	}
	
}