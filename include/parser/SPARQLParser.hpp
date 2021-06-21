//
// Created by InnoFang on 2021/6/7.
//

#ifndef PARSER_SPARQL_PARSER_HPP_
#define PARSER_SPARQL_PARSER_HPP_

#include <vector>
#include <string>
#include <regex>
#include "common/triple.hpp"

class Parser {
public:
    Parser();
    ~Parser();
    void parse(std::string &sparql);
    std::vector<std::string> getQueryVariables();
    void catchVariables(const std::string& raw_variable);
    void catchTriples(const std::string& raw_triple);
    std::vector<Triple> getQueryTriples();

private:
    std::regex pattern_;
    std::string sparql_;
    std::vector<std::string> variables_;
    std::vector<Triple> triples_;
};

#endif //PARSER_SPARQL_PARSER_HPP_
