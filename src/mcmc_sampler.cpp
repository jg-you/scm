// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Author: Jean-Gabriel Young <info@jgyoung.ca>
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Simplicial Configuration Model MCMC sampler
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// STL
#include <cstdlib>   // EXIT_FAILURE, EXIT_SUCCESS
#include <iostream>
#include <fstream> // handle facet_list text file
#include <cmath>   // pow, exp
#include <chrono>  // high_resolution_clock
#include <vector>
#include <random>  // mt19937
#include <algorithm>  // max
// Boost
#include <boost/program_options.hpp>    
#include <boost/math/special_functions/binomial.hpp>
// Program headers
#include "types.h"
#include "scm/scm.h"
#include "io_functions.h"

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
  ("burn_in,b", po::value<unsigned int>(&burn_in),
      "Burn-in time. Defaults to M log M, where M is the sum of degrees.")
  ("sampling_steps,t", po::value<unsigned int>(&sampling_steps),
      "Number of sampling steps.")
  ("sampling_frequency,f", po::value<unsigned int>(&sampling_frequency),
      "Number of step between each sample. Defaults to M log M, where M is the sum of degrees.")
  ("seed,d", po::value<unsigned int>(&seed),
      "Seed of the pseudo random number generator (Mersenne-twister 19937). Seed with time if not specified.")
  ("l_max,l", po::value<unsigned int>(&L_max),
      "Manually set L_max. The correctness of the sampler is not guaranteed if L_max < 2 max s. Defaults to 10% of the sum of facet sizes. ")
  ("exp_prop", "Use exponential proposal distribution.")
  ("pl_prop", "Use power law proposal distribution.")
  ("unif_prop", "Use uniform proposal distribution [default].")
  ("prop_param", po::value<float>(&prop_param),
      "Parameter of the proposal distribution (only works for the exponential and power law proposal distributions).")
  ("cleansed_input,c", "Assume that the input is already cleansed, i.e., that nodes are labeled with 0 indexed contiguous integers and that no facet is included in another.")
  ("verbose,v", "Output log messages.")
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
  if (var_map.count("help") || argc == 1)
  {
      std::cout << "Usage:\n"
                << "  "+std::string(argv[0])+" [--option_1=VAL] ... [--option_n=VAL] path-to-facet-list\n";
      std::cout << description;
      return EXIT_SUCCESS;
  }
  if (!var_map.count("facet_list_path"))
  {
      std::cerr << "No facet list given.\n";
      return EXIT_FAILURE;
  }
  if (!var_map.count("seed")) {
      seed = (unsigned int) std::chrono::high_resolution_clock::now().time_since_epoch().count();
  }



  /* ~~~~~ Load max. facets ~~~~~~~*/
  if (var_map.count("verbose")) std::clog << "Loading facet file.\n";
  adj_list_t maximal_facets;
  vmap_t id_to_vertex;
  std::ifstream file(facet_list_path.c_str());
  if (!file.is_open()) return EXIT_FAILURE;
  unsigned int largest_facet = read_facet_list(maximal_facets, file, var_map.count("cleansed_input") != 0, id_to_vertex);
  file.close();



  /* ~~~~~ Sampling ~~~~~~~*/
  scm_t K(maximal_facets);
  std::mt19937 engine(seed);
  // prepare proposal distribution
  if (!var_map.count("l_max")) 
  {
    L_max = std::min(std::max((unsigned int) 0.1 * K.M(), 2 * largest_facet), K.M());
  }
  if (L_max < 2 * largest_facet && var_map.count("l_max"))
  {
    std::clog << "Warning: Manually set L_max does not guarantee connectivity. ("<< L_max << " < " << 2 * largest_facet << ")\n";
  }
  std::vector<double> weights(L_max + 1, 0);
  if (var_map.count("exp_prop"))
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


  if (!var_map.count("sampling_frequency"))
  {
    sampling_frequency = (unsigned int) K.M() * std::log(K.M());
  }
  if (!var_map.count("burn_in"))
  {
    burn_in = (unsigned int) K.M() * std::log(K.M());
  }
  // finally ready to output params (need initialized proposal for that)
  if (var_map.count("verbose"))
  {
    std::clog << "Parameters:\n";
    std::clog << "\tfacet_list_path: " << facet_list_path << "\n";
    std::clog << "\tburn_in: " << burn_in << "\n";
    std::clog << "\tsampling_steps: " << sampling_steps << "\n";
    std::clog << "\tsampling_frequency: " << sampling_frequency << "\n";
    std::clog << "\tseed: " << seed << "\n";
    std::clog << "\tL_max: " << L_max << "\n";
    std::clog << "\tproposal_distribution: ";
    if (var_map.count("exp_prop")) {std::clog << "exponential\n";}
    else if (var_map.count("pl_prop")) {std::clog << "power law\n";}
    else {std::clog << "uniform\n";}
    std::clog << "\tprop_param: " << prop_param << "\n";
    std::clog << "\tcleansed_input: ";
    if (var_map.count("cleansed_input")) {std::clog << "yes\n";}
    else {std::clog << " no\n";}
  }
  // Burn-in
  if (var_map.count("verbose")) std::clog << "Burn-in in progress\n";
  for (unsigned int t = 0; t < burn_in;)
  {
    unsigned int l = rand_int(engine);
    auto moves = K.random_rewire(l, engine);
    if (K.do_moves(moves)) ++t;
  }
  // Sample
  if (var_map.count("verbose")) std::clog << "Starting sampling\n";  
  unsigned int accepted = 0;
  for (unsigned int t = 1; t < sampling_steps * sampling_frequency + 1; ++t)
  {
    unsigned int l = rand_int(engine);
    auto moves = K.random_rewire(l, engine);
    if (K.do_moves(moves))
    {
      ++accepted;
    } 
    if (t % sampling_frequency == 0)
    {
      output_K(K, std::cout, id_to_vertex);
    }
  }
  float acceptance_ratio = float(accepted) / float(sampling_steps * sampling_frequency);
  if (var_map.count("verbose"))
  {
    std::clog << "# acceptance_ratio=" << acceptance_ratio << "\n";
    std::clog << "Done.\n";
  }

  return EXIT_SUCCESS;
}
