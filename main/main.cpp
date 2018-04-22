// example

#include <iostream>
#include <fstream>
#include <sstream>

#include <adept.h>
#include <adept_arrays.h>

#include "parser/parser.h"
#include "hmcmc/HMCMC.h"

const int N_SAMPLES = 8000;

int main(int argc, char** args)
{
  std::string filename = "test.stan";
  //std::string testProgram;
  std::ifstream input(filename);
  std::stringstream sstr;

  //input.open(filename, std::ifstream::in);

  std::string testProgram( (std::istreambuf_iterator<char>(input) ),
                       (std::istreambuf_iterator<char>()    ) );

  std::cout << testProgram << std::endl;

  try
  {
    restan::parseStan(testProgram);
  }
  catch (restan::ParseError& e)
  {
    std::cerr << "PARSE ERROR" << std::endl;
    std::cerr << e.what() << std::endl;
    return 0;
  }
  adept::Vector q0 = {10};
  adept::Vector samples[N_SAMPLES];
  restan::pi.setVariables(q0);
  restan::HMCMC(restan::getLoss, q0, 0.1, 25, N_SAMPLES, samples);
  for (int i = 0; i < 40; i++)
    std::cout<<samples[i]<<std::endl;
  restan::parseStanCleanup();
  return 0;
}

