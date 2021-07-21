//
// Created by Inno Fang on 2021/7/21.
//

#ifndef RETRIEVE_SYSTEM_UTILS_HPP
#define RETRIEVE_SYSTEM_UTILS_HPP

#include <vector>

uint64_t encodePSO(const uint64_t& pid, const uint64_t& sid, const uint64_t& oid, const uint64_t& so_mask) {
    uint64_t pso = (pid << (so_mask << 3) | (sid << (so_mask << 2) | oid;
    return pso;
}

uint64_t decodePSO(const uint64_t& pso, const uint64_t& p_mask, const uint64_t& so_mask) const {
    uint64_t sid = (pso_ & so_mask) >> (so_mask << 2);
    uint64_t pid = (pso_ &  p_mask) >> (so_mask << 3);
    uint64_t oid = (pso_ & so_mask);

    return std::vector<uint64_t> { sid, pid, oid };
};

#endif //RETRIEVE_SYSTEM_UTILS_HPP
