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
        //bool hasRedChild();
        void replaceWithNewParent(RBTreeNode *new_parent);
        void swapColor(RBTreeNode *other);
        
};

class RedBlackTree
{
    private:
        void leftRotation(RBTreeNode *target);
        void rightRotation(RBTreeNode *target);
        RBTreeNode *find(void *element);
    public:
        
        RBTreeNode *root;
        CompareFunc compare;
        RedBlackTree(CompareFunc comp);
        ~RedBlackTree();        
        void *insert(void *element);
};

#endif
