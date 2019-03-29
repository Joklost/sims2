#include <iostream>
#include <fstream>

#include <sims2/datagen.h>
#include <geo/geomath.h>
#include <http/httpclient.h>
#include <sims2/link.h>
#include <common/strings.h>
#include <sims/datagen.h>
using json = nlohmann::json;

void generate_nodes_data_file(const int count, geo::Location upper, geo::Location lower) {
    auto nodes = sims2::data::generate_nodes(count, upper, lower);
    std::fstream stream("../../../../../tools/linkmodel_visualiser/data/nodes", stream.out | stream.trunc);
    stream << "# node id; latitude; longitude" << std::endl;

    for (const auto &node : nodes) {
        stream
                << node.id << ";"
                << node.location.get_latitude() << ";"
                << node.location.get_longitude()
                << std::endl;
    }

    stream.close();
}

std::vector<sims2::Node> load_nodes_from_file(const std::string &file) {
    std::fstream stream(file, stream.in);
    std::vector<sims2::Node> nodes {};

    for (std::string line;  std::getline(stream, line);) {
        auto split = common::split(line, ";");
        nodes.emplace_back(sims2::Node {
           std::stoul(split[0]),
           geo::Location {std::stod(split[1]), std::stod(split[2])}
        });
    }
    stream.close();
    return nodes;
}


void write_result_to_file(const std::string &file) {
    std::fstream stream(file, std::ios::out | std::ios::trunc);


    stream.close();
}

int main(int argc, char *argv[]) {
    geo::Location upper{57.01266813458001, 10.994625734716218};
    auto lower = geo::square(upper, 2_km);
//    generate_nodes_data_file(20, upper, lower);

    auto nodes = load_nodes_from_file("../../../../../tools/linkmodel_visualiser/data/nodes");
    auto links = sims2::data::create_links(nodes);



    return 0;
}