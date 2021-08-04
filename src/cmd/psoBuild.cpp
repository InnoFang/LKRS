//
// Created by InnoFang on 2021/6/19.
//

#include <iostream>
#include <string>
#include <chrono>
#include "database/database.hpp"

//static const auto io_speed_up = [] {
//    std::istringstream::sync_with_stdio(false);
//    std::ostringstream::sync_with_stdio(false);
//    std::ifstream::sync_with_stdio(false);
//    std::ofstream::sync_with_stdio(false);
//    std::ios::sync_with_stdio(false);
//    std::cin.tie(nullptr);
//    std::cout.tie(nullptr);
//    std::fstream::sync_with_stdio(false);
//    return 0;
//}();

int main (int argc, char* argv[]) {
    const std::string dbname = argv[1];
    const std::string datafile = argv[2];
    std::cout << dbname << " " << datafile << std::endl;

    Database *db = nullptr;
    try {
        auto start_time = std::chrono::high_resolution_clock::now();

        db = new Database(dbname);
        db->create(datafile);

        double used_time = std::chrono::duration_cast<std::chrono::duration<double>>(
                std::chrono::high_resolution_clock::now() - start_time)
                .count();
        std::cout << "Used time: " << used_time << std::endl;
    } catch(std::exception &e) {
        delete db;
        return 0;
    }
    delete db;
    return 0;
}