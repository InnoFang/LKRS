/*
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
#include <utility>

#include "query/sparql_query.hpp"

namespace opt = boost::program_options;

using ResultSet = std::vector<std::unordered_map<std::string, std::string>>;

std::shared_ptr<inno::DatabaseBuilder::Option> db;
std::unique_ptr<inno::SparqlQuery> sparqlQuery;
inno::SparqlParser parser;

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

void load(const httplib::Request &req, httplib::Response &res) {
    spdlog::info("Catch Load Request.");
    res.set_header("Access-Control-Allow-Origin", "*");
//    auto predicates = db->getPredicateStatistics();
    std::unordered_map<uint32_t, nlohmann::json> node_map;
    std::set<std::pair<std::string, std::string>> edge_set;
    std::vector<nlohmann::json> nodes;
    std::vector<nlohmann::json> edges;

                                    //0  1  2  3  4  5   6   7   8   9
    std::vector<uint32_t> pid_list {1, 2, 3, 4, 6, 7, 18, 19, 20, 21};
//    std::vector<std::string> categories{"类别", "子类别", "品牌", "品名", "价格", "评论", "处理器型号", "方面", "观点"};
    std::vector<std::string> categories{"Category", "sub-Category", "Brand", "Product", "Price", "Comment", "CPU", "Aspect", "Opinion"};
    std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> categories_map{
            // {pid, {source_category_idx, target_category_idx}}
            {1,  {0, 1}},
            {2,  {1, 2}},
            {3,  {2, 3}},
            {4,  {2, 4}},
            {6,  {2, 5}},
            {7,  {2, 6}},
            {18, {5, 7}},
            {19, {5, 8}},
            {20, {7, 8}},
    };

    for (auto &pid : pid_list) {
        std::string predicate = db->getPredicateById(pid);
        auto so = db->getS2OByP(pid);
        size_t source_cate_idx = categories_map[pid].first;
        size_t target_cate_idx = categories_map[pid].second;
        int num = 50;
        for (const auto &item : so) {
            if (pid > 7 && num != 0) num--;
            else if (pid > 7) break;

            uint32_t sid = item.first;
            uint32_t oid = item.second;
            std::string sid_str = std::to_string(sid);
            std::string oid_str = std::to_string(oid);

            if (!node_map.count(sid)) {
                nlohmann::json s = {
                        {"id", sid_str},
                        {"name", db->getEntityById(sid)},
                        {"category", source_cate_idx}
                };
                node_map[sid] = s;
                nodes.emplace_back(s);
            }

            if (!node_map.count(oid)) {
                nlohmann::json o = {
                        {"id", oid_str},
                        {"name", db->getEntityById(oid)},
                        {"category", target_cate_idx}
                };
                node_map[oid] = o;
                nodes.emplace_back(o);
            }

            auto pair = std::make_pair(node_map[sid]["id"], node_map[oid]["id"]);
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

    std::vector<std::unordered_map<std::string, std::string>> categories_data;
    categories_data.reserve(categories.size());
    for (std::string category : categories) {
        categories_data.push_back({{"name", category}});
    }

    nlohmann::json j;
    j["nodes"] = nodes;
    j["edges"] = edges;
    j["categories"] = categories_data;
    res.set_content(j.dump(2), "text/plain");
}

void query(const httplib::Request &req, httplib::Response &res) {
    spdlog::info("Catch Query Request.");
    res.set_header("Access-Control-Allow-Origin", "*");
    if (!req.has_param("sparql")) {
        return;
    }

    std::string sparql = req.get_param_value("sparql");
    spdlog::info("Receive SPARQL: {}", sparql);

    nlohmann::json json(execute_query(sparql));
    res.set_content(json.dump(2), "text/plain");
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
        res.set_content("failed", "text/plain");
    }
}

int main(int argc, char **argv) {
    opt::options_description desc("psoHttp");
    desc.add_options()
            ("host,H", opt::value<std::string>()->default_value("127.0.0.1"), "IP address")
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

    db = inno::DatabaseBuilder::Load(db_name);
    sparqlQuery = std::make_unique<inno::SparqlQuery>(db);

    httplib::Server svr;

    std::string base = "/" + db_name;

    // connect
    svr.Get(base, [&](const httplib::Request &, httplib::Response &res){
        res.set_content("connected", "text/plain");
    });

    svr.Get(base + "/load", load);
    svr.Post(base + "/query", query);
    svr.Post(base + "/insert", insert);

    // disconnect
    svr.Get(base + "/disconnect", [&](const httplib::Request &req, httplib::Response &res) {
        svr.stop();
    });
    svr.listen(host.c_str(), port);
    return 0;
}
