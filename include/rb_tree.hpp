/**
 * File: rb_tree.hpp
 * Red-Black Tree definition.
 * Pavlos Spanoudakis (sdi1800184)
 */

#ifndef RB_TREE_HPP
#define RB_TREE_HPP

#include "utils.hpp"

// Red-Black Tree Node color.
enum Color {RED, BLACK};

class RBTreeNode
{
    public:
        Color color;
        void *data;
        RBTreeNode *left;
        RBTreeNode *right;
        RBTreeNode *parent;

        RBTreeNode(Color c, void *element, RBTreeNode *parent_node);
        void destroyDescendants();
        RBTreeNode *sibling();
        bool isLeftChild();
        void replaceWithNewParent(RBTreeNode *new_parent);
        void swapColor(RBTreeNode *other);
        
};

class RedBlackTree
{
    private:
        unsigned int num_elements;
        CompareFunc compare;
        void leftRotation(RBTreeNode *target);
        void rightRotation(RBTreeNode *target);
        void fixRedRedViolation(RBTreeNode *target);
        RBTreeNode* find(void *element);
        void recursiveInsert(RBTreeNode *start, void *element);
    public:        
        RBTreeNode *root;        
        RedBlackTree(CompareFunc comp);
        ~RedBlackTree();        
        void insert(void *element);
        void *search(void *element);
        unsigned int getNumElements();
};

#endif
