//
// Created by InnoFang on 2021/6/10.
//

#include <iostream>
#include "parser/SPARQLParser.hpp"

Parser::Parser() {
    pattern_ = std::regex("^SELECT\\s+(.*)\\s+WHERE\\s*\\{([^}]+)\\}$", std::regex::icase);
}

Parser::~Parser() = default;

void Parser::parse(std::string &sparql) {
    std::smatch match;
    if (std::regex_search(sparql, match, pattern_)) {
//        std::cout << "Match size = " << match.size() << std::endl;
//        std::cout << "Whole match : " << match.str(0) << std::endl;
//        std::cout << "First capturing group is '" << match.str(1)
//                  << "' which is captured at index " << match.position(1)
//                  << std::endl;
//        std::cout << "Second capturing group is '" << match.str(2)
//                  << "' which is captured at index " << match.position(2)
//                  << std::endl;
        catchVariables(match.str(1));
        catchTriples(match.str(2));
    } else {
        std::cerr << "No match is found." << std::endl;
    }
}

void Parser::catchVariables(const std::string &raw_variable) {
    std::istringstream iss(raw_variable);
    using is_iter_str = std::istream_iterator<std::string>;
    auto beg = is_iter_str(iss);
    auto end = is_iter_str();
    variables_.assign(beg, end);
}

void Parser::catchTriples(const std::string &raw_triple) {
    std::regex sep("\\.\\s");
    std::sregex_token_iterator tokens(raw_triple.cbegin(), raw_triple.cend(), sep, -1);
    std::sregex_token_iterator end;
    for(; tokens != end; ++ tokens) {
        triples_.emplace_back(Triple(*tokens));
    }
}

std::vector<std::string> Parser::getQueryVariables() {
    return variables_;
}

std::vector<Triple> Parser::getQueryTriples() {
    return triples_;
}