/*
 * @FileName   : sparql_query.hpp 
 * @CreateAt   : 2021/6/25
 * @Author     : Inno Fang
 * @Email      : innofang@yeah.net
 * @Description: 
 */

#ifndef RETRIEVE_SYSTEM_SPARQL_QUERY_HPP
#define RETRIEVE_SYSTEM_SPARQL_QUERY_HPP

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include "database/database.hpp"
#include "parser/sparql_parser.hpp"

namespace inno {
class SparqlQuery {

public:
    using ResultSet = std::set<std::vector<std::string>>;

public:
    explicit SparqlQuery(const std::shared_ptr<DatabaseBuilder::Option> &db);
    ~SparqlQuery();

    ResultSet query(SparqlParser &parser);
    double getQueryTime() const;

private:
    class Impl;
    std::shared_ptr<Impl> impl_;
};

}

#endif //RETRIEVE_SYSTEM_SPARQL_QUERY_HPP
