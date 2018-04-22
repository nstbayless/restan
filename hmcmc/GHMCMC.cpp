#include "GHMCMC.h"
#include "HMCMC.h"
#include "pi/pi.h"


using namespace restan;
using namespace adept;
using namespace std;

int SampleDiscrete(double* unnormalizedProb, unsigned int size, double Z)
{
	//Random number between 0 and Z
	double sampledDouble = ( ((double) rand()) / (double) RAND_MAX ) * Z;

	for (int i = 0; i < size; i++) {
		sampledDouble -= unnormalizedProb[i];
		if (sampledDouble < 0)
			return i;
	}
	std::cout << "Sampling Error!" << std::endl;
	return -1;
}

void restan::GHMCMC(restan::GradValue (*u)(const adept::Vector&), adept::Vector q0, double epsilon, unsigned int L, unsigned int samples, adept::Vector* samplesOut, unsigned int epoch, unsigned int L2)
{
<<<<<<< HEAD
	while (samples > 0) 
	{
		unsigned int numParams = pi.numParams();
=======
	samples = 1;
	while (samples > 0)
	{
		//Run one sample of HMCMC
		restan::HMCMC(u, q0, epsilon, L, 1, samplesOut);

		//Update one discrete parameter
		unsigned int numParams = pi.numParams;
>>>>>>> bb0bc1d0161afefb57cc93e2aed1ae78e5653af4
		unsigned int discreteIndexStart = pi.discreteIndexStart;
		unsigned int numDiscrete = numParams - discreteIndexStart;
		unsigned int randDiscreteIndex = (rand() % numDiscrete);
		unsigned int randDiscreteParamIndex = randDiscreteIndex + discreteIndexStart;

		unsigned int* domainSizes = pi.discreteDomainLengths;
		unsigned int randDiscreteDomainSize = domainSizes[randDiscreteIndex];
		
		//Run one sample of HMCMC to update continous parameters 
		restan::HMCMC(u, q0, epsilon, L, 1, samplesOut);

		//Update one discrete parameter
		//std::cout << "Rand Discrete Index: " << randDiscreteIndex << " Domain Size: " << randDiscreteDomainSize << std::endl;

		double unnormalizedProb[randDiscreteDomainSize];
		double Z = 0;
		for (int i = 0; i < randDiscreteDomainSize; i++) {
			pi.setParam(randDiscreteParamIndex, i);
			unnormalizedProb[i] = exp(-u(*samplesOut).first);
			Z += unnormalizedProb[i];
			//std::cout << "unnormalizedProb:[ " << i << "] : " << unnormalizedProb[i] << std::endl;
		}
		unsigned int newDiscreteDomainIndex = SampleDiscrete(unnormalizedProb, randDiscreteDomainSize, Z);
		//std::cout <<"newDiscreteDomainIndex: " << newDiscreteDomainIndex << std::endl;

		//Sample and update discrete parameter
		pi.setParam(randDiscreteParamIndex, newDiscreteDomainIndex);

		//Make Samples include discrete variables
		adept::aVector aParams = pi.getParams();
		Vector v(aParams.size());
		for (int i = 0; i < aParams.size(); i++)
		{
			v[i] = aParams[i].value();
		}
		*samplesOut = v;
		//std::cout << "Updated Parameters: " << *samplesOut << std::endl;

		//Generate next sample
		samples--;
		samplesOut++;
	}
}
