// Author: Jean-Gabriel Young <info@jgyoung.ca>
// Simplicial configuration model class implementation
// Reference: https://doi.org/10.1103/PhysRevE.96.032312
// arXiv link:  https://arxiv.org/abs/1705.10298
#include "scm.h"


//***************************************
// CONSTRUCTORS
//***************************************

// Constructor from a maximal facet list
scm_t::scm_t(const adj_list_t & maximal_facets)
  :
  rand_real_(0, 1)
{
  // number of facets is known
  F_ = maximal_facets.size();
  // determine number of vertices and matching
  M_ = 0;
  std::set<unsigned int> vertices;
  for (neighborhood_t f : maximal_facets)
  {
      M_ += f.size();
      vertices.insert(f.begin(), f.end());
  }
  N_ = vertices.size();
  vertices.clear();
  // Load
  facet_neighbors_.resize(F_);
  vertex_neighbors_.resize(N_);
  for (unsigned int f = 0; f < maximal_facets.size() ; ++f)
    for (unsigned int v :  maximal_facets[f])
      connect(f, v);
}

// Constructor from size and degree sequences
scm_t::scm_t(const uint_vec_t & s, const uint_vec_t & d)
  :
  rand_real_(0, 1)
{
  F_ = s.size();
  N_ = d.size();
  // number of matchings
  M_ = 0;
  for (unsigned int i : s)
    M_ += i;
  facet_neighbors_.resize(F_);
  vertex_neighbors_.resize(N_);
  for (unsigned int m = 0, f = 0, v = 0, nf(s[0]), nv(d[0]); m < M_; ++ m)
  {
    // loop over matchings (m) and match facets (f) and vertices (v)
    connect(f, v);
    --nf;
    --nv;
    if (nf == 0) {++f; nf = s[f];}
    if (nv == 0) {++v; nv = d[v];}
  }
}

bool scm_t::is_simplicial_complex() const
{
  return !has_inclusions() && !has_multiedges();
}

bool scm_t::has_multiedges() const
{
  for (id_t f = 0; f < F_; ++f)
  {
    std::set<id_t> tmp(facet_neighbors_[f].begin(), facet_neighbors_[f].end());
    if (facet_neighbors_[f].size() > tmp.size())
      return true;
  }
  return false;
}

bool scm_t::has_inclusions() const
{
  for (id_t f = 0; f < F_; ++f)
  {
    // inclusion test
    if (all_inclusions_of(f).size() > 0)
    {
      return true;
    }
  }
  return false;
}

bool scm_t::included_in(id_t facet_a, id_t facet_b) const
{
  // For X to be NOT included in Y means that X is incident on at least one
  // vertex not in the vertex set of Y. 
  id_t smallest_facet = facet_a;
  if (facet_neighbors_[facet_a].size() < facet_neighbors_[facet_b].size())
    smallest_facet = facet_b;
  std::multiset<id_t> tmp;
  std::set_intersection(facet_neighbors_[facet_a].begin(), facet_neighbors_[facet_a].end(),
                        facet_neighbors_[facet_b].begin(), facet_neighbors_[facet_b].end(),
                        std::inserter(tmp, tmp.begin()));
  return tmp == facet_neighbors_[smallest_facet];
}

neighborhood_t scm_t::all_inclusions_of(id_t facet) const
{
  // Get all the facets in which a facet is included.
  // X is included in Y means if the vertices of X
  // are all connected to a facet Y != X.
  auto v = facet_neighbors_[facet].begin();
  neighborhood_t candidates = vertex_neighbors_[*v];
  candidates.erase(facet);
  for (++v; v != facet_neighbors_[facet].end(); ++v)
  {
    std::multiset<id_t> tmp;
    std::set_intersection(candidates.begin(), candidates.end(),
                          vertex_neighbors_[*v].begin(), vertex_neighbors_[*v].end(),
                          std::inserter(tmp, tmp.begin()));
    candidates = tmp;
    if (candidates.size() == 0)
      return candidates;
  }
  return candidates;
}

/// MCMC UTILITIES
edge_list_t scm_t::get_random_edges(unsigned int l, std::mt19937& engine)
{
  // optimized for small number of edges vs. total number of edges
  // otherwise there is a lot of redraws
  std::set<edge_t> edgeset;
  do
  {
    edge_t e;
    e.first = preferential_pick(vertex_neighbors_, engine);
    e.second = uniform_pick(vertex_neighbors_[e.first], engine);
    edgeset.insert(e);
  } while (edgeset.size() < l);
  return edge_list_t(edgeset.begin(), edgeset.end());
}
edge_list_t scm_t::rewired_edge_list(edge_list_t edgelist, std::mt19937& engine)
{
  edge_list_t new_edgelist(edgelist.begin(), edgelist.end());
  std::shuffle(new_edgelist.begin(), new_edgelist.end(), engine);
  for (unsigned int i = 0; i < edgelist.size(); ++i) 
  {
    new_edgelist[i].first = edgelist[i].first;
  }
  return new_edgelist;
}
std::vector<mcmc_move_t> scm_t::random_rewire(unsigned int l, std::mt19937& engine)
{
  std::vector<mcmc_move_t> moves(2 * l);
  edge_list_t edges_to_detach = get_random_edges(l, engine);
  edge_list_t edges_to_attach = rewired_edge_list(edges_to_detach, engine);
  for (unsigned int i = 0; i < l; ++i)
  {
    moves[i].attach = false;
    moves[i].vertex = edges_to_detach[i].first;
    moves[i].facet = edges_to_detach[i].second;
    moves[i + l].attach = true;
    moves[i + l].vertex = edges_to_attach[i].first;
    moves[i + l].facet = edges_to_attach[i].second;
  }
  return moves;
}
void scm_t::apply_mcmc_moves(std::vector<mcmc_move_t> moves)
{
  for (mcmc_move_t move : moves)
  {
    if (move.attach) connect(move.facet, move.vertex);
    else disconnect(move.facet, move.vertex);
  }
  return;
}
void scm_t::revert_mcmc_moves(std::vector<mcmc_move_t> moves)
{
  for (mcmc_move_t move : moves)
  {
    if (!move.attach) connect(move.facet, move.vertex);
    else disconnect(move.facet, move.vertex);
  }
  return;
}
bool scm_t::is_the_difference(const neighborhood_t & facet_a, const neighborhood_t & facet_b, std::multiset<id_t> difference) const
{
  std::multiset<id_t> tmp;
  std::set_difference(facet_a.begin(), facet_a.end(), facet_b.begin(), facet_b.end(), std::inserter(tmp, tmp.begin()));
  return tmp == difference;
}
bool scm_t::do_moves(std::vector<mcmc_move_t> moves)
{
  // First apply the move, then verify if it preserves s.
  // if not, revert them ove and return false.
  // if it is, leave the complex as is, and return true.
  apply_mcmc_moves(moves);
  // Check for s-conservation
  std::set<id_t> facets_to_check;
  for (mcmc_move_t m: moves)
  {
    facets_to_check.insert(m.facet);
    for (id_t f: vertex_neighbors_[m.vertex])
    {
      facets_to_check.insert(f);
    }
  }

  for (id_t f: facets_to_check)
  {
    // Test for multi-memberships and inclusion
    std::set<id_t> tmp(facet_neighbors_[f].begin(), facet_neighbors_[f].end());
    // Important:
    // The left operand first is evaluated first, and the right operand is only evaluted
    // if the first one is false;  all_inclusions_of is much more expensive than the first test.
    if (facet_neighbors_[f].size() != tmp.size() || all_inclusions_of(f).size() > 0)
    {
      revert_mcmc_moves(moves);
      return false;
    }
  }
  return true;
}
void scm_t::shuffle(std::mt19937& engine)
{
  // inefficient implementation whereby we construct stub lists,
  // shuffle one, and reconnect everything.
  uint_vec_t facet_stubs(M_, 0);
  uint_vec_t vertex_stubs(M_, 0);
  unsigned int m = 0;
  for (id_t f = 0; f < facet_neighbors_.size(); ++f)
  {
    for (auto neighbor: facet_neighbors_[f])
    {
      facet_stubs[m] = f;
      ++m;
    }
  }
  m = 0;
  for (id_t v = 0; v < vertex_neighbors_.size(); ++v)
  {
    for (auto neighbor: vertex_neighbors_[v])
    {
      vertex_stubs[m] = v;
      ++m;
    }
  }
  disconnect_all();
  std::shuffle(vertex_stubs.begin(), vertex_stubs.end(), engine);
  for (m = 0; m < M_; ++m)
  {
    connect(facet_stubs[m], vertex_stubs[m]);
  }
}

// SET accessors
void scm_t::connect(id_t facet, id_t vertex)
{
  facet_neighbors_[facet].insert(vertex);
  vertex_neighbors_[vertex].insert(facet);
}

void scm_t::disconnect(id_t facet, id_t vertex)
{
  assert(facet_neighbors_[facet].count(vertex) > 0);
  assert(vertex_neighbors_[vertex].count(facet) > 0);
  auto itv = facet_neighbors_[facet].find(vertex);
  auto itf = vertex_neighbors_[vertex].find(facet);
  facet_neighbors_[facet].erase(itv);
  vertex_neighbors_[vertex].erase(itf);
}

void scm_t::disconnect_all()
{
  facet_neighbors_.clear();
  facet_neighbors_.resize(F_);
  vertex_neighbors_.clear();
  vertex_neighbors_.resize(N_);
}

// GET accessors
neighborhood_t scm_t::facet_neighbors(id_t facet) const {return facet_neighbors_[facet];}
neighborhood_t scm_t::vertex_neighbors(id_t vertex) const {return vertex_neighbors_[vertex];}
unsigned int scm_t::size(id_t facet) const {return facet_neighbors_[facet].size();}
unsigned int scm_t::degree(id_t vertex) const {return vertex_neighbors_[vertex].size();}
unsigned int scm_t::F() const {return F_;}
unsigned int scm_t::N() const {return N_;}
unsigned int scm_t::M() const {return M_;}


// RNG-related.
id_t scm_t::preferential_pick(std::vector< std::multiset<id_t> > & vec_of_multiset, std::mt19937& engine) {
  /* declarations */
  id_t pick = 0;
  id_t local_count = 0;
  id_t global_count = 0;
  /* choose target "ticket" (# of ticket for node i prop. to. vec_of_multiset[i].size() */
  id_t target_idx = (id_t) ceil(rand_real_(engine)*(double) M_);
  /* find the node to which the ticket belongs */
  do {
    if (local_count == vec_of_multiset[pick].size()) {
      local_count = 0;
      ++pick;
    }
    else {
      ++global_count;
      ++local_count;
    }
  } while (global_count!=target_idx);
  return pick;
}

id_t scm_t::uniform_pick(const std::multiset<id_t> & a_set, std::mt19937& engine)  {
  // safe, since rand_real_(0,1) excludes 1.
  id_t target_idx = (id_t) floor(rand_real_(engine) * (double) a_set.size());
  auto it = a_set.begin();
  std::advance(it, target_idx);
  return *it;
}
