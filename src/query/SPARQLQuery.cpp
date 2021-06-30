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

std::vector<std::unordered_map<std::string, std::string>> SPARQLQuery::query(const std::string& sparql) {
    SPARQLParser* parser = new SPARQLParser(sparql);
    generateQueryPlan(parser->getQueryVariables(), parser->getQueryTriples());
    auto ans = execute();
//
    delete parser;
    return ans;
}

void SPARQLQuery::generateQueryPlan(const std::vector<std::string>& variables, const std::vector<Triple>& triples) {

}

std::vector<std::unordered_map<std::string, std::string>> SPARQLQuery::execute() {
    return std::vector<std::unordered_map<std::string, std::string>>();
}
