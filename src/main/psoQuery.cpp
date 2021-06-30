#include <iostream>
#include <fstream>
#include <string>
#include "parser/SPARQLParser.hpp"
#include "database/DBLoadException.hpp"
#include "database/database.hpp"

static const auto io_speed_up = [] {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
    return 0;
}();

std::string readSPARQLFromFile(const std::string& filepath) {
    std::ifstream infile(filepath, std::ios::in);
    std::ostringstream buf;
    std::string sparql = "";
    char ch;
    if (infile.is_open()) {
        while (buf && infile.get(ch)) {
            buf.put(ch);
        }
        sparql = buf.str();
    } else {
        std::cerr << "cannot open file: " << filepath << std::endl;
    }
    infile.close();
    return sparql;
}

int main(int argc, char** argv) {
    std::string dbname = argv[1];
    std::string query_file = argv[2];

    std::string sparql = readSPARQLFromFile(query_file);
    std::cout << "load data: " << dbname << "\n" << sparql << std::endl;

    Database database(dbname);
    try {
        database.load();
    } catch (const DBLoadException& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "please build db files first." << std::endl;
    }
    return 0;
}