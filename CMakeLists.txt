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

## fixed_hashmap_unittests
add_executable(example
        src/example/example.cc)

target_include_directories(example PRIVATE
        .
)

target_link_libraries(example
        gtest
        pthread
)

## redis_dict static library (C)
add_library(redis_dict STATIC
        redis_dict/dict.c
        redis_dict/mt19937-64.c
        redis_dict/siphash.c
        redis_dict/zmalloc.c
)
target_include_directories(redis_dict PUBLIC redis_dict)
set_target_properties(redis_dict PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
)
target_compile_options(redis_dict PRIVATE
        -Wno-unused-function
        -Wno-unused-parameter
        -Wno-unused-variable
)

## Google Benchmark via FetchContent
include(FetchContent)
FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

## dict_benchmark
add_executable(dict_benchmark
        benchmarks/dict_benchmark.cc)
target_include_directories(dict_benchmark PRIVATE
        .
        redis_dict
)
target_link_libraries(dict_benchmark
        redis_dict
        benchmark
        pthread
)

