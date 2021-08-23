//
// Created by InnoFang on 2021/6/30.
//

#include "query/sparql_query.hpp"
#include <iostream>
#include <chrono>

SparqlQuery::SparqlQuery(std::string& dbname): psoDB_(dbname) {
    psoDB_.load();
}

SparqlQuery::~SparqlQuery() = default;

vec_map_str_int SparqlQuery::query(SparqlParser& parser) {
    this->parser = parser;

    auto start_time = std::chrono::high_resolution_clock::now();

    // preprocessing
    auto intermediate_results =
            preprocessing(parser.getQueryTriples());

    auto stop_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> used_time = stop_time - start_time;
    std::cout << "preprocessing used time: " << used_time.count() << " ms." << std::endl;


    start_time = std::chrono::high_resolution_clock::now();

    // generateQueryPlan
    QueryPlan queryPlan = generateQueryPlan(intermediate_results);

    stop_time = std::chrono::high_resolution_clock::now();
    used_time = stop_time - start_time;
    std::cout << "generateQueryPlan used time: " << used_time.count() << " ms." << std::endl;


    start_time = std::chrono::high_resolution_clock::now();

    // execute
    auto result = execute(queryPlan);

    stop_time = std::chrono::high_resolution_clock::now();
    used_time = stop_time - start_time;
    std::cout << "execute used time: " << used_time.count() << " ms." << std::endl;

    return result;
}

QueryQueue SparqlQuery::preprocessing(const std::vector<gPSO::triplet> &triplets) {
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

QueryPlan SparqlQuery::generateQueryPlan(QueryQueue& query_queue) {

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

vec_map_str_int SparqlQuery::execute(QueryPlan &queryPlan) {
    auto join_query = [&](vec_map_str_int & mat1, vec_map_str_int& mat2) {
        vec_map_str_int ret;

        if (mat1.empty() || mat2.empty()) return ret;

        // find join variables between mat1 and mat2
        std::vector<std::string> join_variables;
        for (const auto &mat2_item : mat2[0]) {
            if (mat1[0].count(mat2_item.first)) {
                join_variables.emplace_back(mat2_item.first);
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

    vec_map_str_int result = queryPlan.front(); queryPlan.pop_front();

    while (!queryPlan.empty() && !result.empty()) {
        vec_map_str_int temp = queryPlan.front(); queryPlan.pop_front();
        result = join_query(result, temp);
    }

    if (parser.isDistinct()) {
        auto pos = std::unique(result.begin(), result.end());
        result.erase(pos, result.end());
    }

    return result;
}

std::vector<std::unordered_map<std::string, std::string>> SparqlQuery::mapQueryResult(vec_map_str_int &query_result) {
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
