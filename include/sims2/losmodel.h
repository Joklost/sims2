#ifndef MANETSIMS_LOSMODEL_H
#define MANETSIMS_LOSMODEL_H

#include <vector>
#include <utility>

#include <geo/geo.h>

namespace sims2 {
    namespace losmodel {
        const double compute_los_pathloss(double percentage);

        const std::vector<std::pair<int, int>> generate_pixel_coordinates(geo::Location pos1, geo::Location pos2);


    }
}

#endif //MANETSIMS_LOSMODEL_H
