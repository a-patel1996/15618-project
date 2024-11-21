#ifndef __FEAST_H__
#define __FEAST_H__

#include <stdint.h>

class LockFreeBST
{
public:
    LockFreeBST();
    ~LockFreeBST();

    auto Search(uint32_t key) -> bool;
    auto Insert(uint32_t key) -> bool;
    auto Delete(uint32_t key) -> bool;

private:
    const uint64_t FLAG_BIT = (1 << 63);
    const uint64_t TAG_BIT = (1 << 62);

    typedef struct Node
    {
        uint32_t key;
        Node *left;
        Node *right;

        Node(uint32_t key) : key(key), left(nullptr), right(nullptr) {};
        Node(uint32_t key, Node *left, Node *right) : key(key), left(left), right(right) {};
    } Node;

    Node *root;

    typedef struct
    {
        Node *ancestor;
        Node *successor;
        Node *parent;
        Node *terminal;
    } SeekRecord;

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
    auto Read(Node **nodeptr_addr) -> std::tuple<bool, bool, Node *>;
};
#endif