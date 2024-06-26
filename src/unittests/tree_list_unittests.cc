
#include "gtest/gtest.h"

#include "smooth/tree_list.h"
#include <iostream>
#include <algorithm>

template <typename T>
class TreeListTest : public ::testing::Test {};

typedef ::testing::Types<int>  TestTypes;
TYPED_TEST_SUITE(TreeListTest, TestTypes);

TYPED_TEST(TreeListTest, EmptyList) {
    tree_list<int> list;
    ASSERT_TRUE(list.empty());
    ASSERT_EQ(0u, list.size());
}

TYPED_TEST(TreeListTest, InsertAndSize) {
    tree_list<int> list;
    list.insert(1);
    list.insert(2);
    list.insert(3);
    ASSERT_EQ(3u, list.size());
    ASSERT_FALSE(list.empty());
}

TYPED_TEST(TreeListTest, InsertMultipleAndSearch) {
    tree_list<int> list;
    list.insert(1);
    list.insert(2);
    list.insert(3);
    list.insert(4);
    list.insert(5);
    ASSERT_NE(list.find(3), list.end());
    ASSERT_EQ(list.find(6), list.end());
}

TYPED_TEST(TreeListTest, TreefyList) {
    tree_list<int> list;
    list.insert(1);
    list.insert(2);
    list.insert(3);
    list.insert(4);
    list.insert(5);
    list.insert(6);
    list.insert(7);
    list.insert(8);
    list.insert(9);
    list.insert(10);
    list.insert(11);

    ASSERT_NE(list.find(1), list.end());
    list.insert(12);
    ASSERT_NE(list.find(12), list.end());

    ASSERT_EQ(12u, list.size());
}

TYPED_TEST(TreeListTest, EraseElement) {
    tree_list<int> list;
    list.insert(1);
    list.insert(2);
    list.insert(3);
    list.erase(2);
    ASSERT_EQ(2u, list.size());
    ASSERT_EQ(list.find(2), list.end());
}


TYPED_TEST(TreeListTest, EraseElementFromTree) {
    tree_list<int> list;
    for (int i = 0; i < 12; ++i) {
        list.insert(i);
    }

    list.erase(5);
    ASSERT_EQ(11u, list.size());
    ASSERT_EQ(list.find(5), list.end());
}


TYPED_TEST(TreeListTest, IterateOverList) {
    tree_list<int> list;
    list.insert(1);
    list.insert(2);
    list.insert(3);

    std::vector<int> expected{3,2, 1};
    auto it = list.begin();
    for (size_t i = 0; it != list.end(); ++it, ++i) {
        ASSERT_EQ(*it, expected[i]);
    }
}

TYPED_TEST(TreeListTest, IterateOverTree) {
    tree_list<int> list;
    for (int i = 0; i < 12; ++i) {
        list.insert(i);
    }

    std::vector<int> expected;
    for (int i = 0; i < 12; ++i) {
        expected.push_back(i);
    }
    auto it = list.begin();
    for (size_t i = 0; it != list.end(); ++it, ++i) {
        ASSERT_EQ(*it, expected[i]);
    }
}

TYPED_TEST(TreeListTest, EraseIterator) {
    tree_list<int> list;
    list.insert(1);
    list.insert(2);
    list.insert(3);

    auto it = list.begin();
    ++it; // Point to the second element
    it = list.erase(it);
    ASSERT_EQ(2u, list.size());

    std::vector<int> expected{3, 1};
    it = list.begin();
    for (size_t i = 0; it != list.end(); ++it, ++i) {
        ASSERT_EQ(*it, expected[i]);
    }
}


TYPED_TEST(TreeListTest, EraseIteratorFromTree) {
    tree_list<int> list;
    for (int i = 0; i < 12; ++i) {
        list.insert(i);
    }

    auto it = list.begin();
    for (int i = 0; i < 5; ++i) {
        ++it;
    }

    it = list.erase(it);
    ASSERT_EQ(11u, list.size());

    std::set<int> expected{0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11};
    it = list.begin();
    for (size_t i = 0; it != list.end(); ++it, ++i) {
        ASSERT_EQ(*it, *expected.find(*it));
    }
}


TYPED_TEST(TreeListTest, EraseAtEnd) {
    tree_list<int> list;
    list.insert(1);
    list.insert(2);
    list.insert(3);

    auto it = list.end(); // Point to the end
    it = list.erase(it); // Erase at the end, should be a no-op
    ASSERT_EQ(3u, list.size());
    std::set<int> expected{1, 2, 3};
    it = list.begin();
    for (size_t i = 0; it != list.end(); ++it, ++i) {
        ASSERT_EQ(*it, *expected.find(*it));
    }
}


TYPED_TEST(TreeListTest, EraseAtEndTree) {
    tree_list<int> list;
    for (int i = 0; i < 12; ++i) {
        list.insert(i);
    }

    auto it = list.end(); // Point to the end
    it = list.erase(it); // Erase at the end, should be a no-op
    ASSERT_EQ(12u, list.size());
    std::vector<int> expected;
    for (int i = 0; i < 12; ++i) {
        expected.push_back(i);
    }
    it = list.begin();
    for (size_t i = 0; it != list.end(); ++it, ++i) {
        ASSERT_EQ(*it, expected[i]);
    }
}


TYPED_TEST(TreeListTest, Clear) {
    tree_list<int> list;
    list.insert(1);
    list.insert(2);
    list.insert(3);

    list.clear();
    ASSERT_EQ(0u, list.size());
    ASSERT_TRUE(list.empty());
}

TYPED_TEST(TreeListTest, ClearTree) {
    tree_list<int> list;
    for (int i = 0; i < 12; ++i) {
        list.insert(i);
    }

    list.clear();
    ASSERT_EQ(0u, list.size());
    ASSERT_TRUE(list.empty());
}


// Const Iterator Tests
TYPED_TEST(TreeListTest, IterateOverConstList) {
    tree_list<int> list;
    list.insert(1);
    list.insert(2);
    list.insert(3);
    std::vector<int> expected{1, 2, 3};
    const auto& const_list = list;
    auto it = const_list.cbegin();
    for (size_t i = 0; it != const_list.cend(); ++it, ++i) {
      ASSERT_NE(std::find(expected.begin(), expected.end(), 1), expected.end());
    }
}

TYPED_TEST(TreeListTest, IterateOverConstTree) {
    tree_list<int> list;
    for (int i = 0; i < 15; ++i) {
        list.insert(i);
    }
    std::vector<int> expected;
    for (int i = 0; i < 15; ++i) {
        expected.push_back(i);
    }
    const auto& const_list = list;
    auto it = const_list.cbegin();
    for (size_t i = 0; it != const_list.cend(); ++it, ++i) {
      ASSERT_NE(std::find(expected.begin(), expected.end(), *it), expected.end());
    }
}

// Operator-> Tests
TYPED_TEST(TreeListTest, OperatorArrowTest) {
    tree_list<int> list;
    list.insert(1);
    auto it = list.begin();
    auto ptr1 = it.operator->();
    ASSERT_EQ(*ptr1, 1); // Use *it and dereference for data
}

// Swap Tests
TYPED_TEST(TreeListTest, SwapTest) {
    tree_list<int> list1;
    tree_list<int> list2;
    list1.insert(1);
    list1.insert(2);
    list1.insert(3);
    list2.insert(4);
    list2.insert(5);
    std::swap(list1, list2); // Swap the lists
    ASSERT_EQ(list1.size(), 2u);
    ASSERT_EQ(list2.size(), 3u);
    auto it1 = list1.begin();
    ASSERT_TRUE((*it1 == 4 || *it1 == 5));

    auto it2 = list2.begin();
    ASSERT_TRUE((*it2 == 1 || *it2 == 2 || *it2 == 3));

}

// STL Algorithm Tests
TYPED_TEST(TreeListTest, TestFind) {
    tree_list<int> list;
    list.insert(1);
    list.insert(2);
    list.insert(3);
    auto it = std::find(list.begin(), list.end(), 2);
    ASSERT_NE(it, list.end());
    ASSERT_EQ(*it, 2);
}

TYPED_TEST(TreeListTest, TestCount) {
    tree_list<int> list;
    list.insert(1);
    list.insert(2);
    list.insert(1);
    ASSERT_EQ(std::count(list.begin(), list.end(), 1), 2);
}

TYPED_TEST(TreeListTest, TestCopy) {
    tree_list<int> list1;
    list1.insert(1);
    list1.insert(2);
    list1.insert(3);
    tree_list<int> list2;

    std::copy(list1.begin(), list1.end(), std::inserter(list2, list2.begin()));
    ASSERT_EQ(list1.size(), list2.size());
}


TYPED_TEST(TreeListTest, TestEraseIteratorFromTree) {
    tree_list<int> list;
    for (int i = 0; i < 15; ++i) {
        list.insert(i);
    }

    auto it = list.begin();
    for (int i = 0; i < 5; ++i) {
        ++it;
    }

    it = list.erase(it);
    ASSERT_EQ(14u, list.size());

    std::vector<int> expected;
    for (int i = 0; i < 15; ++i) {
        if (i != 5) {
            expected.push_back(i);
        }
    }
    it = list.begin();
    for (size_t i = 0; it != list.end(); ++it, ++i) {
        ASSERT_NE(std::find(expected.begin(), expected.end(), *it), expected.end());
    }
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

