//
// Created by InnoFang on 2021/6/10.
//
#include "parser/triple.hpp"

Triple::Triple(const std::string& raw_triple) {
    std::istringstream iss(raw_triple);
    iss >> s >> p >> o;
}

Triple::~Triple() = default;

Triple::Triple(const Triple &triple): s(triple.s), p(triple.p), o(triple.o) {}
