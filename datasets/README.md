# Dataset information

Facet lists for the datasets analyzed in [arxiv:17xx.yyyy](https://arxiv.org/abs/17xx.yyyy), as well as one simple example (`simple_facet_list.txt`).

Format: One facet per line; nodes are identified with 0 indexed contiguous integers.

Multiple transformations separate these facet lists from the original.

1. we have relabeled the nodes;
2. the datasets came as bipartite graphs; they now are represented as simplicial complexes;
3. the facet lists are [*pruned*](../utilities/prune.py), i.e., all included faces are removed.


# References

* `crime_facet_list.txt`: "*Insect--flower relationship in the primary beech forest of Ashu*", Kato et al., Contr. Biol. Lab. Kyoto Univ. 27, [direct link](https://www.researchgate.net/profile/Takao_Itino/publication/236969168_Insect-flower_relationship_in_the_primary_beech_forest_of_Ashu_Kyoto_An_overview_of_the_flowering_phenology_and_the_seasonal_pattern_of_insect_visits/links/53d5f0df0cf2a7fbb2ea62c4.pdf)<br/>
* `diseasome_facet_list.txt`: "*The human disease network*", Goh et al., PNAS **21** (2008). [doi:10.1073/pnas.0701361104](https://dx.doi.org/10.1073/pnas.0701361104)<br/>
* `pollinators_facet_list.txt`: To be added<br/>
