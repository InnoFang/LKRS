//
// Created by InnoFang on 2021/6/30.
//

#include "query/SPARQLQuery.hpp"
#include <iostream>

SPARQLQuery::SPARQLQuery(std::string& dbname): dbname_(dbname) {
    // TODO: check database existence first
    psoDB_ = new Database(dbname);
    psoDB_->load();
}

SPARQLQuery::~SPARQLQuery() {
    delete psoDB_;
}

std::vector<std::vector<std::string>> SPARQLQuery::query(SPARQLParser& parser) {
    QueryPlan queryPlan = generateQueryPlan(parser.getQueryVariables(), parser.getQueryTriples());
    return execute(queryPlan);
}

QueryPlan SPARQLQuery::generateQueryPlan(const std::vector<std::string>& variables, const std::vector<Triple>& triples) {

    auto decode_pso = [&](uint64_t pso_) {
        uint64_t sid = (pso_ & psoDB_->getSMask()) >> (psoDB_->getSOHexLength() << 2);
        uint64_t pid = (pso_ & psoDB_->getPMask()) >> (psoDB_->getSOHexLength() << 3);
        uint64_t oid = (pso_ & psoDB_->getOMask());

        return std::vector<std::string> { psoDB_->getSObyId(sid),
                                          psoDB_->getPbyId(pid),
                                          psoDB_->getSObyId(oid) };
    };

    // <pso, qualified range of PSO corresponding predicate>
    std::unordered_map<uint64_t, std::vector<uint64_t>> qualified_range_;
    // select the range of predicate corresponding PSO values
    std::vector<uint64_t> pso_data = psoDB_->getPSO();

    auto predicate_indices = psoDB_->getPredicateIndices();
    auto predicate_range = psoDB_->getPredicateRange();

    std::vector<std::pair<uint64_t, uint64_t>> pso_mask_pairs;
    for (const auto &triple : triples) {
        uint64_t pso, mask;
        generatePSOandMask(triple, pso, mask);
        pso_mask_pairs.emplace_back(pso, mask);
        auto p_index = (pso & psoDB_->getPMask()) >> (psoDB_->getSOHexLength() << 3);
        std::cout << "p_index: " << p_index << std::endl;
        int range_start = predicate_range[p_index];
        int range_end = predicate_range[p_index + 1];
        std::cout << "range: " << range_start << ", " << range_end << std::endl;
        std::vector<uint64_t> p_range(predicate_indices[p_index]);
        auto ti = std::copy_if(pso_data.begin() + range_start, pso_data.begin() + range_end, p_range.begin(),
                     [&](uint64_t val){
            return ((val & mask) == pso);
        });
        p_range.resize(std::distance(p_range.begin(), ti));
        qualified_range_[pso] = p_range;
    }

//    for (const auto& [pso, range_] : qualified_range_) {
//        std::cout << "PSO: " << pso << "\t the size of query result: " << range_.size() << std::endl;
//        auto s_p_o = decode_pso(pso);
//        std::cout << s_p_o[0] << '\t' << s_p_o[1] << "\t" << s_p_o[2] << std::endl;
//        std::cout << "\nQualified PSOs" << std::endl;
//        for (const auto &item : range_) {
//            std::cout << "PSO: " << item << std::endl;
//            auto s_p_o = decode_pso(item);
//            std::cout << s_p_o[0] << '\t' << s_p_o[1] << "\t" << s_p_o[2] << std::endl;
//        }
//    }

    QueryPlan queryPlan(variables, triples);

    return queryPlan;
}

std::vector<std::vector<std::string>> SPARQLQuery::execute(QueryPlan& queryPlan) {
    auto queryResult = queryPlan.execute();
    std::vector<std::vector<std::string>> ans;
    return ans;
}

void SPARQLQuery::generatePSOandMask(const Triple &triple, uint64_t &pso, uint64_t &mask) {
    pso = mask = 0;
    if (triple.p[0] != '?') {
        uint64_t pid = psoDB_->getIdByP(triple.p);
        pso |= pid << (2 * 4 * psoDB_->getSOHexLength());
        mask |= psoDB_->getPMask();
    }
    if (triple.s[0] != '?') {
        uint64_t sid = psoDB_->getIdBySO(triple.s);
        pso |= sid << (4 * psoDB_->getSOHexLength());
        mask |= psoDB_->getSMask();
    }
    if (triple.o[0] != '?') {
        uint64_t oid = psoDB_->getIdBySO(triple.o);
        pso |= oid;
        mask |= psoDB_->getOMask();
    }
//    std::cout << "P: " << triple.p << "\t" << "S: " << triple.s << "\t" << "O: " << triple.o << std::endl;
//    std::cout << "PSO: " << pso << std::endl;
//    std::cout << "mask: " << mask << std::endl;
//    std::cout << std::endl;
}
