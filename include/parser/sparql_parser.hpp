/*
 * @FileName   : sparql_parser.hpp 
 * @CreateAt   : 2021/10/28
 * @Author     : Inno Fang
 * @Email      : innofang@yeah.net
 * @Description: 
 */

#ifndef RETRIEVE_SYSTEM_SPARQL_PARSER_HPP
#define RETRIEVE_SYSTEM_SPARQL_PARSER_HPP

#include <string>
#include <vector>

#include "common/triplet.hpp"

namespace inno {

class SparqlParser {
public:
    using Triplet = std::tuple<std::string, std::string, std::string>;
public:
    SparqlParser();
    ~SparqlParser();
    void parse(const std::string &sparql);
    std::vector<std::string> getQueryVariables();
    std::vector<std::string> getQueryVariables() const;
    std::vector<Triplet> getQueryTriplets();
    std::vector<Triplet> getQueryTriplets() const;
    std::vector<Triplet> getInsertTriplets();
    std::vector<Triplet> getInsertTriplets() const;
    uint64_t mapTripletIdBy(Triplet &triplet_);
    bool isDistinctQuery();
private:
    class Impl;
    std::shared_ptr<Impl> impl_;
};
}

#endif //RETRIEVE_SYSTEM_SPARQL_PARSER_HPP
