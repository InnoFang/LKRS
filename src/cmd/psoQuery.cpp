#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include "parser/sparql_parser.hpp"
#include "database/database.hpp"
#include "query/sparql_query.hpp"

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
    sparql_query sparqlQuery(dbname);
    sparql_parser parser(sparql);

    auto start_time = std::chrono::high_resolution_clock::now();

    auto result = sparqlQuery.query(parser);

    auto stop_time = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> used_time = stop_time - start_time;

    if (result.empty()) {
        std::cout << "0 result" << std::endl;
    } else {
        std::cout << result.size() << " result(s)" << std::endl;
        auto variables = parser.getQueryVariables();
        std::copy(variables.begin(), variables.end(), std::ostream_iterator<std::string>(std::cout, "\t"));
        std::cout << std::endl;
        for (auto &row : sparqlQuery.mapQueryResult(result)) {
            for (auto &variable: variables) {
                std::cout << row[variable] << "\t";
            }
            std::cout << std::endl;
        }
    }

    std::cout << "Used time: " << used_time.count() << " ms." << std::endl;

    return 0;
}