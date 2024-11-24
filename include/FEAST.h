#ifndef __FEAST_H__
#define __FEAST_H__

#include <stdint.h>
#include "bst.h"

class LockFreeBST : public BinarySearchTree
{
public:
    LockFreeBST();
    ~LockFreeBST();

    auto Search(uint32_t key) -> bool;
    auto Insert(uint32_t key) -> bool;
    auto Delete(uint32_t key) -> bool;

private:
    const uint64_t FLAG_BIT = (1ULL << 63);
    const uint64_t TAG_BIT = (1ULL << 62);
    Node *root;

    typedef struct
    {
        Node *ancestor;
        Node *successor;
        Node *parent;
        Node *terminal;
    } SeekRecord;

    typedef enum
    {
        INJECTION = 0,
        CLEANUP = 1
    } DeleteMode;

    void Seek(uint32_t key, SeekRecord &seekRecord);
    auto Cleanup(SeekRecord &seekRecord) -> bool;

    auto GetNextChildField(uint32_t key, Node *node) -> Node *;
    auto GetSiblingChildField(uint32_t key, Node *node) -> Node *;
    auto GetAddressOfNextChildField(uint32_t key, Node *node) -> Node **;
    auto GetAddressOfNextChildField(Node *node, Node *child) -> Node **;
    auto GetAddressOfSiblingChildField(uint32_t key, Node *node) -> Node **;
    auto GetAddressOfSiblingChildField(Node *node, Node *child) -> Node **;

    auto GetAddress(Node *node) -> Node *;
    auto GetUntagged(Node *node) -> Node *;
    auto IsTagged(Node *node) -> bool;
    auto IsFlagged(Node *node) -> bool;
    auto setFlagged(Node *node) -> Node *;
    auto Read(Node **nodeptr_addr) -> std::tuple<bool, bool, Node *>;
};
#endif