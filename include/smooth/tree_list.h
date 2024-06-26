// Copyright (c) 2024 Tin Project. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstdint>
#include <memory>
#include <cmath>
#include <cassert>

namespace smooth {

template<typename T, typename compare = std::less<T>>
class tree_list_base {
public:
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T *;
    using reference = T &;

    enum node_color {
        k_red = 1, k_black
    };

    // Node structure for the red-black tree.
    class rb_node_type {

    public:
        // Constructor. Newly-created nodes are colored red.
        explicit rb_node_type(const T &data)
                : left_(nullptr),
                  right_(nullptr),
                  parent_(nullptr),
                  color_(k_red),
                  data_(data) {}

        template<typename... Args>
        explicit rb_node_type(Args &&... args)
                : left_(nullptr),
                  right_(nullptr),
                  parent_(nullptr),
                  color_(k_red),
                  data_(std::forward<Args>(args)...) {}

        rb_node_type(const rb_node_type &) = delete;

        rb_node_type &operator=(const rb_node_type &) = delete;

        virtual ~rb_node_type() = default;

        node_color get_color() const { return color_; }

        void set_color(node_color color) { color_ = color; }

        // Fetches the user data.
        T &data() { return data_; }

        T const &data() const { return data_; }

        // Copies all user-level fields from the source rb_node_type, but not
        // internal fields. For example, the base implementation of this
        // method copies the "m_data" field, but not the child or parent
        // fields. Any augmentation information also does not need to be
        // copied, as it will be recomputed. Subclasses must call the
        // superclass implementation.
        virtual void copy_from(rb_node_type *src) { data_ = src->data(); }

        rb_node_type *left() { return left_; }

        rb_node_type const *left() const { return left_; }

        void set_left(rb_node_type *node) { left_ = node; }

        rb_node_type const *right() const { return right_; }

        rb_node_type *right() { return right_; }

        void set_right(rb_node_type *node) { right_ = node; }

        rb_node_type const *parent() const { return parent_; }

        rb_node_type *parent() { return parent_; }

        void set_parent(rb_node_type *node) { parent_ = node; }

        bool is_left_child() const { return parent_ && parent_->left_ == this; }

    private:
        rb_node_type *left_;
        rb_node_type *right_;
        rb_node_type *parent_;
        node_color color_;
        T data_;
    };

    enum data_struct_type {
        k_linked_list,
        k_red_black_tree,
    };

    // Node structure for the linked list.
    struct list_node_type {
        T data;
        list_node_type *next;

        explicit list_node_type(const T &data) : data(data), next(nullptr) {}

        explicit list_node_type(T &&data) : data(std::move(data)), next(nullptr) {}

        template<typename... Args>
        list_node_type(Args &&... args) : data(std::forward<Args>(args)...), next(nullptr) {}
    };

    // Mixed node structure
    struct mixed_node_type {
        mixed_node_type() = default;

        data_struct_type ds_type_;
        union {
            list_node_type *list_node_;
            rb_node_type *tree_node_;
        };

        explicit mixed_node_type(list_node_type *node) : list_node_(node),
                                                         ds_type_(data_struct_type::k_linked_list) {}

        explicit mixed_node_type(rb_node_type *node) : tree_node_(node),
                                                       ds_type_(data_struct_type::k_red_black_tree) {}

        explicit mixed_node_type(list_node_type *node, rb_node_type *, data_struct_type type) : list_node_(node),
                                                                                                ds_type_(type) {}


        bool operator!=(const mixed_node_type &other) const {
            return tree_node_ != other.tree_node_;
        }

        bool operator==(const mixed_node_type &other) const {
            return tree_node_ == other.tree_node_;
        }

        bool is_null() const {
            if (ds_type_ == data_struct_type::k_linked_list) {
                return list_node_ == nullptr;
            }
            return tree_node_ == nullptr;
        }
    };


    // Define a base class for iterators
    template<typename ValueType>
    class iterator_base {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = ValueType;
        using difference_type = std::ptrdiff_t;
        using pointer = ValueType *;
        using reference = ValueType &;

        // Constructor
        explicit iterator_base(mixed_node_type node) : node_(node) {}

        explicit iterator_base(struct list_node_type *ptr)
                : node_{ptr} {}

        // Dereference operator
        ValueType &operator*() const { return get_data(); }

        // Arrow operator
        ValueType *operator->() const { return &get_data(); }

        // Pre-increment operator
        iterator_base &operator++() {
            increment();
            return *this;
        }

        // Post-increment operator
        iterator_base operator++(int) {
            iterator_base temp(node_);
            increment();
            return temp;
        }

        // Equality operator
        bool operator==(const iterator_base &other) const {
            return node_ == other.node_;
        }

        // Inequality operator
        bool operator!=(const iterator_base &other) const {
            return node_ != other.node_;
        }

    protected:
        ValueType &get_data() const {
            if (node_.ds_type_ == data_struct_type::k_linked_list) {
                return node_.list_node_->data;
            }
            return node_.tree_node_->data();
        }

        void increment() {
            if (node_.ds_type_ == data_struct_type::k_linked_list) {
                node_.list_node_ = node_.list_node_->next;
            } else {
                node_.tree_node_ = walk_to_next_node(node_.tree_node_);
            }
        }

        void decrement() {
            if (node_.ds_type_ == data_struct_type::k_linked_list) {
                node_.list_node_ = node_.list_node_->prev;
            } else {
                node_.tree_node_ = walk_to_prev_node(node_.tree_node_);
            }
        }

    private:
        friend class tree_list_base<T>;

    public:
        mixed_node_type node_;
    };

// Non-const iterator
    class iterator : public iterator_base<T> {
        using Base = iterator_base<T>;

    public:
        using Base::Base;  // Inherit constructors
        friend class tree_list_base<T>;  // Grant access from tree_list
    };

// Const iterator
    class const_iterator : public iterator_base<const T> {
        using Base = iterator_base<const T>;

    public:
        using Base::Base;  // Inherit constructors
        explicit const_iterator(const iterator &it) : Base(it.node_) {}

        friend class tree_list_base<T>;  // Grant access from tree_list
    };


    tree_list_base() = default;

    ~tree_list_base() = default;

    tree_list_base(const tree_list_base &) = default;

    tree_list_base &operator=(const tree_list_base &) = default;

    tree_list_base(tree_list_base &&) = default;

    tree_list_base &operator=(tree_list_base &&) = default;


    void clear() {
        clear_list();
        clear_tree();
        assert(size_ == 0);
        ds_type_ = data_struct_type::k_linked_list;
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    template<typename P>
    iterator insert(P &&data) {
        treefy_or_un_treefy();
        if (ds_type_ == data_struct_type::k_linked_list) {
            const auto list_node = list_insert(std::forward<decltype(data)>(data));
            return iterator(list_node);
        }
        auto rb_node = tree_insert(std::forward<decltype(data)>(data));
        return iterator(mixed_node_type(rb_node));
    }

    template<typename... Args>
    iterator emplace(Args &&... args) {
        treefy_or_un_treefy();
        if (ds_type_ == data_struct_type::k_linked_list) {
            auto list_node = list_emplace(std::forward<Args>(args)...);
            return iterator(list_node);
        }
        mixed_node_type mix_node(tree_emplace(std::forward<Args>(args)...));
        return iterator(mix_node);
    }

    iterator insert(iterator pos, const T &data) {
        treefy_or_un_treefy();
        if (ds_type_ == data_struct_type::k_linked_list) {
            auto list_node = list_insert(data);
            return iterator(list_node);
        }
        auto rb_node = tree_insert(data);
        return iterator(mixed_node_type(rb_node));
    }

    // Search for a node with the given data.
    iterator find(const T &data) const {
        if (ds_type_ == data_struct_type::k_linked_list) {
            return iterator(list_search(data));
        }

        mixed_node_type node(tree_search(data));
        return iterator(node);
    }

    bool tree_erase(const T &data) {
        rb_node_type *node = tree_search(data);
        if (node) {
            delete_node(node);
            return true;
        }
        return false;
    }

    void swap(tree_list_base &other) {
        std::swap(size_, other.size_);
        std::swap(ds_type_, other.ds_type_);
        std::swap(root_, other.root_);
    }

    // Erase a node with the given data.
    void erase(const T &data) {
        if (ds_type_ == data_struct_type::k_linked_list) {
            list_erase(data);
        }
        tree_erase(data);
    }

    // Erase an element at the given iterator
    iterator erase(iterator it) {
        if (it.node_.is_null()) {
            return it; // Already at end
        }

        if (it.node_.ds_type_ == data_struct_type::k_linked_list) {
            std::unique_ptr<list_node_type> to_delete(it.node_.list_node_);

            // Handle if it's the head
            if (to_delete.get() == head_) {
                head_ = to_delete->next;
                it = iterator(head_);
            } else {
                list_node_type *prev_node = head_;
                while (prev_node->next != to_delete.get()) {
                    prev_node = prev_node->next;
                }
                prev_node->next = to_delete->next;
                it = iterator(prev_node->next);
            }
            --size_;
        } else {
            delete_node(it.node_.tree_node_);
        }
        return it;
    }

    // Get iterator to the beginning
    iterator begin() noexcept {
        if (ds_type_ == data_struct_type::k_red_black_tree) {
            rb_node_type *rb_node = root_;
            while (rb_node->left() != nullptr) {
                rb_node = rb_node->left();
            }
            return iterator(mixed_node_type(rb_node));
        }
        return iterator(head_);
    }

    // Get iterator to the end
    iterator end() noexcept {
        return iterator(nullptr);
    }

    const_iterator begin() const noexcept {
        if (ds_type_ == data_struct_type::k_linked_list) {
            return const_iterator(head_);
        }

        rb_node_type *rb_node = root_;
        while (rb_node->left() != nullptr) {
            rb_node = rb_node->left();
        }
        return const_iterator(mixed_node_type(rb_node));
    }

    const_iterator end() const noexcept {
        return const_iterator(nullptr);
    }

    // Get const_iterator to the beginning
    const_iterator cbegin() const {
        if (ds_type_ == data_struct_type::k_linked_list) {
            return const_iterator(head_);
        }

        rb_node_type *rb_node = root_;
        while (rb_node->left() != nullptr) {
            rb_node = rb_node->left();
        }
        return const_iterator(mixed_node_type(rb_node));
    }

    // Get const_iterator to the end
    const_iterator cend() const {
        return const_iterator(nullptr);
    }

protected:
    // Private helper functions for list operations
    bool update_node(rb_node_type *) { return false; }

    void un_treefy() {
        traversal_un_treefy(root_);
    }

    void traversal_un_treefy(rb_node_type *node) {
        if (node->left() != nullptr) {
            traversal_un_treefy(node->left());
        }
        if (node->right() != nullptr) {
            traversal_un_treefy(node->right());
        }

        list_insert(std::move(node->data()));
        delete node;
    }

    void treefy_or_un_treefy() {
        if (ds_type_ == data_struct_type::k_linked_list) {
            if (size_ >= 10) {
                treefy();
            }
        } else {
            if (size_ <= 3) {
                un_treefy();
            }
        }
    }

    rb_node_type *treefy() {
        list_node_type *node = head_;
        root_ = nullptr;
        rb_node_type *rb_node = nullptr;
        while (node != nullptr) {
            auto tree_node = new rb_node_type(node->data);
            tree_insert_node(tree_node);
            node = node->next;
        }
        ds_type_ = data_struct_type::k_red_black_tree;
        return rb_node;
    }

    list_node_type *list_search(const T &data) const {
        list_node_type *node = head_;
        while (node != nullptr) {
            if (node->data == data) {
                return node;
            }
            node = node->next;
        }
        return nullptr;
    }

    template<typename P>
    list_node_type *list_insert(P &&data) {
        auto old = head_;
        head_ = new list_node_type(std::forward<decltype(data)>(data));
        head_->next = old;
        ++size_;
        return head_;
    }

    template<typename... Args>
    list_node_type *list_emplace(Args &&... args) {
        auto old = head_;
        head_ = new list_node_type(std::forward<decltype(args)>(args)...);
        head_->next = old;
        ++size_;
        return head_;
    }

    void list_erase(const T &data) {
        list_node_type *prev_node = nullptr;
        list_node_type *current_node = head_;
        std::unique_ptr<list_node_type> to_delete;
        while (current_node != nullptr) {
            if (current_node->data == data) {
                if (prev_node == nullptr) {
                    head_ = current_node->next;
                } else {
                    prev_node->next = std::move(current_node->next);
                }
                to_delete.reset(current_node);
                --size_;
                return;
            }
            prev_node = current_node;
            current_node = current_node->next;
        }
    }

    // Private helper functions for tree operations
    template<typename P>
    rb_node_type *tree_insert(P &&data) {
        auto *new_node = new rb_node_type(std::forward<decltype(data)>(data));
        size_++;
        tree_insert_node(new_node);
        return new_node;
    }

    template<typename... Args>
    rb_node_type *tree_emplace(Args &&... args) {
        auto *new_node = new rb_node_type(std::forward<decltype(args)>(args)...);
        tree_insert_node(new_node);
        size_++;
        return new_node;
    }


    static rb_node_type *walk_to_leftmost_minor(rb_node_type *node) {
        while (node->left() != nullptr) {
            node = node->left();
        }
        return node;
    };

    static rb_node_type *walk_to_right_most_minor(rb_node_type *node) {
        while (node->right() != nullptr) {
            node = node->right();
        }
        return node;
    };


    static rb_node_type *walk_to_next_node(rb_node_type *node) {
        if (node->right() != nullptr) {
            return walk_to_leftmost_minor(node->right());
        }
        rb_node_type *parent = node->parent();
        while (parent != nullptr && node == parent->right()) { // node is right child
            node = parent;
            parent = parent->parent();
        }
        // If a node has no parent, it is the root node, and has no next node.
        return parent;
    }

    static rb_node_type *walk_to_prev_node(rb_node_type *node) {
        if (node->left() != nullptr) {
            return walk_to_right_most_minor(node->left());
        }
        rb_node_type *parent = node->parent();
        while (parent != nullptr && node == parent->left()) {  // node is left child
            node = parent;
            parent = parent->parent();
        }
        // If a node has no parent, it is the root node, and has no prev node.
        return parent;
    }

    // Searches the tree for the given datum.
    rb_node_type *tree_search(const T &data) const {
        return tree_search_normal(root_, data);
    }

    // Searches the tree using the normal comparison operations,
    // suitable for simple data types such as numbers.
    rb_node_type *tree_search_normal(rb_node_type *current, const T &data) const {
        while (current) {
            if (current->data() == data)
                return current;
            if (compare()(data, current->data()))
                current = current->left();
            else
                current = current->right();
        }
        return nullptr;
    }

    void clear_list() {
        if (ds_type_ != data_struct_type::k_linked_list || head_ == nullptr) {
            return;
        }

        auto current = head_;
        while (current != nullptr) {
            list_node_type *next = current->next;
            {
                std::unique_ptr<list_node_type> to_delete(current);
            }
            current = next;
            --size_;
        }
        head_ = nullptr;
    }

    void clear_tree() {
        if (ds_type_ != data_struct_type::k_red_black_tree || root_ == nullptr) {
            return;
        }
        tree_free(root_);
        root_ = nullptr;
    }

    void tree_free(rb_node_type *node) {
        if (!node)
            return;
        if (node->left())
            tree_free(node->left());
        if (node->right())
            tree_free(node->right());

        std::unique_ptr<rb_node_type> to_delete(node);
        --size_;
    }

    // Fix the tree after an insertion to maintain red-black properties.
    void tree_insert_node(rb_node_type *z) {
        rb_node_type *y = nullptr;
        rb_node_type *x = root_;
        while (x) {
            y = x;
            if (compare()(z->data(), x->data()))
                x = x->left();
            else
                x = x->right();
        }
        z->set_parent(y);
        if (!y) {
            root_ = z;
        } else {
            if (compare()(z->data(), y->data()))
                y->set_left(z);
            else
                y->set_right(z);
        }
    }

    // Finds the rb_node_type following the given one in sequential ordering of
    // their data, or null if none exists.
    rb_node_type *tree_successor(rb_node_type *x) {
        if (x->right())
            return tree_minimum(x->right());
        rb_node_type *y = x->parent();
        while (y && x == y->right()) {
            x = y;
            y = y->parent();
        }
        return y;
    }

    // Finds the minimum element in the sub-tree rooted at the given
    // rb_node_type.
    rb_node_type *tree_minimum(rb_node_type *x) {
        while (x->left())
            x = x->left();
        return x;
    }

    // Helper for maintaining the augmented red-black tree.
    void propagate_updates(rb_node_type *start) {
        bool should_continue = true;
        while (start && should_continue) {
            should_continue = update_node(start);
            start = start->parent();
        }
    }

    //----------------------------------------------------------------------
    // Red-Black tree operations
    //

    // Left-rotates the subtree rooted at x.
    // Returns the new root of the subtree (x's right child).
    rb_node_type *left_rotate(rb_node_type *x) {
        // Set y.
        rb_node_type *y = x->right();

        // Turn y's left subtree into x's right subtree.
        x->set_right(y->left());
        if (y->left())
            y->left()->set_parent(x);

        // Link x's parent to y.
        y->set_parent(x->parent());
        if (!x->parent()) {
            root_ = y;
        } else {
            if (x == x->parent()->left())
                x->parent()->set_left(y);
            else
                x->parent()->set_right(y);
        }

        // Put x on y's left.
        y->set_left(x);
        x->set_parent(y);

        // Update nodes lowest to highest.
        update_node(x);
        update_node(y);
        return y;
    }

    // Right-rotates the subtree rooted at y.
    // Returns the new root of the subtree (y's left child).
    rb_node_type *right_rotate(rb_node_type *y) {
        // Set x.
        rb_node_type *x = y->left();

        // Turn x's right subtree into y's left subtree.
        y->set_left(x->right());
        if (x->right())
            x->right()->set_parent(y);

        // Link y's parent to x.
        x->set_parent(y->parent());
        if (!y->parent()) {
            root_ = x;
        } else {
            if (y == y->parent()->left())
                y->parent()->set_left(x);
            else
                y->parent()->set_right(x);
        }

        // Put y on x's right.
        x->set_right(y);
        y->set_parent(x);

        // Update nodes lowest to highest.
        update_node(y);
        update_node(x);
        return x;
    }

    // Inserts the given rb_node_type into the tree.
    void insert_node(rb_node_type *x) {
        tree_insert(x);
        x->set_color(k_red);
        update_node(x);


        // The rb_node_type from which to start propagating updates upwards.
        rb_node_type *update_start = x->parent();

        while (x != root_ && x->parent()->get_color() == k_red) {
            if (x->parent() == x->parent()->parent()->left()) {
                rb_node_type *y = x->parent()->parent()->right();
                if (y && y->get_color() == k_red) {
                    // Case 1
                    x->parent()->set_color(k_black);
                    y->set_color(k_black);
                    x->parent()->parent()->set_color(k_red);
                    update_node(x->parent());
                    x = x->parent()->parent();
                    update_node(x);
                    update_start = x->parent();
                } else {
                    if (x == x->parent()->right()) {
                        // Case 2
                        x = x->parent();
                        left_rotate(x);
                    }
                    // Case 3
                    x->parent()->set_color(k_black);
                    x->parent()->parent()->set_color(k_red);
                    rb_node_type *new_sub_tree_root = right_rotate(x->parent()->parent());
                    update_start = new_sub_tree_root->parent();
                }
            } else {
                // Same as "then" clause with "right" and "left" exchanged.
                rb_node_type *y = x->parent()->parent()->left();
                if (y && y->get_color() == k_red) {
                    // Case 1
                    x->parent()->set_color(k_black);
                    y->set_color(k_black);
                    x->parent()->parent()->set_color(k_red);
                    update_node(x->parent());
                    x = x->parent()->parent();
                    update_node(x);
                    update_start = x->parent();
                } else {
                    if (x == x->parent()->left()) {
                        // Case 2
                        x = x->parent();
                        right_rotate(x);
                    }
                    // Case 3
                    x->parent()->set_color(k_black);
                    x->parent()->parent()->set_color(k_red);
                    rb_node_type *new_sub_tree_root = left_rotate(x->parent()->parent());
                    update_start = new_sub_tree_root->parent();
                }
            }
        }

        propagate_updates(update_start);

        root_->set_color(k_black);
    }

    // Restores the red-black property to the tree after splicing out
    // a rb_node_type. Note that x may be null, which is why xParent must be
    // supplied.
    void delete_fixup(rb_node_type *x, rb_node_type *x_parent) {
        while (x != root_ && (!x || x->get_color() == k_black)) {
            if (x == x_parent->left()) {
                // Note: the text points out that w can not be null.
                // The reason is not obvious from simply looking at
                // the code; it comes about from the properties of the
                // red-black tree.
                rb_node_type *w = x_parent->right();
                if (w->get_color() == k_red) {
                    // Case 1
                    w->set_color(k_black);
                    x_parent->set_color(k_red);
                    left_rotate(x_parent);
                    w = x_parent->right();
                }
                if ((!w->left() || w->left()->get_color() == k_black) &&
                    (!w->right() || w->right()->get_color() == k_black)) {
                    // Case 2
                    w->set_color(k_red);
                    x = x_parent;
                    x_parent = x->parent();
                } else {
                    if (!w->right() || w->right()->get_color() == k_black) {
                        // Case 3
                        w->left()->set_color(k_black);
                        w->set_color(k_red);
                        right_rotate(w);
                        w = x_parent->right();
                    }
                    // Case 4
                    w->set_color(x_parent->get_color());
                    x_parent->set_color(k_black);
                    if (w->right())
                        w->right()->set_color(k_black);
                    left_rotate(x_parent);
                    x = root_;
                    x_parent = x->parent();
                }
            } else {
                // Same as "then" clause with "right" and "left"
                // exchanged.

                // Note: the text points out that w can not be null.
                // The reason is not obvious from simply looking at
                // the code; it comes about from the properties of the
                // red-black tree.
                rb_node_type *w = x_parent->left();
                if (w->get_color() == k_red) {
                    // Case 1
                    w->set_color(k_black);
                    x_parent->set_color(k_red);
                    right_rotate(x_parent);
                    w = x_parent->left();
                }
                if ((!w->right() || w->right()->get_color() == k_black) &&
                    (!w->left() || w->left()->get_color() == k_black)) {
                    // Case 2
                    w->set_color(k_red);
                    x = x_parent;
                    x_parent = x->parent();
                } else {
                    if (!w->left() || w->left()->get_color() == k_black) {
                        // Case 3
                        w->right()->set_color(k_black);
                        w->set_color(k_red);
                        left_rotate(w);
                        w = x_parent->left();
                    }
                    // Case 4
                    w->set_color(x_parent->get_color());
                    x_parent->set_color(k_black);
                    if (w->left())
                        w->left()->set_color(k_black);
                    right_rotate(x_parent);
                    x = root_;
                    x_parent = x->parent();
                }
            }
        }
        if (x)
            x->set_color(k_black);
    }

    // Deletes the given rb_node_type from the tree. Note that this
    // particular rb_node_type may not actually be removed from the tree;
    // instead, another rb_node_type might be removed and its contents
    // copied into z.
    void delete_node(rb_node_type *z) {
        // Y is the rb_node_type to be unlinked from the tree.
        rb_node_type *y;
        if (!z->left() || !z->right())
            y = z;
        else
            y = tree_successor(z);

        // Y is guaranteed to be non-null at this point.
        rb_node_type *x;
        if (y->left())
            x = y->left();
        else
            x = y->right();

        // X is the child of y which might potentially replace y in
        // the tree. X might be null at this point.
        rb_node_type *x_parent;
        if (x) {
            x->set_parent(y->parent());
            x_parent = x->parent();
        } else {
            x_parent = y->parent();
        }
        if (!y->parent()) {
            root_ = x;
        } else {
            if (y == y->parent()->left())
                y->parent()->set_left(x);
            else
                y->parent()->set_right(x);
        }
        if (y != z) {
            z->copy_from(y);
            // This rb_node_type has changed location in the tree and must be updated.
            update_node(z);
            // The parent and its parents may now be out of date.
            propagate_updates(z->parent());
        }

        // If we haven't already updated starting from xParent, do so now.
        if (x_parent && x_parent != y && x_parent != z)
            propagate_updates(x_parent);
        if (y->get_color() == k_black)
            delete_fixup(x, x_parent);

        std::unique_ptr<rb_node_type> to_delete(y);
        --size_;
    }

    // Root node of the tree
    union {
        list_node_type *head_;
        rb_node_type *root_; // NOLINT
    };
    uint64_t size_;
    data_struct_type ds_type_;
};


template<typename T, typename compare = std::less<T>>
class tree_list : public tree_list_base<T, compare> {
public:
    // Constructor
    tree_list() {
        this->size_ = 0;
        this->ds_type_ = tree_list_base<T, compare>::data_struct_type::k_linked_list;
        this->head_ = nullptr;
    }

    ~tree_list() {
        this->clear();
    }

    tree_list(const tree_list &rhs) {
        for (auto &item: rhs) {
            insert(item);
        }
    }

    tree_list &operator=(const tree_list &rhs) {
        this->clear();
        for (auto &item: rhs) {
            insert(item);
        }
        return *this;
    }

    tree_list(tree_list &&rhs) {
        this->size_ = rhs.size_;
        this->ds_type_ = rhs.ds_type_;
        this->root_ = rhs.root_;
        rhs.size_ = 0;
        rhs.ds_type_ = tree_list_base<T, compare>::data_struct_type::k_linked_list;
        rhs.root_ = nullptr;
    }

    tree_list &operator=(tree_list &&rhs) {
        this->clear();
        this->size_ = rhs.size_;
        this->ds_type_ = rhs.ds_type_;
        this->root_ = rhs.root_;
        rhs.size_ = 0;
        rhs.ds_type_ = tree_list_base<T, compare>::data_struct_type::k_linked_list;
        rhs.root_ = nullptr;
        return *this;
    }
};

template<typename T, typename compare = std::less<T>>
class tree_list_trivial : public tree_list_base<T, compare> {
public:
    // Constructor
    tree_list_trivial() = default;

    ~tree_list_trivial() = default;

    tree_list_trivial(const tree_list_trivial &rhs) = default;

    tree_list_trivial &operator=(const tree_list_trivial &rhs) = default;

    tree_list_trivial(tree_list_trivial &&rhs) = default;

    tree_list_trivial &operator=(tree_list_trivial &&rhs) = default;
};



static_assert(std::is_trivially_copyable<tree_list_base<int>>::value, "tree_list_base<int>> is not trivial copyable");
static_assert(!std::is_trivially_copyable<tree_list<int>>::value, "tree_list<int> is trivial copyable");
static_assert(std::is_trivially_copyable<tree_list_trivial<int>>::value, "tree_list_trivial<int>> is not trivial copyable");

};

namespace std {
    template<typename T>
    void swap(smooth::tree_list_trivial <T> &lhs, smooth::tree_list_trivial <T> &rhs) {
        lhs.swap(rhs);
    }
}