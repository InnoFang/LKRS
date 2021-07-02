//
// Created by InnoFang on 2021/6/30.
//

#include "query/SPARQLQuery.hpp"
#include <iostream>

SPARQLQuery::SPARQLQuery(const std::string& dbname): dbname_(dbname) {
    try {
        psoDB_ = new Database(dbname);
        psoDB_->load();
    } catch (const DBLoadException& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "please build db files first." << std::endl;
        exit(0);
    }
}

SPARQLQuery::~SPARQLQuery() {
    delete psoDB_;
}

std::vector<std::vector<std::string>> SPARQLQuery::query(const std::string& sparql) {
    SPARQLParser* parser = new SPARQLParser(sparql);
    QueryPlan queryPlan = generateQueryPlan(parser->getQueryVariables(), parser->getQueryTriples());
    auto ans = execute(queryPlan);
    delete parser;
    return ans;
}

QueryPlan SPARQLQuery::generateQueryPlan(const std::vector<std::string>& variables, const std::vector<Triple>& triples) {
    QueryPlan queryPlan(variables, triples);

    return queryPlan;
}

std::vector<std::vector<std::string>> SPARQLQuery::execute(QueryPlan& queryPlan) {
    auto queryResult = queryPlan.execute();
    std::vector<std::vector<std::string>> ans;
    return ans;
}
