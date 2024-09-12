#pragma once

#include <type_traits>
#include <unordered_set>

template <typename Key, typename Hash = std::hash<Key>>
struct HashSet {
    using hash_t = std::result_of_t<Hash{}, Key>;

    HashSet() : set() {}

    void insert(hash_t hash) { set.insert(hash); }
    void insert(Key key) { set.insert(Hash(key)); }

  private:
    std::unordered_set<hash_t> set;
};
