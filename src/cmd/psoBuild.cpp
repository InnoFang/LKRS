/*
 * @FileName   : psoBuild.cpp
 * @CreateAt   : 2021/6/19
 * @Author     : Inno Fang
 * @Email      : innofang@yeah.net
 * @Description: command-line tool for handle database creation.
 */

#include <iostream>
#include <string>
#include <chrono>

#define LEGACY

#ifdef LEGACY
#include "database/legacy/database.hpp"
#else
#include "database/database.hpp"
#endif


int main (int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "./psoBuild <db_name> <raw_rdf_file_path>" << std::endl;
        return 0;
    }

    std::string dbname = argv[1];
    std::string datafile = argv[2];
    std::cout << dbname << " " << datafile << std::endl;

#ifdef LEGACY
    Database db(dbname);
#else
    inno::DatabaseBuilder db;
#endif
    auto start_time = std::chrono::high_resolution_clock::now();

#ifdef LEGACY
    db.create(datafile);
#else
    db.create(dbname, datafile);
//    auto opt = db.load(dbname);
//    opt->save(dbname + "_test");
#endif

    auto stop_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> used_time = stop_time - start_time;
    std::cout << "Used time: " << used_time.count() << " ms." << std::endl;
    return 0;
}