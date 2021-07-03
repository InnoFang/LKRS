//
// Created by InnoFang on 2021/6/7.
//

#ifndef PARSER_SPARQL_PARSER_HPP_
#define PARSER_SPARQL_PARSER_HPP_

#include <vector>
#include <string>
#include <regex>
#include "common/triple.hpp"

class SPARQLParser {
public:
    SPARQLParser(const std::string &sparql);
    ~SPARQLParser();
    std::vector<std::string> getQueryVariables();
    std::vector<Triple> getQueryTriples();

private:
    std::regex pattern_;
    std::string sparql_;
    std::vector<std::string> variables_;
    std::vector<Triple> triples_;
    void parse(const std::string &sparql);
    void catchVariables(const std::string& raw_variable);
    void catchTriples(const std::string& raw_triple);
};

#endif //PARSER_SPARQL_PARSER_HPP_
