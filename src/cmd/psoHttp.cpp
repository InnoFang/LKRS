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
    res.set_header("Access-Control-Allow-Origin", "*");
//    auto predicates = db->getPredicateStatistics();
    std::unordered_set<uint32_t> node_set;
    std::set<std::pair<uint32_t, uint32_t>> edge_set;
    std::vector<nlohmann::json> nodes;
    std::vector<nlohmann::json> edges;

    std::vector<uint32_t> predicates {1, 2, 3, 4, 6, 7, 18, 19, 20, 21};
//    for (int pid = 1; pid < predicates.size(); ++pid) {
    for (auto &pid: predicates) {
        auto so = db->getSOByP(pid);
        int num = 20;
        for (const auto &item : so) {
            if (pid > 7 && num != 0) num--;
            else if (pid > 7) break;
            std::string sid_str = std::to_string(item.first);
            std::string oid_str = std::to_string(item.second);
            nlohmann::json s = {
                    {"id", sid_str},
                    {"name", db->getEntityById(item.first)}
            };
            nlohmann::json o = {
                    {"id", oid_str},
                    {"name", db->getEntityById(item.second)}
            };
            nlohmann::json p = {
                    {"source", sid_str},
                    {"target", oid_str},
                    {"label", db->getPredicateById(pid)}
            };
            if (!node_set.count(item.first)) {
                node_set.insert(item.first);
                nodes.emplace_back(s);
            }
            if (!node_set.count(item.second)) {
                node_set.insert(item.second);
                nodes.emplace_back(o);
            }

            auto pair = std::make_pair(item.first, item.second);
            if (!edge_set.count(pair)) {
                edge_set.insert(pair);
                edges.emplace_back(p);
            }
        }
    }

    nlohmann::json j;
    j["nodes"] = nodes;
    j["edges"] = edges;
    res.set_content(j.dump(2), "text/plain");
}

void query(const httplib::Request &req, httplib::Response &res) {
    if (!req.has_param("sparql")) {
        return;
    }
    std::string sparql = req.get_param_value("sparql");
    nlohmann::json json(execute_query(sparql));
    res.set_content(json.dump(2), "text/plain");
}

void insert(const httplib::Request &req, httplib::Response &res) {
    if (!req.has_param("sparql")) {
        return;
    }

    std::string sparql = req.get_param_value("sparql");
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
    spdlog::info("{}", host);
    int port = vm["port"].as<int>();
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
