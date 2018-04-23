// example

#include <iostream>
#include <fstream>
#include <sstream>

#include <adept.h>
#include <adept_arrays.h>

#include "parser/parser.h"
#include "hmcmc/HMCMC.h"

const int N_SAMPLES = 70;

int main(int argc, char** args)
{
  std::string filename = "test.stan";
  //std::string testProgram;
  std::ifstream input(filename);
  //std::stringstream sstr;

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
  adept::Vector samples[N_SAMPLES];
  restan::pi.setVariables(v0);
  restan::pi.discreteIndexStart = 2;
  std::cout << "Beginning HMCMC" << std::endl;
  restan::HMCMC(restan::getLoss, q0, 0.1, 80, N_SAMPLES, samples);
  for (int i = 0; i < 50; i++)
    std::cout<<samples[i]<<std::endl;
  restan::parseStanCleanup();
  return 0;
}
