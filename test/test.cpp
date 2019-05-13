#include <catch2/catch.hpp>
#include <Eigen/Core>
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

TEST_CASE("Compute the linkmodel", "[linkmodel]") {
    /*geo::Location upper{57.01266813458001, 10.994625734716218};
    auto lower = geo::square(upper, 2_km);

    double high = 0.0;
    double low = 1000000.0;
    long double total = 0.0;

    for (auto i = 0; i < 30; ++i) {
        auto nodes = sims2::data::generate_nodes(20, upper, lower);
        auto links = sims2::data::create_links(nodes);

//        cholesky part
        auto corr = generate_correlation_matrix_slow(links);
        auto std_deviation = std::pow(11.4, 2);
        vecvec<double> sigma;

        auto n = corr[0].size();
        auto m = corr.size();
        sigma.resize(m, std::vector<double>(n));

        for (auto i = 0; i < m; ++i) {
            for (auto j = 0; j < n; ++j) {
                sigma[i][j] = corr[i][j] * std_deviation;
            }
        }

        auto l = slow_cholesky(sigma);
        auto t = std::vector<double>(l.size(), 1.0);
        auto chol_res = l * t;


        sims2::Linkmodel linkmodel{links};
        auto diff = linkmodel.compute(chol_res);
        if (diff > high)
            high = diff;
        else if (diff < low)
            low = diff;

        total += diff;
        std::cout
                << "difference: " << diff
                << ", link count: " << links.size()
                << ", average: " << diff / links.size()
                << std::endl;
//        std::cout << std::endl << "total difference: " << total << std::endl;
    }
    std::cout << std::endl
              << "highest: " << high
              << ", lowest: " << low
              << ", total: " << total
              << std::endl;*/

//    for (const auto &item : model) {
//        std::cout << item.second << std::endl;
//    }
}