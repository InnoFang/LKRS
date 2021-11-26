﻿/*
 * @FileName   : psoHttp.cpp 
 * @CreateAt   : 2021/10/22
 * @Author     : Inno Fang
 * @Email      : innofang@yeah.net
 * @Description: 
 */

#include <iostream>
#include <string>
#include <unordered_set>
#include <set>

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <utility>

#include "query/sparql_query.hpp"

namespace opt = boost::program_options;
namespace fs = boost::filesystem;

using ResultSet = std::vector<std::unordered_map<std::string, std::string>>;

std::shared_ptr<inno::DatabaseBuilder::Option> db;
std::unique_ptr<inno::SparqlQuery> sparqlQuery;
inno::SparqlParser parser;

std::vector<std::string> listRDFdb() {
    std::vector<std::string> rdf_db_list;
    fs::path path(".");
    if (!fs::exists(path)) {
        return rdf_db_list;
    }

    fs::directory_iterator end_iter;
    for (fs::directory_iterator iter(path); iter!=end_iter; ++iter) {
        if (fs::is_directory(iter->status())) {
            std::string dir_name = iter->path().filename().string();
            auto idx = dir_name.find(".db");
            if (idx != std::string::npos) {
                rdf_db_list.emplace_back(dir_name.substr(0, idx));
            }
        }
    }

    return rdf_db_list;
}

ResultSet execute_query(std::string &sparql) {
    if (sparqlQuery == nullptr) {
        spdlog::error("database doesn't be loaded correctly.");
        return {};
    }

    parser.parse(sparql);

    auto result = sparqlQuery->query(parser);
    if (result.empty()) {
        return {};
    }

    ResultSet ret;

    auto variables = parser.getQueryVariables();
    ret.reserve(variables.size());

    for (const auto &row : result) {
        std::unordered_map<std::string, std::string> item;
        for (size_t i = 0; i < variables.size(); ++i) {
            std::string var = variables[i].substr(1); // exclude the first symbol '?'
            std::string entity = row[i].substr(1);    // exclude the first symbol '"'
            entity.pop_back();                             // remove the last symbol '"'
            item.emplace(var, entity);
        }
        ret.emplace_back(std::move(item));
    }

    return ret;
}

bool execute_insert(std::string &sparql) {
    parser.parse(sparql);
    bool status = db->insert(parser.getInsertTriplets());
    return status;
}

void list(const httplib::Request &req, httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    spdlog::info("Catch list request from http://{}:{}", req.remote_addr, req.remote_port);
    nlohmann::json j;
    j["data"] = listRDFdb();
    res.set_content(j.dump(2), "text/plain;charset=utf-8");
}

void info(const httplib::Request &req, httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    spdlog::info("Catch info request from http://{}:{}", req.remote_addr, req.remote_port);
    std::unordered_map<std::string, uint32_t> data;

    data["triplets"] = db->getTripletSize();
    data["predicates"] = db->getPredicateSize();
    data["entities"] = db->getEntitySize();

    nlohmann::json j;
    j["data"] = data;
    res.set_content(j.dump(2), "text/plain;charset=utf-8");
}

void visualize(const httplib::Request &req, httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    spdlog::info("Catch visualize request from http://{}:{}", req.remote_addr, req.remote_port);
    auto predicate_stat = db->getPredicateStatistics();
    std::unordered_map<uint32_t, nlohmann::json> node_map;
    std::set<std::pair<uint32_t, uint32_t>> edge_set;
    std::vector<nlohmann::json> nodes;
    std::vector<nlohmann::json> edges;

    std::vector<std::string> categories;
    for (size_t pid = 1; pid < predicate_stat.size(); pid++) {
        spdlog::info("test pid: {}", pid);
        categories.push_back(db->getPredicateById(pid));
        spdlog::info("pid:{} = {}", pid, db->getPredicateById(pid));
    }
    // insert object node;
    for (size_t pid = 1; pid < predicate_stat.size(); pid++) {
        size_t num = predicate_stat[pid] % 20; // every predicate limit 20 triplets

        auto so = db->getS2OByP(pid);
        for (const auto &item : so) {
            if (num != 0) num--;
            else break;

            uint32_t oid = item.second;
            std::string oid_str = std::to_string(oid);
            if (!node_map.count(oid)) {
                nlohmann::json object = {
                        {"id", oid_str},
                        {"name", db->getEntityById(oid)},
                        {"category", pid - 1}
                };
                node_map[oid] = object;
                nodes.emplace_back(object);
            }
        }
    }

    // insert subject noe;
    for (size_t pid = 1; pid < predicate_stat.size(); pid++) {
        size_t num = predicate_stat[pid] % 20; // every predicate limit 20 triplets

        auto so = db->getS2OByP(pid);
        std::string category = categories[pid - 1];
        std::string new_category = "Source" + category;
        for (const auto &item : so) {
            if (num != 0) num--;
            else break;

            uint32_t sid = item.first;
            std::string sid_str = std::to_string(sid);
            if (!node_map.count(sid)) {
                // if node_map doesn't have the sid, it means there is a new category
                if (categories.back() != new_category) {
                    // only need to be added once for new category
                    categories.emplace_back(new_category);
                }
                nlohmann::json subject = {
                        {"id", sid_str},
                        {"name", db->getEntityById(sid)},
                        {"category", categories.size() - 1}
                };
                node_map[sid] = subject;
                nodes.emplace_back(subject);
            }
        }
    }

    // insert edges
    for (size_t pid = 1; pid < predicate_stat.size(); pid++) {
        size_t num = predicate_stat[pid] % 20; // every predicate limit 20 triplets

        std::string predicate = db->getPredicateById(pid);
        auto so = db->getS2OByP(pid);
        for (const auto &item : so) {
            if (num != 0) num--;
            else break;

            uint32_t sid = item.first, oid = item.second;
            auto pair = std::make_pair(sid, oid);
            if (!edge_set.count(pair)) {
                nlohmann::json p = {
                        {"source", node_map[sid]["id"]},
                        {"target", node_map[oid]["id"]},
                        {"label", predicate}
                };
                edge_set.insert(pair);
                edges.emplace_back(p);
            }
        }
    }

    std::vector<std::unordered_map<std::string, std::string>> categories_dict;
    categories_dict.reserve(categories.size());
    for (std::string category : categories) {
        categories_dict.push_back({{"name", category}});
    }

    nlohmann::json j;
    j["nodes"] = nodes;
    j["edges"] = edges;
    j["categories"] = categories_dict;
    res.set_content(j.dump(2), "text/plain;charset=utf-8");
}

//void visualize(const httplib::Request &req, httplib::Response &res) {
//    res.set_header("Access-Control-Allow-Origin", "*");
//    spdlog::info("Catch load request from http://{}:{}", req.remote_addr, req.remote_port);
//    auto predicates = db->getPredicateStatistics();
//    std::unordered_map<uint32_t, nlohmann::json> node_map;
//    std::set<std::pair<std::string, std::string>> edge_set;
//    std::vector<nlohmann::json> nodes;
//    std::vector<nlohmann::json> edges;
//
//    for (size_t i = 1; i <= predicates.size(); i++) {
//
//    }
//
//                                    //0  1  2  3  4  5   6   7   8   9
//    std::vector<uint32_t> pid_list {1, 2, 3, 4, 6, 7, 18, 19, 20, 21};
////    std::vector<std::string> categories{"类别", "子类别", "品牌", "品名", "价格", "评论", "处理器型号", "方面", "观点"};
//    std::vector<std::string> categories{"Category", "sub-Category", "Brand", "Product", "Price", "Comment", "CPU", "Aspect", "Opinion"};
//    std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> categories_map{
//            // {pid, {source_category_idx, target_category_idx}}
//            {1,  {0, 1}},
//            {2,  {1, 2}},
//            {3,  {2, 3}},
//            {4,  {2, 4}},
//            {6,  {2, 5}},
//            {7,  {2, 6}},
//            {18, {5, 7}},
//            {19, {5, 8}},
//            {20, {7, 8}},
//    };
//
//    for (auto &pid : pid_list) {
//        std::string predicate = db->getPredicateById(pid);
//        auto so = db->getS2OByP(pid);
//        size_t source_cate_idx = categories_map[pid].first;
//        size_t target_cate_idx = categories_map[pid].second;
//        int num = 50;
//        for (const auto &item : so) {
//            if (pid > 7 && num != 0) num--;
//            else if (pid > 7) break;
//
//            uint32_t sid = item.first;
//            uint32_t oid = item.second;
//            std::string sid_str = std::to_string(sid);
//            std::string oid_str = std::to_string(oid);
//
//            if (!node_map.count(sid)) {
//                nlohmann::json s = {
//                        {"id", sid_str},
//                        {"name", db->getEntityById(sid)},
//                        {"category", source_cate_idx}
//                };
//                node_map[sid] = s;
//                nodes.emplace_back(s);
//            }
//
//            if (!node_map.count(oid)) {
//                nlohmann::json o = {
//                        {"id", oid_str},
//                        {"name", db->getEntityById(oid)},
//                        {"category", target_cate_idx}
//                };
//                node_map[oid] = o;
//                nodes.emplace_back(o);
//            }
//
//            auto pair = std::make_pair(node_map[sid]["id"], node_map[oid]["id"]);
//            if (!edge_set.count(pair)) {
//                nlohmann::json p = {
//                        {"source", node_map[sid]["id"]},
//                        {"target", node_map[oid]["id"]},
//                        {"label", predicate}
//                };
//                edge_set.insert(pair);
//                edges.emplace_back(p);
//            }
//        }
//    }
//
//    std::vector<std::unordered_map<std::string, std::string>> categories_data;
//    categories_data.reserve(categories.size());
//    for (std::string category : categories) {
//        categories_data.push_back({{"name", category}});
//    }
//
//    nlohmann::json j;
//    j["nodes"] = nodes;
//    j["edges"] = edges;
//    j["categories"] = categories_data;
//    res.set_content(j.dump(2), "text/plain;charset=utf-8");
//}

void query(const httplib::Request &req, httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    spdlog::info("Catch query request from http://{}:{}", req.remote_addr, req.remote_port);
    if (!req.has_param("sparql")) {
        return;
    }

    std::string sparql = req.get_param_value("sparql");
    spdlog::info("Receive SPARQL: {}", sparql);

    nlohmann::json json;
    json["data"] = execute_query(sparql);
    res.set_content(json.dump(2), "text/plain;charset=utf-8");
}

void insert(const httplib::Request &req, httplib::Response &res) {
    spdlog::info("Catch Insert Request.");
    res.set_header("Access-Control-Allow-Origin", "*");
    if (!req.has_param("sparql")) {
        return;
    }

    std::string sparql = req.get_param_value("sparql");
    spdlog::info("Receive SPARQL: {}", sparql);

    bool status = execute_insert(sparql);
    if (status) {
        res.set_content("successfully", "text/plain");
    } else {
        res.set_content("failed", "text/plain;charset=utf-8");
    }
}

int main(int argc, char **argv) {
    opt::options_description desc("psoHttp");
    desc.add_options()
            ("host,H", opt::value<std::string>()->default_value("0.0.0.0"), "IP address")
            ("port,P", opt::value<int>()->default_value(8998), "port")
            ("db_name,n", opt::value<std::string>(), "database name")
            ("help,h", "produce help message");

    opt::variables_map vm;
    opt::store(opt::parse_command_line(argc, argv, desc), vm);
    opt::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }

    if (!vm.count("db_name")) {
        spdlog::error("haven't specify database name.");
    }

    std::string host = vm["host"].as<std::string>();
    int port = vm["port"].as<int>();
    spdlog::info("Running at:  http://{}:{}", host, port);
    std::string db_name = vm["db_name"].as<std::string>();

    db = inno::DatabaseBuilder::LoadAll(db_name);
    sparqlQuery = std::make_unique<inno::SparqlQuery>(db);

    httplib::Server svr;

    std::string base = "/" + db_name;

    // connect
    svr.Get(base, [&](const httplib::Request &req, httplib::Response &res){
        spdlog::info("connection from http://{}:{}", req.remote_addr, req.remote_port);
        res.set_content("connected", "text/plain");
    });

    svr.Get("/list", list);
    svr.Get(base + "/info", info);
    svr.Get(base + "/visualize", visualize);
    svr.Post(base + "/query", query);
    svr.Post(base + "/insert", insert);

    // disconnect
    svr.Get(base + "/disconnect", [&](const httplib::Request &req, httplib::Response &res) {
        svr.stop();
        res.set_content("disconnect", "text/plain;charset=utf-8");
    });
    svr.listen(host.c_str(), port);
    return 0;
}