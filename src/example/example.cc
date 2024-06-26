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
