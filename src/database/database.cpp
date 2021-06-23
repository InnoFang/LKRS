//
// Created by InnoFang on 2021/6/19.
//

#include "database/database.hpp"

Database::Database(const std::string& dbname, const std::string& datafile) : dbname_(dbname) {
    std::ifstream read_db("..\\db\\databases", std::ios::in);
    if (read_db.is_open()) {
        std::string db;
        while (read_db >> db) {
            if (db == dbname) {
                std::cout << "database: " << dbname << " found, load it now." << std::endl;
                load(dbname);
                break;
            }
        }
        if (read_db.eof()) {
            std::cout << "database: " << dbname << " has not been created, create it now" << std::endl;
            std::ofstream write_db("..\\db\\databases", std::ios::out);
            if (write_db.is_open()) {
                write_db << dbname << std::endl;
                create(datafile);
                store();
            } else {
                std::cerr << "cannot load or create database: " << dbname << std::endl;
            }
        }
        read_db.close();
    }

    // test
//    for (const auto &pso : pso_) {
//        ull sid = (pso & s_mask_) >> (4 * so_hex_len_);
//        ull pid = (pso & p_mask_) >> (2 * 4 * so_hex_len_);
//        ull oid = (pso & o_mask_);
////        std::cout << sid << "\t" << pid << "\t" << oid << std::endl;
////        std::cout << id2so_[sid] << "\t" << id2p_[pid] << "\t" << id2so_[oid] << std::endl;
//    }
}

Database::~Database() {

}

void Database::create(const std::string& datafile) {
    std::ifstream infile(datafile, std::ios::in);
    if (infile.is_open()) {
        std::string s, p, o;
        triple_size_ = 0;
        p_size_ = 0;
        so_size_ = 0;
        while (infile >> s >> p >> o) {
            Triple triple(s, p, o);
            triples_.push_back(triple);
            if (!p2id_.count(p)) {
                p2id_[p] = p_size_ ++;
                id2p_.push_back(p);
                p_index_.push_back(0);
            }
            p_index_[p2id_[p]] += 1;

            if (!so2id_.count(s)) {
                so2id_[s] = so_size_ ++;
                id2so_.push_back(s);
            }

            if (!so2id_.count(o)) {
                so2id_[o] = so_size_ ++;
                id2so_.push_back(o);
            }
            ++ triple_size_;
        }

        // handle predicate prefix sum
        p_range_.assign(p_index_.size() + 1, 0);
        for (size_t i = 1; i <= p_index_.size(); ++ i) {
            p_range_[i] = p_range_[i - 1] + p_index_[i - 1];
        }
//        std::cout << "triple size: " << triple_size_ << std::endl;
//        std::cout << "predicate size: " << id2p_.size() << std::endl;
//        std::cout << "su/object size: " << id2so_.size() << std::endl;
//        std::cout << "size of each predicate type:" << std::endl;
//        for (const auto& pi: p_index_) {
//            std::cout << pi << " ";
//        }
//        std::cout << "predicate range sum:" << std::endl;
//        for (const auto& pi: p_range_) {
//            std::cout << pi << " ";
//        }
//        std::cout << std::endl;

        // handle hex-number manipulation
        hexManipulation();

        // generate PSO
        generatePSO();

        infile.close();
    } else {
        std::cerr << "cannot open file: "<< datafile << std::endl;
    }
}

int Database::getHexLength(size_t length) {
    return ceil(log(length + 1) / log(16));
}

void Database::hexManipulation() {
    /* hex-manipulation */
    // get the hex-number size of 'p' and 'so'
    p_hex_len_ = getHexLength(p_size_);
    so_hex_len_ = getHexLength(so_size_);
    p_mask_ = (1 << (4 * p_hex_len_)) - 1;
    s_mask_ = o_mask_ = (1 << (4 * so_hex_len_)) - 1;

    p_mask_<<= (2 * 4 * so_hex_len_);
    s_mask_<<= (4 * so_hex_len_);
}

ull Database::convert2pso(const Triple &triple) {
    ull pid = p2id_[triple.p];
    ull sid = so2id_[triple.s];
    ull oid = so2id_[triple.o];
//    std::cout << "conver2pso: " << sid << "\t" << pid << "\t" << oid << std::endl;
    ull ret = (pid << (2 * 4 * so_hex_len_))
            | (sid << (4 * so_hex_len_))
            | oid;
    return ret;
}

void Database::generatePSO() {
    pso_.clear();
    pso_.resize(triple_size_);
    for (size_t i = 0; i < triple_size_; ++ i) {
        pso_[i] = convert2pso(triples_[i]);
    }

//    auto cmp = [&](const ull& a, const ull& b) {
//        ull a_p = (a & p_mask_);
//        ull a_so = (a & s_mask_) | (a & o_mask_);
//        ull b_p = (b & p_mask_);
//        ull b_so = (b & s_mask_) | (b & o_mask_);
//        return a_p == b_p ? a_so < b_so : a_p < b_p;
//    };
    std::sort(pso_.begin(), pso_.end());
}

bool Database::store() {
    std::string baseDir = "..\\db\\" + dbname_ + "\\";
    std::system(("mkdir " + baseDir).c_str());

    // database file 1: info.data
    std::string infoData = baseDir + "info.data";
    std::ofstream infoDataOut(infoData);
    if (infoDataOut.is_open()) {
        infoDataOut << triple_size_ << "\n"
            << p_size_ << "\n"
            << so_size_ << "\n"
            << p_hex_len_ << "\n"
            << so_hex_len_ << "\n"
            << p_mask_ << "\n"
            << s_mask_ << "\n"
            << o_mask_ << "\n";
        for (const auto& index: p_index_) {
            infoDataOut << index << " ";
        }
        infoDataOut << "\n";
        for (const auto& range: p_range_) {
            infoDataOut << range << " ";
        }
        infoDataOut << "\n";
        infoDataOut.close();
    } else {
        std::cerr << "cannot create file: "<< infoData << std::endl;
        return false;
    }

    // database file 2: pid.data
    std::string pidData = baseDir + "pid.data";
    std::ofstream pidDataOut(pidData);
    if (pidDataOut.is_open()) {
        for (size_t i = 0; i < p_size_; ++ i) {
            pidDataOut << i << "\t" << id2p_[i] << "\n";
        }
        pidDataOut.close();
    } else {
        std::cerr << "cannot create file: "<< pidData << std::endl;
        return false;
    }

    // database file 3: soid.data
    std::string soidData = baseDir + "soid.data";
    std::ofstream soidDataOut(soidData);
    if (soidDataOut.is_open()) {
        for (size_t i = 0; i < so_size_; ++ i) {
            soidDataOut << i << "\t" << id2so_[i] << "\n";
        }
        soidDataOut.close();
    } else {
        std::cerr << "cannot create file: "<< pidData << std::endl;
        return false;
    }

    // database file 4: pso.data
    std::string psoData = baseDir + "pso.data";
    std::ofstream psoDataOut(psoData);
    if (psoDataOut.is_open()) {
        for (const auto& pso: pso_) {
            psoDataOut << pso << "\n";
        }
        psoDataOut.close();
    } else {
        std::cerr << "cannot create file: "<< psoData << std::endl;
        return false;
    }
    return true;
}

bool Database::load(const std::string& dbname) {

    std::string baseDir = "..\\db\\" + dbname_ + "\\";

    // database file 1: info.data
    std::string infoData = baseDir + "info.data";
    std::ifstream infoDataIn(infoData);
    if (infoDataIn.is_open()) {
        infoDataIn  >> triple_size_
                    >> p_size_
                    >> so_size_
                    >> p_hex_len_
                    >> so_hex_len_
                    >> p_mask_
                    >> s_mask_
                    >> o_mask_ ;
        p_index_.assign(p_size_, 0);
        for (size_t i = 0; i < p_size_; ++ i) {
            infoDataIn >> p_index_[i];
        }
        p_range_.assign(p_size_ + 1, 0);
        for (size_t i = 1; i <= p_size_; ++ i) {
            infoDataIn >> p_range_[i];
        }
        infoDataIn.close();
    } else {
        std::cerr << "cannot read from file: " << infoData << std::endl;
        return false;
    }

    // database file 2: pid.data
    std::string pidData = baseDir + "pid.data";
    std::ifstream pidDataIn(pidData);
    if (pidDataIn.is_open()) {
        id2p_.clear();
        id2p_.resize(p_size_);
        p2id_.clear();
        for (size_t i = 0; i < p_size_; ++ i) {
            ull index;
            std::string predicate;
            pidDataIn >> index >> predicate;
            id2p_[index] = predicate;
            p2id_[predicate] = index;
        }
        pidDataIn.close();
    } else {
        std::cerr << "cannot read from file: " << pidData << std::endl;
        return false;
    }

    // database file 3: soid.data
    std::string soidData = baseDir + "soid.data";
    std::ifstream soidDataIn(soidData);
    if (soidDataIn.is_open()) {
        id2so_.clear();
        id2so_.resize(so_size_);
        so2id_.clear();
        for (size_t i = 0; i < so_size_; ++ i) {
            ull index;
            std::string suobject;
            soidDataIn >> index >> suobject;
            id2so_[index] = suobject;
            so2id_[suobject] = index;
        }
        soidDataIn.close();
    } else {
        std::cerr << "cannot read from file: " << pidData << std::endl;
        return false;
    }

    // database file 4: pso.data
    std::string psoData = baseDir + "pso.data";
    std::ifstream psoDataIn(psoData);
    if (psoDataIn.is_open()) {
        pso_.clear();
        pso_.resize(triple_size_);
        for (size_t i = 0; i < triple_size_; ++ i) {
            psoDataIn >> pso_[i];
        }
        psoDataIn.close();
    } else {
        std::cerr << "cannot read from file: " << psoData << std::endl;
        return false;
    }

    return true;
}


/*

 database file 1: info.data
 triple_size_
 p_size_
 so_size_
 p_hex_len_
 so_hex_len_
 p_mask_
 s_mask_
 o_mask_
 p_index_
 p_range_

 database file 2: pid.data
 pid predicate

 database file 3: soid.data
 soid su/object

 database file 4: pso.data
 pso
 */