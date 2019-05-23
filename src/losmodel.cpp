#include <utility>
#include <cmath>
#include <fstream>
#include <iostream>
#include <cstring>

#include <sims2/losmodel.h>


const long double line_bearing(const pixelPos &pos1, const pixelPos &pos2) {
    long double x_diff = pos1.x - pos2.x;
    return x_diff == 0 ? x_diff : (pos1.y - pos2.y) / x_diff;
}

const double flat_lat_distance(const geo::Location &pos1, const geo::Location &pos2) {
    return geo::distance_between(pos1, geo::Location{pos2.latitude, pos1.longitude}) * KM;
}

const double flat_lon_distance(const geo::Location &pos1, const geo::Location &pos2) {
    return geo::distance_between(pos1, geo::Location{pos1.latitude, pos2.longitude}) * KM;
}

sims2::LoSModel::LoSModel(BitMap &map, const geo::Location nw, const geo::Location se) {
    this->nw_corner = nw;
    this->se_corner = se;
    this->map = std::move(map);

    this->lat_mtp = flat_lat_distance(this->nw_corner, this->se_corner) / this->map.get_height();
    this->lon_mtp = flat_lon_distance(this->nw_corner, this->se_corner) / this->map.get_width();
}

const pixelPos sims2::LoSModel::gps_to_pixel_pos(const geo::Location &pos) const {
    auto lat_dist = flat_lat_distance(this->nw_corner, pos);
    auto lon_dist = flat_lon_distance(this->nw_corner, pos);

    return pixelPos(
            lon_dist / this->lon_mtp,
            lat_dist / this->lat_mtp
    );
}

const double sims2::LoSModel::pathloss_formula(const double distance) const {
    return -8.127120163683704 * std::log10(distance) + 0.008839017600881068;
}

const double sims2::LoSModel::compute(const geo::Location &pos1, const geo::Location &pos2) const {
    pixelPos pixel_pos1{}, pixel_pos2{};

    if (geo::distance_between(pos1, this->nw_corner) < geo::distance_between(pos2, this->nw_corner)) {
        pixel_pos1 = this->gps_to_pixel_pos(pos1);
        pixel_pos2 = this->gps_to_pixel_pos(pos2);
    } else {
        pixel_pos1 = this->gps_to_pixel_pos(pos2);
        pixel_pos2 = this->gps_to_pixel_pos(pos1);
    }

    auto bearing = line_bearing(pixel_pos1, pixel_pos2);
    auto count = 0, total = 0;

    long double x = pixel_pos2.x, y = pixel_pos2.y;
    do {
        x -= this->step / this->lon_mtp;
        y -= (this->step / this->lat_mtp) * bearing;

        auto color = this->map.get_pixel(x, y);
        for (const auto &map_rgb : this->MAP_BUILDING_RGB) {
            if (color == map_rgb) {
                count++;
                break;
            }
        }
        total++;
    } while (x >= pixel_pos1.x && y >= pixel_pos1.y);

    return ((double) total / 100) * count;
}

sims2::BitMap sims2::LoSModel::visualise_line(const geo::Location &pos1, const geo::Location &pos2) const {
    pixelPos pixel_pos1{}, pixel_pos2{};

    if (geo::distance_between(pos1, this->nw_corner) < geo::distance_between(pos2, this->nw_corner)) {
        pixel_pos1 = this->gps_to_pixel_pos(pos1);
        pixel_pos2 = this->gps_to_pixel_pos(pos2);
    } else {
        pixel_pos1 = this->gps_to_pixel_pos(pos2);
        pixel_pos2 = this->gps_to_pixel_pos(pos1);
    }

    auto bearing = line_bearing(pixel_pos1, pixel_pos2);

    /*std::cout << "max lat distance: " << flat_lat_distance(this->nw_corner, this->se_corner) << std::endl;
    std::cout << "max lon distance: " << flat_lon_distance(this->nw_corner, this->se_corner) << std::endl;
    std::cout << "lat_mtp: " << this->lat_mtp << std::endl;
    std::cout << "lon_mtp: " << this->lon_mtp << std::endl;
    std::cout << "map_width: " << this->map.get_width() << std::endl;
    std::cout << "map_height: " << this->map.get_height() << std::endl;
    std::cout << "distance: " << geo::distance_between(pos1, pos2) * KM << std::endl;
    std::cout << "bearing: " << bearing << std::endl;
    std::cout << "total_steps: " << total_steps << std::endl;
    std::cout << "pixel_pos1: " << pixel_pos1 << std::endl;
    std::cout << "pixel_pos2: " << pixel_pos2 << std::endl << std::endl;*/

    auto to_return = this->map;
    long double x = pixel_pos2.x, y = pixel_pos2.y;

    do {
        pixelPos dot(x, y);

        x -= this->step / this->lon_mtp;
        y -= (this->step / this->lat_mtp) * bearing;

//        std::cout << dot << std::endl;
//      below is block for "paiting" larger pixels, to easier see them
        /*for (unsigned int j = 0, xi = dot.x - 1; j < 3; ++j, xi++) {
            for (unsigned int k = 0, yi = dot.y - 1; k < 3; ++k, yi++) {
                if (this->out_of_map(xi, yi))
                    continue;

                to_return.set_pixel(xi, yi, sims2::RGB{255, 0, 0});
            }
        }*/
        to_return.set_pixel(x, y, sims2::RGB{255, 0, 0});

    } while (x >= pixel_pos1.x && y >= pixel_pos1.y);
    return to_return;
}

const bool sims2::LoSModel::out_of_map(const pixelPos &pos) const {
    return pos.x < 0 || pos.x > this->map.get_width() || pos.y < 0 || pos.y > this->map.get_height();
}

const bool sims2::LoSModel::out_of_map(const long x, const long y) const {
    return x < 0 || x > this->map.get_width() || y < 0 || y > this->map.get_height();
}
