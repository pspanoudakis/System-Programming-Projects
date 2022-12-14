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

/**
 * A Node of the Red-Black Tree.
 */
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
        void destroyDescendants(DestroyFunc destroy);
        RBTreeNode *sibling() const;
        bool isLeftChild() const;
        void replaceWithNewParent(RBTreeNode *new_parent);
        void swapColor(RBTreeNode *other);        
};

/**
 * The Red-Black Tree structure.
 */
class RedBlackTree
{
    private:
        unsigned int num_elements;
        CompareFunc compare;                    // Used for element comparison
        void leftRotation(RBTreeNode *target);
        void rightRotation(RBTreeNode *target);
        void fixRedRedViolation(RBTreeNode *target);
        void recursiveInsert(RBTreeNode *start, void *element);
    public:        
        RBTreeNode *root;        
        RedBlackTree(CompareFunc comp);
        ~RedBlackTree();        
        void insert(void *element);
        void *search(void *element) const;
        unsigned int getNumElements() const;
        void clear(DestroyFunc destroy);
};

#endif
