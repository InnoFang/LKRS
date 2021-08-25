//
// Created by InnoFang on 2021/6/30.
//

#include "query/sparql_query.hpp"
#include <iostream>
#include <chrono>

SparqlQuery::SparqlQuery(std::string& dbname): psoDB_(dbname), UsedTime(0) {
    psoDB_.load();
}

SparqlQuery::~SparqlQuery() = default;

vec_map_str_int SparqlQuery::query(SparqlParser& parser) {

    this->parser = parser;

    // preprocessing_async
    auto start_time = std::chrono::high_resolution_clock::now();

    QueryPlan init_query_plan = preprocessing(parser.getQueryTriples());

    auto stop_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> used_time = stop_time - start_time;
    std::cout << "preprocessing_async used time: " << used_time.count() << " ms." << std::endl;
    UsedTime += used_time.count();


    // rearranging query plan and generate query queue
    start_time = std::chrono::high_resolution_clock::now();

    QueryQueue query_queue = rearrangeQueryPlan(init_query_plan);

    stop_time = std::chrono::high_resolution_clock::now();
    used_time = stop_time - start_time;
    std::cout << "rearrangeQueryPlan used time: " << used_time.count() << " ms." << std::endl;
    UsedTime += used_time.count();


    // execute
    start_time = std::chrono::high_resolution_clock::now();

    auto result = execute(query_queue);

    stop_time = std::chrono::high_resolution_clock::now();
    used_time = stop_time - start_time;
    std::cout << "execute used time: " << used_time.count() << " ms." << std::endl;
    UsedTime += used_time.count();

    return result;
}

QueryPlan SparqlQuery::preprocessing_async(const std::vector<gPSO::triplet> &triplets) {
    subquery_results_.clear();
    subquery_results_.reserve(triplets.size());

//    // record the size of each triplets, use for rearranging query plan
    QueryPlan init_query_plan;
    init_query_plan.reserve(triplets.size());

    auto handleSubquery = [&](Database& db, const gPSO::triplet& triplet_) {
        return db.getQualifiedSOList(triplet_);
    };

    std::vector<std::shared_future<vec_map_str_int>> query_result;

    query_result.reserve(triplets.size());
    for (auto& t : triplets) {
        query_result.emplace_back(
                std::async(std::launch::async, handleSubquery, std::ref(psoDB_), std::ref(t)) );
    }

    for (int i = 0; i < query_result.size(); ++ i) {
        auto start_time = std::chrono::high_resolution_clock::now();

        vec_map_str_int sub_result = query_result[i].get();

        auto stop_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> used_time = stop_time - start_time;

        init_query_plan.emplace_back(triplets[i], sub_result.size());
        subquery_results_.emplace(triplets[i], std::move(sub_result));
        std::cout << i << " handle subquery used time: " << used_time.count() << " ms." << std::endl;
    }

    return init_query_plan;
}

QueryPlan SparqlQuery::preprocessing(const std::vector<gPSO::triplet> &triplets) {

    subquery_results_.reserve(triplets.size());

    // record the size of each triplets, use for rearranging query plan
    QueryPlan init_query_plan;
    init_query_plan.reserve(triplets.size());

    for (const auto &triplet : triplets) {

        vec_map_str_int subquery_result = psoDB_.getQualifiedSOList(triplet);
        // append <triplet, size> pair into init_query_plan
        init_query_plan.emplace_back(triplet, subquery_result.size());

        subquery_results_.emplace(triplet, std::move(subquery_result));
    }

    return init_query_plan;
}

QueryQueue SparqlQuery::rearrangeQueryPlan(QueryPlan& init_query_plan) {
    sort(init_query_plan.begin(), init_query_plan.end(),
         [&](std::pair<gPSO::triplet, size_t>& a, std::pair<gPSO::triplet, size_t>& b) {
             return a.second < b.second;
         });

    uint64_t node_set = 0;
    QueryQueue query_queue { init_query_plan.front().first };

    node_set |= parser.mapTripletIdBy(init_query_plan.front().first);
    init_query_plan.erase(init_query_plan.begin());

    while (!init_query_plan.empty()) {
        auto curr = init_query_plan.begin();
        bool match = false;
        while (curr != init_query_plan.end()) {
            match = ((node_set & parser.mapTripletIdBy(curr->first)) != 0);
            if (match) {
                node_set |= parser.mapTripletIdBy(curr->first);
                query_queue.push_back(curr->first);
                init_query_plan.erase(curr);
                break;
            } else {
                curr ++;
            }
        }
    }

    return query_queue;
}

vec_map_str_int SparqlQuery::execute(QueryQueue &query_queue) {
    auto join_query = [&](vec_map_str_int& mat1, vec_map_str_int& mat2) {
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

    gPSO::triplet front_triplet = query_queue.front(); query_queue.pop_front();
    vec_map_str_int result = subquery_results_[front_triplet];

    while (!query_queue.empty() && !result.empty()) {
        gPSO::triplet temp_triplet = query_queue.front(); query_queue.pop_front();
        vec_map_str_int temp = subquery_results_[temp_triplet];
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

