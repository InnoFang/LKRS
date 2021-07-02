//
// Created by InnoFang on 2021/7/2.
//

#ifndef RETRIEVE_SYSTEM_QUERYPLAN_HPP
#define RETRIEVE_SYSTEM_QUERYPLAN_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "common/triple.hpp"

class QueryPlan {
public:
    QueryPlan();
    QueryPlan(const std::vector<std::string>& variables, const std::vector<Triple>& triples);
    ~QueryPlan();
    void generate();
    std::vector<std::vector<std::string>> execute();
    void setVariables(std::vector<std::string>& variables);
    void setTriples(std::vector<Triple>& triples);

private:
    std::vector<std::string> variables_;
    std::vector<Triple> triples_;
};


#endif //RETRIEVE_SYSTEM_QUERYPLAN_HPP
