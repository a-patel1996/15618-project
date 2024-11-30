#include <random>
#include <vector>
#include <cassert>
#include <chrono>
#include <thread>
#include "coarse_grained_bst.h"

#define NUM_KEYS 100000
#define MAX_KEY_VALUE 10000
#define NUM_THREADS 10

void InsertSearchDeleteSearch(CoarseGrainedBST &bst, uint8_t thread_idx)
{
    const auto increment = 1;

    for (auto key_ref = (thread_idx * NUM_KEYS); key_ref < ((thread_idx + 1) * NUM_KEYS); key_ref += increment)
    {
        for (auto key = key_ref; key < key_ref + increment; key++)
        {
            bst.Insert(key);
        }

        for (auto key = key_ref; key < key_ref + increment; key++)
        {
            assert(bst.Search(key) == true);
        }

        for (auto key = key_ref; key < key_ref + increment; key++)
        {
            bst.Delete(key);
        }

        for (auto key = key_ref; key < key_ref + increment; key++)
        {
            assert(bst.Search(key) == false);
        }
    }
}

int main()
{
    {
        std::cout << "CORRECTNESS TEST" << std::endl;
        std::vector<std::thread> threads;
        auto start = std::chrono::high_resolution_clock::now();
        CoarseGrainedBST bst;

        for (uint8_t thread_idx = 0; thread_idx < NUM_THREADS; thread_idx++)
        {
            threads.emplace_back(std::thread(InsertSearchDeleteSearch, std::ref(bst), thread_idx));
        }

        for (uint8_t thread_idx = 0; thread_idx < NUM_THREADS; thread_idx++)
        {
            threads.at(thread_idx).join();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Execution time: " << duration.count() << std::endl;
        std::cout << "PASSED" << std::endl;
    }

    // std::random_device rd;
    // std::mt19937 gen(rd());
    // std::uniform_int_distribution<uint32_t> dis(0, MAX_KEY_VALUE - 1);

    // std::vector<uint32_t> keys;
    // for (auto i = 0; i < NUM_KEYS; i++)
    // {
    //     keys.emplace_back(dis(gen));
    // }

    // bool res;
    // std::vector<bool> inserted_keys(MAX_KEY_VALUE, false);
    // auto start = std::chrono::high_resolution_clock::now();
    // CoarseGrainedBST bst;
    // for (auto i = 0; i < NUM_KEYS; i++)
    // {
    //     auto key = keys.at(i);
    //     // std::cout << "key " << key;
    //     if (i % 2 == 0)
    //     {
    //         res = bst.Insert(key);
    //         // std::cout << " insert" << (res ? " success" : " fail") << std::endl;
    //         assert(inserted_keys.at(key) != res);
    //         inserted_keys.at(key) = true;
    //     }
    //     else
    //     {
    //         res = bst.Delete(key);
    //         // std::cout << " delete" << (res ? " success" : " fail") << std::endl;
    //         assert(inserted_keys.at(key) == res);
    //         inserted_keys.at(key) = false;
    //     }
    // }

    // for (auto i = 0; i < NUM_KEYS; i++)
    // {
    //     auto key = keys.at(i);
    //     assert(inserted_keys.at(key) == bst.Search(key));
    // }

    // auto end = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    // std::cout << "Execution time: " << duration.count() << std::endl;
    return 0;
}