## A Redis Dict-like C++ Hash Table Implementation with Incremental Rehashing

This repository provides a C++ implementation of a hash table data structure, inspired by Redis Dict. It offers STL compatibility, requires minimal dependencies (C++11 and above), and delivers performance comparable to Redis Dict. Similar to JDK HashMap, it employs red-black trees to handle buckets with excessive key-value pairs. Additionally, it features an **incremental rehashing** mechanism to efficiently handle millions of data points without causing delays.

### Features

* **STL Compatible:** Integrates seamlessly with C++'s Standard Template Library.
* **Header-Only:** No need to compile or link.
* **Lightweight:** Minimal dependencies, requiring only C++11 or higher.
* **High Performance:** Achieves performance close to Redis Dict.
* **Efficient Memory Management:** Utilizes red-black trees for buckets with high collision rates.
* **Incremental Rehashing:** Minimizes performance impact of rehashing operations on large datasets, almost no latency.

### Example Usage
```c++
#include <iostream>
#include "smooth/hashmap.h"

int main()
{
    smooth::hashmap<int, std::string> map;
    map[1] = "one";
    map[2] = "two";
    for (const auto& pair : map)
    {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    return 0;
}
```