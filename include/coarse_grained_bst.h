#ifndef __COARSE_GRAINED_BST_H__
#define __COARSE_GRAINED_BST_H__

#include <mutex>
#include <stdint.h>

class CoarseGrainedBST
{
public:
    CoarseGrainedBST();
    ~CoarseGrainedBST();

    auto Search(uint32_t key) -> bool;
    auto Insert(uint32_t key) -> bool;
    auto Delete(uint32_t key) -> bool;

private:
    typedef struct Node
    {
        uint32_t key;
        Node *left;
        Node *right;

        Node(uint32_t key) : key(key), left(nullptr), right(nullptr) {};
        Node(uint32_t key, Node *left, Node *right) : key(key), left(left), right(right) {};
    } Node;

    Node *root;
    std::mutex op_lock;

    void CoarseGrainedBST::FreeNode(Node *node);
};

#endif