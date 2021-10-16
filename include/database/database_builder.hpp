/*
 * @FileName   : database_builder.hpp
 * @CreateAt   : 2021/6/19
 * @Author     : Inno Fang
 * @Email      : innofang@yeah.net
 * @Description: Database build and operate class,
 *               class `DatabaseBuilder::Impl` contains the details of all implementation,
 *               class `DatabaseBuilder` can `create` or `load` database instance,
 *               class `DatabaseBuilder::Option` contains the common operations about Database,
 *               such as 'save', 'unload', 'insert', etc.
 *               Note that only use `create` or `load` to get the `DatabaseBuilder::Option` instance firstly
 *               can access to call the methods of `DatabaseBuilder::Options`, which is a little bit similar with
 *               Builder Pattern to create and access the instance, the purpose of this is to ensure
 *               that database instance existed and operational when the user operates the database.
 */

#ifndef RETRIEVE_SYSTEM_DATABASE_BUILDER_HPP
#define RETRIEVE_SYSTEM_DATABASE_BUILDER_HPP

#include <string>
#include <memory>

namespace inno {

class DatabaseBuilder {
private:
    class Impl;

public:
    class Option;

public:
    DatabaseBuilder();
    ~DatabaseBuilder();

    /* create RDF database called @db_name from @data_file */
    DatabaseBuilder::Option *create(const std::string &db_name, const std::string &data_file);

    /* load a RDF database named @db_name */
    DatabaseBuilder::Option *load(const std::string &db_name);

public:
    class Option {
    public:
        explicit Option(Impl* impl) : impl_(impl) {}
        ~Option() = default;

        /* save the database data */
        bool save();

        /* unload the database */
        void unload();

        /* insert RDF raw triplet, which is expressed as <s, p, o> */
        bool insert(const std::string &s, const std::string &p, const std::string &o);

    private:
        Impl *impl_;
    };

private:
    Option *opt_;
    Impl *impl_;
};

}
#endif //RETRIEVE_SYSTEM_DATABASE_BUILDER_HPP
