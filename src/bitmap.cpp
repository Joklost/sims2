#include <sims2/bitmap.h>

sims2::BitMap::BitMap(const char *fname) {
    this->read(fname);
}

void sims2::BitMap::read(const char *fname) {
    std::ifstream inp{fname, std::ios_base::binary};
    if (inp) {
        inp.read((char *) &this->file_header, sizeof(this->file_header));
        if (this->file_header.file_type != 0x4D42) {
            throw std::runtime_error("Error! Unrecognized file format.");
        }
        inp.read((char *) &this->bmp_info_header, sizeof(this->bmp_info_header));
        this->channels = this->bmp_info_header.bit_count / 8;

        // The sims2::BitMap::BMPColorHeader is used only for transparent images
        if (this->bmp_info_header.bit_count == 32) {
            // Check if the file has bit mask color information
            if (this->bmp_info_header.size >=
                (sizeof(sims2::BitMap::BMPInfoHeader) + sizeof(sims2::BitMap::BMPColorHeader))) {
                inp.read((char *) &this->bmp_color_header, sizeof(this->bmp_color_header));
                // Check if the pixel data is stored as BGRA and if the color space type is sRGB
                this->check_color_header(this->bmp_color_header);
            } else {
                std::cerr << "Error! The file \"" << fname
                          << "\" does not seem to contain bit mask information\n";
                throw std::runtime_error("Error! Unrecognized file format.");
            }
        }

        // Jump to the pixel data location
        inp.seekg(this->file_header.offset_data, inp.beg);

        // Adjust the header fields for output.
        // Some editors will put extra info in the image file, we only save the headers and the this->data.
        if (this->bmp_info_header.bit_count == 32) {
            this->bmp_info_header.size = sizeof(sims2::BitMap::BMPInfoHeader) + sizeof(sims2::BitMap::BMPColorHeader);
            this->file_header.offset_data =
                    sizeof(sims2::BitMap::BMPFileHeader) + sizeof(sims2::BitMap::BMPInfoHeader) +
                    sizeof(sims2::BitMap::BMPColorHeader);
        } else {
            this->bmp_info_header.size = sizeof(sims2::BitMap::BMPInfoHeader);
            this->file_header.offset_data = sizeof(sims2::BitMap::BMPFileHeader) + sizeof(sims2::BitMap::BMPInfoHeader);
        }
        this->file_header.file_size = this->file_header.offset_data;

        if (this->bmp_info_header.height < 0) {
            throw std::runtime_error(
                    "The program can treat only BMP images with the origin in the bottom left corner!");
        }

        this->data.resize(
                this->bmp_info_header.width * this->bmp_info_header.height * this->bmp_info_header.bit_count / 8);

        // Here we check if we need to take into account row padding
        if (this->bmp_info_header.width % 4 == 0) {
            inp.read((char *) this->data.data(), this->data.size());
            this->file_header.file_size += this->data.size();
        } else {
            this->row_stride = this->bmp_info_header.width * this->bmp_info_header.bit_count / 8;
            uint32_t new_stride = this->make_stride_aligned(4);
            std::vector<uint8_t> padding_row(new_stride - this->row_stride);

            for (int y = 0; y < this->bmp_info_header.height; ++y) {
                inp.read((char *) (this->data.data() + this->row_stride * y), this->row_stride);
                inp.read((char *) padding_row.data(), padding_row.size());
            }
            this->file_header.file_size += this->data.size() + this->bmp_info_header.height * padding_row.size();
        }
    } else {
        throw std::runtime_error("Unable to open the input image file.");
    }
    inp.close();
}

void sims2::BitMap::write(const char *fname) {
    std::ofstream of{fname, std::ios_base::binary};
    if (of) {
        if (this->bmp_info_header.bit_count == 32) {
            this->write_headers_and_data(of);
        } else if (this->bmp_info_header.bit_count == 24) {
            if (this->bmp_info_header.width % 4 == 0) {
                this->write_headers_and_data(of);
            } else {
                uint32_t new_stride = this->make_stride_aligned(4);
                std::vector<uint8_t> padding_row(new_stride - this->row_stride);

                this->write_headers(of);

                for (int y = 0; y < this->bmp_info_header.height; ++y) {
                    of.write((const char *) (this->data.data() + this->row_stride * y), this->row_stride);
                    of.write((const char *) padding_row.data(), padding_row.size());
                }
            }
        } else {
            throw std::runtime_error("The program can treat only 24 or 32 bits per pixel BMP files");
        }
    } else {
        throw std::runtime_error("Unable to open the output image file.");
    }
    of.close();
}

void sims2::BitMap::write_headers(std::ofstream &of) {
    of.write((const char *) &this->file_header, sizeof(this->file_header));
    of.write((const char *) &this->bmp_info_header, sizeof(this->bmp_info_header));
    if (this->bmp_info_header.bit_count == 32) {
        of.write((const char *) &bmp_color_header, sizeof(bmp_color_header));
    }
}

void sims2::BitMap::write_headers_and_data(std::ofstream &of) {
    this->write_headers(of);
    of.write((const char *) this->data.data(), this->data.size());
}

uint32_t sims2::BitMap::make_stride_aligned(uint32_t align_stride) {
    uint32_t new_stride = this->row_stride;
    while (new_stride % align_stride != 0) {
        new_stride++;
    }
    return new_stride;
}

void sims2::BitMap::check_color_header(sims2::BitMap::BMPColorHeader &bmp_color_header) {
    sims2::BitMap::BMPColorHeader expected_color_header;
    if (expected_color_header.red_mask != bmp_color_header.red_mask ||
        expected_color_header.blue_mask != bmp_color_header.blue_mask ||
        expected_color_header.green_mask != bmp_color_header.green_mask ||
        expected_color_header.alpha_mask != bmp_color_header.alpha_mask) {
        throw std::runtime_error(
                "Unexpected color mask format! The program expects the pixel data to be in the BGRA format");
    }
    if (expected_color_header.color_space_type != bmp_color_header.color_space_type) {
        throw std::runtime_error("Unexpected color space type! The program expects sRGB values");
    }
}

void sims2::BitMap::set_pixel(const uint32_t x, const uint32_t y, const sims2::RGB rgb) {
    auto loc_y = this->bmp_info_header.height - y;
    this->data[this->channels * (loc_y * this->bmp_info_header.width + x) + 0] = rgb.blue;
    this->data[this->channels * (loc_y * this->bmp_info_header.width + x) + 1] = rgb.green;
    this->data[this->channels * (loc_y * this->bmp_info_header.width + x) + 2] = rgb.red;
}

sims2::RGB sims2::BitMap::get_pixel(const uint32_t x, const uint32_t y) const {
    auto loc_y = this->bmp_info_header.height - y;
    return sims2::RGB{
            this->data[this->channels * (loc_y * this->bmp_info_header.width + x) + 2],
            this->data[this->channels * (loc_y * this->bmp_info_header.width + x) + 1],
            this->data[this->channels * (loc_y * this->bmp_info_header.width + x) + 0]
    };
}

const int32_t sims2::BitMap::get_height() const {
    return this->bmp_info_header.height;
}

const int32_t sims2::BitMap::get_width() const {
    return this->bmp_info_header.width;
}

