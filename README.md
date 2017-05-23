# scm

![](scm.png)

The Simplicial Configuration Model is random [null model](https://en.wikipedia.org/wiki/Null_model) for [simplicial complexes](https://en.wikipedia.org/wiki/Simplicial_complex), mathematical objects which can be seen as high-order generalizations of [simple graphs](http://mathworld.wolfram.com/SimpleGraph.html) (they incorporate multi-node interactions).
This repository contains a C++ reference implementation of a [Markov chain Monte Carlo (MCMC)](https://en.wikipedia.org/wiki/Markov_chain_Monte_Carlo) sampler for this model, see [arxiv:17xx.yyyy](https://arxiv.org/abs/17xx.yyyy) for more information.<br/>
A summary of compilation / usage instructions can be found below; but see the [tutorial](tutorial_notebook.ipynb) if you are looking for detailed instructions and examples.

## Table of content

1. [Compilation](#compilation)
2. [Using the sampler](#using-the-sampler)
    1. [Facet list analyzer](#facet-list-analyzer)
    2. [MCMC sampler](#mcmc-sampler)
3. [Publications](#publications)


## Compilation

The sampler has two mandatory dependencies, namely `boost::program_options` and `cmake` (for automated building).

To compile from the terminal, use the following commands:

    cmake .;
    make

The resulting binaries will be built in `bin/`. `CMakes` should also allow Windows users to compile the source easily (not tested---let us know!).

If `cmake` is not installed, the following manual compilation lines *should* work (for gcc with `C++11` support):

    g++ -std=c++11 -o3 -c src/scm/scm.cpp -o src/scm/libscm.a  #compile scm library
    g++ -std=c++11 -o3 -L src/scm/ -lboost_program_options -l scm -o bin/mcmc_sampler mcmc_sampler.cpp  #compile the main binaries (mcmc)
    g++ -std=c++11 -o3 -L src/scm/ -lboost_program_options -l scm -o bin/rejection_sampler rejection_sampler.cpp  #compile the main binaries (rejection)


## Using the sampler

The sampler randomizes an initial facet list, and samples uniformly from the space of all simplicial complexes with the same **degree sequence** (degree = number of facet incident on a node) and **size sequence** (size = number of node in a facet).
As such, it requires an initial facet list.<br/>
We thus provide not one, but two important binaries: `facet_list_analyzer` and `mcmc_sampler`.

### Facet list analyzer

The MCMC sampler is not robust to badly formatted input, so we provide a facet list analyzer that:

1. Checks that the facet list indeed (correctly) describes a simplicial complex. That is, it makes sure that there is no included face in the list, and that the nodes IDs are **0 indexed and contiguous integers**.
2. Provides estimates of the *optimal sampling frequency* for the MCMC sampler (`m log m`, where `m` is the sum of the facet size).
3. Computes a number of other properties not directly relevant to sampling, but to estimate sampling complexity, etc., [see the paper](https://arxiv.org/abs/17xx).


### MCMC sampler

Once the facet list is validated, using the sampler is as simple as the following example call:

    bin/mcmc_sampler -f 10000 -b 2000 -t 200 -d 42 seed_facet_list.txt

Here, `-f 10000` specifies that 10000 MCMC move will be applied between each samples (*the sampling frequency*), `-b 2000` specifies that the [*burn-in time*](https://en.wikipedia.org/wiki/Gibbs_sampling#Implementation) equals 2000 (the number of steps to ignore away before sampling begins), `-t 200` sets the number of samples to 200, and `-d 42` sets the seeds of the RNG to `42` (it will be seeded with the time by default).
`seed_facet_list.txt` is the path to the initial condition file (notice how it is the only positional argument).

By default the sampler uses the uniform proposal distribution with L_max = 2 max s,  [see the paper](https://arxiv.org/abs/17xx), but the behavior can be changed.
We provide two parameterizable proposal distributions, and it is straightforward to implement additional ones.

The provided distributions are 

* `exp_prop`: Exponential distribution. Draw L with the p.d.f.  `Pr(l) = exp(lambda * l)/Z` where `lambda` is a parameter, set with `--prop_param LAMBDA`, and `Z` is a normalization constant. `L` is limited to 2,...,L_max.
* `pl_prop`: Power law distribution. Draw L with the p.d.f.  `Pr(l) = l ** (-\lambda)/Z`. `L` is limited to 2,...,L_max.
* `unif_prop`: Uniform distribution [**Default**]. Draw L with the p.d.f.  `Pr(l) = 1 /(L_max-2)`. `L` is limited to 2,...,L_max.


The full list of options is:

    -b [ --burn_in ] arg (=1000)          Burn-in time.
    -t [ --sampling_steps ] arg (=1000)   Number of sampling steps.
    -f [ --sampling_frequency ] arg (=10) Number of step between each sample.
    -d [ --seed ] arg                     Seed of the pseudo random number 
                                          generator (Mersenne-twister 19937). 
                                          Seed with time if not specified.
    -l [ --l_max ] arg                    Manually set L_max. The correctness of 
                                          the sampler is not guaranteed if L_max 
                                          < 2 max s. Defaults to 10% of the sum 
                                          of facet sizes. 
    --exp_prop                            Use exponential proposal distribution.
    --pl_prop                             Use power law proposal distribution.
    --unif_prop                           Use uniform proposal distribution 
                                          [default].
    --prop_param arg                      Parameter of the proposal distribution 
                                          (only works for the exponential and 
                                          power law proposal distributions).
    -v [ --verbose ]                      Output log messages.
    -s [ --sanitized_input ]              Assume that the input is sanitized: 
                                          nodes are labeled via 0 index, 
                                          contiguous integers; no facets is 
                                          included in another. Saves computation 
                                          and storage space.
    -h [ --help ]                         Produce this help message.


## Publications

Please cite:

"*Construction of and efficient sampling from the simplicial configuration model*"<br/>
[J.-G. Young](http://jgyoung.ca), [G. Petri](https://github.com/lordgrilo), F. Vaccarino, and [A. Patania](https://github.com/alpatania) (2017)<br/>
[arxiv:17xx.yyyy](https://arxiv.org/abs/17xx.yyyy)
