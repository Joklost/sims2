#ifndef MANETSIMS_LOSMODEL_H
#define MANETSIMS_LOSMODEL_H

#include <vector>
#include <utility>

#include <geo/geo.h>
#include <ostream>

/*class RGB {
public:
    RGB() = default;

//    RGB(unsigned long red, unsigned long green, unsigned long blue) : red(red & 0xff), green(green & 0xff), blue(blue & 0xff) {}
    RGB(unsigned long red, unsigned long green, unsigned long blue) : red(red), green(green), blue(blue) {}

    bool operator==(const RGB &rhs) const {
        return red == rhs.red &&
               green == rhs.green &&
               blue == rhs.blue;
    }

    bool operator!=(const RGB &rhs) const {
        return !(rhs == *this);
    }

    friend std::ostream &operator<<(std::ostream &os, const RGB &rgb) {
        os << "Red: " << rgb.red << " Green: " << rgb.green << " Blue: " << rgb.blue;
        return os;
    }
    unsigned long red, green, blue;
};*/

/*
struct pixelPair {
    unsigned int x;
    unsigned int y;

    pixelPair() = default;

    pixelPair(unsigned int x, unsigned int y) {
        this->x = x;
        this->y = y;
    }
};

namespace sims2 {
    class BitMap {
    public:
        BitMap() = default;

        BitMap(const char *file_path);

        const RGB get_pixel(unsigned long x, unsigned long y) const;

        void set_pixel(unsigned long x, unsigned long y, RGB color);

        void read_from_file(const char *file_path);

        void write_to_file(const char *file_path);

        unsigned long width, height;

    private:
        std::vector<std::vector<RGB>> img{};
        static const size_t HEADER_SIZE = 54;
        std::array<char, HEADER_SIZE> header{};
    };


    class LoSModel {
    public:
        LoSModel(BitMap map, geo::Location nw, geo::Location se);

        const double compute(const geo::Location &pos1, const geo::Location &pos2) const;

    private:
        const double pathloss_formula(double percentage) const;

        const std::vector<pixelPair> generate_pixel_coordinates(geo::Location &pos1, geo::Location &pos2);

        const pixelPair gps_to_pixel(const geo::Location &pos) const;

        BitMap map{};
        geo::Location nw_corner;
        geo::Location se_corner;
        double pixel_lat_size;
        double pixel_lon_size;
        const double line_step_size = 5;
    };
}
*/
#endif //MANETSIMS_LOSMODEL_H
