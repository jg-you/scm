// Author: Jean-Gabriel Young <info@jgyoung.ca>
// Simplicial configuration model class headers
// Reference: https://doi.org/10.1103/PhysRevE.96.032312
// arXiv link:  https://arxiv.org/abs/1705.10298
#ifndef SCM_H
#define SCM_H

#include <iostream>

#include <algorithm>
#include <random>
#include <set>
#include <vector>
#include <cassert>
#include "../types.h"


/** @class scm_t
  * @brief Simplicial configuration model.
  *
  * This class implements the simplicial configuration model (SCM) ensemble.
  * Simplicial complexes from this ensemble have a fixed maximal facet size
  * sequence and a degree sequence. They are maximally random
  * with respect to everything else.
  * 
  * States are internally represented by two adjacency lists, one for vertices
  * and one for maximal facets. We also provide an alternative representation
  * where the states is stored explicitly as two aligned list of stubs,
  * see scm_stub_list_impl.h.
  */
class scm_t {
public:
  /** @name Constructors.
    * Available constructors for the scmt_t class.
    */
  //@{
  /** Constructs from a list of maximal facets.
    * @param[in] <maximal_facets> List of maximal facets, in any ordering.
    */
  scm_t(const adj_list_t & maximal_facets);
  /** Constructs from a maximal facet size and  degree sequence.
    * @param[in] <s> Facet size sequence.
    * @param[in] <d> Degree sequence.
    * @warning Assume that s[i], d[i] > 0 for all i. 
    * @warning The resulting matching will not be sequence-preserving in most cases;
    *          requires some shuffling until a sequence-preserving state is reached.
    */
  scm_t(const uint_vec_t & s, const uint_vec_t & d);
  //@}


  /** @name Simplicial complex operations
    *
    */
  //@{
  bool is_simplicial_complex() const;
  bool has_multiedges() const;
  bool has_inclusions() const;
  bool included_in(id_t facet_a, id_t facet_b) const; 
  neighborhood_t all_inclusions_of(id_t facet) const;
  //@}

  /** @name MCMC utilities
    * Random modifications to instances of the model.
    */
  //@{
  /// Exchange edges
  std::vector<mcmc_move_t> random_rewire(unsigned int l, std::mt19937& engine);
  /// Act on moves
  bool do_moves(std::vector<mcmc_move_t> moves);
  void apply_mcmc_moves(std::vector<mcmc_move_t> moves);
  void revert_mcmc_moves(std::vector<mcmc_move_t> moves);
  /// Get a random matching, not necessarily sequence-preserving.
  void shuffle(std::mt19937& engine);
  //@}

  /** @name Accessors.
    */
  //@{
  // SET accessors
  void connect(id_t facet, id_t vertex);
  void disconnect(id_t facet, id_t vertex);
  void disconnect_all();
  // GET accessors
  neighborhood_t facet_neighbors(id_t facet) const;
  neighborhood_t vertex_neighbors(id_t vertex) const;
  unsigned int size(id_t facet) const;
  unsigned int degree(id_t vertex) const;
  unsigned int F() const;
  unsigned int N() const;
  unsigned int M() const;
  //@}

private:
  /// State variable
  // Structure
  adj_list_t facet_neighbors_;
  adj_list_t vertex_neighbors_;
  // Number of faces, vertices, and matchings
  unsigned int F_;
  unsigned int N_;
  unsigned int M_;
  /// Internal distribution.
  std::uniform_real_distribution<double> rand_real_;
  /// Private functions
  bool is_the_difference(const neighborhood_t & facet_a, const neighborhood_t & facet_b, std::multiset<id_t> difference) const;
  id_t preferential_pick(std::vector< std::multiset<id_t> > & vec_of_multiset, std::mt19937& engine);
  id_t uniform_pick(const std::multiset<id_t> & a_set, std::mt19937& engine);
  edge_list_t get_random_edges(unsigned int l, std::mt19937& engine);
  edge_list_t rewired_edge_list(edge_list_t edgelist, std::mt19937& engine);
};

#endif // SCM_H