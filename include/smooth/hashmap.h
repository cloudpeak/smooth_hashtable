#pragma once

#include <iostream>
#include <functional>
#include <chrono>
#include <initializer_list>
#include "fixed_hashmap.h"

const static bool k_iter_end = true;
const static bool k_iter_valid = false;

const static bool k_item_found = true;
const static bool k_item_not_found = false;

const static int64_t k_num_items_to_steal = 1;

// Iterator base class
template<typename IteratorType, typename ValueType, typename TableType>
class hashmap_iterator_base {
public:
    hashmap_iterator_base(TableType* current, TableType* old, int which, IteratorType it, bool end)
            : table_current_(current), table_old_(old), it_(it),  end_(end), which_(0) {}

    hashmap_iterator_base(TableType* current, TableType* old, bool end)
            : table_current_(current), table_old_(old), it_(current->end()),  end_(end), which_(0) {}

    // Prefix increment
    hashmap_iterator_base& operator++() {
        increase();
        return *this;
    }

    // Postfix increment
    hashmap_iterator_base operator++(int) {
        hashmap_iterator_base temp = *this;
        increase();
        return temp;
    }

    ValueType & operator*() const { return *it_; }
    ValueType* operator->() const { return &(*it_); }

    bool operator==(const hashmap_iterator_base& other) const {
        if (end_ && other.end_) {
            return true;
        }
        if (end_ || other.end_) {
            return false;
        }

        if(which_ != other.which_) {
            return false;
        }

        return it_ != other.it_;
    }

    bool operator!=(const hashmap_iterator_base& other) const {
        return !(*this == other);
    }

protected:
    void increase() {
        if (end_) {
            throw std::out_of_range("Iterator is at end");
        }

        if (which_ == 0) {
            // Advance the iterator in the current fixed_map.
            ++it_;
            if (it_ != table_current().end()) {
                return;
            }

            // Switch to the old fixed_map.
            which_ = 1;
            it_ = table_old().begin();

            // If the old fixed_map is also empty, mark the iterator as end.
            if (it_ == table_old().end()) {
                which_ = 0;
                end_ = true;
                return;
            }
        } else { // which_ == 1 (old fixed_map)
            // Advance the iterator in the old fixed_map.
            ++it_;
            if (it_ != table_old().end()) {
                return;
            }

            // All elements in both fixed_maps have been iterated, mark the iterator as end.
            which_ = 0;
            end_ = true;
        }
    }

    TableType& table_current() {
        return *table_current_;
    }

    TableType& table_old() {
        return *table_old_;
    }

    IteratorType it_;
    TableType* table_current_;
    TableType* table_old_;
    bool end_;
    int which_; // 0 for current, 1 for old
};

template <typename Key, typename Mapped, typename Hash = std::hash<Key>>
class hashmap {
public:
    using value_type =  std::pair<Key, Mapped>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;
    using size_type = std::size_t;
    using fixed_map_type = fixed_hashmap<Key, Mapped, Hash>;

    // Non-const iterator
    class iterator : public hashmap_iterator_base<typename fixed_map_type::iterator, value_type, fixed_map_type> {
        using Base = hashmap_iterator_base<typename fixed_map_type::iterator, value_type, fixed_map_type>;
    public:
        using Base::Base; // Inherit constructors
        friend class hashmap<Key, Mapped, Hash>;
    };

    class const_iterator : public hashmap_iterator_base<typename fixed_map_type::const_iterator, const value_type, const fixed_map_type> {
        using Base = hashmap_iterator_base<typename fixed_map_type::const_iterator, const value_type, const fixed_map_type>;
    public:
        using Base::Base; // Inherit constructors
        explicit const_iterator(const iterator& it) : Base(it.table_current_, it.table_old_, it.end_) {}
        friend class fixed_hashmap<Key, Mapped, Hash>;
    };

    iterator begin() noexcept {
        // Start by assuming the iterator is pointing to the current set.
        auto it = current_.begin();
        // If the current set is empty, check the old set.
        if (it == current_.end()) {
            it = old_.begin();

            // If both sets are empty, mark the iterator as end.
            if (it == old_.end()) {
                return new_iterator(0, it, k_iter_end);
            }
            return new_iterator( 1, it, k_iter_valid);
        }

        return new_iterator( 0, it, k_iter_valid);
    }

    iterator end() noexcept {
        return iterator(&current_, &old_, k_iter_end);
    }

    const_iterator begin() const noexcept {
        // Start by assuming the iterator is pointing to the current set.
        auto it = current_.begin();
        // If the current set is empty, check the old set.
        if (it == current_.end()) {
            it = old_.begin();

            // If both sets are empty, mark the iterator as end.
            if (it == old_.end()) {
                return new_iterator(0, it, k_iter_end);
            }
            return new_iterator( 1, it, k_iter_valid);
        }

        return new_iterator( 0, it, k_iter_valid);
    }

    const_iterator end() const noexcept {
        return const_iterator(&current_, &old_, k_iter_end);
    }

    const_iterator cbegin() const {
        return begin();
    }

    const_iterator cend() const {
        return end();
    }

    // Constructor
    explicit hashmap(int initial_size = 10, const Hash& hash = Hash())
            : rehashing_(false),
              current_(initial_size, hash),
              old_(initial_size, hash) {}

    explicit hashmap(std::initializer_list<typename fixed_map_type::value_type> pairs,
                     int initial_size = 10, const Hash& hash = Hash())
            : rehashing_(false),
              current_(initial_size, hash),
              old_(initial_size, hash) {
        for(auto& pair : pairs) {
            insert(pair);
        }
    }

    ~hashmap() {
    }

    std::pair<iterator, bool> insert(value_type && kv) {
        move_progressively();
        maybe_rehash_guard guard(*this);
        if(!rehashing_) {
            auto result = current_.insert(std::forward<decltype(kv)>(kv));
            return std::make_pair(iterator( &current_, &old_, 0, result.first, k_iter_valid), result.second);
        }

        auto it_old = old_.find(kv.first);
        if (it_old != old_.end()) {
            return std::make_pair(iterator( &current_, &old_, 1, it_old, k_iter_valid), k_item_not_found);
        }

        auto result = current_.insert(std::forward<decltype(kv)>(kv));
        return std::make_pair(iterator( &current_, &old_, 0, result.first, k_iter_valid), result.second);
    }

    template<typename P>
    std::pair<iterator, bool> insert(P&& kv) {
        move_progressively();
        maybe_rehash_guard guard(*this);
        if(!rehashing_) {
            auto result = current_.insert(std::forward<decltype(kv)>(kv));
            return std::make_pair(iterator( &current_, &old_, 0, result.first, k_iter_valid), result.second);
        }

        auto it_old = old_.find(kv.first);
        if (it_old != old_.end()) {
            return std::make_pair(iterator( &current_, &old_, 1, it_old, k_iter_valid), k_item_not_found);
        }

        auto result = current_.insert(std::forward<decltype(kv)>(kv));
        return std::make_pair(iterator( &current_, &old_, 0, result.first, k_iter_valid), result.second);
    }

    template<typename P>
    std::pair<iterator, bool> emplace(P&& kv) {
        move_progressively();
        maybe_rehash_guard guard(*this);

        if(!rehashing_) {
            auto result = current_.emplace(std::forward<decltype(kv)>(kv));
            return std::make_pair(iterator( &current_, &old_, 0, result.first, k_iter_valid), result.second);
        }

        auto it_old = old_.find(kv.first);
        if (it_old != old_.end()) {
            return std::make_pair(iterator( &current_, &old_, 1, it_old, k_iter_valid), k_item_not_found);
        }

        auto result = current_.emplace(std::forward<decltype(kv)>(kv));
        return std::make_pair(iterator( &current_, &old_, 0, result.first, k_iter_valid), result.second);
    }

    Mapped& at(const Key& key) {
        move_progressively();
        maybe_rehash_guard guard(*this);
        if(!rehashing_) {
            //fast path
            return current_.at(key);
        }

        auto it_old = old_.find(key);
        if (it_old != old_.end()) {
            return it_old->second;
        }

        return current_.at(key);
    }

    const Mapped& at(const Key& key) const {
        if(!rehashing_) {
            //fast path
            return current_.at(key);
        }

        auto it_old = old_.find(key);
        if (it_old != old_.end()) {
            return it_old->second;
        }

        return current_.at(key);
    }

    Mapped& operator[](const Key& key) {
        return at(key);
    }

    const Mapped& operator[](Key&& key) const {
        return at(key);
    }

    // Remove a key-value pair from the hashmap
    size_type erase(const Key& key) {
        move_progressively();
        maybe_rehash_guard guard(*this);
        if(!rehashing_) {
            return current_.erase(key);
        }
        size_t num1 = current_.erase(key);
        size_t num2 = old_.erase(key);
        maybe_rehash();
        return std::max(num1, num2);
    }

    template <typename P>
    size_type erase(const Key& key) {
        move_progressively();
        maybe_rehash_guard guard(*this);
        if(!rehashing_) {
            return current_.erase(key);
        }
        size_type num1 = current_.erase(key);
        size_type num2 = old_.erase(key);
        maybe_rehash();
        return std::max(num1, num2);
    }

    iterator find(const Key& key) {
        if(!rehashing_) {
            auto it = current_.find(key);
            new_iterator(0, it, it == current_.end());
        }
        bool current_is_larger = current_.size() > old_.size();
        auto& larger = current_is_larger ? current_ : old_;
        auto& smaller = current_is_larger ? old_ : current_;
        const int larger_index = current_is_larger ? 0 : 1;

        auto it = larger.find(key);
        if (it != larger.end()) {
           return new_iterator(larger_index, it, k_iter_valid);
        }

        it = smaller.find(key);
        if (it != smaller.end()) {
           return new_iterator(1 - larger_index, it, k_iter_valid);
        }

        // Not found in either table
        return new_iterator(0, current_.end(), k_iter_end);
    }

    const_iterator find(const Key& key ) const {
        if(!rehashing_) {
            auto it = current_.find(key);
            new_iterator(0, it, it == current_.end());
        }
        bool current_is_larger = current_.size() > old_.size();
        const auto& larger = current_is_larger ? current_ : old_;
        const auto& smaller = current_is_larger ? old_ : current_;
        const int larger_index = current_is_larger ? 0 : 1;

        auto it = larger.find(key);
        if (it != larger.end()) {
            return new_iterator(larger_index, it, k_iter_valid);
        }

        it = smaller.find(key);
        if (it != smaller.end()) {
            return new_iterator(1 - larger_index, it, k_iter_valid);
        }

        // Not found in either table
        return new_iterator(0, current_.end(), k_iter_end);
    }

    template <class K>
    iterator find( const K& key ) {
        auto it = current_.find(key);
        if (it == current_.end()) {
            if(!rehashing_) {
                return new_iterator( 0, it, k_iter_end);
            }
            it = old_.find(key);
            if (it == old_.end()) {
                return new_iterator( 0, it, k_iter_end);
            }
            return new_iterator( 1, it, k_iter_valid);
        }
        return  new_iterator(  0, it, k_iter_valid);
    }


    // Check if the hashmap contains a key
    bool contains(const Key& key) const {
        return current_.contains(key) || old_.contains(key);
    }

    template<class P>
    bool contains(const P& key) const {
        return current_.contains(key) || old_.contains(key);
    }

    // Get the number of key-value pairs in the hashmap
    size_type size() const { return current_.size() + old_.size(); }

    void clear() {
        current_.clear();
        old_.clear();
        rehashing_ = false;
    }

private:
    class maybe_rehash_guard {
    public:
        explicit maybe_rehash_guard(hashmap& map) : map_(map) {}
        ~maybe_rehash_guard() {
            map_.maybe_rehash();
        }
    private:
        hashmap& map_;
    };

    void maybe_rehash() {
        if(rehashing_) {
            return;
        }

        size_type map_size = current_.size();
        size_type bucket_size = current_.get_bucket_count();
        // of element count is more than 3/4 of the bucket count
        if(map_size * 4 >= bucket_size * 3 ) {
            rehash(bucket_size * 2);
        } else if(bucket_size > map_size*4 && bucket_size > 16) {
            // When bucket_size = 12 and map_size = 9, the map's bucket_size will be expanded to 24.
            // This means that the single bucket_size is 2.66 times the map_size. This is considered ok.
            // Therefore, we can perform a shrink operation when the multiplier is 4.
            // This will reduce the bucket_size to 3 times the map_size.
            shrink(map_size*3);
        }
    }

    void shrink(size_type new_size) {
        rehash(new_size);
    }

    // Rehash the hashmap
    void rehash(size_type new_size) {
        assert(old_.empty());
        old_ = fixed_hashmap<Key, Mapped, Hash>(new_size);
        old_.swap(current_);
        rehashing_ = true;
    }

    void on_rehashing_finished() {
        // release the old memory
        old_ = fixed_hashmap<Key, Mapped, Hash>(1);
    }

    void move_progressively() {
        if (!rehashing_) {
            return;
        }

        auto elements = old_.steal_elements(k_num_items_to_steal);
        if (elements.empty() && old_.empty()) {
            rehashing_ = false;
            on_rehashing_finished();
            return;
        }

        for (auto& element : elements) {
           current_.insert(std::move(element));
        }
    }

    iterator new_iterator(int which, typename fixed_map_type::iterator it, bool end)  {
        return iterator(&current_, &old_, which, it, end);
    }

    const_iterator new_iterator(int which, typename fixed_map_type::const_iterator it, bool end) const noexcept  {
        return const_iterator(&current_, &old_, which, it, end);
    }

private:
    fixed_hashmap<Key, Mapped, Hash> current_;  // Current container
    fixed_hashmap<Key, Mapped, Hash> old_;     // Old container
    bool rehashing_;
};