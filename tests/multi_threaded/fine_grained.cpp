#include <random>
#include <vector>
#include <cassert>
#include <chrono>
#include <thread>
#include "fine_grained_bst.h"

#define NUM_KEYS 100000
#define MAX_KEY_VALUE 10000
#define NUM_THREADS 8

void InsertSearchDeleteSearch(FineGrainedBST &bst, uint8_t thread_idx)
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
        FineGrainedBST bst;

        for (uint8_t thread_idx = 0; thread_idx < NUM_THREADS; thread_idx++)
        {
            threads.emplace_back(std::thread(InsertSearchDeleteSearch, std::ref(bst), thread_idx));
        }

        std::cout << "Joining threads" << std::endl;

        for (uint8_t thread_idx = 0; thread_idx < NUM_THREADS; thread_idx++)
        {
            threads.at(thread_idx).join();
        }
        std::cout << "PASSED" << std::endl;
    }
    return 0;
}