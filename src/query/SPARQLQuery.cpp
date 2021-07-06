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

std::vector<std::vector<std::string>> SPARQLQuery::query(SPARQLParser& parser) {
    QueryPlan queryPlan = generateQueryPlan(parser.getQueryVariables(), parser.getQueryTriples());
    return execute(queryPlan);
}

QueryPlan SPARQLQuery::generateQueryPlan(const std::vector<std::string>& variables, const std::vector<Triple>& triples) {
    for (const auto &triple : triples) {
        uint64_t pso, mask;
        generatePSOandMask(triple, pso, mask);
    }

    QueryPlan queryPlan(variables, triples);

    return queryPlan;
}

std::vector<std::vector<std::string>> SPARQLQuery::execute(QueryPlan& queryPlan) {
    auto queryResult = queryPlan.execute();
    std::vector<std::vector<std::string>> ans;
    return ans;
}

void SPARQLQuery::generatePSOandMask(const Triple &triple, uint64_t &pso, uint64_t &mask) {
    pso = mask = 0;
    if (triple.p[0] != '?') {
        uint64_t pid = psoDB_->getIdByP(triple.p);
        pso |= pid << (2 * 4 * psoDB_->getSOHexLength());
        mask |= psoDB_->getPMask();
    }
    if (triple.s[0] != '?') {
        uint64_t sid = psoDB_->getIdBySO(triple.s);
        pso |= sid << (4 * psoDB_->getSOHexLength());
        mask |= psoDB_->getSMask();
    }
    if (triple.o[0] != '?') {
        uint64_t oid = psoDB_->getIdBySO(triple.o);
        pso |= oid;
        mask |= psoDB_->getOMask();
    }
//    std::cout << "P: " << triple.p << "\t" << "S: " << triple.s << "\t" << "O: " << triple.o << std::endl;
//    std::cout << "PSO: " << pso << std::endl;
//    std::cout << "mask: " << mask << std::endl;
//    std::cout << std::endl;
}
