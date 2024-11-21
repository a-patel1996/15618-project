#include <iomanip>
#include "bst.h"

void BinarySearchTree::PrintTree(Node* root, int space, int indent) {
    if (!root)
        return;

    // Increase the space between levels
    space += indent;

    // Process the right child first
    PrintTree(root->right, space);

    // Print the current node after the space
    std::cout << std::endl;
    std::cout << std::setw(space) << root->key << std::endl;

    // Process the left child
    PrintTree(root->left, space);
}
