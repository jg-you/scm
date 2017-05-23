// MCMC base class + implementations
// Author: Jean-Gabriel Young <info@jgyoung.ca>
#ifndef MCMC_H
#define MCMC_H

#include <random>
#include "../scm/scm.h"
#include "../types.h"

class mcmc_t
{
public:
  mcmc_t(unsigned int sampling_frequency, unsigned int sampling_steps)
  {
    _sampling_frequency = sampling_frequency;
    _sampling_steps = sampling_steps;
  }
  void burnin(scm_t& K, std::mt19937 & engine, std::discrete_distribution<> & rand_int, unsigned int T)
  {
    for (unsigned int t = 0; t < T;)
    {
      unsigned int l = rand_int(engine);
      auto moves = K.random_rewire(l, engine);
      if (K.do_moves(moves)) ++t;
    }
  }
  virtual void compute_property(const scm_t& K, std::ostream& os) {return;}
  float run(scm_t& K, std::mt19937 & engine, std::discrete_distribution<> & rand_int, std::ostream& os)
  {
    unsigned int accepted = 0;
    for (unsigned int t = 1; t < _sampling_steps * _sampling_frequency + 1; ++t)
    {
      unsigned int l = rand_int(engine);
      auto moves = K.random_rewire(l, engine);
      if (K.do_moves(moves))
      {
        ++accepted;
      } 
      if (t % _sampling_frequency == 0)
      {
        compute_property(K, os);
      }
    }
    return float(accepted) / float(_sampling_steps * _sampling_frequency);
  }
protected:
  unsigned int _L_max;
  unsigned int _sampling_frequency;
  unsigned int _sampling_steps;
};

class simplicial_complex_generator: public mcmc_t
{
public:
  simplicial_complex_generator(unsigned int sampling_frequency, unsigned int sampling_steps) 
    : mcmc_t(sampling_frequency, sampling_steps) {;}
  void compute_property(const scm_t& K, std::ostream& os)
  {
    for (id_t f = 0; f < K.F(); ++f)
    {
      for (auto v: K.facet_neighbors(f))
      {
        os << v << " ";
      }
      os << std::endl;
    }
    os << "#################################" << std::endl;
  }
};


#endif // MCMC_H
