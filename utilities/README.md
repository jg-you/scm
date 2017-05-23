These two python scripts are useful tools to generate clean, sanitized, facet lists from bipartite graphs.

* `bipartite_to_max_facets` generates a list of **faces** from a bipartite graph. Nodes are 0 indexed and contiguous. But the list may contain included faces.<br/>
* `prune.py` removes included facets from a list of facets. In abstract terms: remove any set *X* in a list of sets which is the subset of some *Y!=X* in the list. Does not preserve facet ordering.