#ifndef MANETSIMS_DATAGEN_H
#define MANETSIMS_DATAGEN_H

#include <vector>

#include <geo/geo.h>
#include "link.h"

namespace sims2 {
    namespace data {
        /**
         * Generate a vector of nodes (a graph) within a set of geographical bounds.
         *
         * The nodes are generated using a uniform distribution.
         *
         * @param count The amount of nodes to generate
         * @param upper The upper left geographical bound
         * @param lower The lower right geographical bound
         * @return A vector of nodes (a graph)
         */
        std::vector<sims2::Node> generate_nodes(unsigned long count, geo::Location &upper, geo::Location &lower);

        /**
         * Create a vector of unique (undirected) links from a set of nodes (a graph).
         *
         * If threshold is less than or equal to 0.01, it is assumed that the graph is fully connected.
         * If the graph is fully connected, the length of the vector of links will be
         * ((N * (N + 1)) / 2) - N.
         *
         * @param nodes A vector of nodes
         * @param threshold The threshold radius wherein nodes are assumed reachable
         * @return A vector of unique (undirected) links
         */
        std::vector<sims2::Link> create_links(std::vector<sims2::Node> nodes, double threshold = 0.55);
    }
}

#endif //MANETSIMS_DATAGEN_H
