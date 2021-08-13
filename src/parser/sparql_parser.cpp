//
// Created by InnoFang on 2021/6/10.
//

#include <iostream>
#include "parser/sparql_parser.hpp"

sparql_parser::sparql_parser(const std::string& sparql) {
    pattern_ = std::regex("SELECT\\s+(DISTINCT)?(.*)WHERE\\s*\\{([^}]+)\\}", std::regex::icase);
    parse(sparql);
}

sparql_parser::~sparql_parser() = default;

void sparql_parser::parse(const std::string &sparql) {
    std::smatch match;
    if (std::regex_search(sparql, match, pattern_)) {
        if (match.size() > 2) {
            distinct_ = true;
            catchVariables(match.str(2));
            catchTriples(match.str(3));
        } else {
            distinct_ = false;
            catchVariables(match.str(1));
            catchTriples(match.str(2));
        }
    } else {
        std::cerr << "[SPARQL parser] cannot parse it as SPARQL." << std::endl;
    }
}

void sparql_parser::catchVariables(const std::string &raw_variable) {
    std::istringstream iss(raw_variable);
    using is_iter_str = std::istream_iterator<std::string>;
    auto beg = is_iter_str(iss);
    auto end = is_iter_str();
    variables_.assign(beg, end);
}

void sparql_parser::catchTriples(const std::string &raw_triple) {
    std::regex sep("\\.\\s");
    std::sregex_token_iterator tokens(raw_triple.cbegin(), raw_triple.cend(), sep, -1);
    std::sregex_token_iterator end;
    for(; tokens != end; ++ tokens) {
        triples_.emplace_back(Triple(*tokens));
    }
}

std::vector<std::string> sparql_parser::getQueryVariables() {
    return variables_;
}

std::vector<Triple> sparql_parser::getQueryTriples() {
    return triples_;
}

bool sparql_parser::isDistinct() {
    return distinct_;
}
