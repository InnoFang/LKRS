//
// Created by InnoFang on 2021/6/25.
//

#ifndef RETRIEVE_SYSTEM_SPARQL_QUERY_H
#define RETRIEVE_SYSTEM_SPARQL_QUERY_H

#include <future>
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

using QueryPlan = std::vector<std::pair<gPSO::triplet, size_t>>;
using QueryQueue = std::deque<gPSO::triplet>;

class SparqlQuery {
public:
    double UsedTime;
    explicit SparqlQuery(std::string& dbname);
    ~SparqlQuery();
    vec_map_str_int query(SparqlParser& parser);
    QueryPlan preprocessing_async(const std::vector<gPSO::triplet>& triplets);
    QueryPlan preprocessing(const std::vector<gPSO::triplet>& triplets);
    QueryQueue rearrangeQueryPlan(QueryPlan& init_query_plan);
    vec_map_str_int execute(QueryQueue& intermediate_result);
    std::vector<std::unordered_map<std::string, std::string>> mapQueryResult(vec_map_str_int& query_result);

private:
    std::string sparql_;
    std::string dbname_;
    SparqlParser parser;
    Database psoDB_;
    std::unordered_map<gPSO::triplet, vec_map_str_int, gPSO::triplet_hash> subquery_results_;
};

#endif //RETRIEVE_SYSTEM_SPARQL_QUERY_H
