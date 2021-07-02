//
// Created by InnoFang on 2021/6/25.
//

#ifndef RETRIEVE_SYSTEM_SPARQL_QUERY_H
#define RETRIEVE_SYSTEM_SPARQL_QUERY_H

#include <string>
#include <vector>
#include <unordered_map>
#include "common/triple.hpp"
#include "parser/SPARQLParser.hpp"
#include "database/database.hpp"
#include "database/DBLoadException.hpp"
#include "QueryPlan.hpp"

class SPARQLQuery {
public:
    SPARQLQuery(const std::string& dbname);
    ~SPARQLQuery();
    std::vector<std::vector<std::string>> query(const std::string& sparql);
private:
    std::string sparql_;
    std::string dbname_;
    Database *psoDB_;
    QueryPlan generateQueryPlan(const std::vector<std::string>& variables, const std::vector<Triple>& triples);
    std::vector<std::vector<std::string>> execute(QueryPlan& queryPlan);
};

#endif //RETRIEVE_SYSTEM_SPARQL_QUERY_H
