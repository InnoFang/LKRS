//
// Created by InnoFang on 2021/6/7.
//

#ifndef PARSER_SPARQL_PARSER_HPP_
#define PARSER_SPARQL_PARSER_HPP_

#include <vector>
#include <string>
#include <regex>
#include <unordered_map>
#include "common/triplet.hpp"

class SparqlParser {
public:
    SparqlParser();
    explicit SparqlParser(const std::string &sparql);
    ~SparqlParser();
    std::vector<std::string> getQueryVariables();
    std::vector<gPSO::triplet> getQueryTriples();
    uint64_t mapTripletIdBy(gPSO::triplet& triplet_);
    bool  isDistinct();
private:
    std::regex pattern_;
    std::string sparql_;
    std::vector<std::string> variables_;
    std::vector<gPSO::triplet> triples_;
    std::unordered_map<gPSO::triplet, uint64_t, gPSO::triplet_hash> triple2queryId;
    std::unordered_map<uint64_t, std::string> id2var;
    std::unordered_map<std::string, uint64_t> var2id;
    bool distinct_{};
    void parse(const std::string &sparql);
    void catchVariables(const std::string& raw_variable);
    void catchTriples(const std::string& raw_triple);
};

#endif //PARSER_SPARQL_PARSER_HPP_
