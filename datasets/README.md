# Dataset information

Facet lists for the datasets analyzed in [arxiv:17xx.yyyy](https://arxiv.org/abs/17xx.yyyy), as well as two simple examples (`simple_facet_list.txt` and `olesen_2002.txt`).

**Format**: One facet appears on each line, as whitespaced list of strings.<br>
The sampler skips a lot of pre-processing if vertices are identified with contiguous 0 indexed integers (use the `-s` flag).

Multiple transformations separate the provided facet lists from the original datasets:

1. we have relabeled the nodes;
2. the datasets were originally bipartite graphs;
3. the facet lists are [*pruned*](../utilities/prune.py), i.e., we remove any face included in a larger face upon projection.


# References

Original references to the datasets

* `crime_facet_list.txt`: "*St. Louis Homicide Project: Local Responses to a National Problem*", S. Decker et al., (1991) [dataset](http://konect.uni-koblenz.de/networks/moreno_crime) <br/>
* `diseasome_facet_list.txt`: "*The human disease network*", K.-I. Goh et al., PNAS **21** (2008). [doi:10.1073/pnas.0701361104](https://dx.doi.org/10.1073/pnas.0701361104)<br/>
* `pollinators_facet_list.txt`: "*Insect--flower relationship in the primary beech forest of Ashu*", M. Kato et al., Contr. Biol. Lab. Kyoto Univ. **27**, (1990) [direct link](https://www.researchgate.net/profile/Takao_Itino/publication/236969168_Insect-flower_relationship_in_the_primary_beech_forest_of_Ashu_Kyoto_An_overview_of_the_flowering_phenology_and_the_seasonal_pattern_of_insect_visits/links/53d5f0df0cf2a7fbb2ea62c4.pdf)<br/>
* `olesen_2002.txt`: "*Invasion of pollination networks on oceanic islands: importance of invader complexes and endemic super generalists".  J. M Olesen. et al., Diversity and Distributions **8** (2002) [doi:10.1046/j.1472-4642.2002.00148.x](https://dx.doi.org/10.1046/j.1472-4642.2002.00148.x).