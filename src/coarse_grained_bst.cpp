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
}

auto CoarseGrainedBST::Insert(uint32_t key) -> bool
{
    std::scoped_lock lock(op_lock);

    if (root == nullptr)
    {
        root = new Node(key);
        return true;
    }

    Node* node = root;
    Node* parent = nullptr;
    while(node != nullptr) {
        if ((node->left == nullptr) && (node->right == nullptr)) {
            // node is leaf
            if (node->key == key)
                // key exists
                return false;
            
            // c
            Node* newInternal;
            auto newLeaf = new Node(key);
            auto dupNode = new Node(node->key);

            if (key < node->key)
                newInternal = new Node(node->key, newLeaf, dupNode);
            else
                newInternal = new Node(key, dupNode, newLeaf);

            // replace current leaf with the new internal node
            if (parent != nullptr) {
                if (parent->left == node)
                    parent->left = newInternal;
                else
                    parent->right = newInternal;
            } else {
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
}