//
// Created by InnoFang on 2021/6/19.
//

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
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
    auto start_time = std::chrono::high_resolution_clock::now();

    Database *db = new Database(dbname, datafile);

    double used_time = std::chrono::duration_cast<std::chrono::duration<double>>(
            std::chrono::high_resolution_clock::now() - start_time)
            .count();

    std::cout << "Used time: " << used_time << std::endl;

    delete db;

    return 0;
}