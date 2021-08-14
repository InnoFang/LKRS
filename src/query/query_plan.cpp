//
// Created by InnoFang on 2021/7/2.
//

#include "query/query_plan.hpp"

query_plan::query_plan() {

}

query_plan::query_plan(const std::vector<std::string>& variables, const std::vector<Triple>& triples)
    : variables_(variables), triples_(triples) {

}

query_plan::~query_plan() {

}

void query_plan::setVariables(std::vector<std::string> &variables) {
    variables_ = variables;
}

void query_plan::setTriples(std::vector<Triple> &triples) {
    triples_ = triples;
}

void query_plan::generate() {

}

std::vector<std::vector<std::string>> query_plan::execute() {
    std::vector<std::vector<std::string>> result;
    return result;
}
