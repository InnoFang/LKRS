/*
 * @FileName   : sparql_query.cpp 
 * @CreateAt   : 2021/10/14
 * @Author     : Inno Fang
 * @Email      : innofang@yeah.net
 * @Description: 
 */

#include "query/sparql_query.hpp"

#include <list>
#include <queue>
#include <algorithm>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <utility>

#include <spdlog/spdlog.h>

#include "common/utils.hpp"

namespace inno {

enum QueryType {
    FILTER_S,  // that's O is known, one variable no need to join, filter intermediate Result by S
    FILTER_O,  // that's S is known, one variable no need to join, filter intermediate Result by O
    FILTER_SO, // S and O isn't known, but the previous query variable set contains this S and O, filter by S and O
    JOIN_S,    // S is query variable, and O is not known
    JOIN_O,    // O is query variable, and S is not known
    FIRST_S,   // that's the first query triplet for the whole query statement
    FIRST_O,   // that's the first query triplet for the whole query statement
    FIRST_SO,  // that's the first query triplet for the whole query statement
};

using TempResult = std::vector<std::unordered_map<uint32_t, uint32_t>>;
using TripletId = std::tuple<uint32_t, uint32_t, uint32_t>;  // (Subject, Predicate, Object)
using QueryItem = std::tuple<TripletId, QueryType>;// (TripletId tuple, QueryType, Join/Filter Variable Id)
using QueryQueue = std::deque<QueryItem>;

class SparqlQuery::Impl {
public:
    explicit Impl(std::shared_ptr<DatabaseBuilder::Option> db)
        : db_(std::move(db)), var_idx_(0), query_time_(0) {
        using namespace std::placeholders;
        query_selector_.emplace(JOIN_S, std::bind(&SparqlQuery::Impl::join_s_, this, _1, _2));
        query_selector_.emplace(JOIN_O, std::bind(&SparqlQuery::Impl::join_o_, this, _1, _2));
        query_selector_.emplace(FILTER_S, std::bind(&SparqlQuery::Impl::filter_s_, this, _1, _2));
        query_selector_.emplace(FILTER_O, std::bind(&SparqlQuery::Impl::filter_o_, this, _1, _2));
        query_selector_.emplace(FILTER_SO, std::bind(&SparqlQuery::Impl::filter_so_, this, _1, _2));
    }

    ~Impl() = default;

    void initialize() {
        var_idx_ = 0;
        query_time_ = 0;
        var2id_.clear();
        id2var_.clear();
    }

    ResultSet query(SparqlParser &parser) {
        initialize();

        QueryQueue query_queue = generateQueryPlan(parser);

        TempResult result;
        std::tie(result, query_time_) =
                inno::timeit(std::bind(&SparqlQuery::Impl::execute, this, query_queue));

        return resultMapper(result, parser.getQueryVariables());
    }

    TripletId convert2TripletId(const std::string &s, const std::string &p, const std::string &o) {
        uint32_t pid = db_->getPredicateId(p);
        uint32_t sid, oid;
        if (s[0] == '?') {
            if (!var2id_.count(s)) {
                var2id_[s] = var_idx_;
                id2var_[var_idx_] = s;
                var_idx_++;
            }
            sid = var2id_[s];
        } else {
            sid = db_->getEntityId(s);
        }

        if (o[0] == '?') {
           if (!var2id_.count(o)) {
               var2id_[o] = var_idx_;
               id2var_[var_idx_] = o;
               var_idx_++;
           }
           oid = var2id_[o];
        } else {
           oid = db_->getEntityId(o);
        };

        return {sid, pid, oid};
    }

    QueryQueue generateQueryPlan(SparqlParser &parser) {
        auto triplet_list = parser.getQueryTriplets();
        std::string s, p, o;
        uint16_t sid, pid, oid;
//        if (triplet_list.size() == 1) {
//            std::tie(s, p, o) = triplet_list.back();
//
//            // if predicate is query variables,
//            if (p[0] == '?') {
//                spdlog::error("the 1st query triplet without predicate, cannot handle it!");
//                return  {};
//            }
//            bool is_s_var = s[0] == '?';
//            return { {convert2TripletId(db, s, p, o),
//                      is_s_var ? QueryType::FILTER_O : QueryType::FILTER_S,
//                      is_s_var ? sid : oid} };
//        }

        std::sort(triplet_list.begin(), triplet_list.end(),
                  [this](const SparqlParser::Triplet &a, const SparqlParser::Triplet &b) {
            return db_->getPredicateStatistic(std::get<1>(a)) < db_->getPredicateStatistic(std::get<1>(b));
        });

        // node set contains all query variables
        std::unordered_set<std::string> node_set;

        QueryQueue query_queue;

        std::tie(s, p, o) = triplet_list.front();
        if (p[0] == '?') {
            spdlog::error("the 1st query triplet without predicate, cannot handle it!");
            return {};
        }

        TripletId triplet_id = convert2TripletId(s, p, o);

        // join or filter variable id, that's use to join or filter when execute query
        QueryType type;
        if (s[0] == '?') {
            node_set.emplace(s);
            type = QueryType::FIRST_S;
        }
        if (o[0] == '?') {
            node_set.emplace(o);
            if (s[0] == '?') {
                // special case, s and o are both query variable,
                // which means the join_or_filter_var_id is not 0,
                // so left shift 8 bit so that can use bitwise OR to add oid;
                type = QueryType::FIRST_SO;
                spdlog::info("[0] FIRST_SO, size: {},  {} {} {}", db_->getPredicateStatistic(p), s, p, o);
            } else {
                type = QueryType::FIRST_O;
                spdlog::info("[0] FIRST_O, {} {} {}", db_->getPredicateStatistic(p), s, p, o);
            }
        } else {
            spdlog::info("[0] FIRST_S, {} {} {}", db_->getPredicateStatistic(p), s, p, o);
        }

        // if there is only one query triplet, use the above join_or_filter_var_id by default.
        // But if there are more than one query triplets, when query queue have been chosen the second
        // query triplet, the value of join_or_filter_var_id of the first query triplet should be updated.
        query_queue.emplace_back(triplet_id, type);
        triplet_list.erase(triplet_list.begin());

        std::vector<std::string> type_str {
            "FILTER_S",
            "FILTER_O",
            "FILTER_SO",
            "JOIN_S",
            "JOIN_O",
        };

        int idx = 1;
        while (!triplet_list.empty()) {
            auto curr = triplet_list.begin();
            while (curr != triplet_list.end()) {
                bool match = true;

                std::tie(s, p, o) = *curr;
                std::tie(sid, pid, oid) = convert2TripletId(s, p, o);

                if (p[0] == '?') {
                    spdlog::error("the query triplet({} {} {}) without predicate, cannot handle it!", s, p, o);
                    return {};
                }

                bool is_s_var = s[0] == '?';
                bool is_o_var = o[0] == '?';
                bool is_s_in_node_set = is_s_var && node_set.count(s);
                bool is_o_in_node_set = is_o_var && node_set.count(o);

                if (is_s_var && is_o_var) {
                    if (is_s_in_node_set && is_o_in_node_set) {
                        // special case, which have two var to filter.
                        type = QueryType::FILTER_SO;
                    } else if (is_s_in_node_set) {
                        type = QueryType::JOIN_S;
                        node_set.emplace(o);
                    } else if (is_o_in_node_set) {
                        type = QueryType::JOIN_O;
                        node_set.emplace(s);
                    } else {
                        match = false;
                    }
                } else if (is_s_var && is_s_in_node_set) {
                    type = QueryType::FILTER_S;
                } else if (is_o_var && is_o_in_node_set) {
                    type = QueryType::FILTER_O;
                } else {
                    match = false;
                }

                if (match) {
                    spdlog::info("[{}] {}, size: {},  {} {} {}", idx++, type_str[type], db_->getPredicateStatistic(p), s, p, o);
                    query_queue.emplace_back(convert2TripletId(s, p, o), type);
                    triplet_list.erase(curr);
                    break;
                } else {
                    ++curr;
                }
            }
        }

        return query_queue;
    }

    TempResult execute(QueryQueue &query_queue) {
        TempResult result;
        if (query_queue.empty()) {
            return result;
        }

        double time = 0;
        auto query_item = query_queue.front(); query_queue.pop_front();
        result = first_query_(query_item);
//        std::tie( result, time ) = inno::timeit(std::bind(&SparqlQuery::Impl::first_query_, this, query_item));
//        int idx = 0;
//        spdlog::info("[{}] result size: {}, used {} ms.", idx++, result.size(), time);

        while (!query_queue.empty() && !result.empty()) {
            query_item = query_queue.front(); query_queue.pop_front();
            result = query_selector_.at(std::get<1>(query_item))(result, query_item);
//           std::tie( result, time ) = inno::timeit( query_selector_.at(std::get<1>(query_item)), result, query_item);
//            spdlog::info("[{}] result size: {}, used {} ms.", idx++, result.size(), time);
        }
        return result;
    }

    ResultSet resultMapper(const TempResult &temp_result, const std::vector<std::string> &query_variables) {
        std::vector<uint16_t> query_ids;
        query_ids.reserve(query_variables.size());
        for (const auto &var : query_variables) {
            query_ids.emplace_back(var2id_[var]);
        }

        ResultSet result;
        for (auto &item : temp_result) {
//            std::unordered_map<std::string, std::string> result_item;
            std::vector<std::string> result_item;
            result_item.reserve(query_ids.size());
            for (auto &var_id : query_ids) {
                uint32_t entity_id = item.at(var_id);
                result_item.emplace_back(db_->getEntityById(entity_id));
//                result_item.emplace(id2var_[var_id], db_->getEntityById(entity_id));
            }
            result.insert(std::move(result_item));
        }
        return result;
    }

private:
    using ResultItemType = std::unordered_map<uint32_t, uint32_t>;

private:

    TempResult
    first_query_(const QueryItem &query_item) {
        TripletId tripletId;
        QueryType type;
        std::tie(tripletId, type) = query_item;

        uint32_t sid, pid, oid;
        std::tie(sid, pid, oid) = tripletId;

//        auto data = db_->getSOByP(pid);
        auto data = db_->getS2OByP(pid);

        TempResult result;
        result.reserve(data.size());

        if (type == QueryType::FIRST_S) {
            for (const auto &item : data) {
                if (oid == item.second) {
                    ResultItemType result_item;
                    result_item.emplace(sid, item.first);
                    result.push_back(std::move(result_item));
                }
            }
        } else if (type == QueryType::FIRST_O) {
            for (const auto &item : data) {
                if (sid == item.first) {
                    ResultItemType result_item;
                    result_item.emplace(oid, item.second);
                    result.push_back(std::move(result_item));
                }
            }
        } else {
            for (const auto &item : data) {
                ResultItemType result_item;
                result_item.emplace(sid, item.first);
                result_item.emplace(oid, item.second);
                result.push_back(std::move(result_item));
            }
        }

//        for (const auto &item : data) {
//            ResultItemType result_item;
//            if (type == QueryType::FIRST_S && oid == item.second) {
//                result_item.emplace(sid, item.first);
//            } else if (type == QueryType::FIRST_O && sid == item.first) {
//                result_item.emplace(oid, item.second);
//            } else {
//                result_item.emplace(sid, item.first);
//                result_item.emplace(oid, item.second);
//            }
//            result.push_back(std::move(result_item));
//        }

        return result;
    }

    TempResult
    join_s_(const TempResult &temp_result, const QueryItem &query_item) {
        TripletId tripletId;
        QueryType type;
        std::tie(tripletId, type) = query_item;

        uint32_t sid, pid, oid;
        std::tie(sid, pid, oid) = tripletId;

        auto data = db_->getS2OByP(pid);


        TempResult result;
        result.reserve(temp_result.size());
        for (const auto &item : temp_result) {
            auto range = data.equal_range(item.at(sid));
            for (auto it = range.first; it != range.second; ++it) {
                ResultItemType result_item = item;
                result_item.emplace(oid, it->second);
                result.emplace_back(std::move(result_item));
            }
        }

        return result;
    }

    TempResult
    join_o_(const TempResult &temp_result, const QueryItem &query_item) {
        TripletId tripletId;
        QueryType type;
        std::tie(tripletId, type) = query_item;

        uint32_t sid, pid, oid;
        std::tie(sid, pid, oid) = tripletId;
        auto data = db_->getO2SByP(pid);

        TempResult result;
        result.reserve(temp_result.size());
        for (const auto &item : temp_result) {
            auto range = data.equal_range(item.at(oid));
            for (auto it = range.first; it != range.second; ++it) {
                ResultItemType result_item = item;
                result_item.emplace(sid, it->second);
                result.emplace_back(result_item);
            }
        }
        return result;
    }

    TempResult
    filter_s_(const TempResult &temp_result, const QueryItem &query_item) {
        TripletId tripletId;
        QueryType type;
        std::tie(tripletId, type) = query_item;

        uint32_t sid, pid, oid;
        std::tie(sid, pid, oid) = tripletId;
        auto data = db_->getSByPO(pid, oid);

        TempResult result;
        result.reserve(temp_result.size());
        for (const auto &item : temp_result) {
            if (data.count(item.at(sid))) {
               result.emplace_back(item);
            }
        }
        return result;
    }

    TempResult
    filter_o_(const TempResult &temp_result, const QueryItem &query_item) {
        TripletId tripletId;
        QueryType type;
        std::tie(tripletId, type) = query_item;

        uint32_t sid, pid, oid;
        std::tie(sid, pid, oid) = tripletId;
        auto data = db_->getOBySP(sid, pid);

        TempResult result;
        result.reserve(temp_result.size());
        for (const auto &item : temp_result) {
            if (data.count(item.at(oid))) {
                result.emplace_back(item);
            }
        }
        return result;
    }

    TempResult
    filter_so_(const TempResult &temp_result, const QueryItem &query_item) {
        TripletId tripletId;
        QueryType type;
        std::tie(tripletId, type) = query_item;

        uint32_t sid, pid, oid;
        std::tie(sid, pid, oid) = tripletId;
//        auto data = db_->getSOByP(pid);

        ///////////


        auto data = db_->getS2OByP(pid);

        TempResult result;
        result.reserve(temp_result.size());
        for (const auto &item : temp_result) {
            auto range = data.equal_range(item.at(sid));
            for (auto it = range.first; it != range.second; ++it) {
                if (it->second == item.at(oid)) {
                    result.emplace_back(item);
                }
//                ResultItemType result_item(item.begin(), item.end());
//                result_item.emplace(std::get<2>(tripletId), it->second);
//                result.push_back(std::move(result_item));
            }
        }
        return result;
        //////////

//        TempResult result;
//        result.reserve(temp_result.size());
//        for (const auto &item : temp_result) {
//            if (data.count({item.at(sid), item.at(oid)})) {
//                result.emplace_back(item);
//            }
//        }
//        return result;
    }

public:
    double query_time_;

private:
    uint8_t var_idx_;
    std::unordered_map<uint16_t, std::string> id2var_;
    std::unordered_map<std::string, uint16_t> var2id_;
    std::unordered_map<QueryType, std::function<TempResult(TempResult const&, QueryItem const&)>> query_selector_;
    std::shared_ptr<DatabaseBuilder::Option> db_;
};

SparqlQuery::SparqlQuery(const std::shared_ptr<DatabaseBuilder::Option> &db) : impl_(new Impl(db)) { }

SparqlQuery::~SparqlQuery() { }

//ResultSet<std::string, std::string>
//std::vector<std::unordered_map<std::string, std::string>>
SparqlQuery::ResultSet
SparqlQuery::query(SparqlParser &parser) {
    return impl_->query(parser);
}

double SparqlQuery::getQueryTime() const {
    return impl_->query_time_;
}

}