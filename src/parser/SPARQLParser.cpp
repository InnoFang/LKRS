//
// Created by InnoFang on 2021/6/10.
//

#include <iostream>
#include "parser/SPARQLParser.hpp"

SPARQLParser::SPARQLParser(const std::string& sparql) {
    pattern_ = std::regex("SELECT\\s+(DISTINCT)?(.*)WHERE\\s*\\{([^}]+)\\}", std::regex::icase);
    parse(sparql);
}

SPARQLParser::~SPARQLParser() = default;

void SPARQLParser::parse(const std::string &sparql) {
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
        if (match.size() > 2) {
            distinct_ =  match.str(1) == "distinct";
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

void SPARQLParser::catchVariables(const std::string &raw_variable) {
    std::istringstream iss(raw_variable);
    using is_iter_str = std::istream_iterator<std::string>;
    auto beg = is_iter_str(iss);
    auto end = is_iter_str();
    variables_.assign(beg, end);
}

void SPARQLParser::catchTriples(const std::string &raw_triple) {
    std::regex sep("\\.\\s");
    std::sregex_token_iterator tokens(raw_triple.cbegin(), raw_triple.cend(), sep, -1);
    std::sregex_token_iterator end;
    for(; tokens != end; ++ tokens) {
        triples_.emplace_back(Triple(*tokens));
    }
}

std::vector<std::string> SPARQLParser::getQueryVariables() {
    return variables_;
}

std::vector<Triple> SPARQLParser::getQueryTriples() {
    return triples_;
}

bool SPARQLParser::isDistinct() {
    return distinct_;
}
