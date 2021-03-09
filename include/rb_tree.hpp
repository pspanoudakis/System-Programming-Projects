#ifndef RB_TREE_HPP
#define RB_TREE_HPP

#include "utils.hpp"

enum Color {RED, BLACK};

class RBTreeNode
{
    public:
        Color color;
        void *data;
        RBTreeNode *left;
        RBTreeNode *right;
        RBTreeNode *parent;

        RBTreeNode(Color c);
        void destroyDescendants();
        RBTreeNode *uncle();
        RBTreeNode *sibling();
        bool isLeftChild();
        void replaceWithNewParent(RBTreeNode *new_parent);
        void swapColor(RBTreeNode *other);
        
};

class RedBlackTree
{
    private:
        CompareFunc compare;
        void leftRotation(RBTreeNode *target);
        void rightRotation(RBTreeNode *target);
        void fixRedRedViolation(RBTreeNode *target);
        RBTreeNode *find(void *element);
    public:        
        RBTreeNode *root;        
        RedBlackTree(CompareFunc comp);
        ~RedBlackTree();        
        void *insert(void *element);
};

#endif
