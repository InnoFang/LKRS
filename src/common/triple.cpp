//
// Created by InnoFang on 2021/6/10.
//
#include "common/triple.hpp"

Triple::Triple() {

}

Triple::Triple(const std::string &s, const std::string &p, const std::string &o) {
    this->s = s;
    this->p = p;
    this->o = o;
}

Triple::Triple(const std::string& raw_triple) {
    std::istringstream iss(raw_triple);
    iss >> s >> p >> o;
}

Triple::~Triple() = default;

Triple::Triple(const Triple &triple): s(triple.s), p(triple.p), o(triple.o) {}
