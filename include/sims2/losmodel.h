#ifndef MANETSIMS_LOSMODEL_H
#define MANETSIMS_LOSMODEL_H

#include <vector>
#include <ostream>
#include <utility>

#include <geo/geo.h>
#include <sims2/bitmap.h>
#include <sims2/link.h>


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
    class MapGenerator {
    public:
        MapGenerator() = default;

        MapGenerator(const char *script_path, const char *map_path) : script_path(script_path),
                                                                      map_path(map_path) {};

        MapGenerator(std::string script_path, std::string map_path) : script_path(script_path.c_str()),
                                                                      map_path(map_path.c_str()) {};

        /**
         * Generate new BitMap based on given coorner coordinates, and new nw and se coordinates
         * @return Tuple[BitMap, nw, se]
         */
        std::tuple<sims2::BitMap, geo::Location, geo::Location>
        generate_map(const geo::Location &nw, const geo::Location &se, int zoomlevel) const;

    private:
        std::string gen_file_name(const geo::Location &nw, const geo::Location &se) const;

        geo::Location compute_centroid(const geo::Location &nw, const geo::Location &se) const;

        const double compute_hypotenuse(const geo::Location &centroid, int zoomlevel) const;

        const char *script_path;
        const char *map_path;
    };

    class LoSModel {
    public:
        LoSModel(sims2::BitMap map, geo::Location nw, geo::Location se);

        LoSModel(geo::Location nw, geo::Location se);


        /**
         * Compute the path loss
         * @return Path loss
         */
        const double compute(const geo::Location &pos1, const geo::Location &pos2) const;

        /**
         * Compute the path loss
         * @return Path loss
         */
        const double compute(const sims2::Link &link) const;

        const double compute_chess(const sims2::Link &link) const;

        /**
         * Create new BitMap where building pixels have been changed to show computed building coordinates
         * @return BitMap object
         */
        sims2::BitMap visualise_line(const geo::Location &pos1, const geo::Location &pos2) const;

        geo::Location nw_corner;    /* north-west corner position */
        geo::Location se_corner;    /* south-east corner position */

        static const double cvpl(double distance);

        static const double bopl(double distance);

    private:

        /**
         * Translate GPS position to x,y coordinates in the map
         * @return Pixel position
         */
        const pixelPos gps_to_pixel_pos(const geo::Location &pos) const;

        /**
         * Checks if pixel positions go outside map area
         * @return True if outside map area
         */
        const bool out_of_map(const pixelPos &pos) const;

        /**
         * Checks if pixel positions go outside map area
         * @return True if outside map area
         */
        const bool out_of_map(long x, long y) const;

        /**
         * Checks if GPS position is outside map area
         * @return True if outside map area
         */
        const bool out_of_map(geo::Location &pos) const;

        sims2::BitMap map;          /* the map */
        const double step = 1;      /* step size in meters */
        double lat_mtp;             /* meter to pixels at x axis */
        double lon_mtp;             /* meter to pixels at y axis */

        const std::vector<sims2::RGB> MAP_BUILDING_RGB{
                sims2::RGB(233, 233, 231),
                sims2::RGB(223, 223, 221),
                sims2::RGB(226, 226, 225),
                sims2::RGB(224, 224, 222),
                sims2::RGB(228, 228, 226),
                sims2::RGB(233, 233, 231),
                sims2::RGB(236, 238, 237),
        };
    };

    class LoSModelIndex {
    public:
        LoSModelIndex(std::string &script_path, std::string &map_path);

        LoSModelIndex(const char *script_path, const char *map_path);

        /**
         * Get stored model
         * @return LoSModel object
         */
        sims2::LoSModel &get_model(const geo::Location &pos1, const geo::Location &pos2);

        /**
         * Get stored model
         * @return LoSModel object
         */
        sims2::LoSModel &get_model(const sims2::Link &link);

        sims2::MapGenerator generator{};
        int zoom_level = 16;

    private:
        /**
         * Check if index contains a matching model
         * @return Index for stored model
         */
        const int has_model(const geo::Location &pos1, const geo::Location &pos2) const;

        /**
         * Load locally stored map images and generate models
         */
        void load_models(std::string map_path);

        std::vector<sims2::LoSModel> models{};
        std::vector<std::pair<geo::Location, geo::Location>> index{};
    };
}
#endif //MANETSIMS_LOSMODEL_H
