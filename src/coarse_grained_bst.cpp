#include "coarse_grained_bst.h"

CoarseGrainedBST::CoarseGrainedBST()
{
    root = nullptr;
}

CoarseGrainedBST::~CoarseGrainedBST()
{
    FreeNode(root);
}

void CoarseGrainedBST::FreeNode(Node *node)
{
    if (node != nullptr)
    {
        FreeNode(node->left);
        FreeNode(node->right);
        delete node;
    }
}

auto CoarseGrainedBST::Search(uint32_t key) -> bool
{
    std::scoped_lock lock(op_lock);

    if (root == nullptr)
        return false;

    Node *node = root;
    while (node != nullptr)
    {
        if (key == node->key)
            return true;
        else if (key < node->key)
            node = node->left;
        else if (key > node->key)
            node = node->right;
    }

    return false;
}

auto CoarseGrainedBST::Insert(uint32_t key) -> bool
{
    std::scoped_lock lock(op_lock);

    if (root == nullptr)
    {
        root = new Node(key);
        return true;
    }

    Node *node = root;
    Node *parent = nullptr;
    while (node != nullptr)
    {
        if ((node->left == nullptr) && (node->right == nullptr))
        {
            // node is leaf
            if (key == node->key)
                // key exists
                return false;

            Node *newInternal;
            auto newLeaf = new Node(key);
            auto dupNode = new Node(node->key);

            if (key < node->key)
                newInternal = new Node(node->key, newLeaf, dupNode);
            else
                newInternal = new Node(key, dupNode, newLeaf);

            // replace current leaf with the new internal node
            if (parent != nullptr)
            {
                if (parent->left == node)
                    parent->left = newInternal;
                else
                    parent->right = newInternal;
            }
            else
            {
                root = newInternal;
            }

            delete node;
            return true;
        }

        // traverse to next node
        parent = node;
        if (key < node->key)
            node = node->left;
        else
            node = node->right;
    }
}

auto CoarseGrainedBST::Delete(uint32_t key) -> bool
{
    std::scoped_lock lock(op_lock);

    if (root = nullptr)
        return false;

    Node *parent = nullptr;
    Node *node = root;
    while (node != nullptr)
    {
        if ((node->left == nullptr) && (node->right == nullptr))
        {
            // reached leaf node
            if (node->key == key)
            {
                if (parent != nullptr)
                {
                    if (parent->left == node)
                        parent->left = nullptr;
                    else
                        parent->right = nullptr;

                    // delete parent if it becomes a leaf node
                    if ((parent->left == nullptr) && (parent->right == nullptr))
                    {
                        delete parent;
                        parent = nullptr;
                    }
                }
                else
                {
                    root = nullptr;
                }

                delete node;
                return true;
            }
            else
            {
                // key not found
                return false;
            }
        }

        parent = node;
        if (key < node->key)
            node = node->left;
        else
            node = node->right;
    }

    return false;
}
