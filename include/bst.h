#ifndef __BST_H__
#define __BST_H__

#include <iostream>
#include <string>
#include <stdint.h>

class BinarySearchTree
{
protected:
    typedef struct Node
    {
        uint32_t key;
        Node *left;
        Node *right;

        Node(uint32_t key) : key(key), left(nullptr), right(nullptr) {};
        Node(uint32_t key, Node *left, Node *right) : key(key), left(left), right(right) {};
    } Node;

    void PrintTree(Node *root, int space = 0, int indent = 7);

public:
    virtual ~BinarySearchTree() = default;

    virtual auto Search(uint32_t key) -> bool = 0;
    virtual auto Insert(uint32_t key) -> bool = 0;
    virtual auto Delete(uint32_t key) -> bool = 0;
};

#endif