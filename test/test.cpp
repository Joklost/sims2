#include <catch2/catch.hpp>
//#include <Eigen/Core>
#include <iostream>
#include <chrono>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <geo/geomath.h>
#include <geo/location.h>
#include <sims2/datagen.h>
#include <sims2/constants.h>
#include <sims2/linkmodel.h>
//#include <sims2/losmodel.h>
#include <sims2/bitmap.h>

using namespace std::literals::chrono_literals;
using namespace geo::literals;

TEST_CASE("Compute the distance dependent path loss (double)", "[linkmodel]") {
    REQUIRE(sims2::Linkmodel::distance_pathloss(100) == Approx(91.2).margin(0.1));
    REQUIRE(sims2::Linkmodel::distance_pathloss(141.55) == Approx(99.5).margin(0.01));
}

TEST_CASE("gen map for testing", "[BitMap]") {
//    auto file_path = "/home/r5hej/playground/static_map/test_map.bmp";
    auto file_path = "test_maps/test_map.bmp";
    auto map = sims2::BitMap(file_path);
    for (auto x = 50; x <= 80; ++x) {
        for (auto y = 50; y <= 120; ++y) {
            map.set_pixel(x, y, sims2::RGB{255, 0, 0});
        }
    }

    for (auto x = map.get_width(); x >= map.get_width() - 10; --x) {
        for (auto y = map.get_height(); y >= map.get_height() - 100; --y) {
            map.set_pixel(x, y, sims2::RGB{255, 0, 0});
        }
    }

    map.write("/home/r5hej/playground/static_map/test_write_new.bmp");
}

TEST_CASE("Write pixel change in BitMap to new BitMap", "[BitMap]") {
//    auto s = realpath(".", NULL);
    auto file_path1 = "/home/r5hej/repos/masters/src/libs/sims2/test/test_maps/bitmap_set_pixel_1.bmp";
    auto map1 = sims2::BitMap(file_path1);

    auto file_path2 = "/home/r5hej/repos/masters/src/libs/sims2/test/test_maps/bitmap_set_pixel_2.bmp";
    auto map2 = sims2::BitMap(file_path2);

    /* Change pixels in the first map */
    for (auto x = 50; x <= 80; ++x) {
        for (auto y = 50; y <= 120; ++y) {
            map1.set_pixel(x, y, sims2::RGB{255, 0, 0});
        }
    }

    for (auto x = map1.get_width(); x >= map1.get_width() - 10; --x) {
        for (auto y = map1.get_height(); y >= map1.get_height() - 100; --y) {
            map1.set_pixel(x, y, sims2::RGB{255, 0, 0});
        }
    }

    /* Check if both bitmaps have the same pixels */
    bool is_equal1 = true;
    for (auto x = 50; x <= 80; ++x) {
        for (auto y = 50; y <= 120; ++y) {
            if (map1.get_pixel(x, y) != map2.get_pixel(x, y)) {
                is_equal1 = false;
                break;
            }
        }
    }

    bool is_equal2 = true;
    for (auto x = map1.get_width(); x >= map1.get_width() - 10; --x) {
        for (auto y = map1.get_height(); y >= map1.get_height() - 100; --y) {
            if (map1.get_pixel(x, y) != map2.get_pixel(x, y)) {
                is_equal2 = false;
                break;
            }
        }
    }
    REQUIRE(is_equal1);
    REQUIRE(is_equal2);
}

TEST_CASE("Compute Line of Sight pathloss", "[LoSModel]") {
    auto file_path = "/home/r5hej/playground/static_map/test_map.bmp";
    auto map = sims2::BitMap(file_path);
    auto pos1 = geo::Location{57.0182, 9.9723};
    auto pos2 = geo::Location{57.0106, 9.9948};

    auto losmodel = sims2::LoSModel(map, pos1, pos2);

    auto tpos1 = geo::Location{57.01482, 9.98137};
    auto tpos2 = geo::Location{57.01441, 9.98768};

    std::cout << losmodel.compute(tpos1, tpos2) << std::endl;
}