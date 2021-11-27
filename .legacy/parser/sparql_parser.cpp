//
// Created by InnoFang on 2021/6/10.
//

#include <iostream>

#include "parser/sparql_parser.hpp"

SparqlParser::SparqlParser() {}

SparqlParser::SparqlParser(const std::string& sparql): distinct_(false) {
    pattern_ = std::regex(R"(SELECT\s+(DISTINCT)?(.*)[\s]?WHERE\s*\{([^}]+)\})", std::regex::icase);
    parse(sparql);
}

SparqlParser::~SparqlParser() {
};

void SparqlParser::parse(const std::string &sparql) {
    std::smatch match;
    if (std::regex_search(sparql, match, pattern_)) {
        distinct_ = !match.str(1).empty();
        catchVariables(match.str(2));
        catchTriples(match.str(3));
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
    uint64_t init = 1;
    auto encode_var = [&](std::string& var) {
        if (!var2id.count(var)) {
            var2id[var] = init;
            id2var[init] = var;
            init <<= 1;
        }
        return var2id[var];
    };

    std::regex sep("\\.\\s+");
    std::sregex_token_iterator tokens(raw_triple.cbegin(), raw_triple.cend(), sep, -1);
    std::sregex_token_iterator end;

    std::string s, p, o;
    for(; tokens != end; ++ tokens) {
        std::istringstream iss(*tokens);
        iss >> s >> p >> o;
        gPSO::triplet triplet{s, p, o};
        triples_.emplace_back(triplet);

        /* encode var into id*/
        uint64_t code = 0;
        if (s[0] == '?') code |= encode_var(s);
        if (p[0] == '?') code |= encode_var(p);
        if (o[0] == '?') code |= encode_var(o);
        triple2queryId[triplet] = code;
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

uint64_t SparqlParser::mapTripletIdBy(gPSO::triplet &triplet_) {
    return triple2queryId[triplet_];
}

