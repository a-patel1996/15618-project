#ifndef __FINE_GRAINED_BST_H__
#define __FINE_GRAINED_BST_H__

#include <mutex>
#include <stdint.h>
#include "bst.h"

class FineGrainedBST : public BinarySearchTree
{
public:
    FineGrainedBST();
    ~FineGrainedBST();

    auto Search(uint32_t key) -> bool;
    auto Insert(uint32_t key) -> bool;
    auto Delete(uint32_t key) -> bool;
    void PrintTree();

private:
    typedef struct NodeFG
    {
        uint32_t key;
        NodeFG *left;
        NodeFG *right;
        std::mutex perNodeLock;

        NodeFG(uint32_t key) : key(key), left(nullptr), right(nullptr) {};
        NodeFG(uint32_t key, NodeFG *left, NodeFG *right) : key(key), left(left), right(right) {};
    } NodeFG;

    NodeFG *root;
    std::mutex op_lock;

    void FreeNode(NodeFG *node);
};

#endif