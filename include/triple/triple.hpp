//
// Created by InnoFang on 2021/6/7.
//

#ifndef SIMPLE_SPARQL_PARSER_TRIPLE_HPP
#define SIMPLE_SPARQL_PARSER_TRIPLE_HPP

#include <string>
#include <sstream>

class Triple {
public:
    std::string s;
    std::string p;
    std::string o;
    explicit Triple(const std::string& raw_triple);
    ~Triple();
    Triple(const Triple& triple);
};


Triple::Triple(const std::string& raw_triple) {
    std::istringstream iss(raw_triple);
    iss >> s >> p >> o;
}

Triple::~Triple() = default;

Triple::Triple(const Triple &triple): s(triple.s), p(triple.p), o(triple.o) {}

#endif //SIMPLE_SPARQL_PARSER_TRIPLE_HPP
