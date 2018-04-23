// example

#include <iostream>
#include <fstream>
#include <sstream>

#include <adept.h>
#include <adept_arrays.h>

#include "utils/utils.h"
#include "parser/parser.h"
#include "hmcmc/GHMCMC.h"
#include "data/parseData.h"

const int N_SAMPLES = 70;

int main(int argc, char** args)
{
  std::string filename = args[1];

  std::ifstream input(filename);


  //input.open(filename, std::ifstream::in);

  std::string testProgram( (std::istreambuf_iterator<char>(input) ),
                       (std::istreambuf_iterator<char>()    ) );

  std::cout << testProgram << std::endl;

  try
  {
    restan::parseStan(testProgram);
    std::cout<<"\n";
    restan::pi.getLossStatement()->print();
    std::cout<<"\n";
  }
  catch (restan::ParseError& e)
  {
    std::cerr << "PARSE ERROR" << std::endl;
    std::cerr << e.what() << std::endl;
    return 0;
  }
  adept::Vector q0(restan::pi.numParams);
  for (int i = 0; i < restan::pi.numParams; i++)
    q0(i) = 0;
  adept::Vector v0(restan::pi.numVariables);
  for (int i = 0; i < restan::pi.numVariables; i++)
    v0(i) = 0;
  std::vector<double> samples[N_SAMPLES];
  restan::pi.setVariables(v0);
  if (restan::pi.numObservedData > 0)
  {
    std::cout << "Reading data" << std::endl;
    if (restan::parseData(args[2]))
      return 2;
  }
  std::cout << "Beginning GHMCMC" << std::endl;
  restan::GHMCMC(restan::getLoss, q0, 0.01, 80, N_SAMPLES, samples, 1, 1);
  for (int i = 0; i < 50; i++)
    printVector(samples[i]);
  restan::parseStanCleanup();
  return 0;
}
