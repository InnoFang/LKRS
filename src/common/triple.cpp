//
// Created by InnoFang on 2021/6/10.
//

#include "common/triple.hpp"

namespace gPSO {
    uint64_t encodePSO(uint64_t predicate, uint64_t subject, uint64_t object, int so_hex_len) {
        uint64_t pso = predicate;
        pso <<= (so_hex_len << 2);
        pso |= subject;
        pso <<= (so_hex_len << 2);
        pso |= object;
        return pso;
    }

    pso_triplet decodePSO(uint64_t pso, int so_hex_len) {
        uint64_t so_mask = (1 << (so_hex_len << 2)) - 1;
        uint64_t oid = (pso & so_mask);
        pso >>= (so_hex_len << 2);
        uint64_t sid = (pso & so_mask);
        pso >>= (so_hex_len << 2);
        uint64_t pid = pso;

        return { pid, sid, oid };
    };

    pso_triplet decodePSO(uint64_t pso, uint64_t so_mask, int so_hex_len) {
        uint64_t oid = (pso & so_mask);
        pso >>= (so_hex_len << 2);
        uint64_t sid = (pso & so_mask);
        pso >>= (so_hex_len << 2);
        uint64_t pid = pso;

        return { pid, sid, oid };
    };
}

Triplet::Triplet() = default;

Triplet::Triplet(std::string &s, std::string &p, std::string &o) {
    this->s = s;
    this->p = p;
    this->o = o;
}

Triplet::Triplet(const Triplet &triple): s(triple.s), p(triple.p), o(triple.o) {}

Triplet::Triplet(std::string &raw_triple) {
    std::istringstream iss(raw_triple);
    iss >> s >> p >> o;
}

Triplet::Triplet(std::tuple<std::string, std::string, std::string> &triplet) {
    std::tie(s, p, o) = triplet;
}

Triplet::~Triplet() = default;