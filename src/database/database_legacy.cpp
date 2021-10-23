//
// Created by InnoFang on 2021/6/19.
//

#include "database/database.hpp"

Database::Database(const std::string& dbname) : dbname_(dbname) {
    db_path_ = fs::path(__FILE__).parent_path().parent_path().parent_path()
            .append("db")
            .append(dbname);

    info_path_ = fs::path(db_path_).append("info");
    pid_path_ = fs::path(db_path_).append("pid");
    soid_path_ = fs::path(db_path_).append("soid");
    pso_path_ = fs::path(db_path_).append("pso");

    /* initialize */
    triple_size_ = 0;
    p_size_ = 0;
    so_size_ = 0;
    triples_.clear();
    p2id_.clear();
    id2p_.assign(1, "");
    so2id_.clear();
    id2so_.assign(1, "");
    p_indices_.assign(1, 0);
}

Database::~Database() {}

void Database::create(const std::string& datafile) {
    std::ifstream infile(datafile, std::ifstream::binary);
    std::ifstream::sync_with_stdio(false);
    infile.tie(nullptr);
    if (infile.is_open()) {
        std::string s, p, o;
        while (infile >> s >> p) {
            infile.ignore();
            std::getline(infile, o);
            for (o.pop_back(); o.back() == ' ' || o.back() == '.'; o.pop_back());

            triples_.emplace_back( s, p, o );

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

void Database::generatePSO() {
    pso_.clear();
    pso_.reserve(triple_size_);
    std::string s, p, o;
    for (const auto &triple : triples_) {
        std::tie(s, p, o) = triple;
        pso_.push_back(gPSO::encodePSO(p2id_[p], so2id_[s], so2id_[o], so_hex_len_));
    }

    std::sort(pso_.begin(), pso_.end());
}

bool Database::store() {
    fs::ofstream::sync_with_stdio(false);
    fs::create_directories(db_path_);

    // database file 1: info
    auto store_info = [&]() {
        fs::ofstream infoDataOut(info_path_, std::ofstream::binary);
        infoDataOut.tie(nullptr);
        if (infoDataOut.is_open()) {
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
    auto pid_task = std::async(std::launch::async, [](fs::path& path, std::vector<std::string>& p_list) {

        fs::ofstream pidDataOut(path, fs::ofstream::binary);
        pidDataOut.tie(nullptr);
        if (pidDataOut.is_open()) {
            for (size_t i = 1; i < p_list.size(); ++ i) {
//                pidDataOut << i << "\t" << p_list[i] << "\n";
                std::string id_p = std::to_string(i) + "\t" + p_list[i] + "\n";
                pidDataOut.write(id_p.c_str(), id_p.size());
            }
            pidDataOut.close();
        }

    }, std::ref(pid_path_), std::ref(id2p_));

    // database file 3: soid
    // soid su/object
    auto soid_task = std::async(std::launch::async, [](fs::path& path, std::vector<std::string>& so_list) {

        fs::ofstream soidDataOut(path, std::ofstream::binary);
        soidDataOut.tie(nullptr);
        if (soidDataOut.is_open()) {
            for (size_t i = 1; i < so_list.size(); ++i) {
//                soidDataOut << i << "\t" << so_list[i] << "\n";
                std::string id_so = std::to_string(i) + "\t" + so_list[i] + "\n";
                soidDataOut.write(id_so.c_str(), id_so.size());
//                soidDataOut << i << std::endl;;
            }
            soidDataOut.close();
        }

    }, std::ref(soid_path_), std::ref(id2so_));

    // database file 4: pso
        auto pso_task = std::async(std::launch::async, [&](fs::path& path, std::vector<uint64_t>& pso_list) {
        fs::ofstream psoDataOut(path, std::ofstream::binary);
        psoDataOut.tie(nullptr);
        if (psoDataOut.is_open()) {
            for (auto& pso : pso_list) {
                std::string pso_str = std::to_string(pso) + "\n";
                psoDataOut.write(pso_str.c_str(), pso_str.size());
            }
            psoDataOut.close();
        } else {
            std::cerr << "cannot create file: "<< pso_path_ << std::endl;
        }
    }, std::ref(pso_path_), std::ref(pso_));

    store_info();
    pid_task.wait();
    soid_task.wait();
    pso_task.wait();

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
    // id2p_.clear();
    id2p_.resize(p_size_ + 1);
    p2id_.clear();
    p2id_.reserve(p_size_);
    std::ifstream pidDataIn(pid_path_.string(), std::ifstream::binary);
    if (pidDataIn.is_open()) {
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
    // id2so_.clear();
    id2so_.resize(so_size_ + 1);
    so2id_.clear();
    so2id_.reserve(so_size_);
    std::ifstream soidDataIn(soid_path_.string(), std::ifstream::binary);
    if (soidDataIn.is_open()) {

        for (size_t i = 0; i < so_size_; ++ i) {
            uint64_t index;
            std::string suobject;
            soidDataIn >> index;
            soidDataIn.ignore();
            std::getline(soidDataIn, suobject);
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

std::tuple<uint64_t, uint64_t> Database::getVarPSOAndMask(const gPSO::triplet& triplet) {
    std::string s, p, o;
    std::tie(s, p, o) = triplet;

    uint64_t predicate = (p[0] == '?') ? 0 : p2id_[p];
    uint64_t subject = (s[0] == '?') ? 0 : so2id_[s];
    uint64_t object = (o[0] == '?') ? 0 : so2id_[o];

    uint64_t pso = gPSO::encodePSO(predicate, subject, object, so_hex_len_);
    uint64_t pso_mask = (predicate == 0 ? 0 : p_mask_)
                        | (subject == 0 ? 0 : s_mask_)
                        | (object == 0 ? 0 : o_mask_);
    return std::make_tuple(pso, pso_mask);
}

std::vector<std::unordered_map<std::string, uint64_t>> Database::getQualifiedSOList(const gPSO::triplet& query_triplet) {
    std::string s, p, o;
    std::tie(s, p, o) = query_triplet;

    uint64_t query_pso, query_pso_mask;
    std::tie(query_pso, query_pso_mask) = getVarPSOAndMask(query_triplet);

    if ((query_pso & p_mask_) == 0) {
        std::cout << "without predicate" << std::endl;
        return {};
    }

    // get subject/predicate/object label respectively from triplet
    uint64_t pid, sid, oid;
    std::tie(pid, sid, oid) = gPSO::decodePSO(query_pso, query_pso_mask, so_hex_len_);

    int p_size = p_indices_[pid];
    std::vector<std::unordered_map<std::string, uint64_t>> qualified_so;
    qualified_so.reserve(p_size);

    int start = p_range_[pid], end = p_range_[pid + 1];
    std::for_each(pso_.begin() + start, pso_.begin() + end, [&](uint64_t pso) {
        if ((pso & query_pso_mask) == query_pso) {
            uint64_t match_pso = pso & (~query_pso_mask);

            std::tie(pid, sid, oid) = gPSO::decodePSO(match_pso, so_hex_len_);

            std::unordered_map<std::string, uint64_t> item;

            if (sid != 0) {
                item[s] = sid;
            }
            if (oid != 0) {
                item[o] = oid;
            }

            qualified_so.emplace_back( std::move(item) );
        }
    });
    return qualified_so;
}

std::string Database::getSOByID(const uint64_t& so_id) const {
    return id2so_[so_id];
}

