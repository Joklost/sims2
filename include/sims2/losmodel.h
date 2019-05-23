#ifndef MANETSIMS_LOSMODEL_H
#define MANETSIMS_LOSMODEL_H

#include <vector>
#include <ostream>
#include <utility>

#include <geo/geo.h>
#include <sims2/bitmap.h>


struct pixelPos {
    long x;
    long y;

    pixelPos() = default;

    pixelPos(long x, long y) {
        this->x = x;
        this->y = y;
    }

    friend std::ostream &operator<<(std::ostream &os, const pixelPos &pos) {
        os << "x: " << pos.x << " y: " << pos.y;
        return os;
    }
};


namespace sims2 {
    class LoSModel {
    public:
        LoSModel(sims2::BitMap &map, geo::Location nw, geo::Location se);

        const double compute(const geo::Location &pos1, const geo::Location &pos2) const;

        sims2::BitMap visualise_line(const geo::Location &pos1, const geo::Location &pos2) const;

    private:
        const double pathloss_formula(double distance) const;

        /**
         * Translate GPS position to x,y coordinates in the map
         */
        const pixelPos gps_to_pixel_pos(const geo::Location &pos) const;

        /**
         * Checks if pixel positions go outside map area
         * @param pos x,y coordinates
         * @return True if outside map area
         */
        const bool out_of_map(const pixelPos &pos) const;

        /**
         * Checks if pixel positions go outside map area
         * @param x coordinate
         * @param y coordinate
         * @return True if outside map area
         */
        const bool out_of_map(long x, long y) const;


        sims2::BitMap map;
        geo::Location nw_corner;
        geo::Location se_corner;
        const double step = 5;      /* step size in meters */
        double lat_mtp;             /* meter to pixels at x axis */
        double lon_mtp;             /* meter to pixels at y axis */

        const auto MAP_BUILDING_RGB = std::vector<sims2::RGB>{
                sims2::RGB(217, 208, 201),
                sims2::RGB(197, 183, 165),
                sims2::RGB(198, 185, 164),
                sims2::RGB(191, 177, 163),
                sims2::RGB(197, 184, 166),
                sims2::RGB(199, 186, 163),
                sims2::RGB(207, 196, 186),
                sims2::RGB(208, 198, 189)
        };
    };
}
#endif //MANETSIMS_LOSMODEL_H
