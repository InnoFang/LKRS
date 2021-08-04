#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include "parser/SPARQLParser.hpp"
#include "database/database.hpp"
#include "query/SPARQLQuery.hpp"

static const auto io_speed_up = [] {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
    return 0;
} ();

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
    SPARQLQuery sparqlQuery(dbname);
    SPARQLParser parser(sparql);

    auto start_time = std::chrono::high_resolution_clock::now();

    sparqlQuery.query(parser);

    double used_time = std::chrono::duration_cast<std::chrono::duration<double>>(
            std::chrono::high_resolution_clock::now() - start_time)
            .count();

    std::cout << "Used time: " << used_time << std::endl;

    return 0;
}