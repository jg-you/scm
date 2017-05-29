// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Author: Jean-Gabriel Young <info@jgyoung.ca>
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Simplicial Configuration Model sampler (rejection method)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// STL
#include <cstdlib>   // EXIT_FAILURE, EXIT_SUCCESS

#include <iostream>
#include <fstream>
#include <sstream>

#include <chrono>
#include <vector>
#include <utility>
#include <random>
#include <string>
// Boost
#include <boost/program_options.hpp>    
// Program headers
#include "types.h"
#include "scm/scm.h"

namespace po = boost::program_options;

int main(int argc, char const *argv[])
{
  /* ~~~~~ Program options ~~~~~~~*/
  std::string facet_list_path;
  std::string separator;
  unsigned int num_samples;
  unsigned int seed;

  po::options_description description("Options");
  description.add_options()
  ("num_samples,n", po::value<unsigned int>(&num_samples)->default_value(1),
      "Number of samples.")
  ("separator,s", po::value<std::string>(&separator)->default_value("#######"), 
      "Output separator.")
  ("seed,d", po::value<unsigned int>(&seed),
      "Seed of the pseudo random number generator (Mersenne-twister 19937). Seed with time if not specified.")
  ("verbose,v", "Output log messages.")
  ("help,h", "Produce help message.")
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
  if (var_map.count("verbose")) {std::clog << "Loading facet file.\n";}
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
    }
    file.close();
  }
  // Construct everything
  scm_t K(maximal_facets);
  std::mt19937 engine(seed);
  rejection_sampler_t* sampler;
  if (var_map.count("verbose"))
  {
    std::clog << " F = " << K.F() << "\n";
    std::clog << " N = " << K.N() << "\n";
    std::clog << " M = " << K.M() << "\n";
  }

  for (unsigned int i = 0; i < num_samples; ++i)
  {
    sampler -> randomize(K, engine);
    // Output 
    for (id_t f = 0; f < K.F(); ++f)
    {
      for (auto v: K.facet_neighbors(f))
      {
        std::cout << v << " ";
      }
      std::cout << "\n";
    }
    std::cout << separator + "\n";
  }
  return EXIT_SUCCESS;
}
