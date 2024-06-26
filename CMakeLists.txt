cmake_minimum_required(VERSION 3.01)
project(hashmap)

set(CMAKE_CXX_STANDARD 11)

include_directories(./include)


add_definitions(-DNOMINMAX)

## fixed_hashmap_unittests
add_executable(fixed_hashmap_unittests
        src/unittests/fixed_hashmap_unittests.cc)

target_include_directories(fixed_hashmap_unittests PRIVATE
        .
)

target_link_libraries(fixed_hashmap_unittests
        gtest
        pthread
)

## fixed_hashmap_unittests
add_executable(hashmap_unittests
        src/unittests/hashmap_unittests.cc)

target_include_directories(hashmap_unittests PRIVATE
        .
)

target_link_libraries(hashmap_unittests
        gtest
        pthread
)

## fixed_hashmap_unittests
add_executable(tree_list_unittests
        src/unittests/tree_list_unittests.cc)

target_include_directories(tree_list_unittests PRIVATE
        .
)

target_link_libraries(tree_list_unittests
        gtest
        pthread
)



