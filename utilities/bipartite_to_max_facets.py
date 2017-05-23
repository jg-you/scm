#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Author: Jean-Gabriel Young <info@jgyoung.ca>

Convert a bipartite graph (in KONECT format) to maximal facet format.

Assumes that left side nodes and right side nodes are the different part of
the graph. Will reindex everything from 0, not necessarily conserving the
order.

"""


def read_edge_list(path):
    """Read edge list in KONECT format."""
    with open(path, 'r') as f:
        edge_list = set()
        for line in f:
            if not line.lstrip().startswith("%"):  # ignore commets
                e = line.strip().split()
                edge_list.add((int(e[0]) - 1, int(e[1]) - 1))  # 0 index
        return list(edge_list)


def remap_bipartite_edge_list(edge_list):
    """Create isomoprhic edge list, with labels starting from 0."""
    remap_1 = dict()
    remap_2 = dict()
    new_id_1 = 0
    new_id_2 = 0
    for e in edge_list:
        if remap_1.get(e[0]) is None:
            remap_1[e[0]] = new_id_1
            new_id_1 += 1
        if remap_2.get(e[1]) is None:
            remap_2[e[1]] = new_id_2
            new_id_2 += 1
    return [(remap_1[e[0]], remap_2[e[1]]) for e in edge_list]


def facet_generator(sorted_edge_list, facet_col=0):
    """Generate facet list, from the sorted edge list."""
    vertex_col = int(not facet_col)
    prev_facet = sorted_edge_list[0][facet_col]
    facet_content = []
    for e in sorted_edge_list:
        curr_facet = e[facet_col]
        if curr_facet != prev_facet:
            yield facet_content
            facet_content.clear()
        facet_content.append(e[vertex_col])
        prev_facet = curr_facet


if __name__ == '__main__':
    # Options parser.
    import argparse as ap
    from operator import itemgetter
    prs = ap.ArgumentParser(description='Convert a KONECT bipartite graph ' +
                                        'to a list of maximal facets.')
    prs.add_argument('--col', '-c', type=int, default=0,
                     help='Column to use as facets (0 or 1).')
    prs.add_argument('edge_list_path', type=str, nargs='?',
                     help='Path to edge list.')
    args = prs.parse_args()
    edge_list = read_edge_list(args.edge_list_path)
    if edge_list is not None:
        edge_list = remap_bipartite_edge_list(edge_list)
        edge_list = sorted(edge_list, key=itemgetter(args.col))
        for max_facet in facet_generator(edge_list, args.col):
            print(" ".join([str(v) for v in sorted(max_facet)]))
