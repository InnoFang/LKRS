//
// Created by InnoFang on 2021/6/25.
//

#ifndef RETRIEVE_SYSTEM_SPARQL_QUERY_H
#define RETRIEVE_SYSTEM_SPARQL_QUERY_H

#include <deque>
#include <queue>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include "common/triplet.hpp"
#include "database/database.hpp"
#include "parser/sparql_parser.hpp"

using map_str_int = std::unordered_map<std::string, uint64_t>;
using vec_map_str_int = std::vector<std::unordered_map<std::string, uint64_t>>;

auto query_queue_cmp = [](vec_map_str_int& v1, vec_map_str_int& v2) { return v1.size() > v2.size(); };
using QueryQueue = std::priority_queue<vec_map_str_int, std::vector<vec_map_str_int>, decltype(query_queue_cmp)>;
using QueryPlan = std::deque<vec_map_str_int>;

class sparql_query {
public:
    explicit sparql_query(std::string& dbname);
    ~sparql_query();
    vec_map_str_int query(sparql_parser& parser);
    QueryQueue preprocessing(const std::vector<std::string>& variables, const std::vector<gPSO::triplet>& triplets);
    QueryPlan generateQueryPlan(QueryQueue& query_queue);
    vec_map_str_int execute(QueryPlan& queryPlan);
    std::vector<std::unordered_map<std::string, std::string>> mapQueryResult(vec_map_str_int& query_result);

private:
    std::string sparql_;
    std::string dbname_;
    sparql_parser parser;
    Database psoDB_;
};

#endif //RETRIEVE_SYSTEM_SPARQL_QUERY_H
