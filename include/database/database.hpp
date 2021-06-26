//
// Created by InnoFang on 2021/6/19.
//

#ifndef RETRIEVE_SYSTEM_DATABASE_H
#define RETRIEVE_SYSTEM_DATABASE_H

#include <regex>
#include <cmath>
#include <string>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include "common/triple.hpp"

class Database {
public:
    Database(const std::string& dbname);
    ~Database();
    void create(const std::string& datafile) ;
    bool store();
    bool load();
    void hexManipulation();
    uint64_t convert2pso(const Triple& triple);
    int getHexLength(size_t length);
    void generatePSO();

private:
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
    std::vector<int> p_index_;
    std::vector<int> p_range_;
    std::unordered_map<std::string, uint64_t> so2id_;
    std::unordered_map<std::string, uint64_t> p2id_;
    std::vector<std::string> id2so_;
    std::vector<std::string> id2p_;
    std::vector<Triple> triples_;
    std::vector<uint64_t> pso_;
};

#endif //RETRIEVE_SYSTEM_DATABASE_H
