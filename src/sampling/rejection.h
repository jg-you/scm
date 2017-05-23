// Rejection sampler base class + implementations
// Author: Jean-Gabriel Young <info@jgyoung.ca>
#ifndef REJECTION_H
#define REJECTION_H

#include <iostream>
#include "../scm/scm.h"


class rejection_sampler_t
{
public:
  void randomize(scm_t& K, std::mt19937 & engine)
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
};



#endif // REJECTION_H
