# scm

![](scm.png)

The Simplicial Configuration Model is random [null model](https://en.wikipedia.org/wiki/Null_model) for [simplicial complexes](https://en.wikipedia.org/wiki/Simplicial_complex), mathematical objects which can be seen as high-order generalizations of [simple graphs](http://mathworld.wolfram.com/SimpleGraph.html) (they incorporate multi-node interactions).
This repository contains a C++ reference implementation of a [Markov chain Monte Carlo (MCMC)](https://en.wikipedia.org/wiki/Markov_chain_Monte_Carlo) sampler for this model, see [arxiv:17xx.yyyy](https://arxiv.org/abs/17xx.yyyy) for more information.<br/>
A summary of compilation / usage instructions can be found below; but see the [tutorial](tutorial_notebook.ipynb) if you are looking for detailed instructions and examples.

## Table of content

1. [Compilation](#compilation)
2. [Using the sampler](#using-the-sampler)
    1. [Rejection sampler](#rejection-sampler)
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
As such, it requires an initial facet list, either artificial or taken from a real system.<br/>
We thus provide not one, but two important binaries: `rejection_sampler` and `mcmc_sampler`.

### Rejection sampler

The rejection method is a very **inefficient** sampler for the SCM, see our [paper](https://arxiv.org/abs/17xx.yyyy).
However, since there is---so far---no known constructive procedure to generate SCM instances directly from sequences, it can be used to find **one** instance, which is then plugged into the much more efficient [MCMC sampler](#mcmc-sampler) as an initial condition.

There are two ways to call `bin/rejection_sampler`.<br>
The first (default) takes a single positional argument, the path to a facet list, and tries to sample from the associated SCM ensemble:

    > bin/rejection_sampler datasets/simple_facet_list.txt  
    # Sample:
    0 1 
    2 3 4 
    1 2 3

where the output is a facet list.

The second way to call the rejection sampler is in the *sequence mode*.<br>
This is accomplished by using the flags `--degree_seq_file=path-to-degrees.txt` and `--size_seq_file=path-to-sizes.txt` where `path-to-degrees.txt` and `path-to-sizes.txt` are paths to files containing the integer sequences (no particular organization required; will use all integers in the file).
There is no known simpliciality test yet, so we make no test on the sequences---convergence is not guaranteed and the sampler might run forever.

Here is a simple example, using small sequences, which we know are simplicial:

    > echo "2 2 2 1 1" > d.txt 
    > echo "3 3 2" > s.txt
    > bin/rejection_sampler -s s.txt -k d.txt
    # Sample:
    0 1 4
    0 1 2
    2 3

Note that we have used the the shorthand flags `-k` and `-s` for the sequences, see the full list of option for `rejection_sampler` below:

    Usage:
     [Facet list mode] bin/rejection_sampler [--option_1=VAL] ... [--option_n=VAL] path-to-facet-list
     [Seq. mode] bin/rejection_sampler [--option_1=VAL] ... -k path-to-degrees.txt -s path-to-sizes.txt
    Options:
      -d [ --seed ] arg             Seed of the pseudo random number generator 
                                    (Mersenne-twister 19937). Seed with time if not
                                    specified.
      -c [ --cleansed_input ]       In facet list mode, assume that the input is 
                                    already cleansed, i.e., that nodes are labeled 
                                    with 0 indexed contiguous integers and that no 
                                    facet is included in another.
      -k [ --degree_seq_file ] arg  Path to degree sequence file.
      -s [ --size_seq_file ] arg    Path to size sequence file.
      -v [ --verbose ]              Output log messages.
      -h [ --help ]                 Produce help message.



### MCMC sampler

Once we have an initial condition (either by using the rejection sampler or a real system), the MCMC is called with the following commad:

    bin/mcmc_sampler -f 10000 -b 2000 -t 200 -d 42 seed_facet_list.txt

Here, `-f 10000` specifies that 10000 MCMC move will be applied between each samples (*the sampling frequency*), `-b 2000` specifies that the [*burn-in time*](https://en.wikipedia.org/wiki/Gibbs_sampling#Implementation) equals 2000 (the number of steps to ignore away before sampling begins), `-t 200` sets the number of samples to 200, and `-d 42` sets the seeds of the RNG to `42` (it will be seeded with the time by default).
`seed_facet_list.txt` is the path to the initial condition file (notice how it is the only positional argument).

*Note*: All the above commands have sensible default values and can be omitted.

By default the sampler uses the uniform proposal distribution with L_max = 2 max s,  [see the paper](https://arxiv.org/abs/17xx), but the behavior can be changed.
We provide two parameterizable proposal distributions, and it is straightforward to implement additional ones.

The provided distributions are 

* `exp_prop`: Exponential distribution. Draw L with the p.d.f.  `Pr(l) = exp(lambda * l)/Z` where `lambda` is a parameter, set with `--prop_param LAMBDA`, and `Z` is a normalization constant. `L` is limited to 2,...,L_max.
* `pl_prop`: Power law distribution. Draw L with the p.d.f.  `Pr(l) = l ** (-\lambda)/Z`. `L` is limited to 2,...,L_max.
* `unif_prop`: Uniform distribution [**Default**]. Draw L with the p.d.f.  `Pr(l) = 1 /(L_max-2)`. `L` is limited to 2,...,L_max.


*Note*: The sampler can handle arbitrary facet lists as input (lines beginning with `#` will be ignored). However, it is better if facet lists are cleansed from the get go. By clean we mean that nodes are 0 indexed contiguous integers, and there are no included facet.
If the data is already cleansed, use the flag `-c` to skip the pre-processing cleansing steps. See [scm/utilities/](https://github.com/jg-you/scm/tree/master/utilities) for some lightweight python cleansing tools.

The full list of options for `mcmc_sampler`:

    Usage:
      bin/mcmc_sampler [--option_1=VAL] ... [--option_n=VAL] path-to-facet-list
    Options:
      -b [ --burn_in ] arg                  Burn-in time. Defaults to M log M, 
                                            where M is the sum of degrees.
      -t [ --sampling_steps ] arg           Number of sampling steps.
      -f [ --sampling_frequency ] arg (=10) Number of step between each sample. 
                                            Defaults to M log M, where M is the sum
                                            of degrees.
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
      -c [ --cleansed_input ]               Assume that the input is already 
                                            cleansed, i.e., that nodes are labeled 
                                            with 0 indexed contiguous integers and 
                                            that no facet is included in another.
      -v [ --verbose ]                      Output log messages.
      -h [ --help ]                         Produce this help message.

## Publications

Please cite:

"*Construction of and efficient sampling from the simplicial configuration model*"<br/>
[J.-G. Young](http://jgyoung.ca), [G. Petri](https://github.com/lordgrilo), F. Vaccarino, and [A. Patania](https://github.com/alpatania) (2017)<br/>
[arxiv:17xx.yyyy](https://arxiv.org/abs/17xx.yyyy)
