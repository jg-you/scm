// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Author: Jean-Gabriel Young <info@jgyoung.ca>
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Simplicial Configuration Model MCMC sampler
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// STL
#include <cstdlib>   // EXIT_FAILURE, EXIT_SUCCESS
#include <iostream>
#include <fstream> // handle facet_list text file
#include <sstream> // handle facet_list text file
#include <cmath>   // pow, exp
#include <chrono>  // high_resolution_clock
#include <vector>
#include <random>  // mt19937
#include <string>
#include <algorithm>  // max
// Boost
#include <boost/program_options.hpp>    
#include <boost/math/special_functions/binomial.hpp>
// Program headers
#include "types.h"
#include "scm/scm.h"
#include "sampling/mcmc.h"

namespace po = boost::program_options;

int main(int argc, char const *argv[])
{
  /* ~~~~~ Program options ~~~~~~~*/
  std::string facet_list_path;
  unsigned int burn_in;
  unsigned int sampling_steps;
  unsigned int sampling_frequency;
  unsigned int seed = 0;
  unsigned int L_max = 0;
  float prop_param = 1;

  po::options_description description("Options");
  description.add_options()
  ("burn_in,b", po::value<unsigned int>(&burn_in)->default_value(1000),
      "Burn-in time.")
  ("sampling_steps,t", po::value<unsigned int>(&sampling_steps)->default_value(1000),
      "Number of sampling steps.")
  ("sampling_frequency,f", po::value<unsigned int>(&sampling_frequency)->default_value(10),
      "Number of step between each sample.")
  ("seed,d", po::value<unsigned int>(&seed),
      "Seed of the pseudo random number generator (Mersenne-twister 19937). Seed with time if not specified.")
  ("l_max,l", po::value<unsigned int>(&L_max),
      "Manually set L_max. The correctness of the sampler is not guaranteed if L_max < 2 max s. Defaults to 10% of the sum of facet sizes. ")
  ("exp_prop", "Use exponential proposal distribution.")
  ("pl_prop", "Use power law proposal distribution.")
  ("unif_prop", "Use uniform proposal distribution [default].")
  ("prop_param", po::value<float>(&prop_param),
      "Parameter of the proposal distribution (only works for the exponential and power law proposal distributions).")
  ("verbose,v", "Output log messages.")
  ("sanitized_input,s", "Assume that the input is sanitized: nodes are labeled via 0 index, contiguous integers; no facets is included in another. Saves compuation and storage space.")
  ("help,h", "Produce this help message.")
  ;
  po::options_description hidden;
  hidden.add_options()
  ("facet_list_path", po::value<std::string>(&facet_list_path), 
      "Path to facet list.")
  ;
  po::positional_options_description p;
  p.add("facet_list_path", -1);
  po::options_description all_options;
  all_options.add(description);
  all_options.add(hidden);
  po::variables_map var_map;
  po::store(po::command_line_parser(argc, argv).
          options(all_options).
          positional(p).
          run(),
          var_map);
  po::notify(var_map);
  if (var_map.count("help") > 0 || argc == 1)
  {
      std::cout << "Usage:\n"
                << "  "+std::string(argv[0])+" [--option_1=VAL] ... [--option_n=VAL] path-to-facet-list\n";
      std::cout << description;
      return EXIT_SUCCESS;
  }
  if (var_map.count("facet_list_path") == 0)
  {
      std::cerr << "No facet list given.\n";
      return EXIT_FAILURE;
  }
  if (var_map.count("seed") == 0) {
      // seeding based on the clock
      seed = (unsigned int) std::chrono::high_resolution_clock::now().time_since_epoch().count();
  }

  /* ~~~~~ Load max. facets ~~~~~~~*/
  if (var_map.count("exp_prop") > 0) std::clog << "Loading facet file.\n";
  unsigned int largest_facet = 0;
  adj_list_t maximal_facets;
  { // load facets
    std::ifstream file(facet_list_path.c_str());
    if (!file.is_open()) return EXIT_FAILURE;
    std::string line_buffer;
    unsigned int vertex;
    while (getline(file, line_buffer))
    {
      std::stringstream ls(line_buffer);
      neighborhood_t neighborhood;
      while (ls >> vertex) {
        neighborhood.insert(vertex);
      }
      maximal_facets.push_back(neighborhood);
      if (neighborhood.size() > largest_facet) largest_facet = neighborhood.size();
    }
    file.close();
  }

  /* ~~~~~ Declare sampler ~~~~~~~*/
  scm_t K(maximal_facets);
  std::mt19937 engine(seed);
  simplicial_complex_generator sampler(sampling_frequency, sampling_steps);

  if (var_map.count("l_max") == 0) 
  {
    L_max = std::max((unsigned int) 0.1 * K.M(), 2 * largest_facet);
  }
  if (L_max < 2 * largest_facet)
  {
    std::clog << "Warning: Manually set L_max does not guarantee connectivity. ("<< L_max << " < " << 2 * largest_facet << ")\n";
  }
  std::vector<double> weights(L_max + 1, 0);
  if (var_map.count("exp_prop") > 0)
  {
    for (unsigned int l = 2; l <= L_max; ++l) weights[l] = exp(l * prop_param);
  }
  else if (var_map.count("pl_prop"))
  {
    for (unsigned int l = 2; l <= L_max; ++l) weights[l] = pow(l, -prop_param);
  }
  else 
  { // uniform (default)
    for (unsigned int l = 2; l <= L_max; ++l) weights[l] = 1;
  }
  std::discrete_distribution<> rand_int(weights.begin(), weights.end());

  if (var_map.count("verbose"))
  {
    std::clog << "Parameters:\n";
    std::clog << "\tfacet_list_path: " << facet_list_path << "\n";
    std::clog << "\tsampling_steps: " << sampling_steps << "\n";
    std::clog << "\tsampling_frequency: " << sampling_frequency << "\n";
    std::clog << "\tseed: " << seed << "\n";
    std::clog << "\tL_max: " << L_max << "\n";
    std::clog << "\tproposal_distribution: ";
    if (var_map.count("exp_prop") > 0) {std::clog << "exponential\n";}
    else if (var_map.count("pl_prop") > 0) {std::clog << "power law\n";}
    else {std::clog << "uniform\n";}
    std::clog << "\tprop_param: " << prop_param << "\n";
    std::clog << "\tsanitize: ";
    if (var_map.count("sanitized_input") > 0) {std::clog << "no\n";}
    else {std::clog << " yes\n";}
  }

  // ~~~~~~~~~~~~~~~~ Burn-in ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (var_map.count("verbose")) std::clog << "Burn-in in progress\n";
  sampler.burnin(K, engine, rand_int, burn_in);

  // ~~~~~~~~~~~~~~~~ Smapling ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (var_map.count("verbose")) std::clog << "Starting sampling\n";  
  float acceptance_ratio = sampler.run(K, engine, rand_int, std::cout);
  if (var_map.count("verbose"))
  {
    std::clog << "# acceptance_ratio=" << acceptance_ratio << "\n";
    std::clog << "Done.\n";
  }

  return EXIT_SUCCESS;
}
