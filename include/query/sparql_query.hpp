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
#include "common/triple.hpp"
#include "parser/sparql_parser.hpp"
#include "database/database.hpp"
#include "QueryPlan.hpp"

class sparql_query {
public:
    explicit sparql_query(std::string& dbname);
    ~sparql_query();
    std::vector<std::vector<std::string>> query(sparql_parser& parser);
    QueryPlan generateQueryPlan(const std::vector<std::string>& variables, const std::vector<Triple>& triples);

private:
    std::string sparql_;
    std::string dbname_;
    Database psoDB_;
    std::vector<std::vector<std::string>> execute(QueryPlan& queryPlan);
    void generatePSOandMask(const Triple& triple, uint64_t& pso, uint64_t& mask);
};

#endif //RETRIEVE_SYSTEM_SPARQL_QUERY_H
