#include <cmath>
#include <fstream>

#include <sims2/losmodel.h>
#include <cstring>
#include <iostream>

#if(0)
const double line_bearing(const pixelPair &pos1, const pixelPair &pos2) {
    auto x_diff = pos1.x - pos2.x;
    return x_diff == 0 ? x_diff : (pos1.y - pos2.y) / x_diff;
}

const double flat_lat_distance(const geo::Location &pos1, const geo::Location &pos2) {
    auto tmp = geo::Location{pos2.latitude, pos1.longitude};
    return geo::distance_between(pos1, tmp);
}

const double flat_lon_distance(const geo::Location &pos1, const geo::Location &pos2) {
    auto tmp = geo::Location{pos1.latitude, pos2.longitude};
    return geo::distance_between(pos1, tmp);
}

sims2::LoSModel::LoSModel(BitMap map, const geo::Location nw, const geo::Location se) {
    this->nw_corner = nw;
    this->se_corner = se;
    this->pixel_lat_size = flat_lat_distance(this->nw_corner, this->se_corner) / this->line_step_size;
    this->pixel_lon_size = flat_lon_distance(this->nw_corner, this->se_corner) / this->line_step_size;
    this->map = map;
}

const pixelPair sims2::LoSModel::gps_to_pixel(const geo::Location &pos) const {
    auto tmp_lat = geo::Location{pos.latitude, this->nw_corner.longitude};
    auto lat_distance = geo::distance_between(tmp_lat, this->nw_corner);

    auto tmp_lon = geo::Location{this->nw_corner.latitude, pos.longitude};
    auto lon_distance = geo::distance_between(tmp_lon, this->nw_corner);

    return pixelPair(static_cast<int>(std::round(lat_distance / this->pixel_lat_size)),
                     static_cast<int>(std::round(lon_distance / this->pixel_lon_size)));
}

const double sims2::LoSModel::pathloss_formula(const double percentage) const {
    return 0;
}

const double sims2::LoSModel::compute(const geo::Location &pos1, const geo::Location &pos2) const {
    pixelPair pixel_pos1{}, pixel_pos2{};

    if (geo::distance_between(pos1, this->nw_corner) < geo::distance_between(pos2, this->nw_corner)) {
        pixel_pos1 = this->gps_to_pixel(pos1);
        pixel_pos2 = this->gps_to_pixel(pos2);
    } else {
        pixel_pos1 = this->gps_to_pixel(pos2);
        pixel_pos2 = this->gps_to_pixel(pos1);
    }

    auto bearing = line_bearing(pixel_pos1, pixel_pos2);
    auto count = 0;
    auto total_steps = static_cast<unsigned int>(std::round(geo::distance_between(pos1, pos2) / this->line_step_size));

    for (auto i = 1; i <= total_steps; i++) {

        pixelPair dot{
                static_cast<unsigned int>(std::round(
                        pixel_pos1.x + (i * (this->line_step_size / this->pixel_lat_size)))),
                static_cast<unsigned int>(std::round(
                        pixel_pos1.y + (bearing * (i * (this->line_step_size / this->pixel_lon_size)))))};

        auto color = this->map.get_pixel(dot.x, dot.y);
        if (color == MAP_BUILDING_RGB) {
            count++;
        }
    }

    return (total_steps / 100) * count;
}


sims2::BitMap::BitMap(const char *file_path) {
    this->read_from_file(file_path);
}

const RGB sims2::BitMap::get_pixel(const unsigned long x, const unsigned long y) const {
    return this->img[x][y];
}


void sims2::BitMap::read_from_file(const char *file_path) {
    std::ifstream file(file_path, std::ios::binary);

    /* fill header with default value 0, in case another read is necessary */
    this->header.fill('0');

    /* read the header */
    file.read(this->header.data(), this->header.size());

//    auto fileSize = *reinterpret_cast<uint32_t *>(&header[2]);
//    auto dataOffset = *reinterpret_cast<uint32_t *>(&header[10]);
    this->width = *reinterpret_cast<uint32_t *>(&header[18]);
    this->height = *reinterpret_cast<uint32_t *>(&header[22]);
//    auto bit_pr_pixel = *reinterpret_cast<uint16_t *>(&header[28]);

    /* resize the data vectors */
    this->img.resize(this->height, std::vector<RGB>(this->width));
    auto data_size = this->width * 3 + 3;

    for (auto j = this->height - 1; j >= 0; j--) {
        /* because of unsigned int's, j < 0 is not possible */
        if (j > this->height)
            break;

        std::vector<char> img_data(data_size);
        file.read(img_data.data(), data_size);

        for (auto i = data_size - 4, i_lookup = this->width - 1; i >= 0; i -= 3, i_lookup--) {
            /* because of unsigned int's, i < 0 is not possible */
            if (i > data_size)
                break;

            /* add the RGB object to data vectors */
            this->img.at(j).at(i_lookup) = RGB(img_data[i + 2], img_data[i + 1], img_data[i]);
        }
    }
    file.close();
}

void sims2::BitMap::write_to_file(const char *file_path) {
    std::ofstream file(file_path, std::ios::binary);
    unsigned long padding = 0;

    file.write(this->header.data(), this->header.size());

    for (auto i = this->img.size() - 1; i >= 0; i--) {
        if (i > this->img[i].size())
            break;

        auto size = this->img[i].size();
        for (auto j = size - 1; j >= 0; j--) {
            if (j > size)
                break;

            /*std::array<unsigned int, 4> tmp_vec{padding,
                                                this->img[i][j].red,
                                                this->img[i][j].green,
                                                this->img[i][j].blue
            };*/
            std::array<unsigned long, 4> tmp_vec{this->img[i][j].blue,
                                                 this->img[i][j].green,
                                                 this->img[i][j].red,
                                                 padding
            };

            file.write(reinterpret_cast<const char *>(tmp_vec.data()), tmp_vec.size());
        }
//        file.write(reinterpret_cast<char *>(&tmp_vec), tmp_vec.size());
    }
    file.close();
}

void sims2::BitMap::set_pixel(unsigned long x, unsigned long y, const RGB color) {
    this->img[x][y] = color;
}
#endif