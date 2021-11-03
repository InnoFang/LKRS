/*
 * @FileName   : database.hpp
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

#ifndef RETRIEVE_SYSTEM_DATABASE_HPP
#define RETRIEVE_SYSTEM_DATABASE_HPP

#include <set>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>

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
        explicit Option(Impl *impl): impl_(impl) {}
        ~Option() = default;

        /* save the database data */
        bool save();
        bool save(const std::string &db_name);

        /* unload the database */
        void unload();

        /* insert RDF raw triplet, which is expressed as <s, p, o> */
        bool insert(const std::string &subject, const std::string &predicate, const std::string &object);

        /* get pid corresponding to predicate */
        uint32_t getPredicateId(const std::string &predicate);
        uint32_t getPredicateId(const std::string &predicate) const;

        /* get entity id corresponding to entity (subject and object) */
        uint32_t getEntityId(const std::string &entity);
        uint32_t getEntityId(const std::string &entity) const;

        std::string getEntityById(uint32_t entity_id);
        std::string getEntityById(uint32_t entity_id) const;

        /* get the statistics of pid corresponding to predicate */
        uint32_t getPredicateStatistic(const std::string &predicate) const;
        uint32_t getPredicateStatistic(const std::string &predicate);

        /* For querying */
        std::unordered_set<uint32_t> getSByP(const uint32_t &pid);
        std::unordered_set<uint32_t> getOByP(const uint32_t &pid);

        std::unordered_multimap<uint32_t, uint32_t> getS2OByP(const uint32_t &pid);
        std::unordered_multimap<uint32_t, uint32_t> getO2SByP(const uint32_t &pid);

        std::set<std::pair<uint32_t, uint32_t>> getSOByP(const uint32_t &pid);

    private:
        Impl *impl_;
    };

private:
    Option *opt_;
    Impl *impl_;
};

} // namespace inno

#endif //RETRIEVE_SYSTEM_DATABASE_HPP
