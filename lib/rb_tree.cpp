#include <cstddef>
#include "../include/rb_tree.hpp"

void RBTreeNode::destroyDescendants()
{
    if (this->left != NULL)
    {
        this->left->destroyDescendants();
        delete left;
    }
    if (this->right != NULL)
    {
        this->right->destroyDescendants();
        delete right;
    }
}

bool RBTreeNode::isLeftChild()
{
    if (this->parent == NULL) { return false; }

    return (this->parent->left == this);
}

void RBTreeNode::swapColor(RBTreeNode *other)
{
    Color temp = this->color;
    this->color = other->color;
    other->color = temp;
}

void RBTreeNode::replaceWithNewParent(RBTreeNode *new_parent)
{
    if (this->parent == NULL) { return; }

    if ( this->isLeftChild() )
    {
        this->parent->left = new_parent;
    }
    else
    {
        this->parent->right = new_parent;
    }
    new_parent->parent = this->parent;
    this->parent = new_parent;
    
}

RedBlackTree::RedBlackTree(CompareFunc comp):
compare(comp), root(NULL) { }

RedBlackTree::~RedBlackTree()
{
    if (root != NULL)
    {
        root->destroyDescendants();
        delete root;
    }
}

void RedBlackTree::leftRotation(RBTreeNode *target)
{
    if (target == NULL) { return; }
    RBTreeNode *new_parent = target->right;

    if (this->root == target)
    {
        this->root = new_parent;
    }
    target->replaceWithNewParent(new_parent);
    target->right = new_parent->left;
    if (target->right != NULL)
    {
        target->right->parent = target;
    }
    new_parent->left = target;
}

void RedBlackTree::rightRotation(RBTreeNode *target)
{
    if (target == NULL) { return; }
    RBTreeNode *new_parent = target->left;

    if (this->root == target)
    {
        this->root = new_parent;
    }
    target->replaceWithNewParent(new_parent);
    target->left = new_parent->right;
    if (target->left != NULL)
    {
        target->left->parent = target;
    }
    new_parent->right = target;
}

void RedBlackTree::fixRedRedViolation(RBTreeNode *target)
{
    if ( this->root == target )
    {
        target->color = BLACK;
        return;
    }
    
}

void* RedBlackTree::insert(void *element)
{

}