#include <utility>
#include <cmath>
#include <fstream>
#include <iostream>
#include <cstring>

#include <sims2/losmodel.h>

/*******************************/
/*          LoS model          */
/*******************************/
const long double line_bearing(const pixelPos &pos1, const pixelPos &pos2) {
    long double x_diff = pos1.x - pos2.x;
    return x_diff == 0 ? x_diff : (pos1.y - pos2.y) / x_diff;
}

const double vertical_distance(const geo::Location &pos1, const geo::Location &pos2) {
    return geo::distance_between(pos1, geo::Location{pos2.latitude, pos1.longitude}) * KM;
}

const double horizontal_distance(const geo::Location &pos1, const geo::Location &pos2) {
    return geo::distance_between(pos1, geo::Location{pos1.latitude, pos2.longitude}) * KM;
}

sims2::LoSModel::LoSModel(BitMap map, const geo::Location nw, const geo::Location se) {
    this->nw_corner = nw;
    this->se_corner = se;
    this->map = std::move(map);

    this->lat_mtp = vertical_distance(this->nw_corner, this->se_corner) / this->map.get_height();
    this->lon_mtp = horizontal_distance(this->nw_corner, this->se_corner) / this->map.get_width();
}

const pixelPos sims2::LoSModel::gps_to_pixel_pos(const geo::Location &pos) const {
    auto lat_dist = vertical_distance(this->nw_corner, pos);
    auto lon_dist = horizontal_distance(this->nw_corner, pos);

    return pixelPos(
            lon_dist / this->lon_mtp,
            lat_dist / this->lat_mtp
    );
}

const double sims2::LoSModel::pathloss_formula(const double distance) {
    return 28.63263632987809 * std::log10(distance) + 0.003980704079352521;
//    return 0.008839017600881068 * std::log10(distance) + -8.127120163683704;
}

const double sims2::LoSModel::compute(const geo::Location &pos1, const geo::Location &pos2) const {
    pixelPos pixel_pos1{}, pixel_pos2{};

    /* Translate GPS coordinates to pixel coordinates */
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
        x--;
        y -= 1 * bearing;

        auto color = this->map.get_pixel(x, y);
        for (const auto &map_rgb : this->MAP_BUILDING_RGB) {
            if (color == map_rgb) {
                count++;
                break;
            }
        }
        total++;
    } while (x >= pixel_pos1.x && y >= pixel_pos1.y);

    return (100 * count) / (double) total;
}

sims2::BitMap sims2::LoSModel::visualise_line(const geo::Location &pos1, const geo::Location &pos2) const {
    pixelPos pixel_pos1{}, pixel_pos2{};

    /* Translate GPS coordinates to pixel coordinates */
    if (geo::distance_between(pos1, this->nw_corner) < geo::distance_between(pos2, this->nw_corner)) {
        pixel_pos1 = this->gps_to_pixel_pos(pos1);
        pixel_pos2 = this->gps_to_pixel_pos(pos2);
    } else {
        pixel_pos1 = this->gps_to_pixel_pos(pos2);
        pixel_pos2 = this->gps_to_pixel_pos(pos1);
    }

    auto bearing = line_bearing(pixel_pos1, pixel_pos2);
    auto to_return = this->map;
    long double x = pixel_pos2.x, y = pixel_pos2.y;
    to_return.set_pixel(x, y, sims2::RGB{255, 0, 255});

    do {
        x--;
        y -= 1 * bearing;

        auto trigger = false;
        for (const auto &map_rgb : this->MAP_BUILDING_RGB) {
            if (to_return.get_pixel(x, y) == map_rgb) {
                trigger = true;
                break;
            }
        }
        if (trigger) {
            to_return.set_pixel(x, y, sims2::RGB{255, 0, 0});
        } else {
            to_return.set_pixel(x, y, sims2::RGB{0, 255, 0});
        }

    } while (x >= pixel_pos1.x && y >= pixel_pos1.y);
    to_return.set_pixel(x, y, sims2::RGB{255, 0, 255});

    return to_return;
}

const bool sims2::LoSModel::out_of_map(const pixelPos &pos) const {
    return pos.x < 0 || pos.x > this->map.get_width() || pos.y < 0 || pos.y > this->map.get_height();
}

const bool sims2::LoSModel::out_of_map(const long x, const long y) const {
    return x < 0 || x > this->map.get_width() || y < 0 || y > this->map.get_height();
}

const bool sims2::LoSModel::out_of_map(geo::Location &pos) const {
    return this->nw_corner.latitude <= pos.latitude <= this->se_corner.latitude &&
           this->nw_corner.longitude <= pos.longitude <= this->se_corner.longitude;
}

/*************************************/
/*           Map generator           */
/*************************************/
std::tuple<sims2::BitMap, geo::Location, geo::Location>
sims2::MapGenerator::generate_map(const geo::Location &pos1, const geo::Location &pos2, const int zoomlevel) const {
    auto centroid = this->compute_centroid(pos1, pos2);
    auto hypotenuse = this->compute_hypotenuse(centroid, zoomlevel);
    auto nw = geo::move_location(centroid, hypotenuse, 315);
    auto se = geo::move_location(centroid, hypotenuse, 135);

    auto file_name = this->gen_file_name(nw, se);

    std::string script_args = this->script_path;
    script_args += " " + std::to_string(centroid.latitude)
                   + " " + std::to_string(centroid.longitude)
                   + " " + std::to_string(zoomlevel)
                   + " " + this->map_path + file_name;

    system(script_args.c_str());

    return std::make_tuple(sims2::BitMap(this->map_path + file_name + ".bmp"), nw, se);
}

geo::Location sims2::MapGenerator::compute_centroid(const geo::Location &nw, const geo::Location &se) const {
    return geo::Location{
            (nw.latitude + se.latitude) / 2,
            (nw.longitude + se.longitude) / 2
    };
}

std::string sims2::MapGenerator::gen_file_name(const geo::Location &nw, const geo::Location &se) const {
    return std::string{
            std::to_string(nw.latitude) + "-" + std::to_string(nw.longitude) + "-" + std::to_string(se.latitude) + "-" +
            std::to_string(se.longitude)};
}

const double sims2::MapGenerator::compute_hypotenuse(const geo::Location &centroid, const int zoomlevel) const {
    /* normally 8 but mapbox has more levels so 9 instead */
    auto m_pr_pix = EARTH_EQUATORIAL_CIRCUMFERENCE
                    * std::cos(geo::deg2rad(centroid.latitude))
                    / std::pow(2, zoomlevel + 9);

    auto distance = ((1280 / 2) * m_pr_pix);
    return std::sqrt(std::pow(distance, 2) + std::pow(distance, 2)) / KM;
}

/*************************************/
/*          LoS model index          */
/*************************************/
#if 0
sims2::LoSModel sims2::LoSModelIndex::get_model(const int index_pos) const {
    return this->models[index_pos];
}

void sims2::LoSModelIndex::add_model(sims2::LoSModel model) {
    this->index.emplace_back(std::make_pair(model.nw_corner, model.se_corner));
    this->models.push_back(model);
}

#endif

const int sims2::LoSModelIndex::has_model(const geo::Location &pos1, const geo::Location &pos2) const {
    for (auto i = 0; i < this->index.size(); ++i) {
        auto el = this->index[i];

        if ((el.first.latitude <= pos1.latitude <= el.second.latitude &&
             el.first.longitude <= pos1.longitude <= el.second.longitude) &&
            (el.first.latitude <= pos2.latitude <= el.second.latitude &&
             el.first.longitude <= pos2.longitude <= el.second.longitude)) {

            return i;
        }
    }

    return -1;
}

sims2::LoSModel sims2::LoSModelIndex::get_model(const geo::Location &pos1, const geo::Location &pos2) {
    if (geo::distance_between(pos1, pos2) * KM > 1500)
        throw "Link distance not possible.";

    auto has_model = this->has_model(pos1, pos2);
    if (has_model > -1)
        return this->models[has_model];

    BitMap map;
    geo::Location nw, se;
    std::tie(map, nw, se) = this->generator.generate_map(pos1, pos2, this->zoom_level);

    this->index.emplace_back(std::make_pair(nw, se));
    auto model = sims2::LoSModel{map, nw, se};
    this->models.push_back(model);
    return model;
}

sims2::LoSModelIndex::LoSModelIndex(std::string &script_path, std::string &map_path)
        : generator(script_path, map_path) {}