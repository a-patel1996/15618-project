#include "fine_grained_bst.h"

FineGrainedBST::FineGrainedBST()
{
    root = nullptr;
}

FineGrainedBST::~FineGrainedBST()
{
    FreeNode(root);
}

void FineGrainedBST::FreeNode(NodeFG *node)
{
    if (node != nullptr)
    {
        FreeNode(node->left);
        FreeNode(node->right);
        delete node;
    }
}

auto FineGrainedBST::Search(uint32_t key) -> bool
{
    // op_count++;
    op_lock.lock();
    if (root == nullptr) {
        op_lock.unlock();
        return false;
    }
    
    NodeFG* curr = root;
    curr->perNodeLock.lock();
    NodeFG* next = nullptr;
    op_lock.unlock();

    while (curr) {
        if (key < curr->key) {
            next = curr->left;
            if (next != nullptr)
                next->perNodeLock.lock();
        } else if (key > curr->key) {
            next = curr->right;
            if (next != nullptr)
                next->perNodeLock.lock();
        } else { // key == curr->key
            curr->perNodeLock.unlock();
            return true;
        }
        curr->perNodeLock.unlock();
        curr = next;
    }
    
    return false;
}

auto FineGrainedBST::Insert(uint32_t key) -> bool
{
    // op_count++;
    op_lock.lock();
    if (root == nullptr) {
        root = new NodeFG(key);
        op_lock.unlock();
        return true;
    }
    
    NodeFG* currNode = root;
    currNode->perNodeLock.lock();
    NodeFG* nextNode = nullptr;
    op_lock.unlock();
    
    while (true) {
        if (key < currNode->key) {
            if (currNode->left == nullptr) {
                currNode->left = new NodeFG(key);
                currNode->perNodeLock.unlock();
                return true;
            } else {
                nextNode = currNode->left;
                nextNode->perNodeLock.lock();
                currNode->perNodeLock.unlock();
                currNode = nextNode;
            }
        } else if (key > currNode->key) {
            if (currNode->right == nullptr) {
                currNode->right = new NodeFG(key);
                currNode->perNodeLock.unlock();
                return true;
            } else {
                nextNode = currNode->right;
                nextNode->perNodeLock.lock();
                currNode->perNodeLock.unlock();
                currNode = nextNode;
            }
        } else { // key == curr->key
            currNode->perNodeLock.unlock();
            return false;
        }
    }
}

auto FineGrainedBST::Delete(uint32_t key) -> bool
{
    // op_count++;
    op_lock.lock();
    if (root == nullptr) {
        op_lock.unlock();
        return false;
    }

    NodeFG* parent = nullptr;
    NodeFG* curr = root;
    curr->perNodeLock.lock();

    while (curr) {
        if (key < curr->key) {
            if (parent != nullptr)
                parent->perNodeLock.unlock();
            else
                op_lock.unlock();
            parent = curr;
            curr = curr->left;
        } else if (key > curr->key) {
            if (parent != nullptr) 
                parent->perNodeLock.unlock();
            else 
                op_lock.unlock();
            parent = curr;
            curr = curr->right;
        } else { // key == curr->key
            break;
        }

        if (curr != nullptr) {
            curr->perNodeLock.lock();
        } else { // key not found
            parent->perNodeLock.unlock();
            return false;   
        }  
    }

    if (curr->left == nullptr || curr->right == nullptr) {
        NodeFG *tempNode;
        if (curr->left == nullptr)
            tempNode = curr->right;
        else
            tempNode = curr->left;

        // if current is root
        if (parent == nullptr) {
            root = tempNode;
        } else if (parent->left == curr) {
            parent->left = tempNode;
        } else {
            parent->right = tempNode;
        }
        delete curr;
    } else { // both child nodes exist
        NodeFG* successor_parent = curr;
        NodeFG* successor = curr->right;
        successor->perNodeLock.lock();
        while (successor->left != nullptr) {
            if (successor_parent != curr)
                successor_parent->perNodeLock.unlock();
            successor_parent = successor;
            successor = successor->left;
            successor->perNodeLock.lock();
        }

        // both successor and successor_parent are locked
        if (successor_parent != curr)
            successor_parent->left = successor->right;
        else
            successor_parent->right = successor->right;

        curr->key = successor->key;
        delete successor;

        if (successor_parent != curr)
            successor_parent->perNodeLock.unlock();
        curr->perNodeLock.unlock();
    }

    if (parent == nullptr) {
        op_lock.unlock();
    } else {
        parent->perNodeLock.unlock();
    }

    return true;
}

void FineGrainedBST::PrintTree()
{
}




// #include "fine_grained_bst.h"
// #include <iostream> // Make sure this is included
// #include <thread>

// FineGrainedBST::FineGrainedBST()
// {
//     root = nullptr;
//     std::cout << "[Constructor] BST initialized with null root\n";
// }

// FineGrainedBST::~FineGrainedBST()
// {
//     std::cout << "[Destructor] Starting tree cleanup\n";
//     FreeNode(root);
// }

// void FineGrainedBST::FreeNode(Node *node)
// {
//     if (node != nullptr)
//     {
//         std::cout << "[FreeNode] Freeing node with key: " << node->key << "\n";
//         FreeNode(node->left);
//         FreeNode(node->right);
//         delete node;
//     }
// }

// auto FineGrainedBST::Search(uint32_t key) -> bool
// {
//     std::cout << "\n[Search] Starting search for key: " << key << "\n";
//     search_start:
//         op_lock.lock();
//         if (root == nullptr) {
//             std::cout << "[Search] Tree is empty\n";
//             op_lock.unlock();
//             return false;
//         }

//         Node *node = root;
//         Node *nextNode = nullptr;
        
//         // std::unique_lock<std::mutex> rootLock(node->perNodeLock);
//         if (node->perNodeLock.try_lock()) {
//             op_lock.unlock();
//         } else {
//             op_lock.unlock();
//             goto search_start;
//         }

//     std::cout << "[Search] Acquiring root lock, root key: " << node->key << "\n";
//     // node->perNodeLock.lock();
//     // op_lock.unlock();
    
//     while ((node->left != nullptr) && (node->right != nullptr))
//     {
//         std::cout << "[Search] At internal node with key: " << node->key << "\n";
//         if (key < node->key) {
//             nextNode = node->left;
//             std::cout << "[Search] Going left to key: " << nextNode->key << "\n";
//         } else {
//             nextNode = node->right;
//             std::cout << "[Search] Going right to key: " << nextNode->key << "\n";
//         }
//         std::cout << "[Search] Acquiring lock for next node\n";
//         // std::unique_lock<std::mutex> nextNodeLock(nextNode->perNodeLock);
//         nextNode->perNodeLock.lock();
//         std::cout << "[Search] Releasing current node lock\n";
//         // rootLock.unlock();
//         node->perNodeLock.unlock();
//         node = nextNode;
//         // rootLock = std::move(nextNodeLock);
//     }
//     bool found = (key == node->key);
//     std::cout << "[Search] Reached leaf node with key: " << node->key 
//               << " Search result: " << (found ? "Found" : "Not found") << "\n";
//     node->perNodeLock.unlock();
//     return found;
// }

// auto FineGrainedBST::Insert(uint32_t key) -> bool
// {
//     std::cout << "TID: " << std::this_thread::get_id() << " [Insert] Starting insertion for key: " << key << "\n";
    
//     insert_start:
//         op_lock.lock();
//         if (root == nullptr)
//         {
//             std::cout << "TID: " << std::this_thread::get_id() << " [Insert] Tree is empty, acquiring op_lock\n";
//             // std::scoped_lock lock(op_lock);
//             root = new Node(key);
//             std::cout << "TID: " << std::this_thread::get_id() << " [Insert] Created new root with key: " << key << "\n";
//             op_lock.unlock();
//             return true;
//         }
        
//         Node *node = root;
//         Node *parent = nullptr;
//         Node *nextNode = nullptr;
//         // std::unique_lock<std::mutex> nodeLock(node->perNodeLock);
//         if (node->perNodeLock.try_lock()) {
//             op_lock.unlock();
//         } else {
//             op_lock.unlock();
//             goto insert_start;
//         }
//         // node->perNodeLock.lock();
//         // op_lock.unlock();
//     std::cout << "[Insert] Acquiring root lock, root key: " << node->key << "\n";

// //     0

// //      2
// //     0 2


// //      2
// //     0 3
// //      2 3

// //      2
// //     1 2
// //    0 1

//     while (node != nullptr)
//     {
//         std::cout << "TID: " << std::this_thread::get_id() << " [Insert] At node with key: " << node->key << "\n";
        
//         if ((node->left == nullptr) && (node->right == nullptr))
//         {
//             std::cout << "TID: " << std::this_thread::get_id() << " [Insert] Reached leaf node\n";
//             if (key == node->key) {
//                 std::cout << "TID: " << std::this_thread::get_id() << " [Insert] Key already exists\n";
//                 node->perNodeLock.unlock();
//                 return false;
//             }

//             Node *newInternal;
//             auto newLeaf = new Node(key);
//             std::cout << "TID: " << std::this_thread::get_id() << " [Insert] Created new leaf node with key: " << key << "\n";

//             if (key < node->key) {
//                 newInternal = new Node(node->key, newLeaf, node);
//                 std::cout << "TID: " << std::this_thread::get_id() << " [Insert] Created new internal node, new leaf on left\n";
//             } else {
//                 newInternal = new Node(key, node, newLeaf);
//                 std::cout << "TID: " << std::this_thread::get_id() << " [Insert] Created new internal node, new leaf on right\n";
//             }

//             if (parent != nullptr)
//             {
//                 std::cout << "TID: " << std::this_thread::get_id() << " [Insert] Acquiring parent lock for restructuring\n";
//                 // std::unique_lock<std::mutex> parentLock(parent->perNodeLock);
//                 parent->perNodeLock.lock();
//                 if (parent->left == node) {
//                     parent->left = newInternal;
//                     std::cout << "TID: " << std::this_thread::get_id() << " [Insert] Updated parent's left child\n";
//                 } else {
//                     parent->right = newInternal;
//                     std::cout << "TID: " << std::this_thread::get_id() << " [Insert] Updated parent's right child\n";
//                 }
//                 parent->perNodeLock.unlock();
//             }
//             else
//             {
//                 op_lock.lock();
//                 if (node == root) {
//                     root = newInternal;
//                 } else {
//                     if (newInternal->key < root->key)
//                         root->left = newInternal;
//                     else
//                         root->right = newInternal;
//                 }
//                 op_lock.unlock();
//                 std::cout << "TID: " << std::this_thread::get_id() << " [Insert] Updated root node\n";
//             }
//             // delete node;
//             std::cout << "TID: " << std::this_thread::get_id() << " [Insert] Insertion complete\n";
//             node->perNodeLock.unlock();
//             return true;
//         }

//         parent = node;
//         if (key < node->key) {
//             nextNode = node->left;
//             std::cout << "TID: " << std::this_thread::get_id() << "[Insert] Going left to key: " << nextNode->key << "\n";
//         } else {
//             nextNode = node->right;
//             std::cout << "TID: " << std::this_thread::get_id() << "[Insert] Going right to key: " << nextNode->key << "\n";
//         }
        
//         std::cout << "TID: " << std::this_thread::get_id() << "[Insert] Acquiring next node lock\n";
//         nextNode->perNodeLock.lock();
//         // std::unique_lock<std::mutex> nextNodeLock(nextNode->perNodeLock);
//         std::cout << "TID: " << std::this_thread::get_id() << "[Insert] Releasing current node lock\n";
//         // nodeLock.unlock();
//         node->perNodeLock.unlock();
//         node = nextNode;
//         // nodeLock = std::move(nextNodeLock);
//         // nodeLock = std::unique_lock<std::mutex> (nextNode->perNodeLock);
//     }

//     node->perNodeLock.unlock();
//     std::cout << "[Insert] ERROR: Reached unexpected null node\n";
//     return false;
// }

// auto FineGrainedBST::Delete(uint32_t key) -> bool
// {
//     std::cout << "\n[Delete] Starting deletion for key: " << key << "\n";
    
//     delete_start:
//         op_lock.lock();
//         if (root == nullptr) {
//             std::cout << "[Delete] Tree is empty\n";
//             op_lock.unlock();
//             return false;
//         }

//         Node *grandparent = nullptr;
//         Node *parent = nullptr;
//         Node *node = root;
//         Node *nextNode = nullptr;
//         Node *replacementChild = nullptr;
//         // std::unique_lock<std::mutex> nodeLock(node->perNodeLock);
//         if (node->perNodeLock.try_lock()) {
//             op_lock.unlock();
//         }
//         else {
//             op_lock.unlock();
//             goto delete_start;
//         }
//     std::cout << "[Delete] Acquiring root lock, root key: " << node->key << "\n";

//     while (node != nullptr)
//     {
//         std::cout << "[Delete] At node with key: " << node->key << "\n";
        
//         if ((node->left == nullptr) && (node->right == nullptr))
//         {
//             std::cout << "[Delete] Reached leaf node\n";
//             if (node->key == key)
//             {
//                 if (parent != nullptr)
//                 {
//                     std::cout << "[Delete] Acquiring parent lock\n";
//                     // std::unique_lock<std::mutex> parentLock(parent->perNodeLock);
//                     parent->perNodeLock.lock();
//                     if (key < parent->key) {
//                         replacementChild = parent->right;
//                         std::cout << "[Delete] Selected right sibling as replacement\n";
//                     } else {
//                         replacementChild = parent->left;
//                         std::cout << "[Delete] Selected left sibling as replacement\n";
//                     }
//                     parent->perNodeLock.unlock();
                    
//                     if (grandparent != nullptr) {
//                         std::cout << "[Delete] Acquiring grandparent lock\n";
//                         // std::unique_lock<std::mutex> grandparentLock(grandparent->perNodeLock);
//                         grandparent->perNodeLock.lock();
//                         if (grandparent->left == parent) {
//                             grandparent->left = replacementChild;
//                             std::cout << "[Delete] Updated grandparent's left child\n";
//                         } else {
//                             grandparent->right = replacementChild;
//                             std::cout << "[Delete] Updated grandparent's right child\n";
//                         }
//                         grandparent->perNodeLock.unlock();
//                         std::cout << "[Delete] Deleting parent node\n";
//                         delete parent;
//                     } else {
//                         std::cout << "[Delete] Deleting parent node\n";
//                         op_lock.lock();
//                         delete parent;
//                         root = replacementChild;
//                         op_lock.unlock();
//                         std::cout << "[Delete] Updated root node\n";
//                     }
                    
//                     // std::cout << "[Delete] Deleting parent node\n";
//                     // delete parent;
//                 }
//                 else
//                 {
//                     op_lock.lock();
//                     if (node == root) {
//                         root = nullptr;
//                         op_lock.unlock();
//                     } else {
//                         op_lock.unlock();
//                         node->perNodeLock.unlock();
//                         goto delete_start;
//                     }
                        
//                     // op_lock.unlock();
//                     std::cout << "[Delete] Deleted last node, tree is now empty\n";
//                 }

//                 std::cout << "[Delete] Deleting target node\n";
//                 node->perNodeLock.unlock();
//                 delete node;
//                 return true;
//             }
//             else
//             {
//                 std::cout << "[Delete] Key not found in leaf node\n";
//                 node->perNodeLock.unlock();
//                 return false;
//             }
//         }

//         if (parent != nullptr)
//             grandparent = parent;
//         parent = node;
//         if (key < node->key) {
//             nextNode = node->left;
//             std::cout << "[Delete] Going left to key: " << nextNode->key << "\n";
//         } else {
//             nextNode = node->right;
//             std::cout << "[Delete] Going right to key: " << nextNode->key << "\n";
//         }

//         std::cout << "[Delete] Acquiring next node lock\n";
//         // std::unique_lock<std::mutex> nextNodeLock(nextNode->perNodeLock);
//         nextNode->perNodeLock.lock();
//         std::cout << "[Delete] Releasing current node lock\n";
//         node->perNodeLock.unlock();
//         // nodeLock.unlock();
//         node = nextNode;
//         // nodeLock = std::move(nextNodeLock);
//     }

//     node->perNodeLock.unlock();
//     std::cout << "[Delete] ERROR: Reached unexpected null node\n";
//     return false;
// }

// void FineGrainedBST::PrintTree()
// {
//     std::cout << "\n[PrintTree] Current tree structure:\n";
//     BinarySearchTree::PrintTree(root);
// }