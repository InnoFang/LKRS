//
// Created by InnoFang on 2021/6/19.
//

#include <iostream>
#include <string>
#include <chrono>
#include "database/database.hpp"

int main (int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "./psoBuild <db_name> <raw_rdf_file_path>" << std::endl;
        return 0;
    }

    std::string dbname = argv[1];
    std::string datafile = argv[2];
    std::cout << dbname << " " << datafile << std::endl;

    Database db(dbname);

    auto start_time = std::chrono::high_resolution_clock::now();

    db.create(datafile);

    auto stop_time = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> used_time = stop_time - start_time;

    std::cout << "Used time: " << used_time.count() << " ms." << std::endl;
    return 0;
}