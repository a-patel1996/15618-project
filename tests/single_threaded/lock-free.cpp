#include <random>
#include <vector>
#include <cassert>
#include <chrono>
#include "FEAST.h"

#define NUM_KEYS 1000000
#define MAX_KEY_VALUE 100000

int main()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, MAX_KEY_VALUE - 1);

    std::vector<uint32_t> keys;
    for (auto i = 0; i < NUM_KEYS; i++)
    {
        keys.emplace_back(dis(gen));
    }

    bool res;
    std::vector<bool> inserted_keys(MAX_KEY_VALUE, false);
    auto start = std::chrono::high_resolution_clock::now();
    LockFreeBST bst;
    for (auto i = 0; i < NUM_KEYS; i++)
    {
        auto key = keys.at(i);
        // std::cout << "key " << key;
        if (i % 2 == 0)
        {
            res = bst.Insert(key);
            // std::cout << " insert" << (res ? " success" : " fail") << std::endl;
            assert(inserted_keys.at(key) != res);
            inserted_keys.at(key) = true;
        }
        else
        {
            res = bst.Delete(key);
            // std::cout << " delete" << (res ? " success" : " fail") << std::endl;
            assert(inserted_keys.at(key) == res);
            inserted_keys.at(key) = false;
        }
    }

    for (auto i = 0; i < NUM_KEYS; i++)
    {
        auto key = keys.at(i);
        assert(inserted_keys.at(key) == bst.Search(key));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Execution time: " << duration.count() << std::endl;
    return 0;
}