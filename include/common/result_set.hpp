/*
 * @FileName   : result_set.hpp
 * @CreateAt   : 2021/10/14
 * @Author     : Inno Fang
 * @Email      : innofang@yeah.net
 * @Description:
 */

#ifndef RETRIEVE_SYSTEM_RESULT_SET_HPP
#define RETRIEVE_SYSTEM_RESULT_SET_HPP

#include <vector>
#include <unordered_map>

template<typename Key, typename Value>
class ResultSet {
public:
    using key_type = Key;
    using value_type = Value;

private:
    using item_type = std::unordered_map<key_type, value_type>;

public:
    ResultSet(): result_(0) {}
    explicit ResultSet(const size_t &size): result_(size) {}
    ResultSet(const size_t &size, item_type item): result_(size, item) {};
    ResultSet(const ResultSet<key_type, value_type> &other): result_(other) {};
    ResultSet(ResultSet<key_type, value_type> &&other) noexcept : result_(other) {};
    ~ResultSet() = default;

    typename std::vector<item_type>::iterator begin() { return result_.begin(); }
    typename std::vector<item_type>::iterator begin() const { return result_.begin(); }
    typename std::vector<item_type>::iterator end() { return result_.end(); }
    typename std::vector<item_type>::iterator end() const { return result_.end(); }

    decltype(auto) emplace_back(const item_type &item) {
        result_.emplace_back(item);
    }

    decltype(auto) push_back(const item_type &item) {
        result_.push_back(item);
    }

    decltype(auto) erase(const item_type &item) {
        return result_.erase(item);
    }

private:
    std::vector<item_type> result_;
};

#endif //RETRIEVE_SYSTEM_RESULT_SET_HPP
