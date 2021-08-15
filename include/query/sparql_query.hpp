//
// Created by InnoFang on 2021/6/25.
//

#ifndef RETRIEVE_SYSTEM_SPARQL_QUERY_H
#define RETRIEVE_SYSTEM_SPARQL_QUERY_H

#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <unordered_map>
#include "common/triplet.hpp"
#include "parser/sparql_parser.hpp"
#include "database/database.hpp"
#include "query_plan.hpp"

class sparql_query {
public:
    explicit sparql_query(std::string& dbname);
    ~sparql_query();
    std::vector<std::vector<std::string>> query(sparql_parser& parser);
    query_plan generateQueryPlan(const std::vector<std::string>& variables, const std::vector<gPSO::triplet>& triples);

private:
    std::string sparql_;
    std::string dbname_;
    Database psoDB_;
    std::vector<std::vector<std::string>> execute(query_plan& queryPlan);
    void generatePSOandMask(const gPSO::triplet &triplet, uint64_t& pso, uint64_t& mask);
};

#endif //RETRIEVE_SYSTEM_SPARQL_QUERY_H
