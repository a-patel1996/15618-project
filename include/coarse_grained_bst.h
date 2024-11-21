#ifndef __COARSE_GRAINED_BST_H__
#define __COARSE_GRAINED_BST_H__

#include <mutex>
#include <stdint.h>
#include "bst.h"

class CoarseGrainedBST : public BinarySearchTree
{
public:
    CoarseGrainedBST();
    ~CoarseGrainedBST();

    auto Search(uint32_t key) -> bool;
    auto Insert(uint32_t key) -> bool;
    auto Delete(uint32_t key) -> bool;

private:
    Node *root;
    std::mutex op_lock;

    void CoarseGrainedBST::FreeNode(Node *node);
};

#endif