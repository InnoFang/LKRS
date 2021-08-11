//
// Created by InnoFang on 2021/6/19.
//

#include "database/database.hpp"

Database::Database(std::string& dbname) : dbname_(dbname) {
    db_path_ = fs::current_path()
            .parent_path()
            .parent_path()
            .append("db")
            .append(dbname);

    info_path_ = fs::path(db_path_).append("info");
    pid_path_ = fs::path(db_path_).append("pid");
    soid_path_ = fs::path(db_path_).append("soid");
    pso_path_ = fs::path(db_path_).append("spo");
}

Database::~Database() {}

void Database::create(const std::string& datafile) {
    std::ifstream infile(datafile, std::ifstream::binary);
    std::ifstream::sync_with_stdio(false);
    infile.tie(nullptr);
    if (infile.is_open()) {
        triple_size_ = 0;
        p_size_ = 0;
        so_size_ = 0;
        triples_.clear();
        p2id_.clear();
        id2p_.assign(1, "");
        so2id_.clear();
        id2so_.assign(1, "");
        p_indices_.assign(1, 0);

        std::string s, p, o, dot, line;
        while (infile >> s >> p >> o >> dot) {
            Triple triple(s, p, o);
            triples_.emplace_back(triple);

            if (!p2id_.count(p)) {
                p2id_[p] = ++ p_size_;
                id2p_.push_back(p);
                p_indices_.push_back(0);
            }
            p_indices_[p2id_[p]] += 1;

            if (!so2id_.count(s)) {
                so2id_[s] = ++ so_size_;
                id2so_.push_back(s);
            }

            if (!so2id_.count(o)) {
                so2id_[o] = ++ so_size_;
                id2so_.push_back(o);
            }
            ++ triple_size_;
        }

        // handle predicate prefix sum
        p_range_.assign(p_indices_.size() + 1, 0);
        for (size_t i = 1; i <= p_indices_.size(); ++ i) {
            p_range_[i] = p_range_[i - 1] + p_indices_[i - 1];
        }

        // handle hex-number manipulation
        hexManipulation();

        // generate PSO
        generatePSO();
        store();

        infile.close();
    } else {
        std::cerr << "cannot open rdfFile: "<< datafile << std::endl;
    }
}

int Database::calcHexLength(size_t length) {
    return int(ceil(log(length + 1) / log(16)));
}

void Database::hexManipulation() {
    /* hex-manipulation */
    // get the hex-number size of 'p' and 'so'
    p_hex_len_ = calcHexLength(p_size_);
    so_hex_len_ = calcHexLength(so_size_);
    p_mask_ = (1 << (p_hex_len_ << 2)) - 1;
    s_mask_ = o_mask_ = (1 << (so_hex_len_ << 2)) - 1;

    p_mask_<<= (so_hex_len_ << 3);
    s_mask_<<= (so_hex_len_ << 2);
}

uint64_t Database::convert2pso(const Triple &triple) {
    uint64_t pid = p2id_[triple.p];
    uint64_t sid = so2id_[triple.s];
    uint64_t oid = so2id_[triple.o];
    uint64_t pso = (pid << (so_hex_len_ << 3))
                   | (sid << (so_hex_len_ << 2))
                   | oid;
    return pso;
}

void Database::generatePSO() {
    pso_.clear();
    pso_.resize(triple_size_);
    int i = 0;
    for (const auto &triple : triples_) {
        pso_[i] = convert2pso(triple);
        i ++;
    }

    std::sort(pso_.begin(), pso_.end());
}

bool Database::store() {
    fs::ofstream::sync_with_stdio(false);
    fs::create_directories(db_path_);
    std::mutex info_lock, pid_lock, soid_lock, pso_lock;

    // database file 1: info
    auto store_info = [&]() {
        std::lock_guard<std::mutex> guard{info_lock};

        fs::ofstream infoDataOut(info_path_, std::ofstream::binary);
        infoDataOut.tie(nullptr);
        if (infoDataOut.is_open()) {
            std::cout << "store info" << std::endl;
            infoDataOut << triple_size_ << "\n"
                        << p_size_ << "\n"
                        << so_size_ << "\n"
                        << p_hex_len_ << "\n"
                        << so_hex_len_ << "\n"
                        << p_mask_ << "\n"
                        << s_mask_ << "\n"
                        << o_mask_ << "\n";
            for (auto& index : p_indices_) {
                infoDataOut << index << " ";
            }
            infoDataOut << "\n";
            for (auto& range : p_range_) {
                infoDataOut << range << " ";
            }
            infoDataOut << "\n";
            infoDataOut.close();
        } else {
            std::cerr << "cannot create file: "<< info_path_ << std::endl;
        }
    };

    // database file 2: pid
    auto store_pid = [&]() {
        std::lock_guard<std::mutex> guard{pid_lock};

        fs::ofstream pidDataOut(pid_path_, std::ofstream::binary);
        pidDataOut.tie(nullptr);
        if (pidDataOut.is_open()) {
            std::cout << "store pid" << std::endl;
            for (size_t i = 1; i <= p_size_; ++ i) {
                pidDataOut << i << "\t" << id2p_[i] << "\n";
            }
            pidDataOut.close();
        } else {
            std::cerr << "cannot create file: "<< pid_path_ << std::endl;
        }
    };

    // database file 3: soid
    // soid su/object
    auto store_soid = [&]() {
        std::lock_guard<std::mutex> guard{soid_lock};

        fs::ofstream soidDataOut(soid_path_, std::ofstream::binary);
        soidDataOut.tie(nullptr);
        if (soidDataOut.is_open()) {
            std::cout << "store soid" << std::endl;
            for (size_t i = 1; i <= so_size_; ++ i) {
                soidDataOut << i << "\t" << id2so_[i] << "\n";
            }
            soidDataOut.close();
        } else {
            std::cerr << "cannot create file: "<< soid_path_ << std::endl;
        }
    };

    // database file 4: pso
    auto store_pso = [&]() {
        std::lock_guard<std::mutex> guard{pso_lock};

        fs::ofstream psoDataOut(pso_path_, std::ofstream::binary);
        psoDataOut.tie(nullptr);
        if (psoDataOut.is_open()) {
            std::cout << "store pso" << std::endl;
            for (auto& pso : pso_) {
                psoDataOut << pso << "\n";
            }
            psoDataOut.close();
        } else {
            std::cerr << "cannot create file: "<< pso_path_ << std::endl;
        }
    };

    auto t_pso = std::thread(store_pso);
    auto t_info = std::thread(store_info);
    auto t_pid = std::thread(store_pid);
    auto t_soid = std::thread(store_soid);
    t_pso.join();
    t_info.join();
    t_pid.join();
    t_soid.join();

    return true;
}

bool Database::load() {
    // database file 1: info
    std::ifstream infoDataIn(info_path_.string(), std::ifstream::binary);
    if (infoDataIn.is_open()) {
        infoDataIn  >> triple_size_
                    >> p_size_
                    >> so_size_
                    >> p_hex_len_
                    >> so_hex_len_
                    >> p_mask_
                    >> s_mask_
                    >> o_mask_ ;
        p_indices_.assign(p_size_ + 1, 0);
        for (int & p_index : p_indices_) {
            infoDataIn >> p_index;
        }
        p_range_.assign(p_size_ + 2, 0);
        for (int & p_range : p_range_) {
            infoDataIn >> p_range;
        }
        infoDataIn.close();
    } else {
        std::cerr << "cannot read from file: " << info_path_ << std::endl;
        return false;
    }

    // database file 2: pid
    // pid predicate
    std::ifstream pidDataIn(pid_path_.string(), std::ifstream::binary);
    if (pidDataIn.is_open()) {
        id2p_.clear();
        id2p_.resize(p_size_ + 1);
        p2id_.clear();
        for (size_t i = 0; i < p_size_; ++ i) {
            uint64_t index;
            std::string predicate;
            pidDataIn >> index >> predicate;
            id2p_[index] = predicate;
            p2id_[predicate] = index;
        }
        pidDataIn.close();
    } else {
        std::cerr << "cannot read from file: " << pid_path_ << std::endl;
        return false;
    }

    // database file 3: soid
    std::ifstream soidDataIn(soid_path_.string(), std::ifstream::binary);
    if (soidDataIn.is_open()) {
        id2so_.clear();
        id2so_.resize(so_size_ + 1);
        so2id_.clear();
        for (size_t i = 0; i < so_size_; ++ i) {
            uint64_t index;
            std::string suobject;
            soidDataIn >> index >> suobject;
            id2so_[index] = suobject;
            so2id_[suobject] = index;
        }
        soidDataIn.close();
    } else {
        std::cerr << "cannot read from file: " << soid_path_ << std::endl;
        return false;
    }

    // database file 4: pso
    std::ifstream psoDataIn(pso_path_.string(), std::ifstream::binary);
    if (psoDataIn.is_open()) {
        pso_.clear();
        pso_.resize(triple_size_);
        for (size_t i = 0; i < triple_size_; ++ i) {
            psoDataIn >> pso_[i];
        }
        psoDataIn.close();
    } else {
        std::cerr << "cannot read from file: " << pso_path_ << std::endl;
        return false;
    }

    return true;
}

uint64_t Database::getIdByP(const std::string &p) {
    return p2id_[p];
}

uint64_t Database::getIdBySO(const std::string &so) {
    return so2id_[so];
}

std::string Database::getPbyId(const uint64_t id) {
    return id2p_[id];
}

std::string Database::getSObyId(const uint64_t id) {
    return id2so_[id];
}

uint64_t Database::getPMask() {
    return p_mask_;
}

uint64_t Database::getSMask() {
    return s_mask_;
}

uint64_t Database::getOMask() {
    return o_mask_;
}

int Database::getPHexLength() {
    return p_hex_len_;
}

int Database::getSOHexLength() {
    return so_hex_len_;
}

std::vector<uint64_t> Database::getPSO() const {
    return pso_;
}

std::vector<int> Database::getPredicateIndices() const {
    return p_indices_;
}

std::vector<int> Database::getPredicateRange() const {
    return p_range_;
}
