#include <iostream>
#include "bst.h"

void BinarySearchTree::PrintTree(Node* root, std::string prefix = "", bool isLeft = true) {
    if (!root)
        return;

    // Print the current node with a line connecting it to its parent
    std::cout << prefix;

    if (!prefix.empty()) {
        std::cout << (isLeft ? "├── " : "└── ");
    }

    std::cout << root->key << std::endl;

    // Recur to the left and right children
    std::string childPrefix = prefix + (isLeft ? "│   " : "    ");
    if (root->left || root->right) {
        PrintTree(root->right, childPrefix, false);
        PrintTree(root->left, childPrefix, true);
    }
}