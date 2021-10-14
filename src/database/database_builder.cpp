/*
 * @FileName   : database_builder.cpp
 * @CreateAt   : 2021/6/19
 * @Author     : Inno Fang
 * @Email      : innofang@yeah.net
 * @Description:
 */


#include "database/database_builder.hpp"

#include <vector>
#include <future>
#include <fstream>
#include <unordered_map>

#include <spdlog/spdlog.h>
#include <boost/filesystem.hpp>

#include "common/skip_list.hpp"

namespace inno {

namespace fs = boost::filesystem;

class DatabaseBuilder::Impl {
private:
    using so_pair_list = inno::SkipList<std::pair<uint32_t, uint32_t>>;

public:
    Impl()  { initialize_(); }
    ~Impl() { unload(); }

    bool create(const std::string &db_name, const std::string &data_file) {
        db_name_ = db_name;

        std::ifstream infile(data_file, std::ifstream::binary);
        std::ifstream::sync_with_stdio(false);

        if (infile.is_open()) {
            std::string s, p, o;
            while (infile >> s >> p) {
                infile.ignore();
                std::getline(infile, o);
                for (o.pop_back(); o.back() == ' ' || o.back() == '.'; o.pop_back()) {}

                triplet_size_ ++;

                if (!p2id_.count(p)) {
                    p2id_[p] = ++predicate_size_;
                    id2p_.emplace_back(p);
                }

                if (!so2id_.count(s)) {
                    so2id_[s] = ++ entity_size_;
                    id2so_.emplace_back(s);
                }

                if (!so2id_.count(o)) {
                    so2id_[o] = ++ entity_size_;
                    id2so_.emplace_back(o);
                }

                storage_[p2id_[p]].insert({so2id_[s], so2id_[o]});
            }

            return save();
        } else {
            spdlog::error("Cannot open RDF data file, problem occurs by path '{}'", data_file);
            return false;
        }
    }

    bool save() {
        if (db_name_.empty()) {
            spdlog::info("Save Failed! Haven't specified a database yet, "
                         "you should call create or load before this operation.");
            return false;
        }

        fs::ofstream::sync_with_stdio(false);

        fs::path db_path = fs::current_path().parent_path().append(db_name_ + ".db");
        fs::path info_path("info");
        fs::path id_predicates_path("id_predicates");
        fs::path id_entities_path("id_entities");
        fs::path triplet_path("triplet");

        fs::create_directories(db_path);

        auto info_store_task = std::async(std::launch::async,
                                          &DatabaseBuilder::Impl::store_basic_info,
                                          this,
                                          db_path/info_path);

        auto pid_store_task = std::async(std::launch::async,
                                          &DatabaseBuilder::Impl::store_predicate_ids_,
                                          this,
                                          db_path/id_predicates_path);

        auto soid_store_task = std::async(std::launch::async,
                                         &DatabaseBuilder::Impl::store_entity_ids_,
                                         this,
                                         db_path/id_entities_path);

        auto triplet_store_task = std::async(std::launch::async,
                                         &DatabaseBuilder::Impl::store_triplet_,
                                         this,
                                         db_path/triplet_path);

        info_store_task.get();
        pid_store_task.get();
        soid_store_task.get();
        triplet_store_task.get();

        return true;
    }

    bool load(const std::string &db_name) {
        db_name_ = db_name;

        fs::ifstream::sync_with_stdio(false);

        fs::path db_path = fs::current_path().parent_path().append(db_name + ".db");
        fs::path info_path("info");
        fs::path id_predicates_path("id_predicates");
        fs::path id_entities_path("id_entities");
        fs::path triplet_path("triplet");

        load_basic_info(db_path/info_path);

        auto pid_load_task = std::async(std::launch::async,
                                         &DatabaseBuilder::Impl::load_predicate_ids_,
                                         this,
                                         db_path/id_predicates_path,
                                         predicate_size_);

        auto soid_load_task = std::async(std::launch::async,
                                          &DatabaseBuilder::Impl::load_entity_ids_,
                                          this,
                                          db_path/id_entities_path,
                                          entity_size_);

        auto triplet_load_task = std::async(std::launch::async,
                                             &DatabaseBuilder::Impl::load_triplet_,
                                             this,
                                             db_path/triplet_path,
                                             predicate_size_);

        pid_load_task.get();
        soid_load_task.get();
        triplet_load_task.get();

        return true;
    }

    void unload() {
        db_name_.clear();
        predicate_size_ = 0;
        entity_size_ = 0;
        triplet_size_ = 0;
        so2id_.clear();
        p2id_.clear();
        id2so_.clear();
        id2p_.clear();
        storage_.clear();
    }

private:
    void initialize_() {
        predicate_size_ = 0;
        entity_size_ = 0;
        triplet_size_ = 0;
        id2so_.resize(1);
        id2p_.resize(1);
    }

    /* store database basic information */
    bool store_basic_info(const fs::path &path) const {
        fs::ofstream out(path, fs::ofstream::out | fs::ofstream::binary);
        if (out.is_open()) {
            std::string content = std::to_string(triplet_size_) + "\n" +
                                  std::to_string(predicate_size_) + "\n" +
                                  std::to_string(entity_size_) + "\n";
            out.write(content.c_str(), content.size());
            out.close();
        } else {
            spdlog::error("store_basic_info function occurs problem, "
                          "`info` file cannot be written.");
            return false;
        }
        return true;
    }

    /* load database basic information */
    bool load_basic_info(const fs::path &path) {
        fs::ifstream in(path, fs::ifstream::in | fs::ifstream::binary);
        if (in.is_open()) {
            in >> triplet_size_
               >> predicate_size_
               >> entity_size_;
            in.close();
        } else {
            spdlog::error("load_basic_info function occurs problem, "
                          "`info` file cannot be read.");
            return false;
        }
        return true;
    }

    /* store the mapping between pid and predicates */
    bool store_predicate_ids_(const fs::path &path) const {
        fs::ofstream out(path, fs::ofstream::out | fs::ofstream::binary);
        if (out.is_open()) {
            for (size_t i = 1; i <= predicate_size_; ++ i) {
                std::string item = std::to_string(i) + "\t" +
                                   id2p_[i] + "\n";
                out.write(item.c_str(), item.size());
            }
            out.close();
        } else {
            spdlog::error("store_predicate_ids_ function occurs problem, "
                          "`id_predicates` file cannot be written.");
            return false;
        }
        return true;
    }

    /* load the mapping between pid and predicates */
    bool load_predicate_ids_(const fs::path &path, const uint32_t &predicate_size) {
        p2id_.clear();
        p2id_.reserve(static_cast<size_t>(predicate_size * 0.75));
        id2p_.clear();
        id2p_.resize(predicate_size + 1);

        fs::ifstream in(path, fs::ifstream::in | fs::ifstream::binary);
        if (in.is_open()) {
            for (size_t i = 1; i <= predicate_size_; ++ i) {
                uint32_t pid;
                std::string predicate;
                in >> pid >> predicate;
                id2p_[pid] = predicate;
                p2id_[predicate] = pid;
            }
            in.close();
        } else {
            spdlog::error("load_predicate_ids_ function occurs problem, "
                          "`id_predicates` file cannot be read.");
            return false;
        }
        return true;
    }


    /* store the mapping between soid and entities */
    bool store_entity_ids_(const fs::path &path) const {
        fs::ofstream out(path, fs::ofstream::out | fs::ofstream::binary);
        if (out.is_open()) {
            for (size_t i = 1; i <= entity_size_; ++ i) {
                std::string item = std::to_string(i) + "\t" +
                                   id2so_[i] + "\n";
                out.write(item.c_str(), item.size());
            }
            out.close();
        } else {
            spdlog::error("store_entity_ids_ function occurs problem, "
                          "`id_entities` file cannot be written.");
            return false;
        }
        return true;
    }

    /* load the mapping between soid and entities */
    bool load_entity_ids_(const fs::path &path, const uint32_t &entity_size) {
        so2id_.clear();
        so2id_.reserve(static_cast<size_t>(entity_size * 0.75));
        id2so_.clear();
        id2so_.resize(entity_size + 1);

        fs::ifstream in(path, fs::ifstream::in | fs::ifstream::binary);
        if (in.is_open()) {
            for (size_t i = 1; i <= predicate_size_; ++ i) {
                uint32_t soid;
                std::string entity;
                in >> soid >> entity;
                id2so_[soid] = entity;
                so2id_[entity] = soid;
            }
            in.close();
        } else {
            spdlog::error("load_entity_ids_ function occurs problem, "
                          "`id_entities` file cannot be read.");
            return false;
        }
        return true;
    }


    /* store the predicate -> <subject, object> */
    bool store_triplet_(const fs::path &path) const {
        fs::create_directories(path);
        if (!fs::exists(path)) {
            spdlog::error("store_triplet_ function occurs problem, "
                          "`triplet` directory cannot be created");
            return false;
        }

        std::vector<std::future<bool>> task_list;
        task_list.reserve(storage_.size());

        for (uint32_t i = 1; i <= predicate_size_; ++i) {
            fs::path child_path = path/fs::path(std::to_string(i));

            auto triplet_task = std::async(std::launch::async,
               [](fs::path sub_path, const so_pair_list &so_list) {

                   fs::ofstream out(sub_path, fs::ofstream::out | fs::ofstream::binary);
                   if (out.is_open()) {
                       for (const auto &so : so_list) {
                           std::string item = std::to_string(so.first) + " " +
                                              std::to_string(so.second) + "\n";
                           out.write(item.c_str(), item.size());
                       }
                       out.close();
                       return true;
                   } else {
                       spdlog::error("store_triplet_ function occurs problem, "
                                     "`{}` cannot be written.", sub_path.string());
                       return false;
                   }
               }, child_path, std::cref(storage_.at(i)));

            task_list.emplace_back(std::move(triplet_task));
        }

        for (std::future<bool> &task : task_list) {
            task.get();
        }

        return true;
    }

    /* store the predicate -> <subject, object> */
    bool load_triplet_(const fs::path &path, const uint32_t &predicate_size) {
        if (!fs::exists(path)) {
            spdlog::error("load_triplet_ function occurs problem, "
                          "`triplet` directory cannot be created");
            return false;
        }

        storage_.clear();
        storage_.reserve(static_cast<size_t>(predicate_size * 0.75));

        for (uint32_t pid = 1; pid <= predicate_size; ++pid) {
            fs::path child_path = path/fs::path(std::to_string(pid));

            fs::ifstream in(child_path, fs::ifstream::in | fs::ifstream::binary);
            if (in.is_open()) {
                while (in.eof()) {
                    uint32_t sid, oid;
                    in >> sid >> oid;
                    storage_[pid].insert({sid, oid});
                }
                in.close();
            } else {
                spdlog::error("load_triplet_ function occurs problem, "
                              "`{}` cannot be read.", child_path.string());
                return false;
            }

        }

        return true;
    }

private:
    std::string db_name_;
    uint32_t predicate_size_;
    uint32_t entity_size_;
    size_t triplet_size_;
    std::unordered_map<std::string, uint32_t> so2id_;
    std::unordered_map<std::string, uint32_t> p2id_;
    std::vector<std::string> id2so_;
    std::vector<std::string> id2p_;

    std::unordered_map<uint32_t, so_pair_list> storage_;
};


DatabaseBuilder::DatabaseBuilder()
    : impl_(new DatabaseBuilder::Impl()) {}

DatabaseBuilder::~DatabaseBuilder() {
    unload();
}

bool DatabaseBuilder::create(const std::string &db_name, const std::string &data_file) {
    return impl_->create(db_name, data_file);
}

bool DatabaseBuilder::load(const std::string &db_name) {
    return impl_->load(db_name);
}

bool DatabaseBuilder::save() {
    return impl_->save();
}

void DatabaseBuilder::unload() {
    return impl_->unload();
}

}
