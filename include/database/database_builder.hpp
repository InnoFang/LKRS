/*
 * @FileName   : database_builder.hpp
 * @CreateAt   : 2021/6/19
 * @Author     : Inno Fang
 * @Email      : innofang@yeah.net
 * @Description:
 */

#ifndef RETRIEVE_SYSTEM_DATABASE_BUILDER_HPP
#define RETRIEVE_SYSTEM_DATABASE_BUILDER_HPP

#include <string>
#include <memory>

class DatabaseBuilder {
public:
    DatabaseBuilder();
    ~DatabaseBuilder();

    /* create RDF database called @db_name from @data_file */
    bool create(const std::string &db_name, const std::string &data_file);

    /* load a RDF database named @db_name */
    bool load(const std::string &db_name);

    /* save the database data */
    bool save();

    /* unload the database */
    void unload();

private:
    class Impl;
    std::shared_ptr<Impl> impl_;
};

#endif //RETRIEVE_SYSTEM_DATABASE_BUILDER_HPP
