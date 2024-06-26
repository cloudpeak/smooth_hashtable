#include "gtest/gtest.h"
#include "smooth/hashmap.h"
#include <iostream>

TEST(HashMapTest, Emplace) {
    hashmap<int, std::string> map;

    map.insert(std::make_pair(1, "one"));
    ASSERT_EQ(map.size(), 1);
    ASSERT_TRUE(map.contains(1));

    map.insert(std::make_pair(2, "two"));
    ASSERT_EQ(map.size(), 2);
    ASSERT_TRUE(map.contains(2));

    map.insert(std::make_pair(1, "one")); // Duplicate key, should not be inserted
    ASSERT_EQ(map.size(), 2);
    ASSERT_TRUE(map.contains(1));
}

TEST(HashMapTest, Erase) {
    hashmap<int, std::string> map;

    map.insert(std::make_pair(1, "one"));
    map.insert(std::make_pair(2, "two"));
    map.insert(std::make_pair(3, "three"));

    ASSERT_EQ(map.erase(2), 1);
    ASSERT_EQ(map.size(), 2);
    ASSERT_FALSE(map.contains(2));

    ASSERT_EQ(map.erase(4), 0);
    ASSERT_EQ(map.size(), 2);
}

TEST(HashMapTest, Find) {
    hashmap<int, std::string> map;

    map.insert(std::make_pair(1, "one"));
    map.insert(std::make_pair(2, "two"));

    auto it = map.find(2);
    ASSERT_NE(it, map.end());
    ASSERT_EQ(it->first, 2);
    ASSERT_EQ(it->second, "two");

    it = map.find(3);
    ASSERT_EQ(it, map.end());
}

TEST(HashMapTest, Contains) {
    hashmap<int, std::string> map;

    map.insert(std::make_pair(1, "one"));
    map.insert(std::make_pair(2, "two"));

    ASSERT_TRUE(map.contains(1));
    ASSERT_TRUE(map.contains(2));
    ASSERT_FALSE(map.contains(3));
}

TEST(HashMapTest, Size) {
    hashmap<int, std::string> map;

    ASSERT_EQ(map.size(), 0);

    map.insert(std::make_pair(1, "one"));
    map.insert(std::make_pair(2, "two"));

    ASSERT_EQ(map.size(), 2);
}

TEST(HashMapTest, Clear) {
    hashmap<int, std::string> map;

    map.insert(std::make_pair(1, "one"));
    map.insert(std::make_pair(2, "two"));

    ASSERT_EQ(map.size(), 2);

    map.clear();

    ASSERT_EQ(map.size(), 0);
}

TEST(HashMapTest, Iterator) {
    // Initialize data with initializer list
    std::initializer_list<std::pair<int, std::string>> data = {
            {0, "one"},
            {1, "one"},  // Duplicate key will be ignored by std::map
            {2, "two"},
            {3, "three"},
            {4, "four"},
            {5, "five"}
    };

    // Create a std::map (ordered, unique keys)
    std::map<int, std::string> myMap;  // Use map constructor
    for(auto& pair : data) {
        myMap.emplace(pair);
    }

    hashmap<int, std::string> map;
    map.insert(std::make_pair(0, "one"));
    map.insert(std::make_pair(1, "one"));
    map.insert(std::make_pair(2, "two"));
    map.insert(std::make_pair(3, "three"));
    map.insert(std::make_pair(4, "four"));
    map.insert(std::make_pair(5, "five"));

    // Efficiently insert elements from the vector into the map
    for(auto& pair : data) {
        map.insert(pair);  // Use emplace for potential efficiency gain
    }

    for (auto it = map.begin(); it != map.end(); ++it) {
       ASSERT_EQ(it->second, myMap[it->first]);
    }

    std::cout << "finished" << std::endl;
}

TEST(HashMapTest, ConstIterator) {

    hashmap<int, std::string> map;
    map.insert(std::make_pair(1, "one"));
    map.insert(std::make_pair(2, "two"));
    map.insert(std::make_pair(3, "three"));

    for (auto it = map.cbegin(); it != map.cend(); ++it) {
        ASSERT_TRUE((it->second, "one") || (it->second, "two") || (it->second, "three"));
    }
}


TEST(HashMapTest, Rehashing) {
    hashmap<int, std::string> map(2); // Initial size 2

    // Insert elements to trigger rehashing
    for (int i = 0; i < 10; ++i) {
        map.insert(std::make_pair(i, "value" + std::to_string(i)));
    }

    // Verify all elements are still present after rehashing
    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(map.contains(i));
    }
}

TEST(HashMapTest, MoveProgressively) {
    hashmap<int, std::string> map(2); // Initial size 2
    map.insert(std::make_pair(1, "one"));
    map.insert(std::make_pair(2, "two"));

    // Trigger rehashing
    map.insert(std::make_pair(3, "three"));
    map.insert(std::make_pair(4, "four"));

    // Verify all elements are still present after rehashing
    ASSERT_TRUE(map.contains(1));
    ASSERT_TRUE(map.contains(2));
    ASSERT_TRUE(map.contains(3));
    ASSERT_TRUE(map.contains(4));
    ASSERT_EQ(map.size(), 4);

    map.erase(2);
    ASSERT_EQ(map.size(), 3);
    ASSERT_FALSE(map.contains(2));

    map.insert(std::make_pair(2, "two")); // Insert again
    ASSERT_EQ(map.size(), 4);
    ASSERT_TRUE(map.contains(2));
}

TEST(HashMapTest, InitializerList) {
    std::initializer_list<std::pair<int64_t, std::string>> data = {
            {0, "one"},
            {1, "one"},  // Duplicate key will be ignored by std::map
            {2, "two"},
            {3, "three"},
            {4, "four"},
            {5, "five"}
    };

    hashmap<int64_t, std::string> map(data);
    for (const auto& pair : data) {
        ASSERT_EQ(map[pair.first],  pair.second);
    }
}

TEST(HashMapTest, MassiveInsert) {
    const int kMaxSize = 100000;
    hashmap<int, std::string> map;
    for (int i = 0; i < kMaxSize; ++i) {
        map.insert(std::make_pair(i, "value" + std::to_string(i)));
    }
    for (int i = 0; i < kMaxSize; ++i) {
        ASSERT_EQ(map[i], "value" + std::to_string(i));
    }
}

TEST(HashMapTest, MassiveErase) {
    const int kMaxSize = 100000;
    hashmap<int, std::string> map;
    for (int i = 0; i < kMaxSize; ++i) {
        map.insert(std::make_pair(i, "value" + std::to_string(i)));
    }
    for (int i = 0; i < kMaxSize; i += 2) {
        map.erase(i);
    }
    EXPECT_EQ(map.size(), kMaxSize / 2);

    for (int i = 0; i < kMaxSize; ++i) {
        map.erase(i);
    }
    EXPECT_EQ(map.size(), 0);
}


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}