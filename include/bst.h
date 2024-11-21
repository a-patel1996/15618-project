#ifndef __BST_H__
#define __BST_H__

#include <string>
#include <stdint.h>

class BinarySearchTree {
protected:
    typedef struct Node
    {
        uint32_t key;
        Node *left;
        Node *right;

        Node(uint32_t key) : key(key), left(nullptr), right(nullptr) {};
        Node(uint32_t key, Node *left, Node *right) : key(key), left(left), right(right) {};
    } Node;

public:
    virtual ~BinarySearchTree() = default;

    virtual auto Search(uint32_t key) -> bool = 0;
    virtual auto Insert(uint32_t key) -> bool = 0;
    virtual auto Delete(uint32_t key) -> bool = 0;
    void PrintTree(Node* root, std::string prefix = "", bool isLeft = true);
};

#endif