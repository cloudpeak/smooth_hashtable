#pragma once

#include <iostream>
#include <vector>
#include <list>
#include  <utility>
#include "mmap_array.h"
#include "mmap_array.h"
#include "tree_list.h"

const size_t k_max_steal_iterations = 300;

template<typename IteratorType, typename ValueType, typename TableType, typename BucketType>
class fixed_map_iterator_base {
public:
    using value_type =  ValueType;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    fixed_map_iterator_base(TableType* table, IteratorType bucket_it, size_t index, bool end)
            : table_(table), bucket_it_(bucket_it), index_(index), end_(end) {}

    // Prefix increment
    fixed_map_iterator_base& operator++() {
        increase();
        return *this;
    }

    // Postfix increment
    fixed_map_iterator_base operator++(int) {
        fixed_map_iterator_base temp = *this;
        increase();
        return temp;
    }

    ValueType & operator*() const { return *bucket_it_; }
    ValueType* operator->() const { return &(*bucket_it_); }

    bool operator==(const fixed_map_iterator_base& other) const {
        if (end_ && other.end_) {
            return true;
        }
        if (end_ || other.end_) {
            return false;
        }

        return (bucket_it_ == other.bucket_it_) && (index_ == other.index_);
    }

    bool operator!=(const fixed_map_iterator_base& other) const {
        return !(*this == other);
    }

protected:
    void increase() {
        if (end_) {
            throw std::out_of_range("Iterator is at end");
        }
        ++bucket_it_;
        if (bucket_it_ != table()[index_].end()) {
            return;
        }

        bool found = false;
        while (++index_ < table().size()) {
            if (!table()[index_].empty()) {
                bucket_it_ = table()[index_].begin();
                found = true;
                break;
            }
        }
        if (!found) {
            end_ = true;
        }
    }

    TableType& table() {
        return *table_;
    }

    IteratorType bucket_it_;
    TableType* table_;
    size_t index_ = 0;
    bool end_;
};


template <typename Key, typename Mapped,  typename Hash = std::hash<Key>>
class fixed_hashmap {
public:
    using value_type =  std::pair<Key, Mapped>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    struct ComparePairFirst {
        bool operator()(const std::pair<Key, Mapped>& a, const std::pair<Key, Mapped>& b) const {
            return a.first < b.first;
        }
    };

    // Internal structure for key-value pairs
    typedef tree_list_trivial<std::pair<Key, Mapped>, ComparePairFirst> bucket_type;

    class iterator : public fixed_map_iterator_base<typename bucket_type::iterator, value_type, mmap_array<bucket_type>, bucket_type> {
        using Base = fixed_map_iterator_base<typename bucket_type::iterator, value_type, mmap_array<bucket_type>, bucket_type>;
    public:
        iterator(mmap_array<bucket_type>* table, typename bucket_type::iterator bucket_it, size_t index, bool end) :
                fixed_map_iterator_base<typename bucket_type::iterator, value_type, mmap_array<bucket_type>, bucket_type>(table, bucket_it, index, end) {}
        friend class fixed_hashmap<Key, Mapped>;
    };

    class const_iterator : public fixed_map_iterator_base<typename bucket_type::const_iterator, const value_type , const mmap_array<bucket_type>, bucket_type> {
    public:
        explicit const_iterator(const mmap_array<bucket_type>* table, typename bucket_type::const_iterator bucket_it, size_t index, bool end) :
                fixed_map_iterator_base<typename bucket_type::const_iterator, const value_type, const mmap_array<bucket_type>, bucket_type>(table, bucket_it, index, end) {}

        explicit const_iterator(const iterator& it) :
                fixed_map_iterator_base<typename bucket_type::const_iterator, const value_type, const mmap_array<bucket_type>, bucket_type>(it.table_, it.bucket_it_, it.index_, it.end_) {}
        friend class fixed_hashmap<Key, Mapped, Hash>;
    };

    iterator begin() noexcept {
        for (size_t i = 0; i < table_.size(); ++i) {
            auto& bucket = table_[i];
            if (!bucket.empty()) {
                return iterator(&table_, bucket.begin(), i, false);
            }
        }
        return iterator(&table_, typename bucket_type::iterator(nullptr), 0, true);
    }

    iterator end() noexcept {
        return iterator(&table_, typename bucket_type::iterator(nullptr), 0, true);
    }

    const_iterator begin() const noexcept {
        for (size_t i = 0; i < table_.size(); ++i) {
            auto& bucket = table_[i];
            if (!bucket.empty()) {
                return const_iterator(&table_, bucket.begin(), i, false);
            }
        }
        return const_iterator(&table_, typename bucket_type::const_iterator(nullptr), 0, true);
    }

    const_iterator end() const noexcept {
        return const_iterator(&table_, typename bucket_type::const_iterator(nullptr), 0, true);
    }

    const_iterator cbegin() const {
        for (size_t i = 0; i < table_.size(); ++i) {
            auto& bucket = table_[i];
            if (!bucket.empty()) {
                return const_iterator(&table_, bucket.cbegin(), i, false);
            }
        }
        return const_iterator(&table_, typename bucket_type::const_iterator(nullptr), 0, true);
    }

    const_iterator cend() const {
        return const_iterator(&table_, typename bucket_type::const_iterator(nullptr), 0, true);
    }

    bool empty() const { return size_ == 0; }

public:
    // add swap support
    void swap(fixed_hashmap& other) {
        table_.swap(other.table_);
        std::swap(size_, other.size_);
        std::swap(stolen_bucket_, other.stolen_bucket_);
        std::swap(hash_function_, other.hash_function_);
    }

    // Constructor
    explicit fixed_hashmap(int initial_size = 10, const Hash& hash = Hash())
            : table_(initial_size),
              stolen_bucket_(initial_size - 1),
              size_(0),
              hash_function_(hash) {
    }

    fixed_hashmap(fixed_hashmap&& other) noexcept
            : table_(std::move(other.table_)),
              stolen_bucket_(table_.size() - 1),
              size_(other.size_),
              hash_function_(std::move(other.hash_function_)) {
        other.size_ = 0;
        other.stolen_bucket_ = 0;
    }

    // Move assignment operator
    fixed_hashmap& operator=(fixed_hashmap&& other) noexcept {
        if (this != &other) {
            table_ = std::move(other.table_);
            size_ = other.size_;
            stolen_bucket_ = other.stolen_bucket_;
            hash_function_ = std::move(other.hash_function_);
            other.size_ = 0;
            other.stolen_bucket_ = 0;
        }
        return *this;
    }
    ~fixed_hashmap() {
        clear();
    }

    void clear() {
        for(size_t i = 0; i < table_.size(); i++) {
            table_[i].clear();
        }
        table_.clear();
        size_ = 0;
    }

    // true in the bool field indicates new insertion, false indicates existing key for updating.
    template <typename... Args>
    std::pair<iterator, bool> emplace_with_key(Key && key, Args&&... args) {
        int index = hash(key);
        auto& bucket = table_[index];
        for(auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                return std::pair<iterator, bool>(iterator(&table_, it, index, false), false);
            }
        }
        auto it = table_[index].emplace(std::forward<Args>(args)...);
        size_++;
        return std::pair<iterator, bool>(iterator(&table_, it, index, false), true);
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        value_type v(std::forward<Args>(args)...);
        return emplace_with_key(std::move(v.first), std::move(v));
    }


    template <typename P>
    std::pair<iterator, bool> insert(P&& kv) {
        int index = hash(kv.first);
        auto& bucket = table_[index];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == kv.first) {
                return std::pair<iterator, bool>(iterator(&table_, it, index, false), false);
            }
        }
        auto it = bucket.insert(std::forward<P>(kv));
        size_++;
        return std::pair<iterator, bool>(iterator(&table_, it, index, false), true);
    }

    // Remove a key-value pair from the hashmap
    size_t erase(const Key& key) {
        int index = hash(key);
        auto& bucket = table_[index];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                bucket.erase(it);
                size_--;
                return 1;
            }
        }
        return 0;
    }

    // Erase an element by iterator
    iterator erase(iterator& it) {
        if (it.end_) {
            throw std::out_of_range("Iterator is at end");
        }

        auto next_it = it;
        ++next_it;
        auto bucket_it = it.bucket_it_;
        auto bucket_index = it.index_;
        // Erase the element from the linked list
        bucket_it = table_[bucket_index].erase(bucket_it);
        size_--;
        return next_it;
    }

    template< class K >
    size_t erase( K&& key) {
        int index = hash(key);
        auto& bucket = table_[index];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                bucket.erase(it);
                size_--;
                return 1;
            }
        }
        return 0;
    }

    // Check if the hashmap contains a key
    bool contains(const Key& key) const {
        int index = hash(key);
        auto& bucket = table_[index];
        for (auto it = bucket.cbegin(); it != bucket.cend(); ++it) {
            if (it->first == key) {
                return true;
            }

        }
        return false;
    }

    template <typename P>
    bool contains(P&& key) const {
        int index = hash(key);
        auto& bucket = table_[index];
        for (auto it = bucket.cbegin(); it != bucket.cend(); ++it) {
            if (it->first == key) {
                return true;
            }
        }
        return false;
    }

    std::vector<std::pair<Key, Mapped>> steal_elements(int64_t num_to_steal) {
        std::vector<std::pair<Key, Mapped>> stolen_elements;
        size_t start_bucket = stolen_bucket_;
        while (num_to_steal > 0 && stolen_bucket_ >= 0) {
            if (start_bucket - stolen_bucket_ > k_max_steal_iterations) {
                break;
            }

            auto& bucket = table_[stolen_bucket_];
            while (num_to_steal > 0 && !bucket.empty()) {
                if (stolen_elements.empty()) {
                    stolen_elements.reserve(num_to_steal);
                }
                auto it = bucket.begin();
                stolen_elements.emplace_back(std::move(*it));
                bucket.erase(it);
                num_to_steal--;
                size_--;
            }

            if (stolen_bucket_ == 0) {
                // Reached the end of the table, but bucket may be non-empty.
                if(bucket.empty()) {
                    assert(size_ == 0);
                }
                break;
            }

            if (bucket.empty() && stolen_bucket_ > 0) {
                stolen_bucket_--;
            }
        }
        return stolen_elements;
    }

    // Search for a key and return an iterator to the element
    iterator find(const Key& key) {
        size_t index = hash(key);
        auto& bucket = table_[index];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                return iterator(&table_, it, index, false);
            }
        }
        return iterator(&table_, bucket.end(), index, true);
    }

    // Search for a key and return an iterator to the element
    const_iterator find(const Key& key) const {
        size_t index = hash(key);
        auto& bucket = table_[index];
        for (auto it = bucket.cbegin(); it != bucket.cend(); ++it) {
            if (it->first == key) {
                return const_iterator(&table_, it, index, false);
            }
        }
        return const_iterator(&table_, bucket.end(), index, true);
    }

    template< class K >
    iterator find( const K& key ) {
        size_t index = hash(key);
        auto& bucket = table_[index];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                return iterator(&table_, it, index, false);
            }
        }
        return iterator(&table_, bucket.end(), index, true);
    }

    Mapped& at(const Key& key) {
        size_t index = hash(key);
        auto& bucket = table_[index];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                return  it->second;
            }
        }
        auto it = bucket.emplace(key, Mapped());
        size_++;
        return it->second;
    }

    // operator []
    Mapped& operator[](const Key& key) {
        return at(key);
    }

    const Mapped& at(const Key& key) const {
        size_t index = hash(key);
        auto& bucket = table_[index];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                return  it->second;
            }
        }
        throw std::out_of_range("Key not found");
    }

    const Mapped& operator[](const Key& key) const {
        return at(key);
    }

    // Get the number of key-value pairs in the hashmap
    size_t size() const { return size_; }

    // get bucket size_
    size_t get_bucket_count() const { return table_.size(); }

private:
    mmap_array<bucket_type> table_;
    size_t size_;
    Hash hash_function_;  // Hash
    int64_t stolen_bucket_;

    // Hash function
    size_t hash(const Key& key) const {
        return hash_function_(key) % table_.size();
    }

    template <typename P>
    size_t hash(const P& key) const {
        return hash_function_(key) % table_.size();
    }
};