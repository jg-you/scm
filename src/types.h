#ifndef TYPES_H
#define TYPES_H

#include <map>
#include <vector>
#include <set>
#include <utility>


typedef std::pair<id_t, id_t> edge_t;
typedef std::vector<edge_t> edge_list_t;
typedef std::multiset<id_t> neighborhood_t;
typedef std::vector<neighborhood_t> adj_list_t;

typedef struct mcmc_move_t
{
  id_t vertex;
  id_t facet;
  bool attach;
} mcmc_move_t;


typedef std::vector<unsigned int> uint_vec_t;
typedef std::vector<int> int_vec_t;
typedef std::vector<float> float_vec_t;
typedef std::vector< std::vector<unsigned int> > uint_mat_t;
typedef std::vector< std::vector<int> > int_mat_t;
typedef std::vector< std::vector<float> > float_mat_t;
typedef std::map<id_t, std::string> vmap_t;
#endif // TYPES_H
