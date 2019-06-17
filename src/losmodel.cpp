#include <utility>
#include <cmath>
#include <fstream>
#include <iostream>
#include <cstring>

#include <sims2/losmodel.h>
#include <common/strings.h>
#include <sims2/link.h>


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
    this->map = map;

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


const double sims2::LoSModel::compute(const geo::Location &pos1, const geo::Location &pos2) const {
    pixelPos pixel_pos1{}, pixel_pos2{};

    /* Translate GPS coordinates to pixel coordinates */
    auto tmp_pos1 = this->gps_to_pixel_pos(pos1);
    auto tmp_pos2 = this->gps_to_pixel_pos(pos2);

    /* if they are the same position */
    if (tmp_pos1 == tmp_pos2) {
        auto color = this->map.get_pixel(tmp_pos1.x, tmp_pos1.y);
        for (const auto &map_rgb : this->MAP_BUILDING_RGB) {
            if (color == map_rgb)
                return sims2::LoSModel::bopl(std::round(geo::distance_between(pos1, pos2)));
        }
        return sims2::LoSModel::cvpl(std::round(geo::distance_between(pos1, pos2)));
    }
    if (tmp_pos1.x < tmp_pos2.x) {
        pixel_pos1 = tmp_pos1;
        pixel_pos2 = tmp_pos2;
    } else {
        pixel_pos1 = tmp_pos2;
        pixel_pos2 = tmp_pos1;
    }

    auto bearing = line_bearing(pixel_pos1, pixel_pos2);
    auto count = 0, total = 0;

    long double x = pixel_pos1.x;
    long double y = pixel_pos1.y;
    do {
        if (this->out_of_map(x, y))
            throw "Pixel position is out of map x y range";

        auto color = this->map.get_pixel(x, y);
        for (const auto &map_rgb : this->MAP_BUILDING_RGB) {
            if (color == map_rgb) {
                count++;
                break;
            }
        }

        auto new_pos = this->increment_pos(x, y, bearing);
        x = new_pos.first;
        y = new_pos.second;

        total++;
    } while (x <= pixel_pos2.x);

    /* Compute the path loss */
    auto building_pct = (100 * count) / (double) total;
    double clear_pct = 100 - building_pct;
    auto distance = geo::distance_between(pos1, pos2) * KM;

    return (sims2::LoSModel::bopl(std::round(distance)) / 100) * building_pct +
           (sims2::LoSModel::cvpl(std::round(distance)) / 100) * clear_pct;
}

sims2::BitMap sims2::LoSModel::visualise_line(const geo::Location &pos1, const geo::Location &pos2) const {
    pixelPos pixel_pos1{}, pixel_pos2{};

    /* Translate GPS coordinates to pixel coordinates */
    auto tmp_pos1 = this->gps_to_pixel_pos(pos1);
    auto tmp_pos2 = this->gps_to_pixel_pos(pos2);

    if (tmp_pos1.x < tmp_pos2.x) {
        pixel_pos1 = tmp_pos1;
        pixel_pos2 = tmp_pos2;
    } else {
        pixel_pos1 = tmp_pos2;
        pixel_pos2 = tmp_pos1;
    }

    auto bearing = line_bearing(pixel_pos1, pixel_pos2);
    auto to_return = this->map;

    long double x = pixel_pos1.x;
    long double y = pixel_pos1.y;
    do {
        if (this->out_of_map(x, y))
            throw "Pixel position is out of map x y range";

        bool trigger = false;
        for (const auto &map_rgb : this->MAP_BUILDING_RGB) {
            if (to_return.get_pixel(x, y) == map_rgb) {
                trigger = true;
                break;
            }
        }
        if (trigger)
            to_return.set_pixel(x, y, sims2::RGB{255, 0, 0});
        else
            to_return.set_pixel(x, y, sims2::RGB{0, 255, 0});

        auto new_pos = this->increment_pos(x, y, bearing);
        x = new_pos.first;
        y = new_pos.second;
    } while (x <= pixel_pos2.x);

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

const double sims2::LoSModel::cvpl(const double distance) {
    return distance == 0 ? 0 : 48.5 * (std::log(distance) / std::log(77)) + 37.5;
}

const double sims2::LoSModel::bopl(const double distance) {
    return distance == 0 ? 0 : 67 * (std::log(distance) / std::log(57)) + 11.5;
}

const double sims2::LoSModel::compute(const sims2::Link &link) const {
    return this->compute(link.node1.location, link.node2.location);
}

sims2::LoSModel::LoSModel(geo::Location nw, geo::Location se) {
    this->nw_corner = nw;
    this->se_corner = se;
    this->lat_mtp = 1;
    this->lon_mtp = 1;
}

const double sims2::LoSModel::compute_chess(const sims2::Link &link) const {
    auto pos1 = link.node1.location, pos2 = link.node2.location;
    pixelPos pixel_pos1{}, pixel_pos2{};

    /* Translate GPS coordinates to pixel coordinates */
    auto tmp_pos1 = this->gps_to_pixel_pos(pos1);
    auto tmp_pos2 = this->gps_to_pixel_pos(pos2);

    if (tmp_pos1.x < tmp_pos2.x) {
        pixel_pos1 = tmp_pos1;
        pixel_pos2 = tmp_pos2;
    } else {
        pixel_pos1 = tmp_pos2;
        pixel_pos2 = tmp_pos1;
    }

    auto bearing = line_bearing(pixel_pos1, pixel_pos2);
    auto count = 0, total = 0;
    auto to_return = this->map;

    long int x = pixel_pos1.x;
    long double y = pixel_pos1.y;

    do {
        if (x % 2 == 0 && static_cast<int>(y) % 2 == 0)
            count++;

        auto new_pos = this->increment_pos(x, y, bearing);
        x = new_pos.first;
        y = new_pos.second;

    } while (x <= pixel_pos2.x);

    auto building_pct = (100 * count) / (double) total;
    double clear_pct = 100 - building_pct;
    auto distance = geo::distance_between(pos1, pos2) * KM;

    return (sims2::LoSModel::bopl(std::round(distance)) / 100) * building_pct +
           (sims2::LoSModel::cvpl(std::round(distance)) / 100) * clear_pct;
}

std::pair<long double, long double>
sims2::LoSModel::increment_pos(const long double x, const long double y, const double bearing) const {
    if (std::abs(bearing) > 1)
        return std::make_pair(
                x + 1 / std::abs(bearing),
                y + (bearing > 1 ? 1 : -1));
    else
        return std::make_pair(x + 1, y + bearing);
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
                   + " " + this->map_path
                   + " " + file_name;

    std::system(script_args.c_str());
    std::string mp = this->map_path;
    mp += "/" + file_name + ".bmp";
    return std::make_tuple(sims2::BitMap(mp.c_str()), nw, se);
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
const int sims2::LoSModelIndex::has_model(const geo::Location &pos1, const geo::Location &pos2) const {
    for (auto i = 0; i < this->index.size(); ++i) {
        auto &el = this->index[i];
        auto nw_bearing_pos1 = geo::bearing_between(pos1, el.first);
        auto se_bearing_pos1 = geo::bearing_between(pos1, el.second);
        auto nw_bearing_pos2 = geo::bearing_between(pos2, el.first);
        auto se_bearing_pos2 = geo::bearing_between(pos2, el.second);

        auto cond_nw = nw_bearing_pos1 >= 90
                       && nw_bearing_pos1 <= 180
                       && nw_bearing_pos2 >= 90
                       && nw_bearing_pos2 <= 180;

        if (cond_nw) {
            auto cond_se = se_bearing_pos1 >= 270
                           && (se_bearing_pos1 <= 359.999999 || se_bearing_pos1 == 0)
                           && se_bearing_pos2 >= 270
                           && (se_bearing_pos2 <= 359.999999 || se_bearing_pos2 == 0);

            if (cond_se)
                return i;
        }
    }

    return -1;
}


sims2::LoSModel &sims2::LoSModelIndex::get_model(const sims2::Link &link) {
    return this->get_model(link.node1.location, link.node2.location);
}

sims2::LoSModel &sims2::LoSModelIndex::get_model(const geo::Location &pos1, const geo::Location &pos2) {
    if (geo::distance_between(pos1, pos2) * KM > 1500)
        throw "Link distance not possible.";

    auto has_model = this->has_model(pos1, pos2);
    if (has_model > -1)
        return this->models[has_model];

    BitMap map;
    geo::Location nw, se;
    std::tie(map, nw, se) = this->generator.generate_map(pos1, pos2, this->zoom_level);

    this->index.emplace_back(std::make_pair(nw, se));
    this->models.emplace_back(map, nw, se);
    return this->models.back();
}

sims2::LoSModelIndex::LoSModelIndex(std::string &script_path, std::string &map_path) : generator(script_path,
                                                                                                 map_path) {
    if (map_path.back() == '/')
        map_path.pop_back();

    this->load_models(map_path);
}

sims2::LoSModelIndex::LoSModelIndex(const char *script_path, const char *map_path) : generator(script_path, map_path) {
    std::string p = map_path;
    if (p.back() == '/')
        p.pop_back();

    this->load_models(p);
}

void sims2::LoSModelIndex::load_models(std::string map_path) {
    std::ifstream file(map_path + "/.fetched_maps");
    if (file.good()) {
        for (std::string line; std::getline(file, line);) {
            auto split = common::split(line, "-");
            auto nw = geo::Location{std::stod(split.fpop()), std::stod(split.fpop())};
            auto se = geo::Location{std::stod(split.fpop()), std::stod(split.fpop())};

            auto map = sims2::BitMap{map_path + "/" + line + ".bmp"};
            this->models.emplace_back(map, nw, se);
            this->index.emplace_back(std::make_pair(nw, se));
        }
        file.close();
    }
}

