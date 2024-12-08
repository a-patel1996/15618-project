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
    op_count++;
    std::scoped_lock lock(op_lock);

    if (root == nullptr)
        return false;

    Node *node = root;
    while ((node->left != nullptr) && (node->right != nullptr))
    {
        if (key < node->key)
            node = node->left;
        else
            node = node->right;
    }

    if (key == node->key)
        return true;

    return false;
}

auto CoarseGrainedBST::Insert(uint32_t key) -> bool
{
    op_count++;
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

            if (key < node->key)
                newInternal = new Node(node->key, newLeaf, node);
            else
                newInternal = new Node(key, node, newLeaf);

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

            // delete node;
            return true;
        }

        // traverse to next node
        parent = node;
        if (key < node->key)
            node = node->left;
        else
            node = node->right;
    }

    // should never reach here
    return false;
}

auto CoarseGrainedBST::Delete(uint32_t key) -> bool
{
    op_count++;
    std::scoped_lock lock(op_lock);

    if (root == nullptr)
        return false;

    Node *grandparent = nullptr;
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
                    if (key < parent->key)
                    {
                        if (grandparent != nullptr)
                        {
                            if (grandparent->left == parent)
                                grandparent->left = parent->right;
                            else
                                grandparent->right = parent->right;
                        }
                        else
                        {
                            root = parent->right;
                        }
                    }
                    else
                    {
                        if (grandparent != nullptr)
                        {
                            if (grandparent->left == parent)
                                grandparent->left = parent->left;
                            else
                                grandparent->right = parent->left;
                        }
                        else
                        {
                            root = parent->left;
                        }
                    }

                    delete parent;
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

        if (parent != nullptr)
            grandparent = parent;
        parent = node;
        if (key < node->key)
            node = node->left;
        else
            node = node->right;
    }

    return false;
}

void CoarseGrainedBST::PrintTree()
{
    BinarySearchTree::PrintTree(root);
}
