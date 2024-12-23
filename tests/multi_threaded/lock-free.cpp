#include <random>
#include <vector>
#include <cassert>
#include <chrono>
#include <thread>
#include <set>
#include "FEAST.h"

#define NUM_OF_RUNS 5
#define NUM_OF_OPS 5000000
#define NUM_KEYS 256
#define NUM_THREADS 1

std::atomic<uint_fast64_t> op_count;
volatile bool done;

void InsertSearchDeleteSearch(LockFreeBST &bst, uint8_t thread_idx)
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

void ReadDominated(LockFreeBST &bst)
{
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, NUM_KEYS - 1);

    // while (true)
    while (!done)
    // for (auto i = 0; i < NUM_OF_OPS; i++)
    {
        // uint_fast64_t curr = op_count;
        // if (curr == 0)
        //     break;
        // if (op_count.compare_exchange_strong(curr, curr - 1) == false)
        //     continue;

        auto key = dis(gen);
        double prob = (double)rand() / RAND_MAX;
        if (prob < 0.05)
            bst.Insert(key);
        else if (prob < 0.1)
            bst.Delete(key);
        else
            bst.Search(key);
    }
}

void Balanced(LockFreeBST &bst)
{
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, NUM_KEYS - 1);

    // while (true)
    while (!done)
    // for (auto i = 0; i < NUM_OF_OPS; i++)
    {
        // uint_fast64_t curr = op_count;
        // if (curr == 0)
        //     break;
        // if (op_count.compare_exchange_strong(curr, curr - 1) == false)
        //     continue;

        auto key = dis(gen);
        int val = rand() % 3;
        if (val == 0)
            bst.Insert(key);
        else if (val == 1)
            bst.Delete(key);
        else
            bst.Search(key);
    }   
}

void WriteDominated(LockFreeBST &bst)
{
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, NUM_KEYS - 1);

    // while (true)
    while (!done)
    // for (auto i = 0; i < NUM_OF_OPS; i++)
    {
        // uint_fast64_t curr = op_count;
        // if (curr == 0)
        //     break;
        // if (op_count.compare_exchange_strong(curr, curr - 1) == false)
        //     continue;

        auto key = dis(gen);
        double prob = (double)rand() / RAND_MAX;
        if (prob < 0.5)
            bst.Insert(key);
        else
            bst.Delete(key);
    }
}

void prePopulateTree(LockFreeBST &bst) {
    std::set<uint32_t> insertedKeys;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, NUM_KEYS - 1);

    while (insertedKeys.size() < (NUM_KEYS / 2)) {
        auto key = dis(gen);
        while(insertedKeys.count(key) > 0) {
            key = dis(gen);
        }
        insertedKeys.insert(key);
        bst.Insert(key);
    }
}

int main()
{
    {
        std::cout << "CORRECTNESS TEST" << std::endl;
        std::vector<std::thread> threads;
        LockFreeBST bst;

        auto start = std::chrono::high_resolution_clock::now();

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
    }

    // Read-dominated: 90% search, 5% insert, 5% delete
    float read_val = 0;
    std::chrono::milliseconds read_dur(0);
    for (auto i = 0; i < NUM_OF_RUNS; i++)
    {
        std::cout << "READ DOMINATED TEST" << std::endl;
        // done = false;
        std::vector<std::thread> threads;
        LockFreeBST bst;
        prePopulateTree(bst);
        op_count = NUM_OF_OPS;

        auto start = std::chrono::high_resolution_clock::now();
        for (uint8_t thread_idx = 0; thread_idx < NUM_THREADS; thread_idx++)
        {
            threads.emplace_back(std::thread(ReadDominated, std::ref(bst)));
        }

        // sleep(10);
        // done = true;

        for (uint8_t thread_idx = 0; thread_idx < NUM_THREADS; thread_idx++)
        {
            threads.at(thread_idx).join();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Execution time: " << duration.count() << std::endl;
        std::cout << "System throughput (ops/sec): " << static_cast<float>(bst.GetOpCount())/10000000 << std::endl;
        read_dur += duration;
        read_val += static_cast<float>(bst.GetOpCount())/10000000;
    }

    // Balanced: Equal fractions of search, insert, delete
    float balance_val = 0;
    std::chrono::milliseconds bal_dur(0);
    for (auto i = 0; i < NUM_OF_RUNS; i++)
    {
        std::cout << "BALANCED TEST" << std::endl;
        // done = false;
        std::vector<std::thread> threads;
        LockFreeBST bst;
        prePopulateTree(bst);
        op_count = NUM_OF_OPS;

        auto start = std::chrono::high_resolution_clock::now();
        for (uint8_t thread_idx = 0; thread_idx < NUM_THREADS; thread_idx++)
        {
            threads.emplace_back(std::thread(Balanced, std::ref(bst)));
        }

        // sleep(10);
        // done = true;

        for (uint8_t thread_idx = 0; thread_idx < NUM_THREADS; thread_idx++)
        {
            threads.at(thread_idx).join();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Execution time: " << duration.count() << std::endl;
        std::cout << "System throughput (ops/sec): " << static_cast<float>(bst.GetOpCount())/10000000 << std::endl;
        bal_dur += duration;
        balance_val += static_cast<float>(bst.GetOpCount())/10000000;
    }

    // Write-dominated: 0% search, 50% insert, 50% delete
    float write_val = 0;
    std::chrono::milliseconds write_dur(0);
    for (auto i = 0; i < NUM_OF_RUNS; i++)
    {
        std::cout << "WRITE DOMINATED TEST" << std::endl;
        // done = false;
        std::vector<std::thread> threads;
        LockFreeBST bst;
        prePopulateTree(bst);
        op_count = NUM_OF_OPS;

        auto start = std::chrono::high_resolution_clock::now();
        for (uint8_t thread_idx = 0; thread_idx < NUM_THREADS; thread_idx++)
        {
            threads.emplace_back(std::thread(WriteDominated, std::ref(bst)));
        }

        // sleep(10);
        // done = true;

        for (uint8_t thread_idx = 0; thread_idx < NUM_THREADS; thread_idx++)
        {
            threads.at(thread_idx).join();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Execution time: " << duration.count() << std::endl;
        std::cout << "System throughput (ops/sec): " << static_cast<float>(bst.GetOpCount())/10000000 << std::endl;
        write_dur += duration;
        write_val += static_cast<float>(bst.GetOpCount())/10000000;
    }

    std::cout << "READ: Average Duration (ops/sec): " << read_dur.count()/NUM_OF_RUNS << std::endl;
    std::cout << "BALANCED: Average Duration (ops/sec): " << bal_dur.count()/NUM_OF_RUNS << std::endl;
    std::cout << "WRITE: Average System Duration (ops/sec): " << write_dur.count()/NUM_OF_RUNS << std::endl;
    std::cout << "READ: Average System throughput (ops/sec): " << read_val/NUM_OF_RUNS << std::endl;
    std::cout << "BALANCED: Average System throughput (ops/sec): " << balance_val/NUM_OF_RUNS << std::endl;
    std::cout << "WRITE: Average System throughput (ops/sec): " << write_val/NUM_OF_RUNS << std::endl;

    return 0;
}