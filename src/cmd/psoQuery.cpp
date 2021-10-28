#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <set>
#include "parser/legacy/sparql_parser.hpp"
#include "database/legacy/database.hpp"
#include "query/legacy/sparql_query.hpp"
#include "query/sparql_query.hpp"

std::string readSPARQLFromFile(const std::string& filepath) {
    std::ifstream infile(filepath, std::ios::in);
    std::ifstream::sync_with_stdio(false);
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

void execute_query(SparqlQuery& sparqlQuery, const std::string& query_file) {

    std::string sparql = readSPARQLFromFile(query_file);
    SparqlParser parser(sparql);

    auto result = sparqlQuery.query(parser);

    std::cout << "Used time: " << sparqlQuery.UsedTime << " ms." << std::endl;

    if (result.empty()) {
        std::cout << "[empty result]" << std::endl;
    } else {
        auto variables = parser.getQueryVariables();

        std::set<std::vector<uint64_t>> unique_result;
        for (const auto &row : result) {
            std::vector<uint64_t> item;
            item.reserve(variables.size());
            for (auto &variable: variables) {
                item.emplace_back(row.at(variable));
            }
            unique_result.insert( std::move(item) );
        }

        std::cout << unique_result.size() << " result(s)" << std::endl;

        std::cout << "\n=============================================================\n";

        std::copy(variables.begin(), variables.end(), std::ostream_iterator<std::string>(std::cout, "\t"));
        std::cout << std::endl;
        for (const std::vector<uint64_t> &row : unique_result) {
            for (const uint64_t &so_id : row) {
                std::cout << sparqlQuery.getSOById(so_id) << "\t";
            }
            std::cout << std::endl;
        }
    }
}

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);

    if (argc == 1) {
        std::cout << "psoQuery <db_name> <query_file>" << std::endl;
        std::cout << "psoQuery <db_name>" << std::endl;
        return 1;
    }
    std::string dbname = argv[1];
    SparqlQuery sparqlQuery(dbname);
    if (argc >= 3) {
        std::string query_file = argv[2];
        execute_query(sparqlQuery, query_file);
    } else {
        for (;;) {
            std::cout << "\nquery >  ";
            std::string query_file;
            std::cin >> query_file;
            if (query_file == "exit" || query_file == "quit" || query_file == "q") {
                break;
            }
            execute_query(sparqlQuery, query_file);
        }
    }

    return 0;
}
// D:\Projects\Cpp\retrieve-system\data\lubm\lubm_q1.sql