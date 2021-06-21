//
// Created by InnoFang on 2021/6/7.
//

#ifndef PARSER_TRIPLE_HPP_
#define PARSER_TRIPLE_HPP_

#include <string>
#include <sstream>

class Triple {
public:
    std::string s;
    std::string p;
    std::string o;
    Triple();
    Triple(const std::string& s, const std::string& p, const std::string& o);
    Triple(const std::string& raw_triple);
    ~Triple();
    Triple(const Triple& triple);
};

#endif //PARSER_TRIPLE_HPP_
