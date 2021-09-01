#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <set>
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
    std::string sparql;
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
    SparqlQuery sparqlQuery(dbname);
    SparqlParser parser(sparql);

    auto result = sparqlQuery.query(parser);

    std::cout << "Used time: " << sparqlQuery.UsedTime << " ms." << std::endl;


    if (result.empty()) {
        std::cout << "[empty result]" << std::endl;
    } else {
        auto variables = parser.getQueryVariables();

        std::set<std::vector<uint64_t>> unique_result;
//        for (auto &row : sparqlQuery.mapQueryResult(result)) {
        for (auto &row : result) {
            std::vector<uint64_t> item;
            item.reserve(variables.size());
            for (auto &variable: variables) {
                item.emplace_back(row[variable]);
            }
            unique_result.insert( std::move(item) );
        }

        std::cout << unique_result.size() << " result(s)" << std::endl;
        std::copy(variables.begin(), variables.end(), std::ostream_iterator<std::string>(std::cout, "\t"));
        std::cout << std::endl;
        for (const std::vector<uint64_t> &row : unique_result) {
            for (const uint64_t &so_id : row) {
                std::cout << sparqlQuery.getSOById(so_id) << "\t";
            }
            std::cout << std::endl;
        }
    }

    return 0;
}