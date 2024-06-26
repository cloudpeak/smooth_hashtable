#include "gtest/gtest.h"
#include "smooth/fixed_hashmap.h"
#include <iostream>

TEST(FixedHashMapTest, Emplace) {
  fixed_hashmap<int, std::string> map;

  auto pair = map.emplace(1, "one");
  auto inserted1 = pair.second;
  auto it1 = pair.first;
  ASSERT_TRUE(inserted1);
  ASSERT_EQ(it1->first, 1);
  ASSERT_EQ(it1->second, "one");

  auto pair2 = map.emplace(2, "two");
  auto inserted2 = pair2.second;
  auto it2 = pair2.first;
  ASSERT_TRUE(inserted2);
  ASSERT_EQ(it2->first, 2);
  ASSERT_EQ(it2->second, "two");

  ASSERT_EQ(map.size(), 2);
}



TEST(FixedHashMapTest, Insert) {
  fixed_hashmap<int, std::string> map;

  auto pair = map.insert(std::make_pair(1, "one"));
  auto inserted1 = pair.second;
  auto it1 = pair.first;
  ASSERT_TRUE(inserted1);
  ASSERT_EQ(it1->first, 1);
  ASSERT_EQ(it1->second, "one");

  auto pair2 = map.insert(std::make_pair(2, "two"));
  auto inserted2 = pair2.second;
  auto it2 = pair2.first;
  ASSERT_TRUE(inserted2);
  ASSERT_EQ(it2->first, 2);
  ASSERT_EQ(it2->second, "two");

  ASSERT_EQ(map.size(), 2);
}

TEST(FixedHashMapTest, Contains) {
  fixed_hashmap<int, std::string> map;

  map.emplace(1, "one");
  map.emplace(2, "two");

  ASSERT_TRUE(map.contains(1));
  ASSERT_TRUE(map.contains(2));
  ASSERT_FALSE(map.contains(3));
}

TEST(FixedHashMapTest, Erase) {
  fixed_hashmap<int, std::string> map;

  map.emplace(1, "one");
  map.emplace(2, "two");
  map.emplace(3, "three");

  ASSERT_EQ(map.erase(2), 1);
  ASSERT_EQ(map.size(), 2);
  ASSERT_FALSE(map.contains(2));

  ASSERT_EQ(map.erase(4), 0);
  ASSERT_EQ(map.size(), 2);
}

TEST(FixedHashMapTest, EraseByIterator) {
  fixed_hashmap<int, std::string> map;

  map.emplace(1, "one");
  map.emplace(2, "two");
  map.emplace(3, "three");

  auto it = map.find(2);
  ASSERT_NE(it, map.end());

  it = map.erase(it);
  ASSERT_EQ(map.size(), 2);
  ASSERT_EQ(it->first, 3);
}

TEST(FixedHashMapTest, Find) {
  fixed_hashmap<int, std::string> map;

  map.emplace(1, "one");
  map.emplace(2, "two");

  auto it = map.find(2);
  ASSERT_NE(it, map.end());
  ASSERT_EQ(it->first, 2);
  ASSERT_EQ(it->second, "two");

  it = map.find(3);
  ASSERT_EQ(it, map.end());
}

TEST(FixedHashMapTest, At) {
  fixed_hashmap<int, std::string> map;

  map.emplace(1, "one");

  ASSERT_EQ(map.at(1), "one");

  ASSERT_EQ(map.at(2), "");
}

TEST(FixedHashMapTest, ConstAt) {
    fixed_hashmap<int, std::string> map;
    map.emplace(1, "one");

    const fixed_hashmap<int, std::string> & const_map = map;

    ASSERT_EQ(const_map.at(1), "one");

    ASSERT_THROW(const_map.at(3), std::out_of_range);
}


TEST(FixedHashMapTest, OperatorSquareBrackets) {
  fixed_hashmap<int, std::string> map;

  map[1] = "one";

  ASSERT_EQ(map[1], "one");

  ASSERT_EQ(map.size(), 1);
}

TEST(FixedHashMapTest, Size) {
  fixed_hashmap<int, std::string> map;

  ASSERT_EQ(map.size(), 0);

  map.emplace(1, "one");
  map.emplace(2, "two");

  ASSERT_EQ(map.size(), 2);
}

TEST(FixedHashMapTest, Empty) {
  fixed_hashmap<int, std::string> map;

  ASSERT_TRUE(map.empty());

  map.emplace(1, "one");

  ASSERT_FALSE(map.empty());
}

TEST(FixedHashMapTest, Clear) {
  fixed_hashmap<int, std::string> map;

  map.emplace(1, "one");
  map.emplace(2, "two");

  ASSERT_EQ(map.size(), 2);

  map.clear();

  ASSERT_EQ(map.size(), 0);
  ASSERT_TRUE(map.empty());
}

TEST(FixedHashMapTest, Iterator) {
  fixed_hashmap<int, std::string> map;

  map.emplace(1, "one");
  map.emplace(2, "two");
  map.emplace(3, "three");

  // store the values in a vector
  std::vector<std::pair<int, std::string>> values;
  for (auto it = map.begin(); it != map.end(); ++it) {
      values.push_back(*it);
  }

  // write an anonymous function to retrieve value by id
  auto get_value = [&](int id) {
    for (const auto& value : values) {
      if (value.first == id) {
        return value.second;
      }
    }
    return std::string{};
  };

  // Check forward iteration

  for (auto it = map.begin(); it != map.end(); ++it) {
      ASSERT_EQ(get_value(it->first), it->second);
  }
}

TEST(FixedHashMapTest, StealElements) {
    fixed_hashmap<int, std::string> map(5);
    map.emplace(1, "one");
    map.emplace(2, "two");
    map.emplace(3, "three");
    map.emplace(4, "four");
    map.emplace(5, "five");

    auto stolen = map.steal_elements(3);
    ASSERT_EQ(stolen.size(), 3);
    ASSERT_EQ(map.size(), 2);


    stolen = map.steal_elements(2);
    ASSERT_EQ(stolen.size(), 2);
    ASSERT_EQ(map.size(), 0);
}

TEST(FixedHashMapTest, GetBucketCount) {
  fixed_hashmap<int, std::string> map(10);

  ASSERT_EQ(map.get_bucket_count(), 10);
}

TEST(FixedHashMapTest, MoveConstructor) {
  fixed_hashmap<int, std::string> map1(5);
  map1.emplace(1, "one");
  map1.emplace(2, "two");

  fixed_hashmap<int, std::string> map2(std::move(map1));

  ASSERT_EQ(map2.size(), 2);
  ASSERT_EQ(map1.size(), 0);
}

TEST(FixedHashMapTest, MoveAssignmentOperator) {
  fixed_hashmap<int, std::string> map1(5);
  map1.emplace(1, "one");
  map1.emplace(2, "two");

  fixed_hashmap<int, std::string> map2;
  map2 = std::move(map1);

  ASSERT_EQ(map2.size(), 2);
  ASSERT_EQ(map1.size(), 0);
}

TEST(FixedHashMapTest, Swap) {
  fixed_hashmap<int, std::string> map1(5);
  map1.emplace(1, "one");
  map1.emplace(2, "two");

  fixed_hashmap<int, std::string> map2(5);
  map2.emplace(3, "three");
  map2.emplace(4, "four");

  map1.swap(map2);

  ASSERT_EQ(map2.size(), 2);
  ASSERT_EQ(map2.find(1)->second, "one");
  ASSERT_EQ(map2.find(2)->second, "two");

  ASSERT_EQ(map1.size(), 2);
  ASSERT_EQ(map1.find(3)->second, "three");
  ASSERT_EQ(map1.find(4)->second, "four");
}


TEST(FixedHashMapTest, TransparentHash) {

}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}