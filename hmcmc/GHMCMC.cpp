#include "GHMCMC.h"
#include "HMCMC.h"
#include "pi/pi.h"


using namespace restan;
using namespace adept;
using namespace std;

default_random_engine _generator;
uniform_real_distribution<double> _stdunif(0, 1.0);

int SampleDiscrete(double* unnormalizedProb, unsigned int size, double Z)
{
	//Random number between 0 and Z
	double sampledDouble = _stdunif(_generator) * Z;

	for (int i = 0; i < size; i++) {
		sampledDouble -= unnormalizedProb[i];
		if (sampledDouble < 0)
			return i;
	}
	std::cout << "Sampling Error!" << std::endl;
	return -1;
}

void restan::GHMCMC(restan::GradValue (*u)(const adept::Vector&), adept::Vector q0, double epsilon, unsigned int L, unsigned int numSamples, std::vector<double>* samplesOut, unsigned int epoch, unsigned int L2)
{

	while (numSamples > 0)
	{

		//Run one sample of HMCMC to update continous parameters
		restan::HMCMC(u, q0, epsilon, L, 1, samplesOut);


		//Update one discrete parameter
		unsigned int numParams = pi.numParams;
		unsigned int discreteIndexStart = pi.discreteIndexStart;
		unsigned int numDiscrete = numParams - discreteIndexStart;

		if (numDiscrete != 0)
		{
			// gibbs sampling
			unsigned int randDiscreteIndex = (rand() % numDiscrete);
			unsigned int randDiscreteParamIndex = randDiscreteIndex + discreteIndexStart;

			unsigned int* domainSizes = pi.discreteDomainLengths;
			unsigned int randDiscreteDomainSize = domainSizes[randDiscreteIndex];



			//Update one discrete parameter
			//std::cout << "Rand Discrete Index: " << randDiscreteIndex << " Domain Size: " << randDiscreteDomainSize << std::endl;
			Vector currentParams(q0);
			//Make Samples include discrete variables
			adept::aVector aParams = pi.getParams();
			for (int i = 0; i < aParams.size(); i++)
			{
				currentParams[i] = aParams[i].value();
			}
			double unnormalizedProb[randDiscreteDomainSize];
			double Z = 0;
			for (int i = 0; i < randDiscreteDomainSize; i++) {
				pi.setParam(randDiscreteParamIndex, i);

				unnormalizedProb[i] = exp(-u(currentParams).first);
				Z += unnormalizedProb[i];
				//std::cout << "unnormalizedProb:[ " << i << "] : " << unnormalizedProb[i] << std::endl;
			}
			unsigned int newDiscreteDomainIndex = SampleDiscrete(unnormalizedProb, randDiscreteDomainSize, Z);
			//std::cout <<"newDiscreteDomainIndex: " << newDiscreteDomainIndex << std::endl;

			//Sample and update discrete parameter
			pi.setParam(randDiscreteParamIndex, newDiscreteDomainIndex);
		}

		//TODO:: Change to pi.output
		//Retransforms constrained (log or log-odds) parameters
		*samplesOut = pi.output();
		//std::cout << "Updated Parameters: " << *samplesOut << std::endl;

		//Generate next sample
		numSamples--;
		samplesOut++;
	}
}
