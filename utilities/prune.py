#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Prune facet list: removes all included facets in a facet list.

Author: Alice Patania <alice.patania@isi.it>
Author: Jean-Gabriel Young <info@jgyoung.ca>
"""
import itertools


def prune(facet_list):
    """Remove included facets from a collection of facets.

    Notes
    =====
    Facet list should be a list of frozensets
    """
    # organize facets by sizes
    sizes = {len(f) for f in facet_list}
    facet_by_size = {s: [] for s in sizes}
    for f in facet_list:
        facet_by_size[len(f)].append(f)
    # remove repeated facets
    for s in facet_by_size:
        facet_by_size[s] = list({x for x in facet_by_size[s]})
    # remove included facets and yield
    for ref_size in sorted(list(sizes), reverse=True):
        for ref_set in sorted(facet_by_size[ref_size]):
            for s in sizes:
                if s < ref_size:
                    facet_by_size[s] = [x for x in facet_by_size[s]
                                        if not x.issubset(ref_set)]
        for facet in facet_by_size[ref_size]:
            yield facet


if __name__ == '__main__':
    # Options parser.
    import argparse as ap
    prs = ap.ArgumentParser(description='Prune facet list.')
    prs.add_argument('facet_list', type=str, nargs='?',
                     help='Path to facet list.')
    args = prs.parse_args()
    facet_list = []
    with open(args.facet_list, 'r') as f:
        for line in f:
            # frozenset is mandatory: allows for comparison
            facet = frozenset([int(x) for x in line.strip().split()])
            facet_list.append(facet)
        for facet in prune(facet_list):
            print(" ".join([str(v) for v in facet]))
