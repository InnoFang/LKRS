//
// Created by InnoFang on 2021/6/30.
//

#include "query/sparql_query.hpp"
#include <iostream>

sparql_query::sparql_query(std::string& dbname): psoDB_(dbname) {
    psoDB_.load();
}

sparql_query::~sparql_query() = default;

vec_map_str_int sparql_query::query(sparql_parser& parser) {
    this->parser = parser;
    auto intermediate_results =
            preprocessing(parser.getQueryVariables(), parser.getQueryTriples());
    QueryPlan queryPlan = generateQueryPlan(intermediate_results);
    auto result = execute(queryPlan);
    return result;
}

QueryQueue
sparql_query::preprocessing(const std::vector<std::string> &variables, const std::vector<gPSO::triplet> &triplets) {
    QueryQueue ret(query_queue_cmp);

    for (const auto &triplet : triplets) {
        // get subject/predicate/object label respectively from triplet
        std::string s, p, o;
        std::tie(s, p, o) = triplet;

        uint64_t pso, mask;
        std::tie(pso, mask) = psoDB_.getVarPSOAndMask(triplet);

        vec_map_str_int single_query_match;

        std::vector<std::pair<uint64_t, uint64_t>> qualified_so_list = psoDB_.getQualifiedSOList(pso, mask);
        for (const auto &qualified_so : qualified_so_list) {
            uint64_t sid, oid;
            std::tie(sid, oid) = qualified_so;

            map_str_int item;
            if (sid != 0) {
                item[s] = sid;
            }
            if (oid != 0) {
                item[o] = oid;
            }

            single_query_match.emplace_back(item);
        }
        ret.push(single_query_match);
    }
    return ret;
}

QueryPlan sparql_query::generateQueryPlan(QueryQueue& query_queue) {

    QueryPlan queryPlan { query_queue.top() }; query_queue.pop();
    // temp_queue store the vec_map_str_int that cannot push into queryPlan immediately.
    QueryQueue temp_queue(query_queue_cmp);
    std::unordered_set<std::string> entities;
    for (const auto &item : queryPlan.front()[0]) {
        entities.insert(item.first);
    }

    while (!query_queue.empty()) {
        auto front = query_queue.top(); query_queue.pop();

        bool match = false;
        for (const auto &item : front[0]) {
            if (entities.find(item.first) != entities.end()) {
                match = true;
            }
        }
        if (match) {
            for (const auto &item : front[0]) {
                entities.insert(item.first);
            }
            queryPlan.emplace_back(front);
        } else {
            temp_queue.push(front);
        }
    }

    while (!temp_queue.empty()) {
        queryPlan.push_back(temp_queue.top());
        temp_queue.pop();
    }

    return queryPlan;
}

vec_map_str_int sparql_query::execute(QueryPlan &queryPlan) {
    auto join_query = [&](vec_map_str_int & mat1, vec_map_str_int& mat2) {
        vec_map_str_int ret;

        if (mat1.empty() || mat2.empty()) return ret;

        std::vector<std::string> join_variables;

        for (const auto &mat1_item : mat1[0]) {
            for (const auto &mat2_item : mat2[0]) {
                if (mat1_item.first == mat2_item.first) {
                    join_variables.emplace_back(mat1_item.first);
                }
            }
        }

        for (auto& mat1_map: mat1) {
            for (auto& mat2_map: mat2) {
                int match = 0;
                for (const auto &join_variable: join_variables) {
                    if (mat1_map[join_variable] == mat2_map[join_variable]) {
                        match ++;
                    } else break;
                }
                if (match == join_variables.size()) {
                    map_str_int tmp(mat1_map);
                    tmp.insert(mat2_map.begin(), mat2_map.end());
                    ret.emplace_back(tmp);
                }
            }
        }
        return ret;
    };

    // when the size of queryPlan larger than 1, that's mean contain join query
    while (queryPlan.size() > 1) {
        vec_map_str_int mat1 = queryPlan.front(); queryPlan.pop_front();
        vec_map_str_int mat2 = queryPlan.front(); queryPlan.pop_front();
        vec_map_str_int temp = join_query(mat1, mat2);
        if (temp.empty()) return {};
        queryPlan.emplace_front(temp);
    }

    // if the size of queryPlan is 1, that's mean the last result
    vec_map_str_int last = queryPlan.front();

    vec_map_str_int result;
    result.reserve(last.size());
    for (map_str_int &item: last) {
        map_str_int result_item;
        for (std::string &var : parser.getQueryVariables()) {
            result_item[var] = item[var];
        }
        result.emplace_back(result_item);
    }

    if (parser.isDistinct()) {
        vec_map_str_int::iterator pos = std::unique(result.begin(), result.end());
        result.erase(pos, result.end());
    }

    return result;
}

std::vector<std::unordered_map<std::string, std::string>> sparql_query::mapQueryResult(vec_map_str_int &query_result) {
    std::vector<std::unordered_map<std::string, std::string>> ret;
    ret.reserve(query_result.size());
    for (const map_str_int &row : query_result) {
        std::unordered_map<std::string, std::string> tmp;
        for (const auto &item: row) {
            tmp[item.first] = psoDB_.getSOByID(item.second);
        }
        ret.emplace_back(tmp);
    }
    return ret;
}
