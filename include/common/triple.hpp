//
// Created by InnoFang on 2021/6/7.
//

#ifndef PARSER_TRIPLE_HPP_
#define PARSER_TRIPLE_HPP_

#include <string>
#include <sstream>
#include <utility>
#include <vector>

namespace gPSO {
    using triplet = std::tuple<std::string, std::string, std::string>;
    using pso_triplet = std::tuple<uint64_t , uint64_t, uint64_t>;

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
