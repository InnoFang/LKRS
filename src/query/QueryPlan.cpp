//
// Created by InnoFang on 2021/7/2.
//

#include "query/QueryPlan.hpp"

QueryPlan::QueryPlan() {

}

QueryPlan::QueryPlan(const std::vector<std::string>& variables, const std::vector<Triple>& triples)
    : variables_(variables), triples_(triples) {

}

QueryPlan::~QueryPlan() {

}

void QueryPlan::setVariables(std::vector<std::string> &variables) {
    variables_ = variables;
}

void QueryPlan::setTriples(std::vector<Triple> &triples) {
    triples_ = triples;
}

void QueryPlan::generate() {

}

std::vector<std::vector<std::string>> QueryPlan::execute() {
    std::vector<std::vector<std::string>> result;
    return result;
}
