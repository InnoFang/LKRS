//
// Created by InnoFang on 2021/6/10.
//

#include <iostream>
#include "parser/sparql_parser.hpp"

SparqlParser::SparqlParser() {}

SparqlParser::SparqlParser(const std::string& sparql) {
    pattern_ = std::regex(R"(SELECT\s+(DISTINCT)?(.*)[\s]?WHERE\s*\{([^}]+)\})", std::regex::icase);
    parse(sparql);
}

SparqlParser::~SparqlParser() = default;

void SparqlParser::parse(const std::string &sparql) {
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

void SparqlParser::catchVariables(const std::string &raw_variable) {
    std::istringstream iss(raw_variable);
    using is_iter_str = std::istream_iterator<std::string>;
    auto beg = is_iter_str(iss);
    auto end = is_iter_str();
    variables_.assign(beg, end);
}

void SparqlParser::catchTriples(const std::string &raw_triple) {
    std::regex sep("\\.\\s");
    std::sregex_token_iterator tokens(raw_triple.cbegin(), raw_triple.cend(), sep, -1);
    std::sregex_token_iterator end;

    std::string s, p, o;
    for(; tokens != end; ++ tokens) {
        std::istringstream iss(*tokens);
        iss >> s >> p >> o;
        triples_.emplace_back(s, p, o);
    }
}

std::vector<std::string> SparqlParser::getQueryVariables() {
    return variables_;
}

std::vector<gPSO::triplet> SparqlParser::getQueryTriples() {
    return triples_;
}

bool SparqlParser::isDistinct() {
    return distinct_;
}
