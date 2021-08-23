//
// Created by InnoFang on 2021/6/7.
//

#ifndef PARSER_TRIPLE_HPP_
#define PARSER_TRIPLE_HPP_

#include <string>
#include <sstream>
#include <utility>
#include <vector>
#include <tuple>

namespace gPSO {
    /* S, P, O */
    using triplet = std::tuple<std::string, std::string, std::string>;
    /* P, S, O */
    using pso_triplet = std::tuple<uint64_t , uint64_t, uint64_t>;

    extern uint64_t encodePSO(uint64_t predicate, uint64_t subject, uint64_t object, int so_hex_len);

    extern pso_triplet decodePSO(uint64_t pso, int so_hex_len);

    extern pso_triplet decodePSO(uint64_t pso, uint64_t so_mask, int so_hex_len);

    struct triplet_hash {
        size_t operator() (const gPSO::triplet& t) const {
            std::string s, p, o;
            std::tie(s, p, o) = t;
            return std::hash<std::string>()(s + p + o);
        }
    };
}

#endif //PARSER_TRIPLE_HPP_