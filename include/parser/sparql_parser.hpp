//
// Created by InnoFang on 2021/6/7.
//

#ifndef PARSER_SPARQL_PARSER_HPP_
#define PARSER_SPARQL_PARSER_HPP_

#include <vector>
#include <string>
#include <regex>
#include "common/triplet.hpp"

class sparql_parser {
public:
    sparql_parser();
    explicit sparql_parser(const std::string &sparql);
    ~sparql_parser();
    std::vector<std::string> getQueryVariables();
    std::vector<gPSO::triplet> getQueryTriples();
    bool  isDistinct();

private:
    std::regex pattern_;
    std::string sparql_;
    std::vector<std::string> variables_;
    std::vector<gPSO::triplet> triples_;
    bool distinct_{};
    void parse(const std::string &sparql);
    void catchVariables(const std::string& raw_variable);
    void catchTriples(const std::string& raw_triple);
};

#endif //PARSER_SPARQL_PARSER_HPP_
