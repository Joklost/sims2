#include <cmath>
#include <fstream>

#include <sims2/losmodel.h>

const double line_bearing(const pixelPair &pos1, const pixelPair &pos2) {
    return (pos1.y - pos2.y) / (pos1.x - pos2.x);
}

const double flat_lat_distance(const geo::Location &pos1, const geo::Location &pos2) {
    auto tmp = geo::Location{pos2.latitude, pos1.longitude};
    return geo::distance_between(pos1, tmp);
}

const double flat_lon_distance(const geo::Location &pos1, const geo::Location &pos2) {
    auto tmp = geo::Location{pos1.latitude, pos2.longitude};
    return geo::distance_between(pos1, tmp);
}

sims2::losmodel::losmodel(const geo::Location nw, const geo::Location se) {
    // must take a image, and set state properties
    this->nw_corner = nw;
    this->se_corner = se;
    this->pixel_lat_size = flat_lat_distance(this->nw_corner, this->se_corner) / this->line_step_size;
    this->pixel_lon_size = flat_lon_distance(this->nw_corner, this->se_corner) / this->line_step_size;
}

const pixelPair sims2::losmodel::gps_to_pixel(const geo::Location &pos) const {
    auto tmp_lat = geo::Location{pos.latitude, this->nw_corner.longitude};
    auto lat_distance = geo::distance_between(tmp_lat, this->nw_corner);

    auto tmp_lon = geo::Location{this->nw_corner.latitude, pos.longitude};
    auto lon_distance = geo::distance_between(tmp_lon, this->nw_corner);

    return pixelPair(static_cast<int>(std::round(lat_distance / this->pixel_lat_size)),
                     static_cast<int>(std::round(lon_distance / this->pixel_lon_size)));
}

const double sims2::losmodel::pathloss_formula(const double percentage) const {
    return 0;
}

const double sims2::losmodel::compute(const geo::Location &pos1, const geo::Location &pos2) const {
    auto tmp_pos1 = this->gps_to_pixel(pos1);
    auto tmp_pos2 = this->gps_to_pixel(pos2);
    pixelPair pixel_pos1{}, pixel_pos2{};

    if (tmp_pos1 > tmp_pos2) {
        pixel_pos1 = tmp_pos2;
        pixel_pos2 = tmp_pos1;
    } else {
        pixel_pos1 = tmp_pos1;
        pixel_pos2 = tmp_pos2;
    }

    auto bearing = line_bearing(pixel_pos1, pixel_pos2);
    auto count = 1;
    std::vector<pixelPair> spots;
    while (true) {
        pixelPair spot{
                static_cast<int>(std::round(pixel_pos1.x + (count * (this->line_step_size / this->pixel_lat_size)))),
                static_cast<int>(std::round(
                        pixel_pos1.y + (bearing * (count * (this->line_step_size / this->pixel_lon_size)))))};

        if (spot > pixel_pos2)
            break;

//      look up the color of the pixel
    }

    return 0;
}


sims2::BitMap::BitMap(const char *file_path) {
    std::ifstream file(file_path, std::ios::binary);
    constexpr size_t HEADER_SIZE = 54;

    std::array<char, HEADER_SIZE> header{};
    file.read(header.data(), header.size());

    auto fileSize = *reinterpret_cast<uint32_t *>(&header[2]);
    auto dataOffset = *reinterpret_cast<uint32_t *>(&header[10]);
    auto width = *reinterpret_cast<uint32_t *>(&header[18]);
    auto height = *reinterpret_cast<uint32_t *>(&header[22]);
    auto bit_pr_pixel = *reinterpret_cast<uint16_t *>(&header[28]);

    this->img.resize(height, std::vector<RGB>(width));

//    auto dataSize = ((width * 3 + 3) & (~3)) * height;
    auto data_size = ((width * 3 + 3) & (~3));

    char temp = 0;
    for (auto j = height; j >= 0; j--) {
        std::vector<char> img_data(data_size);
        file.read(img_data.data(), data_size);

        for (auto i = data_size - 4; i >= 0; i -= 3) {
            this->img[j][i] = RGB(img_data[i + 2], img_data[i + 1], img_data[i]);

//            temp = img[i];
//            img[i] = img[i + 2];
//            img[i + 2] = temp;
//
//            std::cou << "R: " << int(img[i] & 0xff) << " G: " << int(img[i + 1] & 0xff) << " B: "
//                     << int(img[i + 2] & 0xff)
//                     << std::endl;
        }
    }

    file.close();
}
