//
// Created by InnoFang on 2021/6/19.
//

#ifndef RETRIEVE_SYSTEM_DATABASE_H
#define RETRIEVE_SYSTEM_DATABASE_H

#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <unordered_map>
#include <algorithm>
#include "common/triple.hpp"

using ull = unsigned long long;

class Database {
public:
    Database(const std::string& dbname, const std::string& datafile);
    ~Database();
    void create(const std::string& datafile) ;
    bool store();
    bool load(const std::string& dbname);
    void hexManipulation();
    ull convert2pso(const Triple& triple);
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
    ull p_mask_;
    ull s_mask_;
    ull o_mask_;
    std::vector<int> p_index_;
    std::vector<int> p_range_;
    std::unordered_map<std::string, ull> so2id_;
    std::unordered_map<std::string, ull> p2id_;
    std::vector<std::string> id2so_;
    std::vector<std::string> id2p_;
    std::vector<Triple> triples_;
    std::vector<ull> pso_;
};

#endif //RETRIEVE_SYSTEM_DATABASE_H
