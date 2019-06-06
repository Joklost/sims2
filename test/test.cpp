#include <catch2/catch.hpp>
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
#include <sims2/losmodel.h>

using namespace std::literals::chrono_literals;
using namespace geo::literals;

TEST_CASE("Compute the distance dependent path loss (double)", "[linkmodel]") {
//    REQUIRE(sims2::Linkmodel::distance_pathloss(100) == Approx(91.2).margin(0.1));
//    REQUIRE(sims2::Linkmodel::distance_pathloss(141.55) == Approx(99.5).margin(0.01));
}

TEST_CASE("Change pixels in bitmap and confirm changes", "[BitMap]") {
    auto map_path = "map1.bmp";
    auto map = sims2::BitMap(map_path);

    /* Change pixels in the first map */
    for (auto x = 50; x <= 80; ++x) {
        for (auto y = 50; y <= 120; ++y) {
            map.set_pixel(x, y, sims2::RGB{255, 0, 0});
        }
    }
    for (auto x = map.get_width() - 122; x >= map.get_width() - 760; --x) {
        for (auto y = map.get_height() - 80; y >= map.get_height() - 300; --y) {
            map.set_pixel(x, y, sims2::RGB{255, 0, 0});
        }
    }

    /* check if maps are equal */
    auto comparison_map = sims2::BitMap("map1_modified.bmp");
    REQUIRE(map == comparison_map);
}

TEST_CASE("Lookup a model in index and compute building percentage", "[LoSModelIndex]") {
    std::string script_path = "./gen_map";
    std::string map_path = "./";
    auto pos1 = geo::Location{14.608507, 121.074975};
    auto pos2 = geo::Location{14.607126, 121.082250};

    auto index = sims2::LoSModelIndex{script_path, map_path};

    /* Get the desired model, in this case it will create a new since the index is empty */
    auto model = index.get_model(pos1, pos2);

    REQUIRE(model.compute(pos1, pos2) == Approx(52.6).margin(0.1));
}
