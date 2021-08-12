//
// Created by InnoFang on 2021/6/19.
//

#ifndef RETRIEVE_SYSTEM_DATABASE_H
#define RETRIEVE_SYSTEM_DATABASE_H

#include <list>
#include <mutex>
#include <regex>
#include <cmath>
#include <string>
#include <thread>
#include <utility>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <boost/filesystem.hpp>

#include "common/triple.hpp"

namespace fs = boost::filesystem;

class Database {
public:
    explicit Database(std::string& dbname);
    ~Database();
    void create(const std::string& datafile) ;
    bool store();
    bool load();
    void hexManipulation();
    uint64_t convert2pso(const gPSO::triplet& triple);
    int calcHexLength(size_t length);
    void generatePSO();
    uint64_t getIdByP(const std::string& p);
    uint64_t getIdBySO(const std::string& so);
    std::string getPbyId(uint64_t id);
    std::string getSObyId(uint64_t id);
    uint64_t getPMask();
    uint64_t getSMask();
    uint64_t getOMask();
    int getPHexLength();
    int getSOHexLength();
    std::vector<uint64_t> getPSO() const;
    std::vector<int> getPredicateIndices() const;
    std::vector<int> getPredicateRange() const;

private:
    fs::path db_path_;
    fs::path info_path_;
    fs::path pid_path_;
    fs::path soid_path_;
    fs::path pso_path_;

    std::string dbname_;
    std::string datafile_;
    size_t triple_size_;
    size_t p_size_;
    size_t so_size_;
    int p_hex_len_;
    int so_hex_len_;
    uint64_t p_mask_;
    uint64_t s_mask_;
    uint64_t o_mask_;
    std::vector<int> p_indices_;
    std::vector<int> p_range_;
    std::unordered_map<std::string, uint64_t> so2id_;
    std::unordered_map<std::string, uint64_t> p2id_;
    std::vector<std::string> id2so_;
    std::vector<std::string> id2p_;
    std::list<gPSO::triplet> triples_;
    std::vector<uint64_t> pso_;
};

#endif //RETRIEVE_SYSTEM_DATABASE_H
