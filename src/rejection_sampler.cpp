// Author: Jean-Gabriel Young <info@jgyoung.ca>
// Simplicial Configuration Model sampler (rejection method)
// Reference: https://arxiv.org/abs/1705.10298
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
#include "io_functions.h"

namespace po = boost::program_options;

int main(int argc, char const *argv[])
{
  /* ~~~~~ Program options ~~~~~~~*/
  std::string facet_list_path;
  std::string degree_seq_file;
  std::string size_seq_file;
  unsigned int num_samples;
  unsigned int seed;

  po::options_description description("Options");
  description.add_options()
  ("seed,d", po::value<unsigned int>(&seed),
      "Seed of the pseudo random number generator (Mersenne-twister 19937). Seed with time if not specified.")
  ("cleansed_input,c", "In facet list mode, assume that the input is already cleansed, i.e., that nodes are labeled with 0 indexed contiguous integers and that no facet is included in another.")
  ("degree_seq_file,k", po::value<std::string>(&degree_seq_file),
    "Path to degree sequence file.")
  ("size_seq_file,s", po::value<std::string>(&size_seq_file),
    "Path to size sequence file.")
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
  if (var_map.count("help") || argc == 1)
  {
      std::cout << "Usage:\n"
                << " [Facet list mode] "+std::string(argv[0])+" [--option_1=VAL] ... [--option_n=VAL] path-to-facet-list\n"
                << " [Seq. mode] "+std::string(argv[0])+" [--option_1=VAL] ... -k path-to-degrees.txt -s path-to-sizes.txt\n";
      std::cout << description;
      return EXIT_SUCCESS;
  }
  if (!var_map.count("facet_list_path") && (!var_map.count("degree_seq_file") && !var_map.count("size_seq_file")))
  {
      std::cerr << "Missing facet list or sequences files.\n";
      return EXIT_FAILURE;
  }
  if (!var_map.count("seed")) {
      // seeding based on the clock
      seed = (unsigned int) std::chrono::high_resolution_clock::now().time_since_epoch().count();
  }


  if (var_map.count("facet_list_path"))
  {
    // facet  list mode
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
    if (var_map.count("verbose"))
    {
      unsigned int tries = 1;
      do
      {
        K.shuffle(engine);
        std::clog << "\rnum_tries: " << tries;
        ++tries;
      } while(!K.is_simplicial_complex());
      std::clog << "\n";
    }
    else
    {
      do
      {
        K.shuffle(engine);
      } while(!K.is_simplicial_complex());      
    }
    output_K(K, std::cout, id_to_vertex);
  }
  else 
  {
    // seq mode
    /* ~~~~~ Load max. facets ~~~~~~~*/
    if (var_map.count("verbose")) std::clog << "Loading sequence files.\n";
    uint_vec_t d;
    uint_vec_t s;
    {
      std::ifstream file(degree_seq_file.c_str());
      read_sequence_file(file, d);
      file.close();
    }
    {
      std::ifstream file(size_seq_file.c_str());
      read_sequence_file(file, s);
      file.close();
    }
    /* ~~~~~ Sampling ~~~~~~~*/
    scm_t K(s, d);
    std::mt19937 engine(seed);
    if (var_map.count("verbose"))
    {
      unsigned int tries = 1;
      do
      {
        K.shuffle(engine);
        std::clog << "\rnum_tries: " << tries;
        ++tries;
      } while(!K.is_simplicial_complex());
      std::clog << "\n";
    }
    else
    {
      do
      {
        K.shuffle(engine);
      } while(!K.is_simplicial_complex());      
    }
    output_K(K, std::cout);
  }
  return EXIT_SUCCESS;
}
