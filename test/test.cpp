#include <catch2/catch.hpp>
//#include <Eigen/Core>
#include <iostream>
#include <chrono>

#include <geo/geomath.h>
#include <geo/location.h>
#include <sims2/datagen.h>
#include <sims2/constants.h>
#include <sims2/linkmodel.h>
#include <sims2/losmodel.h>

using namespace std::literals::chrono_literals;
using namespace geo::literals;

TEST_CASE("Compute the distance dependent path loss (double)", "[linkmodel]") {
    REQUIRE(sims2::Linkmodel::distance_pathloss(100) == Approx(91.2).margin(0.1));
    REQUIRE(sims2::Linkmodel::distance_pathloss(141.55) == Approx(99.5).margin(0.01));
}

TEST_CASE("Load BitMap", "[BitMap]") {
    auto file_path = "/home/r5hej/playground/static_map/map.bmp";
    auto map = sims2::BitMap(file_path);

}