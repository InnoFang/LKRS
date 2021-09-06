//
// Created by InnoFang on 2021/6/10.
//

#include "common/triplet.hpp"

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

        return std::make_tuple( pid, sid, oid );
    };

    pso_triplet decodePSO(uint64_t pso, uint64_t so_mask, int so_hex_len) {
        uint64_t oid = (pso & so_mask);
        pso >>= (so_hex_len << 2);
        uint64_t sid = (pso & so_mask);
        pso >>= (so_hex_len << 2);
        uint64_t pid = pso;

        return std::make_tuple( pid, sid, oid );
    };
}
