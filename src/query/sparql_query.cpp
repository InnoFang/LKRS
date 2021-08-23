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

    // generateQueryQueue
    auto start_time = std::chrono::high_resolution_clock::now();
    auto queryPlan = generateQueryQueue(parser.getQueryTriples());
    auto stop_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> used_time = stop_time - start_time;
    std::cout << "generateQueryQueue used time: " << used_time.count() << " ms." << std::endl;


    // execute
    start_time = std::chrono::high_resolution_clock::now();
    auto result = execute(queryPlan);
    stop_time = std::chrono::high_resolution_clock::now();
    used_time = stop_time - start_time;
    std::cout << "execute used time: " << used_time.count() << " ms." << std::endl;

    return result;
}

QueryQueue SparqlQuery::generateQueryQueue(const std::vector<gPSO::triplet> &triplets) {

    std::unordered_map<gPSO::triplet, vec_map_str_int, gPSO::triplet_hash> single_query_match_map;

    // record the size of each triplets, use for rearranging query plan
    std::vector<std::pair<gPSO::triplet, size_t>> triplet_size_list;

    for (const auto &triplet : triplets) {
        // get subject/predicate/object label respectively from triplet
        std::string s, p, o;
        std::tie(s, p, o) = triplet;

        uint64_t pso, mask;
        std::tie(pso, mask) = psoDB_.getVarPSOAndMask(triplet);

        vec_map_str_int single_query_match;

        std::vector<std::pair<uint64_t, uint64_t>> qualified_so_list = psoDB_.getQualifiedSOList(pso, mask);
        // append <triplet, size> pair into triplet_size_list
        triplet_size_list.emplace_back(triplet, qualified_so_list.size());

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

            single_query_match.emplace_back( std::move(item) );
        }
        single_query_match_map.emplace(triplet, std::move(single_query_match));
    }

    std::vector<gPSO::triplet> query_plan = rearrangeQueryPlan(triplet_size_list);

    QueryQueue ret;
    for (const auto &item : query_plan) {
        ret.emplace_back( single_query_match_map[item] );
    }
    return ret;
}

std::vector<gPSO::triplet> SparqlQuery::rearrangeQueryPlan(std::vector<std::pair<gPSO::triplet, size_t>>& triplet_size_list) {
    sort(triplet_size_list.begin(), triplet_size_list.end(),
         [&](std::pair<gPSO::triplet, size_t>& a, std::pair<gPSO::triplet, size_t>& b) {
             return a.second < b.second;
         });

    uint64_t node_set = 0;
    std::vector<gPSO::triplet> query_plan { triplet_size_list.front().first };
    node_set |= parser.mapTripletIdBy(triplet_size_list.front().first);
    triplet_size_list.erase(triplet_size_list.begin());

    while (!triplet_size_list.empty()) {
        auto curr = triplet_size_list.begin();
        bool match = false;
        while (curr != triplet_size_list.end()) {
            match = ((node_set & parser.mapTripletIdBy(curr->first)) != 0);
            if (match) {
                node_set |= parser.mapTripletIdBy(curr->first);
                query_plan.push_back(curr->first);
                triplet_size_list.erase(curr);
                break;
            } else {
                curr ++;
            }
        }
    }

    return query_plan;
}

vec_map_str_int SparqlQuery::execute(QueryQueue &query_queue) {
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

    vec_map_str_int result = query_queue.front(); query_queue.pop_front();

    while (!query_queue.empty() && !result.empty()) {
        vec_map_str_int temp = query_queue.front(); query_queue.pop_front();
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

