//
// Created by InnoFang on 2021/6/19.
//

#include <iostream>
#include <fstream>
#include <string>
#include "database/database.hpp"

static const auto io_speed_up = [] {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
    return 0;
}();

int main (int argc, char* argv[]) {
    const std::string dbname = argv[1];
    const std::string datafile = argv[2];
    std::cout << dbname << " " << datafile << std::endl;
    Database database(dbname, datafile);
    database.load(dbname);
    return 0;
}