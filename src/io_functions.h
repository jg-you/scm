// Author: Jean-Gabriel Young <info@jgyoung.ca>
#ifndef IO_FUNCTIONS
#define IO_FUNCTIONS

#include <iostream>
#include <fstream> // handle facet_list text file
#include <string>
#include <sstream> // handle facet_list text file
#include <set>
#include <map>
#include <algorithm>
#include "types.h"
#include "scm/scm.h"


void output_K(const scm_t& K, std::ostream& os, const  vmap_t & id_to_vertex)
{
  os << "# Sample:" << std::endl;
  if (id_to_vertex.size() == 0)
  {
    for (id_t f = 0; f < K.F(); ++f)
    {
      for (auto v: K.facet_neighbors(f))
      {
        os << v << " ";
      }
      os << std::endl;
    }
  }
  else
  { 
    for (id_t f = 0; f < K.F(); ++f)
    {
      for (auto v: K.facet_neighbors(f))
      {
        os << id_to_vertex.at(v) << " ";
      }
      os << std::endl;
    }
  }
}
void output_K(const scm_t& K, std::ostream& os)
{
  os << "# Sample:" << std::endl;
  for (id_t f = 0; f < K.F(); ++f)
  {
    for (auto v: K.facet_neighbors(f))
    {
      os << v << " ";
    }
    os << std::endl;
  }
}

unsigned int read_facet_list(adj_list_t & maximal_facets, std::ifstream& file, bool cleansed_input, vmap_t & id_to_vertex)
{
  std::string line_buffer;
  id_t v = 0;
  unsigned int largest_facet = 0;
  if (!cleansed_input)
  {
    // read facet list and setup map
    std::map<std::string, id_t> vertex_to_id; // potentially useless, declase in case the input has to be sanetized
    while (getline(file, line_buffer))
    {
      std::string vertex;
      std::stringstream ls(line_buffer);
      if (!line_buffer.empty())
      {
        neighborhood_t neighborhood;
        while (ls >> vertex)
        {
          auto id = vertex_to_id.find(vertex);
          if (id == vertex_to_id.end())
          {
            vertex_to_id[vertex] = v;
            id_to_vertex[v] = vertex;
            neighborhood.insert(v);
            ++v;
          }
          else
          {
            neighborhood.insert(id->second);
          }
        }
        maximal_facets.push_back(neighborhood);
        if (neighborhood.size() > largest_facet) largest_facet = neighborhood.size();
      }
    }
    vertex_to_id.clear();  // only needed to setup id_to_vertex faster.
    // prune
    unsigned int original_size = maximal_facets.size();
    std::set<unsigned int> sizes;
    for (auto f: maximal_facets) sizes.insert(f.size());
    std::map<unsigned int, adj_list_t> facet_by_size;
    for (auto f: maximal_facets)
    {
      facet_by_size[f.size()].push_back(f);
    }
    // remove repetitions
    for (auto s: facet_by_size)
    {
      std::set<neighborhood_t> tmp(facet_by_size[s.first].begin(), facet_by_size[s.first].end());
      facet_by_size[s.first].clear();
      facet_by_size[s.first] = adj_list_t(tmp.begin(), tmp.end());
    }
    // remopve included facets
    auto ref_size_it = sizes.end();
    for (--ref_size_it; ref_size_it != sizes.begin(); --ref_size_it) // starts pass back()
    {
      for (auto ref_set: facet_by_size[*ref_size_it])
      {
        for (auto s: sizes)
        {
          if (s < *ref_size_it)
          {
            adj_list_t tmp;
            tmp.reserve(facet_by_size[s].size());
            for (auto f: facet_by_size[s])
            {
              if (!std::includes(ref_set.begin(), ref_set.end(), f.begin(), f.end())) tmp.push_back(f);
            }
            facet_by_size[s] = tmp;
          }
        }
      }
    }
    // put in memory
    maximal_facets.clear();
    for (auto s: facet_by_size)
    {
      maximal_facets.insert(maximal_facets.end(), s.second.begin(), s.second.end());
    }
  }
  else
  {
    while (getline(file, line_buffer))
    {
      std::stringstream ls(line_buffer);
      neighborhood_t neighborhood;
      while (ls >> v) {
        neighborhood.insert(v);
      }
      maximal_facets.push_back(neighborhood);
      if (neighborhood.size() > largest_facet) largest_facet = neighborhood.size();
    }
  }
  return largest_facet; // weird return.. but speed up things a bit
}

void read_sequence_file(std::ifstream& file, uint_vec_t & seq)
{
  seq.clear();
  std::string line_buffer;
  getline(file, line_buffer);
  std::stringstream ls(line_buffer);
  unsigned int x;
  while (ls >> x)
  {
    seq.push_back(x);
  }
}

#endif