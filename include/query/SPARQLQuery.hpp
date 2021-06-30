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

class SPARQLQuery {
public:
    SPARQLQuery(const std::string& dbname);
    ~SPARQLQuery();
    std::vector<std::unordered_map<std::string, std::string>> query(const std::string& sparql);
private:
    std::string sparql_;
    std::string dbname_;
    Database *psoDB_;
    void generateQueryPlan(const std::vector<std::string>& variables, const std::vector<Triple>& triples);
    std::vector<std::unordered_map<std::string, std::string>> execute();
};

#endif //RETRIEVE_SYSTEM_SPARQL_QUERY_H
