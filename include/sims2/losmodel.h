#ifndef MANETSIMS_LOSMODEL_H
#define MANETSIMS_LOSMODEL_H

#include <vector>
#include <utility>

#include <geo/geo.h>

struct RGB {
    RGB() = default;

    RGB(char red, char green, char blue) {
        this->red = int(red & 0xff);
        this->green = int(green & 0xff);
        this->blue = int(blue & 0xff);
    }

    int red, green, blue;
};


struct pixelPair {
    pixelPair() = default;

    pixelPair(int x, int y) {
        this->x = x;
        this->y = y;
    }

    bool operator<(const pixelPair &rhs) const {
        if (this->x < rhs.x)
            return true;
        if (rhs.x < this->x)
            return false;
        return this->y < rhs.y;
    }

    bool operator>(const pixelPair &rhs) const {
        return rhs < *this;
    }

    bool operator<=(const pixelPair &rhs) const {
        return !(rhs < *this);
    }

    bool operator>=(const pixelPair &rhs) const {
        return !(*this < rhs);
    }

    int x;
    int y;
};

namespace sims2 {
    class BitMap {
    public:
        BitMap(const char *file_path);

    private:
        std::vector<std::vector<RGB>> img{};
    };

    class losmodel {
    public:
        losmodel();

    private:
        const double pathloss_formula(double percentage) const;

        const std::vector<pixelPair> generate_pixel_coordinates(geo::Location &pos1, geo::Location &pos2);

        const pixelPair gps_to_pixel(const geo::Location &pos) const;

        const double compute(const geo::Location &pos1, const geo::Location &pos2) const;


        // sfml::Image map
        int map_width;
        int map_height;
        geo::Location nw_corner;
        geo::Location se_corner;
        double pixel_lat_size;
        double pixel_lon_size;
        const double line_step_size = 5;
    };
}

#endif //MANETSIMS_LOSMODEL_H
