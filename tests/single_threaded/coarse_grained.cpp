#include <random>
#include "coarse_grained_bst.h"

void main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX);

    CoarseGrainedBST bst;
    for (auto i = 0; i < 10; i++)
        bst.Insert(dis(gen));
}