// example

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

#include <adept.h>
#include <adept_arrays.h>

#include <cxxopts.hpp>

#include "utils/utils.h"
#include "parser/parser.h"
#include "hmcmc/GHMCMC.h"
#include "data/parseData.h"

int N_SAMPLES = 70;

int main(int argc, char* argv[])
{

  std::string filename;
	std::string datafilename;
	bool print_mean = false;
	bool print_stdev = false;
	bool print_ast = false;

	try {

		cxxopts::Options options(argv[0], " - example command line options");

		std::cout << argv[0];

    options
      .positional_help("[optional args]")
			.show_positional_help();

		options
      .add_options()
      ("p,program", "stan program", cxxopts::value<std::string>(filename), "STAN_PROGRAM")
      ("d,data", "data input", cxxopts::value<std::string>(datafilename), "DATA_CSV")
      ("s,samples", "number of samples", cxxopts::value<int>(N_SAMPLES), "SAMPLES")
      ("m,mean", "print mean", cxxopts::value<bool>(print_mean), "MEAN")
      ("v,stdev", "print stdev", cxxopts::value<bool>(print_stdev), "STDEV")
      ("ast", "print abstract syntax", cxxopts::value<bool>(print_ast), "PRINT_AST")
			;

		options.parse_positional({"program", "samples", "data"});

		auto result = options.parse(argc, argv);

  } catch (const cxxopts::OptionException& e)
  {
		std::cout << "error parsing options: " << e.what() << std::endl;
		exit(1);
	}


	//std::cout << print_mean << print_stdev << std::endl;

  std::ifstream input(filename);
  //input.open(filename, std::ifstream::in);

  std::string testProgram( (std::istreambuf_iterator<char>(input) ),
                       (std::istreambuf_iterator<char>()    ) );

  std::cout << testProgram << std::endl;

  try
  {
    restan::parseStan(testProgram);
    std::cout<<"\n";
		if (print_ast)
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
    if (restan::parseData(datafilename.c_str()))
      return 2;
  }
  std::cout << "Beginning GHMCMC" << std::endl;
  restan::GHMCMC(restan::getLoss, q0, 0.01, 80, N_SAMPLES, samples, 1, 1);
	long double sum = 0; long count = 0;
	long double mean = -1; long double stdev = -1;
	long double sumDiff = 0;
	std::vector<double> data;
  for (int i = 0; i < N_SAMPLES; i++) {
    printVector(samples[i]);
		data.push_back(samples[i][0]);
    sum += data[i];
		count++;
	}
	mean = sum / count;
  for (int i = 0; i < N_SAMPLES; i++) {
		sumDiff += (data[i]-mean)*(data[i]-mean);
	}
	stdev = sumDiff / (count-1);

	if (print_mean)
		std::cout << "Mean: " << mean << std::endl;
	if (print_stdev)
		std::cout << "Standard Deviation: " << stdev << std::endl;

  restan::parseStanCleanup();
  return 0;
}
