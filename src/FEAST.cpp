#include <algorithm>
#include <tuple>
#include "FEAST.h"

LockFreeBST::LockFreeBST()
{
    root = new Node(UINT32_MAX);
    root->right = new Node(UINT32_MAX);
    root->left = new Node(UINT32_MAX - 1);
    root->left->right = new Node(UINT32_MAX - 1);
    root->left->left = new Node(UINT32_MAX - 2);
}

LockFreeBST::~LockFreeBST()
{
    delete root->left->left;
    delete root->left->right;
    delete root->left;
    delete root->right;
    delete root;
}

// check whether or not the tree contains a given key
auto LockFreeBST::Search(uint32_t key) -> bool
{
    SeekRecord seekRecord;
    Seek(key, seekRecord);
    if (seekRecord.terminal->key == key)
        return true;
    else
        return false;
}

auto LockFreeBST::Insert(uint32_t key) -> bool
{
    SeekRecord seekRecord;
    while (true)
    {
        Seek(key, seekRecord);
        auto terminal_key = seekRecord.terminal->key;
        if (terminal_key != key)
        {
            // key not present in the tree
            auto parent = seekRecord.parent;
            auto terminal = seekRecord.terminal;

            // obtain the address of the child field that needs to be modified
            auto addressOfChildField = GetAddressOfNextChildField(key, parent);

            Node *newInternal;
            auto newLeaf = new Node(key);

            if (key < terminal_key)
                newInternal = new Node(terminal_key, newLeaf, terminal);
            else
                newInternal = new Node(key, terminal, newLeaf);

            // injection step: try to add the new nodes to the tree
            auto result = __sync_bool_compare_and_swap(addressOfChildField, GetAddress(terminal), newInternal);
            if (result)
            {
                // insertion successful
                return true;
            }
            else
            {
                // insertion failed; help the conflicting delete operation
                delete newInternal;
                delete newLeaf;

                bool flagged, tagged;
                Node *address;
                std::tie(flagged, tagged, address) = Read(addressOfChildField);
                if ((address == terminal) && (flagged || tagged))
                    /*
                        Address of the child has not changed and either the
                        leaf node or its sibling has been flagged for deletion.
                    */
                    Cleanup(seekRecord);
            }
        }
        else
        {
            // key already present in the tree
            return false;
        }
    }
}

auto LockFreeBST::Delete(uint32_t key) -> bool
{
    SeekRecord seekRecord;
    DeleteMode mode = INJECTION; // start with INJECTION mode
    while (true)
    {
        Seek(key, seekRecord);
        auto parent = seekRecord.parent;
        auto addressOfChildField = GetAddressOfNextChildField(key, parent);

        if (mode == INJECTION)
        {
            auto terminal = seekRecord.terminal;
            if (terminal->key != key)
            {
                return false;
            }

            // injection step: try to flag the edge to the leaf node
            auto result = __sync_bool_compare_and_swap(addressOfChildField, GetAddress(terminal), setFlagged(terminal));
            if (result)
            {
                mode = CLEANUP;
                auto done = Cleanup(seekRecord);
                if (done)
                {
                    return true;
                }
                else
                {
                    bool flagged, tagged;
                    Node *address;
                    std::tie(flagged, tagged, address) = Read(addressOfChildField);
                    if ((address == terminal) && (flagged || tagged))
                    {
                        Cleanup(seekRecord);
                    }
                }
            }
        }
        else
        {
            auto terminal = seekRecord.terminal;
            // Cleanup mode: Check if the leaf node that was flagged in the INJECTION mode is still present in tree
            if (seekRecord.terminal != terminal)
            {
                return true;
            }
            else
            {
                auto done = Cleanup(seekRecord);
                if (done)
                {
                    return true;
                }
            }
        }
    }
}

/*
    A helper function used by all operations. It traverses
    the tree from the root node to a leaf node and returns
    the addresses of four nodes in the access path.
*/
void LockFreeBST::Seek(uint32_t key, SeekRecord &seekRecord)
{
    auto ancestor = root;
    auto successor = ancestor->left;
    auto parent = ancestor->left;
    auto current = GetAddress(parent->left);
    auto childFieldAtParent = parent->left;
    auto childFieldAtCurrent = current->left;

    // get the address of the next node on the access path
    auto next = GetAddress(childFieldAtCurrent);

    while (next != NULL)
    {
        // move down the tree
        // check if the edge from the (current) parent node in the access path is tagged
        if (!IsTagged(childFieldAtParent))
        {
            // found an untagged edge in access path; advance ancestor & successor pointers
            ancestor = parent;
            successor = current;
        }

        // advance parent & current pointers
        parent = current;
        current = next;

        // update other variables used in traversal
        childFieldAtParent = childFieldAtCurrent;
        childFieldAtCurrent = GetNextChildField(key, current);

        // get address of the next node on the access-path
        next = GetAddress(childFieldAtCurrent);
    }
    // traversal complete
    // populate seek record & return
    seekRecord.ancestor = ancestor;
    seekRecord.successor = successor;
    seekRecord.parent = parent;
    seekRecord.terminal = current;
}

/*
    Removes a leaf node, which is currently under
    deletion, and its parent from the tree.
*/
auto LockFreeBST::Cleanup(SeekRecord &seekRecord) -> bool
{
    // retrieve all addresses stored in the seek record for easy access
    auto ancestor = seekRecord.ancestor;
    auto successor = seekRecord.successor;
    auto parent = seekRecord.parent;
    auto terminal = seekRecord.terminal;

    // obtain the addresses on which atomic instructions will be executed
    // first obtain the address of the field of the ancestor node that will be modified
    auto addressOfSuccessorField = GetAddressOfNextChildField(ancestor, successor);

    // now obtain the addresses of the children fields of the parent node
    auto addressOfChildField = GetAddressOfNextChildField(parent, terminal);
    auto addressOfSiblingField = GetAddressOfSiblingChildField(parent, terminal);

    bool flagged, tagged;
    Node *address;
    std::tie(flagged, tagged, address) = Read(addressOfChildField);

    if (!flagged)
        /*
            The leaf node is not flagged for deletion.
            Thus the sibling node must be flagged for
            deletion. Switch the sibling address.
        */
        addressOfSiblingField = addressOfChildField;

    /*
        Freeze step: tag the sibling edge if not already tagged.
        No modify operation can occur at this edge now.
    */
    if (!tagged)
        __sync_fetch_and_or(addressOfSiblingField, TAG_BIT);

    /*
        The flag field will be copied to the new edge that will be created.
        Prune step: make the sibling node a direct child of the ancestor node.
    */
    return __sync_bool_compare_and_swap(addressOfSuccessorField, GetAddress(successor), GetUntagged(*addressOfSiblingField));
}

/*
    Retrieve contents of the child field that contains
    address of the next node on the access-path.
*/
auto LockFreeBST::GetNextChildField(uint32_t key, Node *node) -> Node *
{
    if (key < node->key)
        return node->left;
    else
        return node->right;
}

/*
    Retrieve contents of the child field that contains
    address of the sibling of the next node on the access-path.
*/
auto LockFreeBST::GetSiblingChildField(uint32_t key, Node *node) -> Node *
{
    if (key < node->key)
        return node->right;
    else
        return node->left;
}

/*
    Returns address of the child field that contains
    address of the next node on the access path.
*/
auto LockFreeBST::GetAddressOfNextChildField(uint32_t key, Node *node) -> Node **
{
    if (key < node->key)
        return &(node->left);
    else
        return &(node->right);
}

auto LockFreeBST::GetAddressOfNextChildField(Node *node, Node *child) -> Node **
{
    if (child->key < node->key)
        return &(node->left);
    else
        return &(node->right);
}

/*
    Returns the address of the child field that contains address
    of the sibling of the next node on the access path.
*/
auto LockFreeBST::GetAddressOfSiblingChildField(uint32_t key, Node *node) -> Node **
{
    if (key < node->key)
        return &(node->right);
    else
        return &(node->left);
}

auto LockFreeBST::GetAddressOfSiblingChildField(Node *node, Node *child) -> Node **
{
    if (child->key < node->key)
        return &(node->right);
    else
        return &(node->left);
}

inline auto LockFreeBST::GetAddress(Node *node) -> Node *
{
    return reinterpret_cast<Node *>(reinterpret_cast<uint64_t>(node) & (~(FLAG_BIT | TAG_BIT)));
}

inline auto LockFreeBST::GetUntagged(Node *node) -> Node *
{
    return reinterpret_cast<Node *>(reinterpret_cast<uint64_t>(node) & ~TAG_BIT);
}

inline auto LockFreeBST::IsTagged(Node *node) -> bool
{
    return (reinterpret_cast<uint64_t>(node) & TAG_BIT);
}

inline auto LockFreeBST::IsFlagged(Node *node) -> bool
{
    return (reinterpret_cast<uint64_t>(node) & FLAG_BIT);
}

inline auto LockFreeBST::setFlagged(Node *node) -> Node *
{
    return reinterpret_cast<Node *>(reinterpret_cast<uint64_t>(node) | FLAG_BIT);
}

auto LockFreeBST::Read(Node **nodeptr_addr) -> std::tuple<bool, bool, Node *>
{
    auto nodeptr = *nodeptr_addr;
    bool flagged = reinterpret_cast<uint64_t>(nodeptr) & FLAG_BIT;
    bool tagged = reinterpret_cast<uint64_t>(nodeptr) & TAG_BIT;
    auto address = reinterpret_cast<Node *>(reinterpret_cast<uint64_t>(nodeptr) & (~(FLAG_BIT | TAG_BIT)));
    return std::make_tuple(flagged, tagged, address);
}
