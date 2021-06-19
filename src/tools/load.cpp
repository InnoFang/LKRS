//
// Created by InnoFang on 2021/6/19.
//

#include <iostream>
#include <fstream>
#include <string>

int main (int argc, char** argv) {
    std::string dbname = argv[1];
    std::string datafile = argv[2];
    std::cout << dbname << " " << datafile << std::endl;
    return 0;
}